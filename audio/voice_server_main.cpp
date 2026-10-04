#include "voice_bridge.hpp"
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTimer>
#include <iostream>
using namespace mnm::audio;
int main(int argc,char** argv){
    QCoreApplication app(argc,argv);QCommandLineParser parser;parser.addHelpOption();
    parser.addOption({"channel","New native voice channel path","file"});
    parser.addOption({"silent","Fixture-only mode; no output device or advancement"});
    parser.addOption({"seconds","Bounded server lifetime (1..30 seconds)","seconds","10"});parser.process(app);
    bool valid=false;const auto seconds=parser.value("seconds").toInt(&valid);
    if(!parser.isSet("channel") || !valid || seconds<1 || seconds>30)parser.showHelp(2);
    VoiceBroker broker;broker.failed=[&](const QString& error){std::cerr<<error.toStdString()<<'\n';app.exit(1);};
    if(!broker.create(parser.value("channel"),!parser.isSet("silent"))){std::cerr<<broker.lastError().toStdString()<<'\n';return 1;}
    std::cout<<"Native voice broker ready\n"<<std::flush;
    QTimer::singleShot(seconds*1000,&app,[&]{broker.stop();app.quit();});return app.exec();
}
