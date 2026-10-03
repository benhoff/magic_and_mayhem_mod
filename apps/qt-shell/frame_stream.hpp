#pragma once
#include <QFile>
#include <QImage>
class FrameStream {
public:
    static constexpr qint64 Size=64+2048*2048*4;
    ~FrameStream();
    bool create(const QString& path);
    bool open(const QString& path);
    QImage nextFrame();
    quint32 status() const;
    QString error() const{return error_;}
private:
    bool map();QFile file_;uchar* mapping_=nullptr;quint32 lastFrame_=0;QString error_;
};
