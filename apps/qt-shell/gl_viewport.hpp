#pragma once
#include <QImage>
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
    bool ready() const{return ready_;}
    QString error() const{return error_;}
protected:
    void initializeGL() override;
    void paintGL() override;
private:
    void release();
    QImage frame_;bool dirty_=false,ready_=false;QString error_;
    QOpenGLShaderProgram shader_;QOpenGLBuffer vertices_{QOpenGLBuffer::VertexBuffer};
    QOpenGLVertexArrayObject vao_;GLuint texture_=0;
};
