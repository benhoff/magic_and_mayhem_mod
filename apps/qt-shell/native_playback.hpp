#pragma once
#include "../../protocols/include/mnm/media_v1.h"
#include "playback_audio_devices.hpp"
#include <QAudioBufferOutput>
#include <QAudioOutput>
#include <QCryptographicHash>
#include <QImage>
#include <QJsonObject>
#include <QJsonArray>
#include <QMediaPlayer>
#include <QVideoSink>
#include <QTimer>
#include <functional>
class QAudioDevice;

// One finite movie or WinMM-style file-sound channel, owned by the Qt thread.
class NativePlayback final:public QObject {
public:
    NativePlayback(bool movie,bool audible,
                   std::unique_ptr<PlaybackAudioDevices> devices=makePlaybackAudioDevices(),
                   bool allowInitialSilence=false);
    ~NativePlayback() override;
    void start(const QString& path,bool loop=false,bool presentVideo=true);
    void stop(unsigned result=MNM_MEDIA_V1_STATUS_CANCELLED);
    void pause(bool value);
    void setVolume(float volume);
    void setAudioDevice(const QAudioDevice& device);
    bool active() const{return !finished_;}
    QJsonObject report() const;
    std::function<void(QImage)> frame;
    std::function<void()> accepted;
    std::function<void(unsigned)> finished;
private:
    void finish(unsigned result);
    void refreshOutput();
    bool movie_,audible_,finished_=true,accepted_=false,presentVideo_=true;
    unsigned presentedFrames_=0;
    QJsonArray firstColors_;int pixelFormat_=0;QString path_,error_;unsigned result_=0,videoFrames_=0;
    qint64 audioFrames_=0,audioBytes_=0;QSize videoSize_;
    QMediaPlayer player_;QAudioOutput output_;QVideoSink video_;
    QAudioBufferOutput audio_;
    QCryptographicHash audioHash_{QCryptographicHash::Sha256},videoHash_{QCryptographicHash::Sha256};
    QByteArray firstFrameHash_;
    std::unique_ptr<PlaybackAudioDevices> devices_;
    QTimer outputRefresh_,outputRetry_;
    bool followDefault_=true,waitingOutput_=false,requestedPaused_=false;
    bool allowInitialSilence_=false,initialSilent_=false;
    unsigned outputLosses_=0,outputRecoveries_=0,outputChanges_=0;
};
