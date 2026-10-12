#include "../audio/voice_bridge.hpp"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QTimer>
#include <QtEndian>
#include <iostream>
#include <stdexcept>

int main(int argc,char** argv) try {
    QCoreApplication app(argc,argv);
    if(argc!=6)throw std::runtime_error("Expected channel, status, command, final report, sounds root");
    mnm::audio::VoiceBroker broker;QString failure;QTimer timer;timer.setInterval(20);
    QElapsedTimer clock;clock.start();
    auto report=[&]{const auto a=broker.activity();return QJsonObject{
        {"requests",qint64(a.requests)},{"failures",qint64(a.failures)},
        {"wave_loads",qint64(a.waveLoads)},{"wave_bytes",qint64(a.waveBytes)},
        {"output_available",a.outputAvailable},{"output_silent",a.outputSilent},{"initial_silence",a.initialSilence},
        {"output_losses",qint64(a.outputLosses)},
        {"output_recoveries",qint64(a.outputRecoveries)},{"output_attempts",qint64(a.outputAttempts)},
        {"generated_frames",qint64(a.generatedFrames)},{"consumed_frames",qint64(a.consumedFrames)},
        {"ready",a.ready},{"error",broker.lastError()},{"fixture_error",failure},
        {"elapsed_ms",clock.elapsed()}};};
    auto save=[](const QString& path,const QJsonObject& object){QSaveFile file(path);
        return file.open(QIODevice::WriteOnly) && file.write(QJsonDocument(object).toJson())>0 && file.commit();};
    auto finish=[&]{timer.stop();auto final=report();broker.stop();QFile channel(QString::fromLocal8Bit(argv[1]));
        bool retired=false;if(channel.open(QIODevice::ReadOnly)){const auto header=channel.read(128);
            retired=header.size()==128 && qFromLittleEndian<quint32>(header.constData()+80)==0;}
        final.insert("ready_after_stop",!retired);final.insert("success",failure.isEmpty() && retired);
        const bool saved=save(QString::fromLocal8Bit(argv[4]),final);app.exit(saved && failure.isEmpty() && retired?0:1);};
    broker.failed=[&](const QString& error){failure=error;finish();};
    if(!broker.create(QString::fromLocal8Bit(argv[1]),true,QString::fromLocal8Bit(argv[5])))
        throw std::runtime_error(broker.lastError().toStdString());
    QObject::connect(&timer,&QTimer::timeout,&app,[&]{
        if(clock.elapsed()>25000){failure="Voice backend fixture timeout";finish();return;}
        QFile command(QString::fromLocal8Bit(argv[3]));
        if(command.exists() && command.open(QIODevice::ReadOnly)){
            const auto object=QJsonDocument::fromJson(command.read(4097)).object();
            if(object.value("schema").toInt()!=1 || object.value("operation").toString()!="stop")failure="Invalid test-only stop command";
            finish();return;
        }
        if(!save(QString::fromLocal8Bit(argv[2]),report())){failure="Cannot save voice progress";finish();}
    });
    timer.start();return app.exec();
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
