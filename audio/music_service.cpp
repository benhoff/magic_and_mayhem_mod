#include "music_service.hpp"
#include <QPointer>
#include <QThread>
#include <QTimer>
#include <algorithm>
#include <stdexcept>
namespace mnm::audio {
MusicService::MusicService(std::unique_ptr<MusicOutput> output,QObject* parent):QObject(parent),output_(std::move(output)){
    if(!output_)throw std::invalid_argument("Music service requires an output");
    if(auto* object=dynamic_cast<QObject*>(output_.get())){
        if(object->thread()!=thread())throw std::invalid_argument("Music output belongs to another thread");
        object->setParent(this);
    }
}
MusicService::~MusicService(){changed={};close();}
void MusicService::checkThread()const{if(QThread::currentThread()!=thread())throw std::logic_error("Music service used outside its owner thread");}
MusicSnapshot MusicService::snapshot()const{
    checkThread();auto result=state_;
    if(result.phase==MusicPhase::playing||result.phase==MusicPhase::paused||result.phase==MusicPhase::loading)result.positionMs=output_->positionMs();
    return result;
}
void MusicService::publish(){if(changed)changed(snapshot());}
void MusicService::retire(){
    ++state_.generation;recoveryPending_=false;output_->ready={};output_->complete={};output_->failed={};output_->lost={};output_->outputAvailable={};output_->stop();
}
bool MusicService::initialize(const std::filesystem::path& root){
    checkThread();close();
    try{catalog_=std::make_unique<MusicCatalog>(MusicCatalog::load(root));state_.phase=MusicPhase::stopped;state_.error.clear();output_->setVolume(state_.volume);const auto generation=state_.generation;publish();return generation==state_.generation&&bool(catalog_);}
    catch(const std::exception& error){state_.phase=MusicPhase::failed;state_.error=QString::fromUtf8(error.what());publish();return false;}
}
void MusicService::close(){checkThread();retire();catalog_.reset();state_.phase=MusicPhase::closed;state_.track=0;state_.positionMs=0;state_.loop=false;state_.error.clear();publish();}
void MusicService::stop(){
    checkThread();retire();state_.phase=catalog_?MusicPhase::stopped:MusicPhase::closed;state_.track=0;state_.positionMs=0;state_.loop=false;state_.error.clear();publish();
}
bool MusicService::play(std::uint32_t track,bool loop,qint64 offsetMs){checkThread();return launch(track,loop,offsetMs,false,false);}
bool MusicService::launch(std::uint32_t track,bool loop,qint64 offsetMs,bool paused,bool recovery){
    if(!catalog_)return false;
    const auto* entry=catalog_->track(track);
    if(!entry||entry->file.empty()||offsetMs<0||std::uint64_t(offsetMs)>=(entry->frames*1000+44099)/44100)return false;
    if(!output_->available())return false; // Refused requests are not replayed later.
    retire();state_.track=track;state_.loop=loop;state_.positionMs=offsetMs;state_.error.clear();state_.phase=MusicPhase::loading;resumePaused_=paused;
    const auto generation=state_.generation;const QPointer<MusicService> guard(this);
    output_->ready=[guard,generation]{if(guard)QTimer::singleShot(0,guard,[guard,generation]{
        if(!guard||generation!=guard->state_.generation||guard->state_.phase!=MusicPhase::loading)return;
        guard->state_.phase=guard->resumePaused_?MusicPhase::paused:MusicPhase::playing;guard->publish();});};
    output_->complete=[guard,generation]{if(guard)QTimer::singleShot(0,guard,[guard,generation]{
        if(!guard||generation!=guard->state_.generation||(guard->state_.phase!=MusicPhase::loading&&guard->state_.phase!=MusicPhase::playing&&guard->state_.phase!=MusicPhase::paused))return;
        guard->state_.positionMs=guard->output_->positionMs();++guard->state_.completions;guard->state_.phase=MusicPhase::completed;guard->publish();});};
    output_->failed=[guard,generation](const QString& error){if(guard)QTimer::singleShot(0,guard,[guard,generation,error]{
        if(!guard||generation!=guard->state_.generation||(guard->state_.phase!=MusicPhase::loading&&guard->state_.phase!=MusicPhase::playing&&guard->state_.phase!=MusicPhase::paused))return;
        guard->state_.positionMs=guard->output_->positionMs();guard->retire();guard->state_.phase=MusicPhase::failed;guard->state_.error=error;guard->publish();});};
    output_->lost=[guard,generation](const QString& error){if(guard)QTimer::singleShot(0,guard,[guard,generation,error]{
        if(guard&&generation==guard->state_.generation)guard->outputLost(error);});};
    watchAvailability();output_->setVolume(state_.volume);const auto admittedGeneration=state_.generation;publish();
    if(admittedGeneration!=state_.generation)return false;
    QString error;const auto path=QString::fromStdString(entry->file.string());
    if(!output_->start(path,loop,offsetMs,paused,error)){
        if(admittedGeneration==state_.generation){retire();state_.phase=MusicPhase::failed;state_.error=error;publish();}return false;
    }
    if(recovery&&admittedGeneration==state_.generation)++state_.recoveries;
    return admittedGeneration==state_.generation;
}
bool MusicService::pause(){
    checkThread();if(state_.phase==MusicPhase::unavailable){resumePaused_=true;return true;}
    if(state_.phase!=MusicPhase::playing&&state_.phase!=MusicPhase::loading&&state_.phase!=MusicPhase::paused)return false;
    if(!output_->pause(true))return false;
    resumePaused_=true;state_.phase=MusicPhase::paused;publish();return true;
}
bool MusicService::resume(){
    checkThread();if(state_.phase==MusicPhase::unavailable){resumePaused_=false;return true;}
    if(state_.phase!=MusicPhase::paused)return false;
    if(!output_->pause(false))return false;
    resumePaused_=false;state_.phase=MusicPhase::playing;publish();return true;
}
bool MusicService::seek(qint64 milliseconds){
    checkThread();const auto* track=catalog_?catalog_->track(state_.track):nullptr;
    if(!track||milliseconds<0||std::uint64_t(milliseconds)>=(track->frames*1000+44099)/44100)return false;
    if(state_.phase==MusicPhase::unavailable){state_.positionMs=milliseconds;return true;}
    if(state_.phase!=MusicPhase::playing&&state_.phase!=MusicPhase::paused)return false;
    return output_->seek(milliseconds);
}
bool MusicService::volume(std::uint16_t level){checkThread();state_.volume=level;output_->setVolume(level);publish();return true;}
void MusicService::outputLost(const QString& error){
    if(state_.phase!=MusicPhase::playing&&state_.phase!=MusicPhase::paused&&state_.phase!=MusicPhase::loading)return;
    resumePaused_=state_.phase==MusicPhase::paused;state_.positionMs=std::max<qint64>(0,output_->positionMs());
    const auto* track=catalog_?catalog_->track(state_.track):nullptr;
    if(track)state_.positionMs=std::min(state_.positionMs,qint64((track->frames*1000+44099)/44100)-1);
    retire();++state_.losses;state_.phase=MusicPhase::unavailable;state_.error=error;watchAvailability();publish();
    if(output_->available()&&output_->outputAvailable)output_->outputAvailable();
}
void MusicService::watchAvailability(){
    const auto generation=state_.generation;const QPointer<MusicService> guard(this);
    output_->outputAvailable=[guard,generation]{
        if(!guard||generation!=guard->state_.generation||guard->state_.phase!=MusicPhase::unavailable||guard->recoveryPending_)return;
        guard->recoveryPending_=true;QTimer::singleShot(0,guard,[guard,generation]{
            if(!guard||generation!=guard->state_.generation)return;
            guard->recoveryPending_=false;guard->recover();});
    };
}
bool MusicService::recover(){checkThread();if(state_.phase!=MusicPhase::unavailable)return false;
    return launch(state_.track,state_.loop,state_.positionMs,resumePaused_,true);
}
}
