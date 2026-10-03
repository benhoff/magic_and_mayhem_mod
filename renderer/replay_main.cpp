#include "blit.hpp"
#include "capture.hpp"
#include <QCryptographicHash>
#include <QFile>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <cstdio>
#include <stdexcept>

int main(int argc,char** argv){
    if(argc==2 && QByteArray(argv[1])=="--help"){
        std::puts("Usage: mnm-render-replay CAPTURE --output NATIVE_PIXELS\n"
                  "Runs the native integer OpenGL blitter; never launches the game.\n"
                  "Output must be a new file. JSON diagnostics are written to stdout.");return 0;
    }
    QJsonObject report;
    try {
        if(argc!=4 || QByteArray(argv[2])!="--output")throw std::runtime_error("Expected CAPTURE --output NATIVE_PIXELS");
        const QString output=QString::fromLocal8Bit(argv[3]);
        QFile file(QString::fromLocal8Bit(argv[1]));
        if(!file.open(QIODevice::ReadOnly))throw std::runtime_error(file.errorString().toStdString());
        if(file.size()>mnm::render::maxCaptureBytes)throw std::runtime_error("Oversized capture");
        const auto data=file.read(mnm::render::maxCaptureBytes+1);
        const auto command=mnm::render::decodeCapture(data);
        report["capture_sha256"]=QString::fromLatin1(QCryptographicHash::hash(data,QCryptographicHash::Sha256).toHex());
        // Input rejection does not require a display or OpenGL initialization.
        QGuiApplication app(argc,argv);
        mnm::render::GlBlitter blitter;
        const auto rendered=blitter.draw(command);
        const auto pixels=mnm::render::encodeNative(rendered,command.bits);
        const auto driver=blitter.driver();
        report["vendor"]=QString::fromStdString(driver.vendor);report["renderer"]=QString::fromStdString(driver.renderer);
        report["version"]=QString::fromStdString(driver.version);report["backend"]="opengl_native_integer";
        report["output_sha256"]=QString::fromLatin1(QCryptographicHash::hash(pixels,QCryptographicHash::Sha256).toHex());
        report["width"]=rendered.width;report["height"]=rendered.height;report["bits"]=int(command.bits);
        QFile destination(output);
        if(!destination.open(QIODevice::WriteOnly|QIODevice::NewOnly))throw std::runtime_error(destination.errorString().toStdString());
        if(destination.write(pixels)!=pixels.size() || !destination.flush()){
            destination.remove();throw std::runtime_error("Failed to write native output");
        }
        report["rendered"]=true;
        std::puts(QJsonDocument(report).toJson(QJsonDocument::Compact).constData());return 0;
    }catch(const std::exception& error){
        report["rendered"]=false;report["error"]=error.what();
        std::puts(QJsonDocument(report).toJson(QJsonDocument::Compact).constData());return 2;
    }
}
