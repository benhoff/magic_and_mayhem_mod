#include "../audio/music_service.hpp"
#include "../audio/music_bridge.hpp"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonDocument>
#include <QSaveFile>
#include <QTimer>
#include <iostream>
#include <stdexcept>
int main(int argc,char** argv) try {
    QCoreApplication app(argc,argv);
    if(argc!=5)throw std::runtime_error("Expected catalog, status, command, final report");
    mnm::audio::MusicBroker startup;
    if(!startup.create(QString::fromLocal8Bit(argv[2])+".channel",QString::fromLocal8Bit(argv[1]),true))
        throw std::runtime_error(startup.lastError().toStdString());
    startup.stop();
    mnm::audio::MusicService music(mnm::audio::makeQtMusicOutput(true));QTimer timer;timer.setInterval(20);
    QElapsedTimer clock;clock.start();quint64 last=0;QString failure;
    auto report=[&]{const auto s=music.snapshot();auto r=music.outputReport();
        r.insert("phase",int(s.phase));r.insert("track",int(s.track));r.insert("loop",s.loop);
        r.insert("position_ms",s.positionMs);r.insert("elapsed_ms",clock.elapsed());
        r.insert("losses",qint64(s.losses));r.insert("recoveries",qint64(s.recoveries));
        r.insert("completions",qint64(s.completions));r.insert("level",s.volume);
        r.insert("command_ack",qint64(last));r.insert("fixture_error",failure);return r;};
    auto save=[](const QString& path,const QJsonObject& object){QSaveFile f(path);
        return f.open(QIODevice::WriteOnly)&&f.write(QJsonDocument(object).toJson())>0&&f.commit();};
    auto finish=[&]{timer.stop();auto r=report();music.close();r.insert("closed",music.snapshot().phase==mnm::audio::MusicPhase::closed);
        r.insert("success",failure.isEmpty());bool ok=save(QString::fromLocal8Bit(argv[4]),r);app.exit(ok&&failure.isEmpty()?0:1);};
    QObject::connect(&timer,&QTimer::timeout,&app,[&]{
        if(clock.elapsed()>25000){failure="Music backend fixture timeout";finish();return;}
        QFile f(QString::fromLocal8Bit(argv[3]));
        if(f.open(QIODevice::ReadOnly)){
            const auto cmd=QJsonDocument::fromJson(f.read(4097)).object();const auto serial=quint64(cmd.value("serial").toInteger());
            if(serial>last){last=serial;const auto operation=cmd.value("operation").toString();bool ok=false;
                if(operation=="pause")ok=music.pause();else if(operation=="resume")ok=music.resume();
                else if(operation=="seek")ok=music.seek(cmd.value("milliseconds").toInteger());
                else if(operation=="finish"){finish();return;}
                if(!ok){failure="Music control refused: "+operation;finish();return;}
            }
        }
        if(!save(QString::fromLocal8Bit(argv[2]),report())){failure="Cannot save music status";finish();}
    });
    if(!music.initialize(QString::fromLocal8Bit(argv[1]).toStdString())||!music.volume(12345)||!music.play(2,true))
        throw std::runtime_error("Music backend startup refused");
    timer.start();return app.exec();
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
