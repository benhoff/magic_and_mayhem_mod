#pragma once
#include <QImage>
#include <functional>
#include "blit.hpp"
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
class GlViewport final:public QOpenGLWidget,protected QOpenGLFunctions {
public:
    enum class Scaling { Sharp, Smooth, Integer };
    explicit GlViewport(QWidget* parent=nullptr);
    ~GlViewport() override;
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
    void initializeGL() override;
    void paintGL() override;
private:
    QRect physicalImageRect() const;
    Scaling scaling_=Scaling::Sharp;
    void release();
    mnm::render::GpuFrame gpuFrame_;std::uint64_t imageUploads_=0;
    QImage frame_;QSize textureSize_;bool dirty_=false,ready_=false;QString error_;
    QOpenGLShaderProgram shader_,gpuShader_;QOpenGLBuffer vertices_{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject vao_;GLuint texture_=0;
};
