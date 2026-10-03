#include "frame_stream.hpp"
#include <QtEndian>
#include <cstring>
FrameStream::~FrameStream(){if(mapping_)file_.unmap(mapping_);}
bool FrameStream::map(){
    if(file_.size()!=Size){error_="Invalid frame stream length.";return false;}
    mapping_=file_.map(0,Size);
    if(!mapping_){error_=file_.errorString();return false;}
    if(std::memcmp(mapping_,"MNMGL001",8) || qFromLittleEndian<quint32>(mapping_+8)!=1 || qFromLittleEndian<quint32>(mapping_+12)!=64){error_="Invalid frame stream header.";file_.unmap(mapping_);mapping_=nullptr;return false;}
    return true;
}
bool FrameStream::create(const QString& path){
    if(file_.isOpen())return false;
    file_.setFileName(path);
    if(!file_.open(QIODevice::ReadWrite|QIODevice::NewOnly)||!file_.resize(Size)){error_=file_.errorString();return false;}
    QByteArray header(64,0);std::memcpy(header.data(),"MNMGL001",8);
    qToLittleEndian<quint32>(1,header.data()+8);qToLittleEndian<quint32>(64,header.data()+12);
    if(file_.write(header)!=64 || !file_.flush()){error_=file_.errorString();return false;}return map();
}
bool FrameStream::open(const QString& path){
    if(file_.isOpen())return false;
    file_.setFileName(path);
    if(!file_.open(QIODevice::ReadOnly)){error_=file_.errorString();return false;}return map();
}
quint32 FrameStream::status() const {
    return mapping_?qFromLittleEndian(__atomic_load_n(reinterpret_cast<const quint32*>(mapping_+36),__ATOMIC_ACQUIRE)):0;
}
QImage FrameStream::nextFrame(){
    if(!mapping_)return {};
    const auto* words=reinterpret_cast<const quint32*>(mapping_);
    const auto sequence=qFromLittleEndian(__atomic_load_n(words+4,__ATOMIC_ACQUIRE));
    if(sequence&1)return {};
    quint32 header[7];std::memcpy(header,mapping_+20,sizeof(header));
    for(auto& value:header)value=qFromLittleEndian(value);
    const auto width=header[0],height=header[1],pitch=header[2],format=header[3],counter=header[5];
    if(!counter || counter==lastFrame_)return {};
    if(!width || !height || width>2048 || height>2048 || pitch!=width*4 || format!=1){error_="Invalid frame dimensions or format.";return {};}
    QImage frame(int(width),int(height),QImage::Format_RGBA8888);
    if(frame.isNull()){error_="Cannot allocate frame image.";return {};}
    std::memcpy(frame.bits(),mapping_+64,width*height*4);
    __atomic_thread_fence(__ATOMIC_ACQUIRE);
    if(qFromLittleEndian(__atomic_load_n(words+4,__ATOMIC_ACQUIRE))!=sequence)return {};
    lastFrame_=counter;return frame;
}
