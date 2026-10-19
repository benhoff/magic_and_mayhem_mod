#pragma once
#include <QImage>
#include <QPointer>
#include <functional>
#include "blit.hpp"
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
class GlViewportCanvas;
class GlViewport final:public QWidget,protected QOpenGLFunctions {
    Q_OBJECT
public:
    enum class Scaling { Sharp, Smooth, Integer };
    explicit GlViewport(QWidget* parent=nullptr);
    ~GlViewport() override;
    QOpenGLContext* context() const;
    void makeCurrent();
    void doneCurrent();
    QImage grabFramebuffer();
    void update();
    void repaint();
    bool rasterPresentation() const{return raster_;}
signals:
    void frameSwapped();
    void aboutToCompose();
public:
    void setFrame(QImage image);
    void setGpuFrame(mnm::render::GpuFrame frame);
    std::uint64_t imageUploads() const{return imageUploads_;}
    QSize frameSize() const{return gpuFrame_.valid()?gpuFrame_.size():frame_.size();}
    QRectF imageRect() const;
    void setScaling(Scaling scaling){scaling_=scaling;update();}
    Scaling scaling() const{return scaling_;}
    bool imagePoint(QPointF position,QPoint& point,bool clamp=false) const;
    bool ready() const{return ready_;}
    QString error() const{return error_;}
    // Optional native diagnostic observer: false begins painting, true completes.
    std::function<void(bool)> framePaintTiming;
    std::function<bool()> recoverGraphics;
    // The application owns desired visibility; Qt hides widgets on surface loss.
    std::function<bool()> shouldRestorePlatformSurface;
    enum class Activity { PresentationPoll, RouteCapture, ReferenceRead, ScreenCapture, NativeReadback, ImageEncoding, DiagnosticCapture, CampaignPoll, ProgressPublication, CaptureRequest, WindowDiscovery, InputHeartbeat, RenderStatusPickup };
    std::function<void(Activity,bool)> activityTiming;
    class ActivityScope {
    public:
        ActivityScope(GlViewport* viewport,Activity activity):viewport_(viewport),activity_(activity){if(viewport_&&viewport_->activityTiming)viewport_->activityTiming(activity_,false);}
        ~ActivityScope(){if(viewport_&&viewport_->activityTiming)viewport_->activityTiming(activity_,true);}
        ActivityScope(const ActivityScope&)=delete;
        ActivityScope& operator=(const ActivityScope&)=delete;
    private:
        GlViewport* viewport_;Activity activity_;
    };
protected:
    void initializeGL();
    void paintGL();
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    bool event(QEvent*) override;
    bool eventFilter(QObject*,QEvent*) override;
    void showEvent(QShowEvent*) override;
private:
    friend class GlViewportCanvas;
    bool raster_=false,platformRecoveryQueued_=false,platformRecoveryCancelled_=false;
    QPointer<QWidget> platformHost_;
    QPointer<QWindow> platformWindow_;
    GlViewportCanvas* canvas_=nullptr;
    void paintRaster(QPainter&);
    QRect physicalImageRect() const;
    Scaling scaling_=Scaling::Sharp;
    void release();
    mnm::render::GpuFrame gpuFrame_;std::uint64_t imageUploads_=0;
    QImage frame_;QSize textureSize_;bool dirty_=false,ready_=false;QString error_;
    QOpenGLShaderProgram shader_,gpuShader_;QOpenGLBuffer vertices_{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject vao_;GLuint texture_=0;
};
