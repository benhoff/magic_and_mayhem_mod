#include "voice_bridge.hpp"
#include <QMediaDevices>
#include <QtEndian>
#include <cstring>
#include <limits>
namespace mnm::audio {
namespace {
std::uint32_t status(Error error){
    switch(error){case Error::ok:return 0;case Error::unsupported:return 0x80004001;
    case Error::busy:return 0x887800aa;case Error::limit:return 0x8007000e;
    default:return 0x80070057;}
}
}
BridgeReply VoiceCommands::execute(const std::uint32_t* w,const QByteArray& payload){
    const BufferId id=w[6];Error error=Error::invalid;BufferId created=0;
    switch(w[5]){
    case MNM_AUDIO_PING:return {};
    case MNM_AUDIO_CREATE:{
        if(w[10]>2 || w[11]>16 || w[12]>4 || w[14]>65535)return {0x80070057,0};
        PcmFormat f{std::uint16_t(w[14]),std::uint16_t(w[10]),w[9],w[13],std::uint16_t(w[12]),std::uint16_t(w[11]),0};
        error=device_.createStatic(w[15],f,w[8],created);break;
    }
    case MNM_AUDIO_PRIMARY_GET_VOLUME:return {0,std::uint32_t(device_.primaryState().volume)};
    case MNM_AUDIO_PRIMARY_VOLUME:{
        const auto signedValue=std::int64_t(w[7])-(w[7]>0x7fffffff?4294967296ll:0);
        error=device_.setPrimaryVolume(std::int32_t(signedValue));break;
    }
    case MNM_AUDIO_PRIMARY_PLAY:error=device_.playPrimary(w[7]);break;
    case MNM_AUDIO_PRIMARY_STOP:device_.stopPrimary();return {};
    case MNM_AUDIO_PRIMARY_STATUS:return {0,device_.primaryState().status()};
    case MNM_AUDIO_PRIMARY_FORMAT:{
        if(w[10]>2 || w[11]>16 || w[12]>4 || w[14]>65535)return {0x80070057,0};
        error=device_.setPrimaryOutputFormat({std::uint16_t(w[14]),std::uint16_t(w[10]),w[9],w[13],std::uint16_t(w[12]),std::uint16_t(w[11]),0});break;
    }
    case MNM_AUDIO_DUPLICATE:error=device_.duplicate(id,created);break;
    case MNM_AUDIO_RELEASE:error=device_.release(id);break;
    case MNM_AUDIO_PLAY:error=device_.play(id,w[7]);break;
    case MNM_AUDIO_STOP:error=device_.stop(id);break;
    case MNM_AUDIO_RESET:if(w[7])return {0x80004001,0};error=device_.resetPosition(id);break;
    case MNM_AUDIO_VOLUME:case MNM_AUDIO_PAN:{
        const auto value=w[7]<=std::uint32_t(std::numeric_limits<std::int32_t>::max())?std::int32_t(w[7]):std::int32_t(std::int64_t(w[7])-4294967296ll);
        error=w[5]==MNM_AUDIO_VOLUME?device_.setVolume(id,value):device_.setPan(id,value);break;
    }
    case MNM_AUDIO_STATUS:{const auto voice=device_.voice(id);return voice?BridgeReply{0,voice->status()}:BridgeReply{0x80070057,0};}
    case MNM_AUDIO_UPLOAD:{
        if(w[8]!=std::uint32_t(payload.size()) || !w[8] || w[8]>MNM_AUDIO_PAYLOAD)return {0x80070057,0};
        WriteLock lock;error=device_.lock(id,w[7],w[8],0,lock);
        if(error!=Error::ok)break;
        std::memcpy(lock.first.data,payload.constData(),lock.first.size);
        if(lock.second.size)std::memcpy(lock.second.data,payload.constData()+lock.first.size,lock.second.size);
        error=device_.unlock(lock,lock.first.size,lock.second.size);break;
    }
    default:return {0x80004001,0};
    }
    return {status(error),error==Error::ok?std::uint32_t(created):0};
}
VoiceBroker::VoiceBroker(QObject* parent):QObject(parent){timer_.setInterval(2);connect(&timer_,&QTimer::timeout,this,[this]{poll();});}
VoiceBroker::~VoiceBroker(){stop();}
bool VoiceBroker::create(const QString& path,bool audible){
    stop();error_.clear();
    QAudioFormat format;
    if(audible)format=selectOutputFormat(QMediaDevices::defaultAudioOutput());
    if(audible && !format.isValid()){error_="No supported native voice output device";return false;}
    device_=std::make_unique<Device>(MNM_AUDIO_PAYLOAD,128,audible?std::uint32_t(format.sampleRate()):48000);
    commands_=std::make_unique<VoiceCommands>(*device_);
    if(audible){
        output_=std::make_unique<QtOutput>(*device_);
        if(!output_->start(QMediaDevices::defaultAudioOutput())){error_=output_->lastError();stop();return false;}
        output_->failed=[this](const QString& error){
            error_=error;if(map_)__atomic_store_n(reinterpret_cast<quint32*>(map_)+20,0u,__ATOMIC_RELEASE);
            timer_.stop();if(failed)failed(error);
        };
    }
    file_.setFileName(path);
    if(!file_.open(QIODevice::ReadWrite|QIODevice::NewOnly) || !file_.resize(MNM_AUDIO_SIZE)){
        error_="Cannot create native voice channel";stop();return false;
    }
    map_=file_.map(0,MNM_AUDIO_SIZE);
    if(!map_){error_="Cannot map native voice channel";stop();return false;}
    std::memset(map_,0,128);std::memcpy(map_,"MNMAUD01",8);
    auto* words=reinterpret_cast<quint32*>(map_);
    words[2]=qToLittleEndian(MNM_AUDIO_VERSION);words[3]=qToLittleEndian(MNM_AUDIO_SIZE);
    __atomic_store_n(words+20,qToLittleEndian(1u),__ATOMIC_RELEASE);
    last_=0;timer_.start();return true;
}
void VoiceBroker::stop(){
    timer_.stop();
    if(map_){__atomic_store_n(reinterpret_cast<quint32*>(map_)+20,0u,__ATOMIC_RELEASE);file_.unmap(map_);map_=nullptr;}
    file_.close();output_.reset();commands_.reset();device_.reset();last_=0;
}
void VoiceBroker::poll(){
    if(!map_)return;
    auto* words=reinterpret_cast<quint32*>(map_);
    __atomic_add_fetch(words+19,1u,__ATOMIC_RELEASE);
    const auto request=qFromLittleEndian(__atomic_load_n(words+4,__ATOMIC_ACQUIRE));
    if(!request || request==last_)return;
    std::uint32_t input[16];for(unsigned i=0;i<16;++i)input[i]=qFromLittleEndian(words[i]);
    BridgeReply reply{0x80070057,0};
    try{
        if(input[8]<=MNM_AUDIO_PAYLOAD){
            const auto payload=input[5]==MNM_AUDIO_UPLOAD?QByteArray(reinterpret_cast<const char*>(map_+128),int(input[8])):QByteArray{};
            reply=commands_->execute(input,payload);
        }
    }catch(const std::exception&){reply={0x8007000e,0};}
    last_=request;words[17]=qToLittleEndian(reply.status);words[18]=qToLittleEndian(reply.value);
    __atomic_store_n(words+16,qToLittleEndian(request),__ATOMIC_RELEASE);
}
}
