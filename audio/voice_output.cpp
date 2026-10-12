#include "voice_output.hpp"
#include <exception>

namespace mnm::audio {
VoiceOutput::VoiceOutput(Device& device,QObject* parent):
    VoiceOutput(std::make_unique<QtOutput>(device),[]{return QMediaDevices::defaultAudioOutput();},true,parent){}
VoiceOutput::VoiceOutput(std::unique_ptr<QtOutput> output,Selector selector,bool monitor,QObject* parent):
    QObject(parent),output_(std::move(output)),select_(std::move(selector)),retry_(this){
    retry_.setInterval(500);
    connect(&retry_,&QTimer::timeout,this,[this]{recover();});
    if(monitor){
        devices_=std::make_unique<QMediaDevices>();
        connect(devices_.get(),&QMediaDevices::audioOutputsChanged,this,[this]{rescan();});
    }
    output_->failed=[this](const QString& error){lost(error);};
}
VoiceOutput::~VoiceOutput(){stop();output_->failed={};}
bool VoiceOutput::start(bool allowInitialSilence){
    stop();active_=true;recovering_=false;error_.clear();
    const auto selected=select_();
    if(allowInitialSilence && selected.isNull()){
        ++attempts_;
        output_->startSilent();recovering_=true;error_.clear();retry_.start();
        if(changed)changed();
        return true;
    }
    const auto started=attempt(selected);
    // Initial admission stays strict; automatic recovery concerns admitted output.
    if(!started){active_=false;retry_.stop();}
    return started;
}
void VoiceOutput::stop(){
    active_=false;recovering_=false;retry_.stop();output_->stop();selected_.clear();
}
bool VoiceOutput::attempt(const QAudioDevice& device){
    ++attempts_;
    try{
        if(output_->start(device)){
            selected_=device.id();error_.clear();retry_.stop();
            if(recovering_)++recoveries_;
            recovering_=false;if(changed)changed();return true;
        }
        error_=output_->lastError();
    }catch(const std::exception& error){error_=QString::fromUtf8(error.what());output_->stop();}
    if(error_.isEmpty())error_="Native voice output unavailable";
    if(active_)retry_.start();
    if(changed)changed();
    return false;
}
void VoiceOutput::lost(const QString& error){
    if(!active_)return;
    if(!recovering_)++losses_;
    recovering_=true;error_=error;retry_.start();if(changed)changed();
}
bool VoiceOutput::recover(){
    if(!active_)return false;
    if(silent()){
        const auto selected=select_();if(selected.isNull())return true;
        if(attempt(selected))return true;
        // An incompatible newly arrived endpoint must not stop logical cues.
        output_->startSilent();retry_.start();return true;
    }
    if(available())return true;
    return attempt(select_());
}
void VoiceOutput::rescan(){
    if(!active_)return;
    if(silent()){recover();return;}
    const auto selected=select_();
    if(available() && !selected.isNull() && selected.id()==selected_)return;
    output_->stop();lost("Native voice output selection changed");attempt(selected);
}
}
