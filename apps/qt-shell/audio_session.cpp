#include "audio_session.hpp"
#include <QThread>
#include <algorithm>
#include <stdexcept>
namespace r=mnm::reconstruction::audio;
namespace {
class SinkOutput final:public AudioSessionOutput {
public:
    explicit SinkOutput(QAudioDevice device):device_(std::move(device)){}
    std::uint32_t rate(QString& error) override{
        const auto format=mnm::audio::selectOutputFormat(device_);
        if(!format.isValid()){error="No supported stereo Int16 audio output";return 0;}
        return std::uint32_t(format.sampleRate());
    }
    bool start(mnm::audio::Device& device,QString& error) override{
        stop();sink_=std::make_unique<mnm::audio::QtOutput>(device);
        if(sink_->start(device_))return true;
        error=sink_->lastError();sink_.reset();return false;
    }
    void stop() override{if(sink_)sink_->stop();sink_.reset();}
    bool running() const override{return sink_ && sink_->running();}
    QString error() const override{return sink_?sink_->lastError():QString();}
private:
    QAudioDevice device_;std::unique_ptr<mnm::audio::QtOutput> sink_;
};
}
std::unique_ptr<AudioSessionOutput> makeQtSessionOutput(const QAudioDevice& device){return std::make_unique<SinkOutput>(device);}
AudioSession::AudioSession(std::unique_ptr<AudioSessionOutput> output,QObject* parent):QObject(parent),output_(std::move(output)){
    if(!output_)throw std::invalid_argument("Audio session requires an output adapter");
}
AudioSession::~AudioSession(){stop();}
void AudioSession::checkThread() const{
    if(QThread::currentThread()!=thread())throw std::logic_error("Audio session used outside its Qt thread");
}
bool AudioSession::running() const{return backend_ && output_->running() && manager_.cache.manager.active;}
QString AudioSession::lastError() const{const auto sinkError=output_->error();return sinkError.isEmpty()?error_:sinkError;}
std::uint32_t AudioSession::outputRate() const{return backend_?backend_->device().outputRate():0;}
bool AudioSession::start(const QString& root,std::uint32_t map,r::NativeSourcePathPolicy policy){
    checkThread();stop();error_.clear();report_={};
    try{
        auto made=mnm::assets::AssetStore::create(root.toStdString());
        if(const auto* e=std::get_if<mnm::assets::Error>(&made))throw std::runtime_error(e->detail);
        auto assets=std::get<mnm::assets::AssetStore>(std::move(made));
        report_=r::preflightCatalog(assets,policy);
        if(!report_.valid())throw std::runtime_error(report_.diagnostic);
        QString error;const auto rate=output_->rate(error);
        if(!rate)throw std::runtime_error(error.toStdString());
        manager_={};globals_={};clock_.start();random_.seed(0x4d4e4d);
        backend_=std::make_unique<r::NativeManagerBackend>(std::move(assets),manager_,
            [this]{return std::uint32_t(clock_.elapsed());},[this]{return random_.generate();},rate,policy);
        if(r::initializeManager(backend_->services(),manager_,globals_,0,0,".") ||
           r::initializeSourcePool(*backend_,manager_.cache,manager_.sources,map))
            throw std::runtime_error(backend_->diagnostic().empty()?"Audio manager/map initialization failed":backend_->diagnostic());
        // One spare caller slot allows admission to retire/evict a full ring.
        slots_.assign(std::size_t(globals_.simultaneousLimit)+1,0);
        if(!output_->start(backend_->device(),error))throw std::runtime_error(error.toStdString());
        return true;
    }catch(const std::exception& e){error_=QString::fromUtf8(e.what());stop();return false;}
}
bool AudioSession::play(std::int32_t sound,bool looping,std::int32_t volume,std::int32_t pan){
    checkThread();error_.clear();
    if(!running()){error_="Audio session is not running";return false;}
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
void AudioSession::stop(){
    checkThread();output_->stop(); // Discard sink/queue before releasing any samples.
    if(backend_){
        // Supported startup paths have a complete schedule ring. Partial native
        // construction failures unwind via host ownership, not unsafe original cleanup.
        if(manager_.cache.manager.initialized && manager_.schedules.head)
            r::destroyManager(backend_->services(),manager_,globals_);
        backend_.reset();
    }
    slots_.clear();manager_={};globals_={};
}
