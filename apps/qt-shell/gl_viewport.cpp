#include "gl_viewport.hpp"
#include <QOpenGLContext>
GlViewport::GlViewport(QWidget* parent):QOpenGLWidget(parent){setMinimumSize(320,240);}
GlViewport::~GlViewport(){if(context()){disconnect(context(),nullptr,this,nullptr);makeCurrent();release();doneCurrent();}}
void GlViewport::release(){if(texture_)glDeleteTextures(1,&texture_);texture_=0;vertices_.destroy();vao_.destroy();shader_.removeAllShaders();ready_=false;}
void GlViewport::setFrame(QImage image){frame_=image.convertToFormat(QImage::Format_RGBA8888);dirty_=true;update();}
void GlViewport::initializeGL(){
    initializeOpenGLFunctions();
    connect(context(),&QOpenGLContext::aboutToBeDestroyed,this,[this]{makeCurrent();release();doneCurrent();},Qt::DirectConnection);
    if(!shader_.addShaderFromSourceCode(QOpenGLShader::Vertex,
        "#version 330 core\nlayout(location=0) in vec2 position;layout(location=1) in vec2 uv;out vec2 texcoord;void main(){gl_Position=vec4(position,0,1);texcoord=uv;}") ||
       !shader_.addShaderFromSourceCode(QOpenGLShader::Fragment,
        "#version 330 core\nin vec2 texcoord;out vec4 color;uniform sampler2D frame;void main(){color=texture(frame,texcoord);}") || !shader_.link()){
        error_=shader_.log();return;
    }
    const float data[]={-1,-1,0,1, 1,-1,1,1, -1,1,0,0, 1,1,1,0};
    vao_.create();vao_.bind();vertices_.create();vertices_.bind();vertices_.allocate(data,sizeof(data));
    shader_.bind();shader_.enableAttributeArray(0);shader_.enableAttributeArray(1);
    shader_.setAttributeBuffer(0,GL_FLOAT,0,2,4*sizeof(float));shader_.setAttributeBuffer(1,GL_FLOAT,2*sizeof(float),2,4*sizeof(float));
    vertices_.release();vao_.release();shader_.release();
    glGenTextures(1,&texture_);glBindTexture(GL_TEXTURE_2D,texture_);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    ready_=true;dirty_=true;
}
void GlViewport::paintGL(){
    const int width=qRound(this->width()*devicePixelRatioF()),height=qRound(this->height()*devicePixelRatioF());
    glViewport(0,0,width,height);glClearColor(0,0,0,1);glClear(GL_COLOR_BUFFER_BIT);
    if(!ready_ || frame_.isNull())return;
    glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,texture_);
    if(dirty_){glPixelStorei(GL_UNPACK_ALIGNMENT,1);glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,frame_.width(),frame_.height(),0,GL_RGBA,GL_UNSIGNED_BYTE,frame_.constBits());dirty_=false;}
    const auto scale=qMin(double(width)/frame_.width(),double(height)/frame_.height());
    const int w=qRound(frame_.width()*scale),h=qRound(frame_.height()*scale);
    glViewport((width-w)/2,(height-h)/2,w,h);
    shader_.bind();shader_.setUniformValue("frame",0);vao_.bind();glDrawArrays(GL_TRIANGLE_STRIP,0,4);vao_.release();shader_.release();
}
