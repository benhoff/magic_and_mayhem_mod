#include "media_cli.hpp"
#include "media_broker.hpp"
#include <QFileInfo>
#include <QJsonDocument>
#include <QKeyEvent>
#include <QSaveFile>
#include <cstdio>

void addMediaOptions(QCommandLineParser& p){
    p.addOption({"native-media","Opt in to Qt movie and supported file-sound playback."});
    p.addOption({"media","Preview an AVI movie or WAV sound without the game.","file"});
    p.addOption({"media-test","Decode media without audio output; write measured frames and PCM."});
    p.addOption({"media-probe","Stop a decoder test after its first video frame and audio buffer."});
    p.addOption({"media-report","Write playback evidence to this JSON file.","file"});
    p.addOption({"media-server-test","Serve synthetic media requests without the game or audio output.","channel"});
    p.addOption({"assets","Trusted media asset root for the synthetic server.","directory"});
    p.addOption({"media-count","Exit the synthetic server after this many recorded results.","count","1"});
}
namespace {
class PlaybackControls final:public QObject {
public:
    explicit PlaybackControls(NativePlayback& playback):playback_(playback){}
protected:
    bool eventFilter(QObject*,QEvent* event) override{
        if(event->type()!=QEvent::KeyPress)return false;
        const auto key=static_cast<QKeyEvent*>(event)->key();
        if(key==Qt::Key_Escape)playback_.stop(MNM_MEDIA_V1_STATUS_CANCELLED);
        else if(key==Qt::Key_Space){paused_=!paused_;playback_.pause(paused_);}
        else if(key==Qt::Key_Plus || key==Qt::Key_Equal){volume_=qMin(1.0f,volume_+0.1f);playback_.setVolume(volume_);}
        else if(key==Qt::Key_Minus){volume_=qMax(0.0f,volume_-0.1f);playback_.setVolume(volume_);}
        else return false;
        event->accept();return true;
    }
private:
    NativePlayback& playback_;bool paused_=false;float volume_=1.0f;
};
bool save(const QString& path,const QJsonDocument& report){
    if(path.isEmpty()){std::puts(report.toJson().constData());return true;}
    QSaveFile file(path);return file.open(QIODevice::WriteOnly) && file.write(report.toJson())>0 && file.commit();
}
}
int runMedia(QApplication& app,const QCommandLineParser& p){
    GlViewport viewport;viewport.resize(640,480);viewport.setWindowTitle("Magic & Mayhem — native media");viewport.show();
    const auto report=p.value("media-report");
    if(p.isSet("media-server-test")){
        bool ok=false;const int count=p.value("media-count").toInt(&ok);
        if(!ok || count<1 || count>128 || !p.isSet("assets"))return 2;
        MediaBroker broker(viewport,p.value("assets"),false);broker.skipFixture=true;
        broker.frame=[&](QImage image){viewport.setFrame(std::move(image));};
        broker.recorded=[&]{if(broker.history().size()>=count){
            const bool written=save(report,QJsonDocument(broker.history()));app.exit(written?0:8);
        }};
        if(!broker.create(p.value("media-server-test")))return 2;
        QTimer::singleShot(20000,&app,[&]{save(report,QJsonDocument(broker.history()));app.exit(9);});
        return app.exec();
    }
    const bool movie=QFileInfo(p.value("media")).suffix().compare("avi",Qt::CaseInsensitive)==0;
    NativePlayback playback(movie,!p.isSet("media-test"));
    PlaybackControls controls(playback);viewport.installEventFilter(&controls);viewport.setFocus();
    if(!p.isSet("media-test"))viewport.setWindowTitle("Native media — Space: pause, Escape: stop, +/-: volume");
    QTimer probe;probe.setInterval(25);
    QObject::connect(&probe,&QTimer::timeout,&app,[&]{
        const auto evidence=playback.report();
        if(evidence.value("video_frames").toInt()>0 && evidence.value("audio_frames").toInteger()>0)playback.stop(MNM_MEDIA_V1_STATUS_CANCELLED);
    });
    playback.frame=[&](QImage image){viewport.setFrame(std::move(image));};
    playback.finished=[&](unsigned result){
        const bool written=save(report,QJsonDocument(playback.report()));
        if(p.isSet("media-test"))app.exit(written && (result==MNM_MEDIA_V1_STATUS_COMPLETE || (p.isSet("media-probe") && result==MNM_MEDIA_V1_STATUS_CANCELLED))?0:8);
    };
    if(p.isSet("media-probe"))probe.start();
    QTimer::singleShot(0,&app,[&]{playback.start(QFileInfo(p.value("media")).absoluteFilePath());});
    if(p.isSet("media-test"))QTimer::singleShot(15000,&app,[&]{playback.stop(MNM_MEDIA_V1_STATUS_DECODER_ERROR);app.exit(9);});
    return app.exec();
}
