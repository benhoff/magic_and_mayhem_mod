#include "gl_viewport.hpp"
#include <QOpenGLContext>
#include <cmath>
GlViewport::GlViewport(QWidget* parent):QOpenGLWidget(parent){setMinimumSize(320,240);setFocusPolicy(Qt::StrongFocus);setMouseTracking(true);}
GlViewport::~GlViewport(){if(context()){disconnect(context(),nullptr,this,nullptr);makeCurrent();release();doneCurrent();}}
void GlViewport::release(){if(texture_)glDeleteTextures(1,&texture_);texture_=0;textureSize_={};vertices_.destroy();vao_.destroy();shader_.removeAllShaders();gpuShader_.removeAllShaders();ready_=false;}
void GlViewport::setFrame(QImage image){if(ready_)error_.clear();gpuFrame_={};frame_=image.convertToFormat(QImage::Format_RGBA8888);dirty_=true;update();}
void GlViewport::setGpuFrame(mnm::render::GpuFrame frame){if(ready_)error_.clear();gpuFrame_=std::move(frame);frame_={};dirty_=false;update();}
QRect GlViewport::physicalImageRect() const{
    if(frameSize().isEmpty())return {};
    const double ratio=devicePixelRatioF();
    const int width=qRound(this->width()*ratio),height=qRound(this->height()*ratio);
    double scale=qMin(double(width)/frameSize().width(),double(height)/frameSize().height());
    // Integer enlargement uses physical pixels. Small windows still fit the whole frame.
    if(scaling_==Scaling::Integer && scale>=1)scale=std::floor(scale);
    const int w=qRound(frameSize().width()*scale),h=qRound(frameSize().height()*scale);
    return QRect((width-w)/2,height-h-(height-h)/2,w,h);
}
QRectF GlViewport::imageRect() const{
    const auto rect=physicalImageRect();const auto ratio=devicePixelRatioF();
    return QRectF(rect.x()/ratio,rect.y()/ratio,rect.width()/ratio,rect.height()/ratio);
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
        "#version 330 core\nin vec2 texcoord;out vec4 color;uniform sampler2D frame;uniform bool smoothScaling;"
        "vec4 pixel(ivec2 p){return texelFetch(frame,clamp(p,ivec2(0),textureSize(frame,0)-1),0);}"
        "void main(){if(!smoothScaling){color=texture(frame,texcoord);return;}"
        "vec2 p=texcoord*vec2(textureSize(frame,0))-0.5;ivec2 lo=ivec2(floor(p));vec2 f=fract(p);"
        "color=mix(mix(pixel(lo),pixel(lo+ivec2(1,0)),f.x),mix(pixel(lo+ivec2(0,1)),pixel(lo+ivec2(1,1)),f.x),f.y);}") || !shader_.link()){
        error_=shader_.log();return;
    }
    if(!gpuShader_.addShaderFromSourceCode(QOpenGLShader::Vertex,
        "#version 330 core\nlayout(location=0) in vec2 position;layout(location=1) in vec2 uv;out vec2 texcoord;void main(){gl_Position=vec4(position,0,1);texcoord=uv;}") ||
       !gpuShader_.addShaderFromSourceCode(QOpenGLShader::Fragment,
        // Integer textures cannot use GL_LINEAR; filter in the presentation shader only.
        "#version 330 core\nin vec2 texcoord;out vec4 color;uniform usampler2D frame;uniform bool smoothScaling;"
        "vec4 pixel(ivec2 p){return vec4(texelFetch(frame,clamp(p,ivec2(0),textureSize(frame,0)-1),0))/255.0;}"
        "void main(){if(!smoothScaling){color=vec4(texture(frame,texcoord))/255.0;return;}"
        "vec2 p=texcoord*vec2(textureSize(frame,0))-0.5;ivec2 lo=ivec2(floor(p));vec2 f=fract(p);"
        "color=mix(mix(pixel(lo),pixel(lo+ivec2(1,0)),f.x),mix(pixel(lo+ivec2(0,1)),pixel(lo+ivec2(1,1)),f.x),f.y);}") || !gpuShader_.link()){
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
    const auto rect=physicalImageRect();
    glViewport(rect.x(),height-rect.y()-rect.height(),rect.width(),rect.height());
    auto& shader=gpuFrame_.valid()?gpuShader_:shader_;
    shader.bind();shader.setUniformValue("frame",0);shader.setUniformValue("smoothScaling",scaling_==Scaling::Smooth);
    vao_.bind();glDrawArrays(GL_TRIANGLE_STRIP,0,4);vao_.release();shader.release();
    if(gpuFrame_.valid())try {gpuFrame_.samplingComplete();}
    catch(const std::exception& e){error_=QString::fromUtf8(e.what());}
}
