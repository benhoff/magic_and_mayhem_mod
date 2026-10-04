#include "menu_music_controller.hpp"
#include "native_playback.hpp"
#include <QMediaDevices>
#include <QAudioDevice>

namespace {
class QtMenuMusicOutput final:public MenuMusicOutput {
public:
    ~QtMenuMusicOutput() override{stop();}
    bool start(const QString& file,bool loop,QString& error) override{
        stop();
        if(QMediaDevices::defaultAudioOutput().isNull()){error="No audio output device is available for music";return false;}
        playback_=std::make_unique<NativePlayback>(false,true);playback_->setVolume(volume_);
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
    std::unique_ptr<NativePlayback> playback_;
};
}
std::unique_ptr<MenuMusicOutput> makeQtMenuMusicOutput(){return std::make_unique<QtMenuMusicOutput>();}
