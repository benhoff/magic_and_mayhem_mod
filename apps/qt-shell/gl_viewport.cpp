#include "gl_viewport.hpp"
#include <QOpenGLContext>
#include <cmath>
GlViewport::GlViewport(QWidget* parent):QOpenGLWidget(parent){setMinimumSize(320,240);setFocusPolicy(Qt::StrongFocus);setMouseTracking(true);}
GlViewport::~GlViewport(){if(context()){disconnect(context(),nullptr,this,nullptr);makeCurrent();release();doneCurrent();}}
void GlViewport::release(){if(texture_)glDeleteTextures(1,&texture_);texture_=0;textureSize_={};vertices_.destroy();vao_.destroy();shader_.removeAllShaders();gpuShader_.removeAllShaders();ready_=false;}
void GlViewport::setFrame(QImage image){if(ready_)error_.clear();gpuFrame_={};frame_=image.convertToFormat(QImage::Format_RGBA8888);dirty_=true;update();}
void GlViewport::setGpuFrame(mnm::render::GpuFrame frame){if(ready_)error_.clear();gpuFrame_=std::move(frame);frame_={};dirty_=false;update();}
QRectF GlViewport::imageRect() const{
    if(frameSize().isEmpty())return {};
    const double ratio=devicePixelRatioF();
    const int width=qRound(this->width()*ratio),height=qRound(this->height()*ratio);
    const double scale=qMin(double(width)/frameSize().width(),double(height)/frameSize().height());
    const int w=qRound(frameSize().width()*scale),h=qRound(frameSize().height()*scale);
    // OpenGL measures the viewport's vertical offset from the bottom.
    return QRectF((width-w)/2/ratio,(height-h-(height-h)/2)/ratio,w/ratio,h/ratio);
}
bool GlViewport::imagePoint(QPointF position,QPoint& point,bool clamp) const{
    const auto rect=imageRect();if(rect.isEmpty())return false;
    const double x=(position.x()-rect.x())*frameSize().width()/rect.width();
    const double y=(position.y()-rect.y())*frameSize().height()/rect.height();
    if(!clamp && (x<0 || y<0 || x>=frameSize().width() || y>=frameSize().height()))return false;
    point={qBound(0,int(std::floor(x)),frameSize().width()-1),qBound(0,int(std::floor(y)),frameSize().height()-1)};return true;
}
void GlViewport::initializeGL(){
    initializeOpenGLFunctions();
    connect(context(),&QOpenGLContext::aboutToBeDestroyed,this,[this]{makeCurrent();release();doneCurrent();},Qt::DirectConnection);
    if(!shader_.addShaderFromSourceCode(QOpenGLShader::Vertex,
        "#version 330 core\nlayout(location=0) in vec2 position;layout(location=1) in vec2 uv;out vec2 texcoord;void main(){gl_Position=vec4(position,0,1);texcoord=uv;}") ||
       !shader_.addShaderFromSourceCode(QOpenGLShader::Fragment,
        "#version 330 core\nin vec2 texcoord;out vec4 color;uniform sampler2D frame;void main(){color=texture(frame,texcoord);}") || !shader_.link()){
        error_=shader_.log();return;
    }
    if(!gpuShader_.addShaderFromSourceCode(QOpenGLShader::Vertex,
        "#version 330 core\nlayout(location=0) in vec2 position;layout(location=1) in vec2 uv;out vec2 texcoord;void main(){gl_Position=vec4(position,0,1);texcoord=uv;}") ||
       !gpuShader_.addShaderFromSourceCode(QOpenGLShader::Fragment,
        "#version 330 core\nin vec2 texcoord;out vec4 color;uniform usampler2D frame;void main(){color=vec4(texture(frame,texcoord))/255.0;}") || !gpuShader_.link()){
        error_=gpuShader_.log();return;
    }
    const float data[]={-1,-1,0,1, 1,-1,1,1, -1,1,0,0, 1,1,1,0};
    vao_.create();vao_.bind();vertices_.create();vertices_.bind();vertices_.allocate(data,sizeof(data));
    shader_.bind();shader_.enableAttributeArray(0);shader_.enableAttributeArray(1);
    shader_.setAttributeBuffer(0,GL_FLOAT,0,2,4*sizeof(float));shader_.setAttributeBuffer(1,GL_FLOAT,2*sizeof(float),2,4*sizeof(float));
    vertices_.release();vao_.release();shader_.release();
    glGenTextures(1,&texture_);glBindTexture(GL_TEXTURE_2D,texture_);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    ready_=true;dirty_=!gpuFrame_.valid();
}
void GlViewport::paintGL(){
    const int width=qRound(this->width()*devicePixelRatioF()),height=qRound(this->height()*devicePixelRatioF());
    glViewport(0,0,width,height);glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT);
    if(!ready_ || frameSize().isEmpty())return;
    glActiveTexture(GL_TEXTURE0);
    try {glBindTexture(GL_TEXTURE_2D,gpuFrame_.valid()?gpuFrame_.textureForCurrentContext():texture_);}
    catch(const std::exception& e){error_=QString::fromUtf8(e.what());return;}
    if(dirty_){glPixelStorei(GL_UNPACK_ALIGNMENT,1);
        if(textureSize_!=frame_.size()){
            glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,frameSize().width(),frameSize().height(),0,GL_RGBA,GL_UNSIGNED_BYTE,frame_.constBits());textureSize_=frame_.size();
        }else glTexSubImage2D(GL_TEXTURE_2D,0,0,0,frameSize().width(),frameSize().height(),GL_RGBA,GL_UNSIGNED_BYTE,frame_.constBits());
        ++imageUploads_;dirty_=false;}
    const auto scale=qMin(double(width)/frameSize().width(),double(height)/frameSize().height());
    const int w=qRound(frameSize().width()*scale),h=qRound(frameSize().height()*scale);
    glViewport((width-w)/2,(height-h)/2,w,h);
    auto& shader=gpuFrame_.valid()?gpuShader_:shader_;
    shader.bind();shader.setUniformValue("frame",0);vao_.bind();glDrawArrays(GL_TRIANGLE_STRIP,0,4);vao_.release();shader.release();
    if(gpuFrame_.valid())try {gpuFrame_.samplingComplete();}
    catch(const std::exception& e){error_=QString::fromUtf8(e.what());}
}
