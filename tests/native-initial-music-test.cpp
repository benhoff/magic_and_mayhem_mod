#include "../audio/music_service.hpp"
#include "../audio/music_bridge.hpp"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QSaveFile>
#include <QTimer>
#include <iostream>
#include <stdexcept>

int main(int argc,char** argv) try {
    QCoreApplication app(argc,argv);
    if(argc!=4)throw std::runtime_error("Expected catalog, progress, final report");
    mnm::audio::MusicBroker startup;
    if(!startup.create(QString::fromLocal8Bit(argv[2])+".channel",QString::fromLocal8Bit(argv[1]),true))
        throw std::runtime_error(startup.lastError().toStdString());
    startup.stop();
    mnm::audio::MusicService music(mnm::audio::makeQtMusicOutput(true));QTimer timer;timer.setInterval(20);
    QElapsedTimer clock;clock.start();
    auto snapshot=[&]{const auto state=music.snapshot();auto report=music.outputReport();
        report.insert("phase",int(state.phase));report.insert("position_ms",state.positionMs);
        report.insert("elapsed_ms",clock.elapsed());
        report.insert("completions",qint64(state.completions));report.insert("service_error",state.error);return report;};
    auto save=[](const QString& path,const QJsonObject& object){QSaveFile output(path);
        return output.open(QIODevice::WriteOnly) && output.write(QJsonDocument(object).toJson())>0 && output.commit();};
    QObject::connect(&timer,&QTimer::timeout,&app,[&]{
        auto report=snapshot();const auto state=music.snapshot();
        if(!save(QString::fromLocal8Bit(argv[2]),report))app.exit(1);
        if(state.phase==mnm::audio::MusicPhase::completed || state.phase==mnm::audio::MusicPhase::failed || clock.elapsed()>10000){
            timer.stop();const bool success=state.phase==mnm::audio::MusicPhase::completed && state.completions==1 && state.error.isEmpty();
            report.insert("success",success);const bool saved=save(QString::fromLocal8Bit(argv[3]),report);music.close();app.exit(success && saved?0:1);
        }
    });
    if(!music.initialize(QString::fromLocal8Bit(argv[1]).toStdString()) || !music.volume(0) || !music.play(2))
        throw std::runtime_error("Native music silent startup refused");
    timer.start();return app.exec();
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
