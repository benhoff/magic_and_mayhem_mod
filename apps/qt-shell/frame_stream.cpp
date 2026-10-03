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
QString FrameStream::diagnostic() const {
    switch(status()){
    case 0:return "The frame bridge has not loaded. Check Wine startup and the launch log.";
    case 1:return "OpenGL presentation active.";
    case 2:return "DirectDraw frame readback failed to lock the primary surface.";
    case 3:return "DirectDraw frame readback rejected the surface layout or palette.";
    case 4:return "Frame bridge loaded; no startup diagnostics available from this older bridge.";
    case 5:return "DirectDraw hook armed; waiting for the game to call DirectDrawCreate.";
    case 6:return "Wine is initializing DirectDraw. If this persists, retry with --software-rendering.";
    case 7:return "DirectDraw initialized; waiting for the first captured primary-surface frame.";
    case 8:{const auto result=qFromLittleEndian(__atomic_load_n(reinterpret_cast<const quint32*>(mapping_+44),__ATOMIC_ACQUIRE));
        return QString("DirectDrawCreate failed (HRESULT 0x%1). Check the Wine graphics log.").arg(result,8,16,QLatin1Char('0'));}
    case 11:return "Wine is enumerating display adapters before DirectDrawCreate. Check the Wine graphics log.";
    case 12:return "Display adapter enumeration completed; waiting for the game to create DirectDraw.";
    case 13:{const auto result=qFromLittleEndian(__atomic_load_n(reinterpret_cast<const quint32*>(mapping_+52),__ATOMIC_ACQUIRE));
        return QString("Display adapter enumeration failed (HRESULT 0x%1).").arg(result,8,16,QLatin1Char('0'));}
    case 10:return "DirectDrawCreate returned without an intercepted drawing interface. Check bridge compatibility.";
    case 9:return "Frame bridge could not install the guarded DirectDraw hook. No frames can be captured.";
    default:return "Unknown frame bridge status.";
    }
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
