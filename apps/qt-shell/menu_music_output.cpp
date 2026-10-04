#include "menu_music_controller.hpp"
#include "native_playback.hpp"
#include "menu_music_devices.hpp"
#include <QMediaDevices>
#include <QAudioDevice>

namespace {
class QtMenuMusicOutput final:public QObject,public MenuMusicOutput {
public:
    QtMenuMusicOutput(){
        connect(&devices_,&QMediaDevices::audioOutputsChanged,this,[this]{
            const auto current=QMediaDevices::defaultAudioOutput();std::vector<QByteArray> ids;
            for(const auto& device:QMediaDevices::audioOutputs())ids.push_back(device.id());
            const auto change=musicDeviceChange(playback_ && playback_->active(),device_.id(),ids,current.id());
            if(change!=MenuMusicDeviceChange::none && failed)
                failed(change==MenuMusicDeviceChange::disconnected?QString("Music output disconnected"):QString("Default music output changed"));
            if(!current.isNull() && available)available();
        });
    }
    ~QtMenuMusicOutput() override{stop();}
    bool start(const QString& file,bool loop,QString& error) override{
        stop();
        device_=QMediaDevices::defaultAudioOutput();
        if(device_.isNull()){error="No audio output device is available for music";return false;}
        playback_=std::make_unique<NativePlayback>(false,true);playback_->setAudioDevice(device_);playback_->setVolume(volume_);
        playback_->accepted=[this]{if(ready)ready();};
        playback_->finished=[this](unsigned result){
            if(result==MNM_MEDIA_V1_STATUS_CANCELLED)return;
            const auto error=playback_->report().value("error").toString();
            if(failed)failed(error.isEmpty()?QString("Music playback ended unexpectedly"):error);
        };
        playback_->start(file,loop);return true;
    }
    void stop() override{
        if(playback_){playback_->accepted={};playback_->finished={};playback_->stop();playback_.reset();}
    }
    void setVolume(float value) override{volume_=value;if(playback_)playback_->setVolume(value);}
private:
    float volume_=1.0f;
    QMediaDevices devices_;QAudioDevice device_;
    std::unique_ptr<NativePlayback> playback_;
};
}
std::unique_ptr<MenuMusicOutput> makeQtMenuMusicOutput(){return std::make_unique<QtMenuMusicOutput>();}
