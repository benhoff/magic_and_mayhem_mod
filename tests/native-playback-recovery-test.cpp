#include "../apps/qt-shell/native_playback.hpp"
#include <QGuiApplication>
#include <QMediaDevices>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QSaveFile>
#include <iostream>
#include <stdexcept>

class Devices final : public PlaybackAudioDevices {
public:
    QAudioDevice current;
    QAudioDevice defaultOutput() const override { return current; }
    void publish(QAudioDevice device) { current=device;if(changed)changed(); }
};

int main(int argc,char** argv) try {
    QGuiApplication app(argc,argv);
    if(argc!=5)throw std::runtime_error("Expected mode, movie/sound, asset, report");
    const QString mode=QString::fromLocal8Bit(argv[1]);
    const auto real=QMediaDevices::defaultAudioOutput();
    if(real.isNull())throw std::runtime_error("Fixture requires an actual Qt output; endpoint topology is injected");
    auto devices=std::make_unique<Devices>();auto* topology=devices.get();
    NativePlayback playback(QString::fromLocal8Bit(argv[2])=="movie",true,std::move(devices));
    playback.setVolume(0); // Decode real PCM without emitting the test signal.
    QElapsedTimer clock;clock.start();QTimer poll;poll.setInterval(10);
    QString failure;int phase=0;qint64 phaseAt=0,frozen=0;unsigned accepted=0;
    bool initialWait=false,retryRecovered=false,lossPaused=false,callerPauseRetained=false;
    auto fail=[&](const QString& reason){failure=reason;playback.stop(MNM_MEDIA_V1_STATUS_DECODER_ERROR);};
    playback.accepted=[&]{++accepted;};
    playback.finished=[&](unsigned result){
        poll.stop();auto report=playback.report();
        bool success=failure.isEmpty() && result==MNM_MEDIA_V1_STATUS_COMPLETE && phase==5 && accepted==1 &&
            initialWait && retryRecovered && lossPaused && callerPauseRetained &&
            report.value("output_losses").toInt()==1 && report.value("output_recoveries").toInt()==2 &&
            report.value("default_output_changes").toInt()==0;
        if(mode=="cancel")success=failure.isEmpty() && result==MNM_MEDIA_V1_STATUS_CANCELLED && initialWait;
        if(mode=="explicit")success=failure.isEmpty() && result==MNM_MEDIA_V1_STATUS_COMPLETE && accepted==1 &&
            report.value("output_losses").toInt()==0 && report.value("output_recoveries").toInt()==0;
        report.insert("success",success);report.insert("failure",failure);report.insert("accepted_callbacks",int(accepted));
        report.insert("initial_absence_waited",initialWait);report.insert("timer_rescan_recovered",retryRecovered);
        report.insert("loss_paused_position",lossPaused);report.insert("caller_pause_retained",callerPauseRetained);
        QSaveFile output(QString::fromLocal8Bit(argv[4]));
        const bool saved=output.open(QIODevice::WriteOnly) && output.write(QJsonDocument(report).toJson())>0 && output.commit();
        app.exit(success && saved?0:1);
    };
    QObject::connect(&poll,&QTimer::timeout,&app,[&]{
        if(!playback.active())return;
        const auto report=playback.report();const auto position=report.value("position_ms").toInteger();
        const auto elapsed=clock.elapsed();
        if(elapsed>15000){fail("Recovery fixture timed out");return;}
        if(mode=="explicit"){
            if(elapsed>200 && phase==0){topology->publish({});phase=1;}
            return;
        }
        if(phase==0 && elapsed>=200){
            initialWait=report.value("waiting_for_audio_output").toBool() && position==0 &&
                report.value("audio_frames").toInteger()==0 && report.value("video_frames").toInt()==0;
            if(!initialWait){fail("Initial absence advanced playback");return;}
            if(mode=="cancel"){playback.stop();topology->publish(real);return;}
            // Deliberately omit notification: the 500 ms rescan must recover.
            topology->current=real;phase=1;
        }else if(phase==1 && position>=300){
            retryRecovered=report.value("output_recoveries").toInt()==1;
            topology->publish({});topology->publish({});phase=2;phaseAt=elapsed;
        }else if(phase==2 && elapsed-phaseAt>=100){
            if(!report.value("waiting_for_audio_output").toBool()){fail("Loss did not suspend playback");return;}
            frozen=position;phase=3;phaseAt=elapsed;
        }else if(phase==3 && elapsed-phaseAt>=200){
            lossPaused=position==frozen;
            if(!lossPaused){fail("Unavailable output advanced media position");return;}
            playback.pause(true);topology->publish(real);phase=4;phaseAt=elapsed;
        }else if(phase==4 && elapsed-phaseAt>=200){
            callerPauseRetained=report.value("caller_paused").toBool() && !report.value("waiting_for_audio_output").toBool() && position==frozen;
            if(!callerPauseRetained){fail("Recovery erased caller pause");return;}
            topology->publish(real);playback.pause(false);phase=5;
        }
    });
    if(mode=="explicit")playback.setAudioDevice(real);
    playback.start(QString::fromLocal8Bit(argv[3]));poll.start();return app.exec();
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
