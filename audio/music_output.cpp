#include "music_service.hpp"
#include <QAudioBuffer>
#include <QAudioBufferOutput>
#include <QAudioDevice>
#include <QAudioOutput>
#include <QCryptographicHash>
#include <QMediaDevices>
#include <QMediaPlayer>
#include <QPointer>
#include <QUrl>
#include <algorithm>
namespace mnm::audio {
namespace {
QAudioFormat captureFormat(){QAudioFormat format;format.setSampleRate(44100);format.setChannelCount(2);format.setSampleFormat(QAudioFormat::Int16);return format;}
class QtMusicOutput final:public QObject,public MusicOutput {
public:
    explicit QtMusicOutput(bool audible):audible_(audible){
        devices_.setParent(this);output_.setParent(this);
        connect(&devices_,&QMediaDevices::audioOutputsChanged,this,[this]{
            const auto current=QMediaDevices::defaultAudioOutput();
            const auto devices=QMediaDevices::audioOutputs();bool present=false;
            for(const auto& device:devices)present|=device.id()==device_.id();
            if(audible_&&player_&&player_->playbackState()!=QMediaPlayer::StoppedState&&(!present||current.id()!=device_.id())){
                if(lost)lost(present?QString("Default music output changed"):QString("Music output disconnected"));
            }
            if(available()&&outputAvailable)outputAvailable();
        });
    }
    ~QtMusicOutput()override{stop();}
    bool available()const override{return !audible_||!QMediaDevices::defaultAudioOutput().isNull();}
    bool start(const QString& file,bool loop,qint64 offsetMs,bool paused,QString& error)override{
        stop();if(!available()){error="No music output device is available";return false;}
        device_=QMediaDevices::defaultAudioOutput();if(audible_)output_.setDevice(device_);
        file_=file;frames_=bytes_=0;position_=0;accepted_=ended_=false;error_.clear();hash_.reset();
        desiredPaused_=paused;offset_=offsetMs;pendingSeek_=true;setVolume(level_);
        auto* player=new QMediaPlayer(this);player_=player;auto* buffers=new QAudioBufferOutput(captureFormat(),player);
        player->setAudioBufferOutput(buffers);if(audible_)player->setAudioOutput(&output_);player->setLoops(loop?QMediaPlayer::Infinite:1);
        const auto generation=generation_;
        connect(buffers,&QAudioBufferOutput::audioBufferReceived,this,[this,generation](const QAudioBuffer& buffer){
            if(generation!=generation_||!buffer.isValid())return;
            frames_+=buffer.frameCount();bytes_+=buffer.byteCount();
            hash_.addData(QByteArrayView(buffer.constData<char>(),buffer.byteCount()));
        });
        connect(player,&QMediaPlayer::mediaStatusChanged,this,[this,generation](QMediaPlayer::MediaStatus status){
            if(generation!=generation_||!player_)return;
            if(status==QMediaPlayer::LoadedMedia||status==QMediaPlayer::BufferedMedia){
                if(!player_->hasAudio()){error_="Music asset has no audio track";if(failed)failed(error_);return;}
                if(pendingSeek_){pendingSeek_=false;if(offset_)player_->setPosition(offset_);if(desiredPaused_)player_->pause();}
                if(!accepted_){accepted_=true;if(ready)ready();}
            }
            if(status==QMediaPlayer::EndOfMedia&&!ended_){ended_=true;position_=player_->position();if(complete)complete();}
        });
        connect(player,&QMediaPlayer::positionChanged,this,[this,generation](qint64 value){if(generation==generation_)position_=value;});
        connect(player,&QMediaPlayer::errorOccurred,this,[this,generation](QMediaPlayer::Error,const QString& error){
            if(generation!=generation_)return;
            error_=error;if(failed)failed(error);
        });
        player->setSource(QUrl::fromLocalFile(file));player->play();return true;
    }
    void stop()override{
        ++generation_;
        if(player_){auto* previous=player_.data();position_=previous->position();player_=nullptr;
            previous->disconnect(this);previous->stop();previous->setAudioOutput(nullptr);previous->setAudioBufferOutput(nullptr);previous->deleteLater();}
    }
    bool pause(bool value)override{
        if(!player_||ended_)return false;
        desiredPaused_=value;if(value)player_->pause();else player_->play();return true;
    }
    bool seek(qint64 milliseconds)override{if(!player_||ended_)return false;pendingSeek_=false;offset_=milliseconds;player_->setPosition(milliseconds);return true;}
    qint64 positionMs()const override{return player_?player_->position():position_;}
    void setVolume(std::uint16_t value)override{level_=value;output_.setVolume(float(value)/65535.0f);}
    QJsonObject report()const override{return {{"file",file_},{"accepted",accepted_},{"completed",ended_},{"error",error_},
        {"audio_frames",qint64(frames_)},{"audio_bytes",qint64(bytes_)},{"pcm_s16le_stereo_44100_sha256",QString::fromLatin1(hash_.result().toHex())},
        {"audible_output_requested",audible_},{"audio_device_available",!device_.isNull()},{"volume",int(level_)}};}
private:
    bool audible_,accepted_=false,ended_=false,desiredPaused_=false,pendingSeek_=false;
    std::uint16_t level_=65535;std::uint64_t generation_=0,frames_=0,bytes_=0;
    qint64 position_=0,offset_=0;
    QString file_,error_;
    QPointer<QMediaPlayer> player_;QMediaDevices devices_;QAudioOutput output_;QAudioDevice device_;
    mutable QCryptographicHash hash_{QCryptographicHash::Sha256};
};
}
std::unique_ptr<MusicOutput> makeQtMusicOutput(bool audible){return std::make_unique<QtMusicOutput>(audible);}
}
