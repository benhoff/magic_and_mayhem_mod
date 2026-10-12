#include "voice_bridge.hpp"
#include "wave_source.hpp"
#include "profile_source.hpp"
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
    constexpr std::uint32_t bufferLost=0x88780096u;
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
    case MNM_AUDIO_PRIMARY_PLAY:if(output_ && !output_->available())return {bufferLost,0};error=device_.playPrimary(w[7]);break;
    case MNM_AUDIO_PRIMARY_STOP:device_.stopPrimary();return {};
    case MNM_AUDIO_PRIMARY_STATUS:return {0,device_.primaryState().status()|(output_&&!output_->available()?2u:0u)};
    case MNM_AUDIO_PRIMARY_FORMAT:{
        if(w[10]>2 || w[11]>16 || w[12]>4 || w[14]>65535)return {0x80070057,0};
        error=device_.setPrimaryOutputFormat({std::uint16_t(w[14]),std::uint16_t(w[10]),w[9],w[13],std::uint16_t(w[12]),std::uint16_t(w[11]),0});break;
    }
    case MNM_AUDIO_DUPLICATE:error=device_.duplicate(id,created);break;
    case MNM_AUDIO_RELEASE:error=device_.release(id);break;
    case MNM_AUDIO_PLAY:
        if(!device_.voice(id))return {0x80070057,0};
        if(w[7]>1)return {0x80004001,0};
        if(output_ && !output_->available())return {bufferLost,0};
        error=timeline_?timeline_->play(id,w[7]):device_.play(id,w[7]);break;
    case MNM_AUDIO_STOP:error=timeline_?timeline_->stop(id):device_.stop(id);break;
    case MNM_AUDIO_RESET:if(w[7])return {0x80004001,0};error=timeline_?timeline_->seek(id,0):device_.resetPosition(id);break;
    case MNM_AUDIO_SEEK:error=timeline_?timeline_->seek(id,w[7]):device_.setPosition(id,w[7]);break;
    case MNM_AUDIO_FREQUENCY:error=device_.setFrequency(id,w[7]);break;
    case MNM_AUDIO_GET_FREQUENCY:{const auto frequency=device_.frequency(id);return frequency?BridgeReply{0,*frequency}:BridgeReply{0x80070057,0};}
    case MNM_AUDIO_CURSOR:{const auto cursor=timeline_?timeline_->cursor(id):device_.generatedCursor(id);return cursor?BridgeReply{0,std::uint32_t(*cursor)}:BridgeReply{0x80070057,0};}
    case MNM_AUDIO_VOLUME:case MNM_AUDIO_PAN:{
        const auto value=w[7]<=std::uint32_t(std::numeric_limits<std::int32_t>::max())?std::int32_t(w[7]):std::int32_t(std::int64_t(w[7])-4294967296ll);
        error=w[5]==MNM_AUDIO_VOLUME?device_.setVolume(id,value):device_.setPan(id,value);break;
    }
    case MNM_AUDIO_STATUS:{const auto voice=timeline_?timeline_->voice(id):device_.voice(id);return voice?BridgeReply{0,voice->status()|(output_&&!output_->available()?2u:0u)}:BridgeReply{0x80070057,0};}
    case MNM_AUDIO_RESTORE:
        if(id && !device_.voice(id))return {0x80070057,0};
        return !output_ || output_->recover()?BridgeReply{}:BridgeReply{bufferLost,0};
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
    if(timeline_ && error==Error::ok)switch(w[5]){
        case MNM_AUDIO_CREATE:case MNM_AUDIO_DUPLICATE:timeline_->changed(created);break;
        case MNM_AUDIO_RELEASE:timeline_->changed(id);break;
        default:break;
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
    bool create(const QString& path,bool audible,const QAudioDevice& outputDevice,const QString& soundsRoot,bool nativeProfiles){
        if(nativeProfiles && soundsRoot.isEmpty()){error_="Native profiles require a Sounds root";return false;}
        if(!soundsRoot.isEmpty()){
            auto assets=mnm::assets::AssetStore::create(soundsRoot.toStdString());
            if(const auto* error=std::get_if<mnm::assets::Error>(&assets)){error_=QString::fromStdString(error->detail);return false;}
            if(nativeProfiles)profileSource_=std::make_unique<ProfileSource>(std::get<mnm::assets::AssetStore>(assets));
            waveSource_=std::make_unique<WaveSource>(std::get<mnm::assets::AssetStore>(std::move(assets)),true);
        }
        QAudioFormat format;
        if(audible)format=selectOutputFormat(outputDevice);
        bool initialSilence=audible && outputDevice.isNull();
        if(initialSilence){format.setSampleRate(48000);format.setChannelCount(2);format.setSampleFormat(QAudioFormat::Int16);}
        if(audible&&!format.isValid()){error_="No supported native voice output device";return false;}
        device_=std::make_unique<Device>(MNM_AUDIO_PAYLOAD,128,audible?std::uint32_t(format.sampleRate()):48000);
        if(audible){
            output_=std::make_unique<VoiceOutput>(*device_);
            if(!output_->start(true)){error_=output_->lastError();return false;}
            initialSilence=output_->silent();
        }
        commands_=std::make_unique<VoiceCommands>(*device_,output_?&output_->timeline():nullptr,output_.get());
        file_.setFileName(path);
        if(!file_.open(QIODevice::ReadWrite|QIODevice::NewOnly)||!file_.resize(MNM_AUDIO_SIZE)){
            error_="Cannot create native voice channel";return false;
        }
        map_=file_.map(0,MNM_AUDIO_SIZE);
        if(!map_){error_="Cannot map native voice channel";return false;}
        std::memset(map_,0,128);std::memcpy(map_,"MNMAUD01",8);
        auto* words=reinterpret_cast<quint32*>(map_);
        words[2]=qToLittleEndian(MNM_AUDIO_VERSION);words[3]=qToLittleEndian(MNM_AUDIO_SIZE);
        words[31]=qToLittleEndian(MNM_AUDIO_CAP_EXTENDED_CONTROLS|MNM_AUDIO_CAP_OUTPUT_RECOVERY|
                                  (audible?MNM_AUDIO_CAP_CONSUMED_CLOCK:0u)|(waveSource_?MNM_AUDIO_CAP_NATIVE_WAVE:0u)|
                                  (initialSilence?MNM_AUDIO_CAP_INITIAL_SILENCE:0u)|(profileSource_?MNM_AUDIO_CAP_PROFILE_READS:0u));
        __atomic_store_n(words+20,qToLittleEndian(1u),__ATOMIC_RELEASE);
        activity_.audible=audible;activity_.initialSilence=initialSilence;timer_.start();return true;
    }
    VoiceActivity activity() const{
        auto result=activity_;
        if(output_){
            result.outputAvailable=output_->audible();result.outputSilent=output_->silent();result.outputLosses=output_->losses();
            result.outputRecoveries=output_->recoveries();result.outputAttempts=output_->attempts();
            result.generatedFrames=output_->timeline().generatedFrames();result.consumedFrames=output_->timeline().consumedFrames();
        }
        if(map_){
            const auto* words=reinterpret_cast<const quint32*>(map_);
            result.ready=qFromLittleEndian(__atomic_load_n(words+20,__ATOMIC_ACQUIRE))==1;
            result.nativeSelections=qFromLittleEndian(__atomic_load_n(words+21,__ATOMIC_ACQUIRE));
            result.fallbacks=qFromLittleEndian(__atomic_load_n(words+22,__ATOMIC_ACQUIRE));
            result.adapterFailures=qFromLittleEndian(__atomic_load_n(words+23,__ATOMIC_ACQUIRE));
            result.waveHookInstalled=(qFromLittleEndian(__atomic_load_n(words+31,__ATOMIC_ACQUIRE))&MNM_AUDIO_WAVE_INSTALLED)!=0;
            const auto capabilities=qFromLittleEndian(__atomic_load_n(words+31,__ATOMIC_ACQUIRE));
            result.nativeFactoryOnly=(capabilities&MNM_AUDIO_FACTORY_NATIVE_ONLY)!=0;
            result.profileHookInstalled=(capabilities&MNM_AUDIO_PROFILE_INSTALLED)!=0;
            result.unsupportedCalls=capabilities>>24;
        }
        return result;
    }
    QString currentError() const{return output_&&!output_->available()?output_->lastError():error_;}
    void stop(){
        timer_.stop();
        if(map_){__atomic_store_n(reinterpret_cast<quint32*>(map_)+20,0u,__ATOMIC_RELEASE);file_.unmap(map_);map_=nullptr;}
        file_.close();output_.reset();commands_.reset();device_.reset();waveSource_.reset();profileSource_.reset();
    }
    QFile file_;uchar* map_=nullptr;std::unique_ptr<Device> device_;
    std::unique_ptr<VoiceCommands> commands_;std::unique_ptr<VoiceOutput> output_;
    std::unique_ptr<WaveSource> waveSource_;
    std::unique_ptr<ProfileSource> profileSource_;
    QTimer timer_;std::uint32_t last_=0;QString error_;VoiceActivity activity_;
    std::function<void(const QString&)> failed;
    void poll(){
        if(!map_)return;
        if(output_)output_->refreshClock();
        auto* words=reinterpret_cast<quint32*>(map_);
        __atomic_add_fetch(words+19,1u,__ATOMIC_RELEASE);
        const auto request=qFromLittleEndian(__atomic_load_n(words+4,__ATOMIC_ACQUIRE));
        if(!request || request==last_)return;
        std::uint32_t input[16];for(unsigned i=0;i<16;++i)input[i]=qFromLittleEndian(words[i]);
        BridgeReply reply{0x80070057,0};
        for(unsigned i=24;i<31;++i)words[i]=0;
        try{
            if(input[5]>=MNM_AUDIO_PROFILE_STRING && input[5]<=MNM_AUDIO_PROFILE_INTEGER){
                if(!profileSource_)reply={0x80004001,0};
                else if(input[8]>0 && input[8]<=1024 && !input[6] && !input[15]){
                    bool reserved=false;for(unsigned i=9;i<15;++i)reserved|=input[i]!=0;
                    if(reserved)throw std::invalid_argument("Reserved native profile fields");
                    const std::string text(reinterpret_cast<const char*>(map_+128),input[8]);
                    std::vector<std::string> fields;std::size_t start=0;
                    for(std::size_t i=0;i<text.size();++i){
                        const auto c=static_cast<unsigned char>(text[i]);
                        if(!c){fields.push_back(text.substr(start,i-start));start=i+1;}
                        else if(c<32 || c>=127)throw std::invalid_argument("Native profile request requires ASCII");
                    }
                    const auto op=input[5];const auto expected=op==MNM_AUDIO_PROFILE_SECTION?1u:op==MNM_AUDIO_PROFILE_INTEGER?2u:3u;
                    if(start!=text.size() || fields.size()!=expected || fields[0].empty() || fields[0].size()>127 ||
                       (expected>1 && (fields[1].empty() || fields[1].size()>127)) || (expected==3 && fields[2].size()>259))
                        throw std::invalid_argument("Invalid native profile text fields");
                    std::string bytes;std::uint32_t result=0;
                    if(op==MNM_AUDIO_PROFILE_INTEGER){result=profileSource_->integer(fields[0],fields[1],input[7]);++activity_.profileIntegers;}
                    else{
                        if(!input[7] || input[7]>16384 || (op==MNM_AUDIO_PROFILE_SECTION && input[7]<2))throw std::invalid_argument("Native profile capacity");
                        if(op==MNM_AUDIO_PROFILE_STRING){bytes=profileSource_->value(fields[0],fields[1],input[7],fields[2]);result=std::uint32_t(bytes.size());bytes.push_back(0);++activity_.profileStrings;}
                        else{const auto section=profileSource_->section(fields[0],input[7]);result=section.returned;
                            for(const auto& entry:section.entries){bytes+=entry;bytes.push_back(0);}
                            bytes.push_back(0);if(bytes.size()==1)bytes.push_back(0);++activity_.profileSections;}
                        if(bytes.size()>input[7])throw std::invalid_argument("Native profile reply capacity");
                    }
                    if(!bytes.empty())std::memcpy(map_+128,bytes.data(),bytes.size());
                    words[24]=qToLittleEndian(std::uint32_t(bytes.size()));reply={0,result};
                }
            }else if(input[5]==MNM_AUDIO_LOAD_WAVE){
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
bool VoiceBroker::create(const QString& path,bool audible,const QString& soundsRoot,bool nativeProfiles){
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
            created=state_->create(path,audible,outputDevice,soundsRoot,nativeProfiles);error=state_->error_;
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
    if(worker_)QMetaObject::invokeMethod(worker_,[&]{result=state_->currentError();},Qt::BlockingQueuedConnection);
    return result;
}
void VoiceBroker::stop(){
    ++generation_;
    if(!worker_)return;
    QMetaObject::invokeMethod(worker_,[&]{
        if(state_){lastActivity_=state_->activity();error_=state_->currentError();state_->stop();lastActivity_.ready=false;state_.reset();}
    },Qt::BlockingQueuedConnection);
    workerThread_.quit();workerThread_.wait();worker_=nullptr;
}
}
