#pragma once
#include <QImage>
#include "blit.hpp"
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
class GlViewport final:public QOpenGLWidget,protected QOpenGLFunctions {
public:
    explicit GlViewport(QWidget* parent=nullptr);
    ~GlViewport() override;
    void setFrame(QImage image);
    void setGpuFrame(mnm::render::GpuFrame frame);
    std::uint64_t imageUploads() const{return imageUploads_;}
    QSize frameSize() const{return gpuFrame_.valid()?gpuFrame_.size():frame_.size();}
    QRectF imageRect() const;
    bool imagePoint(QPointF position,QPoint& point,bool clamp=false) const;
    bool ready() const{return ready_;}
    QString error() const{return error_;}
protected:
    void initializeGL() override;
    void paintGL() override;
private:
    void release();
    mnm::render::GpuFrame gpuFrame_;std::uint64_t imageUploads_=0;
    QImage frame_;QSize textureSize_;bool dirty_=false,ready_=false;QString error_;
    QOpenGLShaderProgram shader_,gpuShader_;QOpenGLBuffer vertices_{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject vao_;GLuint texture_=0;
};
