#include "../apps/qt-shell/native_playback.hpp"
#include <QGuiApplication>
#include <QFile>
#include <QSaveFile>
#include <QJsonDocument>
#include <QElapsedTimer>
#include <iostream>
#include <stdexcept>

int main(int argc,char** argv) try {
    QGuiApplication app(argc,argv);
    if(argc!=6)throw std::runtime_error("Expected movie/sound, asset, status, command, final report");
    NativePlayback playback(QString::fromLocal8Bit(argv[1])=="movie",true);
    playback.setVolume(0);
    QElapsedTimer clock;clock.start();QTimer poll;poll.setInterval(20);
    unsigned accepted=0,lastCommand=0;QString failure;
    auto snapshot=[&]{auto report=playback.report();report.insert("active",playback.active());
        report.insert("command_ack",int(lastCommand));report.insert("accepted_callbacks",int(accepted));
        report.insert("elapsed_ms",clock.elapsed());report.insert("fixture_error",failure);return report;};
    auto save=[](const QString& path,const QJsonObject& object){
        QSaveFile output(path);return output.open(QIODevice::WriteOnly) &&
            output.write(QJsonDocument(object).toJson())>0 && output.commit();
    };
    playback.accepted=[&]{++accepted;};
    playback.finished=[&](unsigned result){
        poll.stop();auto report=snapshot();const bool success=failure.isEmpty() && result==MNM_MEDIA_V1_STATUS_COMPLETE && accepted==1;
        report.insert("success",success);
        const bool saved=save(QString::fromLocal8Bit(argv[5]),report);
        save(QString::fromLocal8Bit(argv[3]),report);app.exit(success && saved?0:1);
    };
    QObject::connect(&poll,&QTimer::timeout,&app,[&]{
        if(clock.elapsed()>20000){failure="Backend fixture timed out";playback.stop(MNM_MEDIA_V1_STATUS_DECODER_ERROR);return;}
        QFile command(QString::fromLocal8Bit(argv[4]));
        if(command.exists() && command.open(QIODevice::ReadOnly)){
            const auto document=QJsonDocument::fromJson(command.read(4097));const auto request=document.object();
            const auto sequence=request.value("sequence").toInt();
            if(request.value("schema").toInt()!=1 || sequence<1 || request.value("operation").toString()!="pause" || !request.value("value").isBool()){
                failure="Invalid test-only version1 playback command";playback.stop(MNM_MEDIA_V1_STATUS_DECODER_ERROR);return;
            }
            if(unsigned(sequence)>lastCommand){lastCommand=unsigned(sequence);playback.pause(request.value("value").toBool());}
        }
        if(!save(QString::fromLocal8Bit(argv[3]),snapshot())){
            failure="Cannot save backend progress";playback.stop(MNM_MEDIA_V1_STATUS_DECODER_ERROR);
        }
    });
    playback.start(QString::fromLocal8Bit(argv[2]));poll.start();return app.exec();
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
