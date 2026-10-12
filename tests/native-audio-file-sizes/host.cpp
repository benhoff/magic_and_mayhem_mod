#include "../../audio/voice_bridge.hpp"
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>
#include <QTimer>
#include <iostream>
int main(int argc,char** argv){
    QCoreApplication app(argc,argv);if(argc!=5)return 2;
    mnm::audio::VoiceBroker broker;
    if(!broker.create(QString::fromLocal8Bit(argv[1]),false,QString::fromLocal8Bit(argv[2]),false,true)){
        std::cerr<<broker.lastError().toStdString()<<'\n';return 3;
    }
    QTimer timeout;timeout.setSingleShot(true);QTimer poll;poll.setInterval(20);
    QObject::connect(&timeout,&QTimer::timeout,&app,[&]{app.exit(4);});
    QObject::connect(&poll,&QTimer::timeout,&app,[&]{if(QFile::exists(QString::fromLocal8Bit(argv[3]))){
        const auto a=broker.activity();broker.stop();QSaveFile f(QString::fromLocal8Bit(argv[4]));
        const QJsonObject r{{"requests",qint64(a.requests)},{"failures",qint64(a.failures)},
            {"file_size_queries",qint64(a.fileSizeQueries)},{"ready_after_stop",broker.activity().ready}};
        const auto bytes=QJsonDocument(r).toJson();const bool ok=f.open(QIODevice::WriteOnly)&&f.write(bytes)==bytes.size()&&f.commit();
        app.exit(ok?0:5);
    }});
    timeout.start(90000);poll.start();return app.exec();
}
