#include "audio_session.hpp"
#include <QThread>
#include <QMediaDevices>
#include <algorithm>
#include <stdexcept>
namespace r=mnm::reconstruction::audio;
namespace {
class SinkOutput final:public QObject,public AudioSessionOutput {
public:
    explicit SinkOutput(QAudioDevice device,bool followDefault=false):device_(std::move(device)),followDefault_(followDefault){
        if(followDefault_ || !device_.isNull())connect(&devices_,&QMediaDevices::audioOutputsChanged,this,[this]{
            if(sink_ && sink_->running()){
                const auto outputs=QMediaDevices::audioOutputs();
                const bool present=std::any_of(outputs.begin(),outputs.end(),[this](const auto& d){return d.id()==device_.id();});
                const bool changedDefault=followDefault_ && QMediaDevices::defaultAudioOutput().id()!=device_.id();
                if(!present || changedDefault){
                    error_=!present?"Audio output disconnected":"Default audio output changed";
                    if(failed)failed(error_);
                }
            }
            if(available)available();
        });
    }
    std::uint32_t rate(QString& error) override{
        if(followDefault_)device_=QMediaDevices::defaultAudioOutput();
        else if(!device_.isNull()){
            const auto id=device_.id();device_={};
            for(const auto& candidate:QMediaDevices::audioOutputs())if(candidate.id()==id){device_=candidate;break;}
            // Keep the requested identity for a later reconnect.
            if(device_.isNull()){missingId_=id;}
        }else if(!missingId_.isEmpty()){
            for(const auto& candidate:QMediaDevices::audioOutputs())if(candidate.id()==missingId_){device_=candidate;break;}
        }
        const auto format=mnm::audio::selectOutputFormat(device_);
        if(!format.isValid()){error="No supported stereo Int16 audio output";error_=error;return 0;}
        error_.clear();return std::uint32_t(format.sampleRate());
    }
    bool start(mnm::audio::Device& device,QString& error) override{
        stop();error_.clear();sink_=std::make_unique<mnm::audio::QtOutput>(device);
        sink_->failed=[this](const QString& message){error_=message;if(failed)failed(message);};
        if(sink_->start(device_))return true;
        error=error_.isEmpty()?sink_->lastError():error_;error_=error;sink_.reset();return false;
    }
    void stop() override{if(sink_){sink_->failed={};sink_->stop();}sink_.reset();}
    bool running() const override{return sink_ && sink_->running();}
    QString error() const override{return error_;}
private:
    QAudioDevice device_;bool followDefault_;QByteArray missingId_;QString error_;
    QMediaDevices devices_;std::unique_ptr<mnm::audio::QtOutput> sink_;
};
}
std::unique_ptr<AudioSessionOutput> makeQtSessionOutput(const QAudioDevice& device){return std::make_unique<SinkOutput>(device);}
std::unique_ptr<AudioSessionOutput> makeQtSessionOutput(){return std::make_unique<SinkOutput>(QAudioDevice{},true);}
AudioSession::AudioSession(std::unique_ptr<AudioSessionOutput> output,QObject* parent):QObject(parent),output_(std::move(output)){
    if(!output_)throw std::invalid_argument("Audio session requires an output adapter");
    output_->failed=[this](const QString& message){outputFailed(message);};
    output_->available=[this]{outputAvailable();};
}
AudioSession::~AudioSession(){changed={};output_->failed={};output_->available={};stop();}
void AudioSession::checkThread() const{
    if(QThread::currentThread()!=thread())throw std::logic_error("Audio session used outside its Qt thread");
}
bool AudioSession::running() const{return state_==AudioSessionState::running && !failurePending_ && backend_ && output_->running() && manager_.cache.manager.active;}
QString AudioSession::lastError() const{return error_;}
std::uint32_t AudioSession::outputRate() const{return backend_?backend_->device().outputRate():0;}
bool AudioSession::start(const QString& root,std::uint32_t map,r::NativeSourcePathPolicy policy){
    checkThread();stop();
    return launch({root,map,policy});
}
bool AudioSession::launch(const Configuration& requested){
    const auto config=requested; // Recovery can replace/reset the stored optional.
    clearRuntime();++generation_;failurePending_=availabilityPending_=false;
    error_.clear();report_={};bool outputStage=false;
    try{
        const auto& root=config.root;const auto map=config.map;const auto policy=config.policy;
        auto made=mnm::assets::AssetStore::create(root.toStdString());
        if(const auto* e=std::get_if<mnm::assets::Error>(&made))throw std::runtime_error(e->detail);
        auto assets=std::get<mnm::assets::AssetStore>(std::move(made));
        report_=r::preflightCatalog(assets,policy);
        if(!report_.valid())throw std::runtime_error(report_.diagnostic);
        configuration_=config;outputStage=true;
        QString error;const auto rate=output_->rate(error);
        if(!rate)throw std::runtime_error(error.toStdString());
        outputStage=false;manager_={};globals_={};clock_.start();random_.seed(0x4d4e4d);
        backend_=std::make_unique<r::NativeManagerBackend>(std::move(assets),manager_,
            [this]{return std::uint32_t(clock_.elapsed());},[this]{return random_.generate();},rate,policy);
        if(r::initializeManager(backend_->services(),manager_,globals_,0,0,".") ||
           r::initializeSourcePool(*backend_,manager_.cache,manager_.sources,map))
            throw std::runtime_error(backend_->diagnostic().empty()?"Audio manager/map initialization failed":backend_->diagnostic());
        // One spare caller slot allows admission to retire/evict a full ring.
        slots_.assign(std::size_t(globals_.simultaneousLimit)+1,0);
        if(backend_->volume(manager_.cache.manager.primary,masterVolume_))throw std::runtime_error("Cannot restore native master volume");
        outputStage=true;
        if(!output_->start(backend_->device(),error))throw std::runtime_error(error.toStdString());
        if(failurePending_)throw std::runtime_error(error_.toStdString());
        setState(AudioSessionState::running);return running();
    }catch(const std::exception& e){
        error_=QString::fromUtf8(e.what());++generation_;failurePending_=availabilityPending_=false;
        clearRuntime();if(!outputStage)configuration_.reset();setState(AudioSessionState::failed);return false;
    }
}
bool AudioSession::play(std::int32_t sound,bool looping,std::int32_t volume,std::int32_t pan){
    checkThread();
    if(!running()){if(error_.isEmpty())error_="Audio session is not running";return false;}
    error_.clear();
    if(!report_.playable(sound)){error_="Sound or randomized group is unavailable in catalog preflight";return false;}
    if(volume< -10000 || volume>0 || pan< -10000 || pan>10000){error_="Volume/pan outside native control range";return false;}
    const auto slot=std::find(slots_.begin(),slots_.end(),0u);
    if(slot==slots_.end()){error_="No caller slot available";return false;}
    r::AdmissionRequest request;request.sound=sound;request.looping=looping;request.volume=volume;request.pan=pan;
    request.output=&*slot;request.outputAddress=std::uint32_t(slot-slots_.begin()+1);
    try{
        const auto status=r::admitVoice(*backend_,manager_.cache.manager,manager_.cache.catalog,
            manager_.sources.head,manager_.schedules.head,0,request);
        backend_->collectDisposed();
        if(!status && *slot)return true;
        error_=status?QString::fromStdString(backend_->diagnostic()):"Audio schedule has no admissible slot";
        if(error_.isEmpty())error_="Native audio admission failed";
    }catch(const std::exception& e){error_=QString::fromUtf8(e.what());}
    return false;
}
bool AudioSession::setMasterVolume(std::int32_t level){
    checkThread();
    if(level< -10000 || level>0){error_="Master volume outside native control range";return false;}
    masterVolume_=level; // Accepted settings can change while the device is absent.
    if(!running())return true;
    error_.clear();
    if(backend_->volume(manager_.cache.manager.primary,level)){error_="Cannot apply native master volume";return false;}
    return true;
}
bool AudioSession::clearVoices(){
    checkThread();
    if(!running()){if(error_.isEmpty())error_="Audio session is not running";return false;}
    error_.clear();
    output_->stop(); // Discard already mixed PCM, including partial writes.
    try{
        // Host cancellation stops every cached root/duplicate, even if its
        // recovered wall-clock deadline expired before the output consumed it.
        auto* root=manager_.sources.head;
        if(root)do{
            for(auto* voice=root;voice;voice=voice->duplicate)if(voice->buffer){
                if(backend_->stop(voice->buffer) || backend_->position(voice->buffer,0))
                    throw std::runtime_error("Cannot reset native menu voice");
            }
            root=root->next;
        }while(root!=manager_.sources.head);
        r::clearSchedules(*backend_,manager_.schedules.head,true);
        std::fill(slots_.begin(),slots_.end(),0u);
        QString error;
        if(!output_->start(backend_->device(),error))throw std::runtime_error(error.toStdString());
        return true;
    }catch(const std::exception& e){
        error_=QString::fromUtf8(e.what());++generation_;failurePending_=availabilityPending_=false;
        clearRuntime();setState(AudioSessionState::failed);return false;
    }
}
void AudioSession::clearRuntime(){
    output_->stop(); // Discard sink/queue before releasing any samples.
    if(backend_){
        // Supported startup paths have a complete schedule ring. Partial native
        // construction failures unwind via host ownership, not unsafe original cleanup.
        if(manager_.cache.manager.initialized && manager_.schedules.head)
            r::destroyManager(backend_->services(),manager_,globals_);
        backend_.reset();
    }
    slots_.clear();manager_={};globals_={};
}

void AudioSession::setState(AudioSessionState state){
    state_=state;if(changed)changed(state,error_);
}
void AudioSession::stop(){
    checkThread();++generation_;configuration_.reset();failurePending_=availabilityPending_=false;
    clearRuntime();
    if(state_!=AudioSessionState::stopped)setState(AudioSessionState::stopped);
}
bool AudioSession::recover(){
    checkThread();
    if(!configuration_ || failurePending_ || state_==AudioSessionState::running || state_==AudioSessionState::recovering)return false;
    const auto config=*configuration_;const auto generation=generation_;
    setState(AudioSessionState::recovering);
    if(generation!=generation_ || !configuration_)return false;
    return launch(config);
}
void AudioSession::outputFailed(const QString& message){
    checkThread();
    if(failurePending_ || !configuration_ || (state_!=AudioSessionState::running && state_!=AudioSessionState::recovering && !backend_))return;
    error_=message.isEmpty()?"Audio output failed":message;failurePending_=true;
    const auto generation=generation_;
    QTimer::singleShot(0,this,[this,generation]{
        if(generation!=generation_ || !failurePending_)return;
        const bool retry=availabilityPending_;failurePending_=availabilityPending_=false;
        clearRuntime();setState(AudioSessionState::failed);
        if(retry)outputAvailable();
    });
}
void AudioSession::outputAvailable(){
    checkThread();
    if(failurePending_){availabilityPending_=true;return;}
    if(!configuration_ || state_!=AudioSessionState::failed || availabilityPending_)return;
    availabilityPending_=true;const auto generation=generation_;
    QTimer::singleShot(0,this,[this,generation]{
        if(generation!=generation_)return;
        availabilityPending_=false;
        if(state_==AudioSessionState::failed && configuration_)recover();
    });
}
