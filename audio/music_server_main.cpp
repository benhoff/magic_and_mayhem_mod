#include "music_bridge.hpp"
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QTimer>
#include <cstdio>
int main(int argc,char** argv){
    QCoreApplication app(argc,argv);QCommandLineParser parser;parser.addHelpOption();
    parser.addOption({"channel","Owned music channel","path"});parser.addOption({"catalog","Prepared music root","path"});
    parser.addOption({"silent","Decode without audible output"});parser.addOption({"seconds","Bound server lifetime","seconds","30"});parser.addOption({"report","New output JSON report","path"});parser.process(app);
    bool valid=false;const auto seconds=parser.value("seconds").toInt(&valid);
    if(!valid||seconds<1||seconds>600||parser.value("channel").isEmpty()||parser.value("catalog").isEmpty())parser.showHelp(2);
    mnm::audio::MusicBroker broker;if(!broker.create(parser.value("channel"),parser.value("catalog"),!parser.isSet("silent"))){std::fprintf(stderr,"%s\n",qPrintable(broker.lastError()));return 2;}
    QTimer::singleShot(seconds*1000,&app,&QCoreApplication::quit);const auto result=app.exec();const auto before=broker.activity();broker.stop();
    if(parser.isSet("report")){
        auto report=before;report["ready_after_stop"]=broker.activity().value("ready");report["success"]=true;
        QFile output(parser.value("report"));if(!output.open(QIODevice::WriteOnly|QIODevice::NewOnly)||output.write(QJsonDocument(report).toJson())<0)return 3;
    }
    return result;
}
