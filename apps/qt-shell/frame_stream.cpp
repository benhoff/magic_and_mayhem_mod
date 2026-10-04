#include "frame_stream.hpp"
#include <QtEndian>
#include <cstring>
FrameStream::~FrameStream(){if(mapping_)file_.unmap(mapping_);}
bool FrameStream::map(){
    if(file_.size()!=Size){error_="Invalid frame stream length.";return false;}
    mapping_=file_.map(0,Size);
    if(!mapping_){error_=file_.errorString();return false;}
    if(std::memcmp(mapping_,MNM_FRAME_V1_MAGIC,MNM_FRAME_V1_MAGIC_SIZE) || qFromLittleEndian<quint32>(mapping_+MNM_FRAME_V1_VERSION_OFFSET)!=MNM_FRAME_V1_VERSION || qFromLittleEndian<quint32>(mapping_+MNM_FRAME_V1_DECLARED_SIZE_OFFSET)!=MNM_FRAME_V1_DECLARED_SIZE){error_="Invalid frame stream header.";file_.unmap(mapping_);mapping_=nullptr;return false;}
    return true;
}
bool FrameStream::create(const QString& path){
    if(file_.isOpen())return false;
    file_.setFileName(path);
    if(!file_.open(QIODevice::ReadWrite|QIODevice::NewOnly)||!file_.resize(Size)){error_=file_.errorString();return false;}
    QByteArray header(MNM_FRAME_V1_HEADER_SIZE,0);std::memcpy(header.data(),MNM_FRAME_V1_MAGIC,MNM_FRAME_V1_MAGIC_SIZE);
    qToLittleEndian<quint32>(MNM_FRAME_V1_VERSION,header.data()+MNM_FRAME_V1_VERSION_OFFSET);qToLittleEndian<quint32>(MNM_FRAME_V1_DECLARED_SIZE,header.data()+MNM_FRAME_V1_DECLARED_SIZE_OFFSET);
    if(file_.write(header)!=MNM_FRAME_V1_HEADER_SIZE || !file_.flush()){error_=file_.errorString();return false;}return map();
}
bool FrameStream::open(const QString& path){
    if(file_.isOpen())return false;
    file_.setFileName(path);
    if(!file_.open(QIODevice::ReadOnly)){error_=file_.errorString();return false;}return map();
}
quint32 FrameStream::status() const {
    return mapping_?qFromLittleEndian(__atomic_load_n(reinterpret_cast<const quint32*>(mapping_+MNM_FRAME_V1_STATUS_OFFSET),__ATOMIC_ACQUIRE)):0;
}
QString FrameStream::diagnostic() const {
    switch(status()){
    case MNM_FRAME_V1_STATUS_NOT_LOADED:return "The frame bridge has not loaded. Check Wine startup and the launch log.";
    case MNM_FRAME_V1_STATUS_FRAME_PUBLISHED:return "OpenGL presentation active.";
    case MNM_FRAME_V1_STATUS_LOCK_FAILED:return "DirectDraw frame readback failed to lock the primary surface.";
    case MNM_FRAME_V1_STATUS_SURFACE_REJECTED:return "DirectDraw frame readback rejected the surface layout or palette.";
    case MNM_FRAME_V1_STATUS_DLL_LOADED:return "Frame bridge loaded; no startup diagnostics available from this older bridge.";
    case MNM_FRAME_V1_STATUS_HOOK_ARMED:return "DirectDraw hook armed; waiting for the game to call DirectDrawCreate.";
    case MNM_FRAME_V1_STATUS_INSIDE_CREATE:return "Wine is initializing DirectDraw. If this persists, retry with --software-rendering.";
    case MNM_FRAME_V1_STATUS_INTERFACE_INTERCEPTED:return "DirectDraw initialized; waiting for the first captured primary-surface frame.";
    case MNM_FRAME_V1_STATUS_CREATE_FAILED:{const auto result=qFromLittleEndian(__atomic_load_n(reinterpret_cast<const quint32*>(mapping_+MNM_FRAME_V1_CREATE_HRESULT_OFFSET),__ATOMIC_ACQUIRE));
        return QString("DirectDrawCreate failed (HRESULT 0x%1). Check the Wine graphics log.").arg(result,8,16,QLatin1Char('0'));}
    case MNM_FRAME_V1_STATUS_ENUMERATING:return "Wine is enumerating display adapters before DirectDrawCreate. Check the Wine graphics log.";
    case MNM_FRAME_V1_STATUS_ENUMERATION_COMPLETE:return "Display adapter enumeration completed; waiting for the game to create DirectDraw.";
    case MNM_FRAME_V1_STATUS_ENUMERATION_FAILED:{const auto result=qFromLittleEndian(__atomic_load_n(reinterpret_cast<const quint32*>(mapping_+MNM_FRAME_V1_ENUMERATION_HRESULT_OFFSET),__ATOMIC_ACQUIRE));
        return QString("Display adapter enumeration failed (HRESULT 0x%1).").arg(result,8,16,QLatin1Char('0'));}
    case MNM_FRAME_V1_STATUS_INTERCEPTION_FAILED:return "DirectDrawCreate returned without an intercepted drawing interface. Check bridge compatibility.";
    case MNM_FRAME_V1_STATUS_HOOK_FAILED:return "Frame bridge could not install the guarded DirectDraw hook. No frames can be captured.";
    default:return "Unknown frame bridge status.";
    }
}
QImage FrameStream::nextFrame(){
    if(!mapping_)return {};
    const auto* words=reinterpret_cast<const quint32*>(mapping_);
    const auto sequence=qFromLittleEndian(__atomic_load_n(words+MNM_FRAME_V1_SEQUENCE_OFFSET/4,__ATOMIC_ACQUIRE));
    if(sequence&1)return {};
    quint32 header[(MNM_FRAME_V1_CREATE_HRESULT_OFFSET+MNM_FRAME_V1_CREATE_HRESULT_SIZE-MNM_FRAME_V1_WIDTH_OFFSET)/4];std::memcpy(header,mapping_+MNM_FRAME_V1_WIDTH_OFFSET,sizeof(header));
    for(auto& value:header)value=qFromLittleEndian(value);
    const auto field=[&](unsigned offset){return header[(offset-MNM_FRAME_V1_WIDTH_OFFSET)/4];};
    const auto width=field(MNM_FRAME_V1_WIDTH_OFFSET),height=field(MNM_FRAME_V1_HEIGHT_OFFSET);
    const auto pitch=field(MNM_FRAME_V1_STRIDE_OFFSET),format=field(MNM_FRAME_V1_PIXEL_FORMAT_OFFSET);
    const auto counter=field(MNM_FRAME_V1_FRAME_COUNT_OFFSET);
    if(!counter || counter==lastFrame_)return {};
    if(!width || !height || width>MNM_FRAME_V1_MAX_WIDTH || height>MNM_FRAME_V1_MAX_HEIGHT || pitch!=width*MNM_FRAME_V1_BYTES_PER_PIXEL || format!=MNM_FRAME_V1_PIXEL_FORMAT_RGBA8888){error_="Invalid frame dimensions or format.";return {};}
    QImage frame(int(width),int(height),QImage::Format_RGBA8888);
    if(frame.isNull()){error_="Cannot allocate frame image.";return {};}
    std::memcpy(frame.bits(),mapping_+MNM_FRAME_V1_PIXELS_OFFSET,width*height*MNM_FRAME_V1_BYTES_PER_PIXEL);
    __atomic_thread_fence(__ATOMIC_ACQUIRE);
    if(qFromLittleEndian(__atomic_load_n(words+MNM_FRAME_V1_SEQUENCE_OFFSET/4,__ATOMIC_ACQUIRE))!=sequence)return {};
    lastFrame_=counter;return frame;
}
