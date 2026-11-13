#include "commands.hpp"
#include <QGuiApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLVersionFunctionsFactory>
#include <QSurfaceFormat>
#include <cstdio>
#include <stdexcept>
int main(int argc,char** argv){
    QJsonObject report{{"original_game_runs",0},{"live_World_credit",false}};
    try{
        if(argc!=3)throw std::runtime_error("Expected bounded component command input and new RGBA output");
        qunsetenv("MNM_NATIVE_DEVICE_RECOVERY");
        QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);
        QGuiApplication app(argc,argv);QOpenGLContext consumer;consumer.setFormat(format);
        if(!consumer.create())throw std::runtime_error("Cannot create component consumer context");
        QOffscreenSurface surface;surface.setFormat(consumer.format());surface.create();
        if(!surface.isValid()||!consumer.makeCurrent(&surface))throw std::runtime_error("Cannot activate component consumer");
        auto* gl=QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_3_Core>(&consumer);
        if(!gl||!gl->initializeOpenGLFunctions())throw std::runtime_error("Missing GL3.3functions");
        QFile input(QString::fromLocal8Bit(argv[1]));if(!input.open(QIODevice::ReadOnly)||input.size()>mnm::render::maxCommandBytes)throw std::runtime_error("Invalid bounded input");
        const auto commands=mnm::render::decodeCommands(input.readAll());
        mnm::render::GlBlitter renderer(&consumer);mnm::render::GpuFrame frame;
        const auto result=mnm::render::replayCommandsGpu(commands,renderer,[&](auto next){frame=std::move(next);});
        if(!frame.valid()||!consumer.makeCurrent(&surface))throw std::runtime_error("Missing actual GPU component frame");
        GLuint framebuffer=0;gl->glGenFramebuffers(1,&framebuffer);gl->glBindFramebuffer(GL_FRAMEBUFFER,framebuffer);
        gl->glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,frame.textureForCurrentContext(),0);
        if(gl->glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)throw std::runtime_error("Incomplete component frame framebuffer");
        const auto size=frame.size();QByteArray pixels(size.width()*size.height()*4,Qt::Uninitialized);
        gl->glReadPixels(0,0,size.width(),size.height(),GL_RGBA_INTEGER,GL_UNSIGNED_BYTE,pixels.data());
        if(gl->glGetError()!=GL_NO_ERROR)throw std::runtime_error("Component frame read failed");
        frame.samplingComplete();gl->glBindFramebuffer(GL_FRAMEBUFFER,0);gl->glDeleteFramebuffers(1,&framebuffer);
        QFile output(QString::fromLocal8Bit(argv[2]));if(!output.open(QIODevice::WriteOnly|QIODevice::NewOnly)||output.write(pixels)!=pixels.size()||!output.flush())throw std::runtime_error("Cannot retain exact component frame");
        const auto driver=renderer.driver();
        report["success"]=true;report["native_word_checks"]=int(result.checks);report["presentations"]=int(result.presents);
        report["renderer"]=QString::fromStdString(driver.renderer);report["vendor"]=QString::fromStdString(driver.vendor);report["version"]=QString::fromStdString(driver.version);
        report["width"]=size.width();report["height"]=size.height();frame={};
        std::puts(QJsonDocument(report).toJson(QJsonDocument::Compact).constData());return 0;
    }catch(const std::exception& e){report["success"]=false;report["error"]=e.what();std::puts(QJsonDocument(report).toJson(QJsonDocument::Compact).constData());return 1;}
}
