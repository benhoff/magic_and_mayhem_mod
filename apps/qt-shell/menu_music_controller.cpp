#include "menu_music_controller.hpp"
#include "menu_audio_preferences.hpp"
#include "menu_preview.hpp"
#include <QFile>
#include <QFileInfo>
#include <QStackedWidget>
#include <QTimer>
#include <QThread>
#include <cmath>
#include <stdexcept>

namespace {
float gain(int level){return level== -10000?0.0f:float(std::pow(10.0,double(level)/2000.0));}
}
MenuMusicController::MenuMusicController(std::unique_ptr<MenuMusicOutput> output,QSettings* settings,QObject* parent)
    :QObject(parent),output_(std::move(output)),settings_(settings){
    if(!output_)throw std::invalid_argument("Menu music requires an output");
    musicLevel_=menuAudioPreferences::read(settings,"audio/v1/musicLevel",-1500);
}
MenuMusicController::~MenuMusicController(){changed={};failed={};stop();}
void MenuMusicController::attach(MenuPreview& preview){
    checkThread();
    if(preview_)throw std::logic_error("Menu music already attached");
    preview_=&preview;
    QString error;
    if(settings_ && !preview.setMusicVolume(musicLevel_,&error) && failed)failed(error);
    connect(&preview,&MenuPreview::musicRetryRequested,this,[this]{recover();});
    connect(&preview,&MenuPreview::screenReady,this,[this](QWidget* screen){bind(screen);});
    connect(&preview,&MenuPreview::closed,this,[this]{stop();});
    connect(&preview,&QObject::destroyed,this,[this]{preview_=nullptr;bound_.clear();stop();});
    if(auto* stack=preview.findChild<QStackedWidget*>())for(int i=0;i<stack->count();++i)bind(stack->widget(i));
}
void MenuMusicController::publish(MenuMusicState state,const QString& message){
    state_=state;
    if(preview_)preview_->setMusicStatus(state==MenuMusicState::loading?QString("Loading music…"):
        state==MenuMusicState::playing?QString("Music playing"):
        state==MenuMusicState::recovering?QString("Restarting music…"):
        state==MenuMusicState::failed?QString("Music paused: %1").arg(message):QString("Music stopped"),canRecover());
    if(changed)changed(state,message);
}
void MenuMusicController::checkThread() const{
    if(QThread::currentThread()!=thread())throw std::logic_error("Menu music used outside its Qt thread");
}
void MenuMusicController::watchAvailability(){
    if(track_.isEmpty()){output_->available={};return;}
    const auto generation=generation_;const QPointer<MenuMusicController> guard(this);
    output_->available=[guard,generation]{if(guard && generation==guard->generation_)guard->outputAvailable();};
}
void MenuMusicController::fault(const QString& message){
    const bool retry=availabilityPending_;
    ++generation_;failurePending_=availabilityPending_=false;
    output_->ready={};output_->failed={};output_->available={};output_->stop();
    error_=message.isEmpty()?QString("Music playback failed"):message;
    watchAvailability();const auto generation=generation_;const auto diagnostic=error_;
    publish(MenuMusicState::failed,diagnostic);
    if(failed)failed(diagnostic);
    if(retry && generation==generation_)outputAvailable();
}
void MenuMusicController::outputFailed(const QString& message){
    checkThread();
    if(failurePending_ || (state_!=MenuMusicState::loading && state_!=MenuMusicState::playing && state_!=MenuMusicState::recovering))return;
    failurePending_=true;error_=message.isEmpty()?QString("Music playback failed"):message;
    const auto generation=generation_;const QPointer<MenuMusicController> guard(this);
    QTimer::singleShot(0,this,[guard,generation]{
        if(guard && generation==guard->generation_ && guard->failurePending_)guard->fault(guard->error_);
    });
}
void MenuMusicController::outputAvailable(){
    checkThread();
    if(failurePending_){availabilityPending_=true;return;}
    if(!canRecover() || availabilityPending_)return;
    availabilityPending_=true;const auto generation=generation_;const QPointer<MenuMusicController> guard(this);
    QTimer::singleShot(0,this,[guard,generation]{
        if(!guard || generation!=guard->generation_)return;
        guard->availabilityPending_=false;
        if(guard->canRecover())guard->recover();
    });
}
bool MenuMusicController::start(const QString& file){
    checkThread();stop();return launch(file,false);
}
bool MenuMusicController::recover(){
    checkThread();if(!canRecover())return false;
    const auto file=track_;const auto generation=generation_;
    publish(MenuMusicState::recovering);
    if(generation!=generation_ || track_.isEmpty())return false;
    return launch(file,true);
}
bool MenuMusicController::launch(const QString& file,bool recovery){
    ++generation_;failurePending_=availabilityPending_=false;error_.clear();
    output_->ready={};output_->failed={};output_->available={};output_->stop();
    const QFileInfo info(file);QFile input(info.absoluteFilePath());
    if(file.isEmpty() || !info.isFile() || !input.open(QIODevice::ReadOnly)){
        fault("Music track is not a readable local file");return false;
    }
    input.close();track_=info.absoluteFilePath();watchAvailability();
    const auto generation=generation_;const QPointer<MenuMusicController> guard(this);
    output_->ready=[guard,generation]{
        if(!guard)return;
        QTimer::singleShot(0,guard,[guard,generation]{
            if(guard && generation==guard->generation_ && !guard->failurePending_ &&
               (guard->state_==MenuMusicState::loading || guard->state_==MenuMusicState::recovering))guard->publish(MenuMusicState::playing);
        });
    };
    output_->failed=[guard,generation](const QString& message){
        if(guard && generation==guard->generation_)guard->outputFailed(message);
    };
    output_->setVolume(gain(musicLevel_)); // Before any decoder/output activity.
    if(!recovery)publish(MenuMusicState::loading);
    if(generation!=generation_)return false; // Closure during notification wins.
    QString error;
    if(!output_->start(track_,true,error)){
        if(generation==generation_)fault(failurePending_?error_:error);
        return false;
    }
    return generation==generation_;
}
void MenuMusicController::stop(){
    checkThread();++generation_;track_.clear();failurePending_=availabilityPending_=false;
    output_->ready={};output_->failed={};output_->available={};output_->stop();
    publish(MenuMusicState::stopped);
}
void MenuMusicController::applyMusicLevel(int level){
    checkThread();
    if(level< -10000 || level>0){if(failed)failed("Music level outside native range");return;}
    musicLevel_=level;output_->setVolume(gain(level));
    if(!menuAudioPreferences::save(settings_,"audio/v1/musicLevel",level) && failed)
        failed("Could not save music volume; accepted value remains active for this session");
}
void MenuMusicController::bind(QWidget* screen){
    if(bound_.contains(screen))return;
    bound_.insert(screen);connect(screen,&QObject::destroyed,this,[this,screen]{bound_.remove(screen);});
    if(auto* preferences=qobject_cast<PreferencesWidget*>(screen)){
        if(!settings_){musicLevel_=preferences->settings().musicLevel;output_->setVolume(gain(musicLevel_));}
        connect(preferences,&PreferencesWidget::settingsApplied,this,[this](const auto& settings){applyMusicLevel(settings.musicLevel);});
    }
}
