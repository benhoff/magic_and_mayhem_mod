#include "gl_viewport.hpp"
#include "native_render_target.hpp"
#include <QOpenGLContext>
#include <cmath>
#include <QPainter>
#include <QResizeEvent>
#include <QPlatformSurfaceEvent>
#include <QWindow>
#include <QTimer>
#include <QShowEvent>
class GlViewportCanvas final:public QOpenGLWidget {
public:
    explicit GlViewportCanvas(GlViewport& owner):QOpenGLWidget(&owner),owner_(owner){setAttribute(Qt::WA_TransparentForMouseEvents);setFocusPolicy(Qt::NoFocus);}
protected:
    void initializeGL() override{owner_.initializeGL();}
    void paintGL() override{owner_.paintGL();}
private:
    GlViewport& owner_;
};
GlViewport::GlViewport(QWidget* parent):QWidget(parent){
    setMinimumSize(320,240);setFocusPolicy(Qt::StrongFocus);setMouseTracking(true);
    raster_=mnm::render::GlBlitter::isolatedRecoveryEnabled();
    if(raster_){ready_=true;setAttribute(Qt::WA_OpaquePaintEvent);}
    else{canvas_=new GlViewportCanvas(*this);connect(canvas_,&QOpenGLWidget::frameSwapped,this,&GlViewport::frameSwapped);connect(canvas_,&QOpenGLWidget::aboutToCompose,this,&GlViewport::aboutToCompose);}
}
void GlViewport::update(){if(canvas_)canvas_->update();QWidget::update();}
void GlViewport::repaint(){if(canvas_)canvas_->repaint();else QWidget::repaint();}
QOpenGLContext* GlViewport::context() const{return canvas_?canvas_->context():nullptr;}
void GlViewport::makeCurrent(){if(canvas_)canvas_->makeCurrent();}
void GlViewport::doneCurrent(){if(canvas_)canvas_->doneCurrent();}
void GlViewport::resizeEvent(QResizeEvent* event){if(canvas_)canvas_->setGeometry(rect());QWidget::resizeEvent(event);}
bool GlViewport::event(QEvent* event){
    // Native window/platform recreation retains the CPU frame and render session.
    const bool result=QWidget::event(event);
    if(raster_ && event->type()==QEvent::PlatformSurface && static_cast<QPlatformSurfaceEvent*>(event)->surfaceEventType()==QPlatformSurfaceEvent::SurfaceCreated)update();
    return result;
}
void GlViewport::showEvent(QShowEvent* event){
    QWidget::showEvent(event);
    if(!raster_)return;
    platformRecoveryCancelled_=false;
    auto* host=this->window();
    if(platformHost_!=host){if(platformHost_)platformHost_->removeEventFilter(this);platformHost_=host;host->installEventFilter(this);}
    auto* window=host->windowHandle();
    if(platformWindow_!=window){if(platformWindow_)platformWindow_->removeEventFilter(this);platformWindow_=window;if(window)window->installEventFilter(this);}
}
bool GlViewport::eventFilter(QObject* object,QEvent* event){
    if((object==platformHost_ || object==platformWindow_) && event->type()==QEvent::Close)platformRecoveryCancelled_=true;
    if(raster_ && object==platformWindow_ && event->type()==QEvent::PlatformSurface &&
       static_cast<QPlatformSurfaceEvent*>(event)->surfaceEventType()==QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed &&
       shouldRestorePlatformSurface && shouldRestorePlatformSurface() && !platformRecoveryCancelled_ && !platformRecoveryQueued_){
        platformRecoveryQueued_=true;platformRecoveryCancelled_=false;const auto window=platformWindow_;const auto host=platformHost_;
        QTimer::singleShot(0,this,[this,window,host]{
            platformRecoveryQueued_=false;
            if(!window || window!=platformWindow_ || !host || host!=platformHost_ || platformRecoveryCancelled_ || !shouldRestorePlatformSurface || !shouldRestorePlatformSurface())return;
            // Qt also destroys surfaces during reparent/hide/teardown. Only a
            // window still requested by the application is restored next turn. Qt
            // may hide its QWidget while destroying the platform surface, so
            // use the captured visibility intent and resynchronize QWidget state.
            host->hide();host->show();host->update();update();
        });
    }
    return QWidget::eventFilter(object,event);
}
void GlViewport::paintRaster(QPainter& painter){
    painter.fillRect(rect(),Qt::black);
    if(frameSize().isEmpty())return;
    painter.setRenderHint(QPainter::SmoothPixmapTransform,scaling_==Scaling::Smooth);
    painter.drawImage(imageRect(),gpuFrame_.valid()?gpuFrame_.raster():frame_);
}
void GlViewport::paintEvent(QPaintEvent* event){
    if(!raster_){QWidget::paintEvent(event);return;}
    emit aboutToCompose();
    if(gpuFrame_.valid() && framePaintTiming)framePaintTiming(false);
    {QPainter painter(this);paintRaster(painter);}
    if(gpuFrame_.valid() && framePaintTiming)framePaintTiming(true);
    emit frameSwapped();
}
QImage GlViewport::grabFramebuffer(){
    if(canvas_)return canvas_->grabFramebuffer();
    const auto ratio=devicePixelRatioF();QImage image(qRound(width()*ratio),qRound(height()*ratio),QImage::Format_RGBA8888);image.setDevicePixelRatio(ratio);
    QPainter painter(&image);paintRaster(painter);return image;
}
GlViewport::~GlViewport(){if(context()){disconnect(context(),nullptr,this,nullptr);makeCurrent();release();doneCurrent();}}
void GlViewport::release(){if(texture_)glDeleteTextures(1,&texture_);texture_=0;textureSize_={};vertices_.destroy();vao_.destroy();shader_.removeAllShaders();gpuShader_.removeAllShaders();ready_=false;}
void GlViewport::setFrame(QImage image){if(ready_)error_.clear();gpuFrame_={};frame_=image.convertToFormat(QImage::Format_RGBA8888);dirty_=true;if(canvas_)canvas_->update();else update();}
void GlViewport::setGpuFrame(mnm::render::GpuFrame frame){if(frame.valid() && raster_!=frame.hasRaster())throw std::runtime_error("Frame transport does not match viewport");if(ready_)error_.clear();gpuFrame_=std::move(frame);frame_={};dirty_=false;if(canvas_)canvas_->update();else update();}
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
    if(gpuFrame_.valid() && framePaintTiming)framePaintTiming(false);
    glActiveTexture(GL_TEXTURE0);
    try {glBindTexture(GL_TEXTURE_2D,gpuFrame_.valid()?gpuFrame_.textureForCurrentContext():texture_);}
    catch(const std::exception& e){error_=QString::fromUtf8(e.what());if(recoverGraphics && recoverGraphics())error_.clear();return;}
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
    catch(const std::exception& e){error_=QString::fromUtf8(e.what());if(recoverGraphics && recoverGraphics())error_.clear();return;}
    if(gpuFrame_.valid() && error_.isEmpty() && framePaintTiming)framePaintTiming(true);
}

NativeRenderTarget viewportRenderTarget(GlViewport& v){
    return {[&v]{return v.ready();},[&v]{return v.context();},
        [&v](auto frame){v.setGpuFrame(std::move(frame));},[&v]{return v.error();},nullptr};
}
