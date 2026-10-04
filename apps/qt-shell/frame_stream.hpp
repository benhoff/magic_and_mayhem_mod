#pragma once
#include "../../protocols/include/mnm/frame_v1.h"
#include <QFile>
#include <QImage>
class FrameStream {
public:
    static constexpr qint64 Size=MNM_FRAME_V1_SIZE;
    ~FrameStream();
    bool create(const QString& path);
    bool open(const QString& path);
    QImage nextFrame();
    quint32 status() const;
    QString diagnostic() const;
    QString error() const{return error_;}
private:
    bool map();QFile file_;uchar* mapping_=nullptr;quint32 lastFrame_=0;QString error_;
};
