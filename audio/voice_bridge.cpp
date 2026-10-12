#include "voice_bridge.hpp"
#include "wave_source.hpp"
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
// Device, dispatch, PCM pumping and channel ownership share this worker thread.
// No Device operation runs concurrently with mixing or depends on GUI progress.
struct VoiceBroker::State:QObject {
    explicit State(QObject* parent):QObject(parent),timer_(this){
        timer_.setInterval(2);timer_.setTimerType(Qt::PreciseTimer);
        connect(&timer_,&QTimer::timeout,this,[this]{poll();});
    }
    ~State() override{stop();}
    bool create(const QString& path,bool audible,const QAudioDevice& outputDevice,const QString& soundsRoot){
        if(!soundsRoot.isEmpty()){
            auto assets=mnm::assets::AssetStore::create(soundsRoot.toStdString());
            if(const auto* error=std::get_if<mnm::assets::Error>(&assets)){error_=QString::fromStdString(error->detail);return false;}
            waveSource_=std::make_unique<WaveSource>(std::get<mnm::assets::AssetStore>(std::move(assets)),true);
        }
        QAudioFormat format;
        if(audible)format=selectOutputFormat(outputDevice);
        if(audible&&!format.isValid()){error_="No supported native voice output device";return false;}
        device_=std::make_unique<Device>(MNM_AUDIO_PAYLOAD,128,audible?std::uint32_t(format.sampleRate()):48000);
        commands_=std::make_unique<VoiceCommands>(*device_);
        if(audible){
            output_=std::make_unique<QtOutput>(*device_);
            if(!output_->start(outputDevice)){error_=output_->lastError();return false;}
            output_->failed=[this](const QString& error){
                error_=error;
                if(map_)__atomic_store_n(reinterpret_cast<quint32*>(map_)+20,0u,__ATOMIC_RELEASE);
                timer_.stop();if(failed)failed(error);
            };
        }
        file_.setFileName(path);
        if(!file_.open(QIODevice::ReadWrite|QIODevice::NewOnly)||!file_.resize(MNM_AUDIO_SIZE)){
            error_="Cannot create native voice channel";return false;
        }
        map_=file_.map(0,MNM_AUDIO_SIZE);
        if(!map_){error_="Cannot map native voice channel";return false;}
        std::memset(map_,0,128);std::memcpy(map_,"MNMAUD01",8);
        auto* words=reinterpret_cast<quint32*>(map_);
        words[2]=qToLittleEndian(MNM_AUDIO_VERSION);words[3]=qToLittleEndian(MNM_AUDIO_SIZE);
        words[31]=qToLittleEndian(waveSource_?MNM_AUDIO_CAP_NATIVE_WAVE:0u);
        __atomic_store_n(words+20,qToLittleEndian(1u),__ATOMIC_RELEASE);
        activity_.audible=audible;timer_.start();return true;
    }
    VoiceActivity activity() const{
        auto result=activity_;
        if(map_){
            const auto* words=reinterpret_cast<const quint32*>(map_);
            result.ready=qFromLittleEndian(__atomic_load_n(words+20,__ATOMIC_ACQUIRE))==1;
            result.nativeSelections=qFromLittleEndian(__atomic_load_n(words+21,__ATOMIC_ACQUIRE));
            result.fallbacks=qFromLittleEndian(__atomic_load_n(words+22,__ATOMIC_ACQUIRE));
            result.adapterFailures=qFromLittleEndian(__atomic_load_n(words+23,__ATOMIC_ACQUIRE));
            result.waveHookInstalled=(qFromLittleEndian(__atomic_load_n(words+31,__ATOMIC_ACQUIRE))&MNM_AUDIO_WAVE_INSTALLED)!=0;
        }
        return result;
    }
    void stop(){
        timer_.stop();
        if(map_){__atomic_store_n(reinterpret_cast<quint32*>(map_)+20,0u,__ATOMIC_RELEASE);file_.unmap(map_);map_=nullptr;}
        file_.close();output_.reset();commands_.reset();device_.reset();waveSource_.reset();
    }
    QFile file_;uchar* map_=nullptr;std::unique_ptr<Device> device_;
    std::unique_ptr<VoiceCommands> commands_;std::unique_ptr<QtOutput> output_;
    std::unique_ptr<WaveSource> waveSource_;
    QTimer timer_;std::uint32_t last_=0;QString error_;VoiceActivity activity_;
    std::function<void(const QString&)> failed;
    void poll(){
        if(!map_)return;
        auto* words=reinterpret_cast<quint32*>(map_);
        __atomic_add_fetch(words+19,1u,__ATOMIC_RELEASE);
        const auto request=qFromLittleEndian(__atomic_load_n(words+4,__ATOMIC_ACQUIRE));
        if(!request || request==last_)return;
        std::uint32_t input[16];for(unsigned i=0;i<16;++i)input[i]=qFromLittleEndian(words[i]);
        BridgeReply reply{0x80070057,0};
        for(unsigned i=24;i<31;++i)words[i]=0;
        try{
            if(input[5]==MNM_AUDIO_LOAD_WAVE){
                if(!waveSource_)reply={0x80004001,0};
                else if(input[8]>0&&input[8]<=259&&input[6]==0&&input[7]==0&&input[15]==0){
                    const std::string leaf(reinterpret_cast<const char*>(map_+128),input[8]);
                    auto loaded=waveSource_->load(leaf);const auto& f=loaded.pcm.format;
                    const std::uint32_t metadata[]={std::uint32_t(loaded.pcm.samples.size()),f.rate,f.bytesPerSecond,
                        std::uint32_t(f.tag)|(std::uint32_t(f.channels)<<16),
                        std::uint32_t(f.alignment)|(std::uint32_t(f.bits)<<16),loaded.durationMs,f.extra};
                    std::memcpy(map_+128,loaded.pcm.samples.data(),loaded.pcm.samples.size());
                    for(unsigned i=0;i<7;++i)words[24+i]=qToLittleEndian(metadata[i]);
                    reply={0,metadata[0]};++activity_.waveLoads;activity_.waveBytes+=metadata[0];
                }
            }else if(input[8]<=MNM_AUDIO_PAYLOAD){
                const auto payload=input[5]==MNM_AUDIO_UPLOAD?QByteArray(reinterpret_cast<const char*>(map_+128),int(input[8])):QByteArray{};
                reply=commands_->execute(input,payload);
            }
        }catch(const std::bad_alloc&){reply={0x8007000e,0};}
        catch(const std::exception&){reply={0x80004005,0};}
        ++activity_.requests;
        if(reply.status)++activity_.failures;
        else switch(input[5]){
            case MNM_AUDIO_CREATE:++activity_.creates;break;
            case MNM_AUDIO_UPLOAD:++activity_.uploads;activity_.uploadedBytes+=input[8];break;
            case MNM_AUDIO_PLAY:++activity_.plays;break;
            case MNM_AUDIO_RELEASE:++activity_.releases;break;
            default:break;
        }
        last_=request;words[17]=qToLittleEndian(reply.status);words[18]=qToLittleEndian(reply.value);
        __atomic_store_n(words+16,qToLittleEndian(request),__ATOMIC_RELEASE);
    }
};
VoiceBroker::VoiceBroker(QObject* parent):QObject(parent){workerThread_.setObjectName("native-audio");}
VoiceBroker::~VoiceBroker(){stop();}
bool VoiceBroker::create(const QString& path,bool audible,const QString& soundsRoot){
    stop();error_.clear();lastActivity_={};
    const auto outputDevice=audible?QMediaDevices::defaultAudioOutput():QAudioDevice{};
    worker_=new QObject;worker_->moveToThread(&workerThread_);
    connect(&workerThread_,&QThread::finished,worker_,&QObject::deleteLater);
    workerThread_.start();
    bool created=false;QString error;const auto generation=generation_;
    QMetaObject::invokeMethod(worker_,[&]{
        try{
            state_=std::make_unique<State>(worker_);
            state_->failed=[this,generation](const QString& message){
                QMetaObject::invokeMethod(this,[this,generation,message]{
                    if(generation!=generation_)return;
                    error_=message;if(failed)failed(message);
                },Qt::QueuedConnection);
            };
            created=state_->create(path,audible,outputDevice,soundsRoot);error=state_->error_;
        }catch(const std::exception& exception){error=QString::fromUtf8(exception.what());}
    },Qt::BlockingQueuedConnection);
    if(!created){stop();error_=error;return false;}
    return true;
}
VoiceActivity VoiceBroker::activity() const{
    auto result=lastActivity_;
    if(worker_)QMetaObject::invokeMethod(worker_,[&]{result=state_->activity();},Qt::BlockingQueuedConnection);
    return result;
}
QString VoiceBroker::lastError() const{
    auto result=error_;
    if(worker_)QMetaObject::invokeMethod(worker_,[&]{result=state_->error_;},Qt::BlockingQueuedConnection);
    return result;
}
void VoiceBroker::stop(){
    ++generation_;
    if(!worker_)return;
    QMetaObject::invokeMethod(worker_,[&]{
        if(state_){lastActivity_=state_->activity();state_->stop();lastActivity_.ready=false;error_=state_->error_;state_.reset();}
    },Qt::BlockingQueuedConnection);
    workerThread_.quit();workerThread_.wait();worker_=nullptr;
}
}
