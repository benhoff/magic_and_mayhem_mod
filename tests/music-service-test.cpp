#include "music_service.hpp"
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonDocument>
#include <QThread>
#include <QTimer>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace mnm::audio;
static void check(bool ok,const char* reason){if(!ok)throw std::runtime_error(reason);}
struct FakeOutput final:MusicOutput {
    unsigned starts=0,stops=0;bool available_=true,failStart=false,paused=false,loop=false;
    qint64 position=0,offset=0;std::uint16_t volume=0;
    bool start(const QString&,bool repeat,qint64 at,bool hold,QString& error)override{
        ++starts;loop=repeat;offset=position=at;paused=hold;if(failStart){error="Fixture decoder refusal";return false;}return true;
    }
    void stop()override{++stops;}
    bool pause(bool value)override{paused=value;return true;}
    bool seek(qint64 value)override{position=value;return true;}
    qint64 positionMs()const override{return position;}
    bool available()const override{return available_;}
    void setVolume(std::uint16_t value)override{volume=value;}
    QJsonObject report()const override{return {};}
};
static void events(){QCoreApplication::processEvents();QCoreApplication::processEvents();}
static void fake(const std::filesystem::path& root){
    auto backend=std::make_unique<FakeOutput>();auto* output=backend.get();MusicService service(std::move(backend));
    check(service.initialize(root),"catalog initialize");check(service.catalog()->tracks().size()==3,"catalog audio/data spans");
    check(service.volume(0)&&output->volume==0,"mute before decoder activity");
    check(!service.play(1)&&!service.play(0)&&!service.play(4)&&!service.play(2,false,-1)&&!service.play(2,false,std::numeric_limits<qint64>::max())&&!output->starts,"invalid track/seek admission");
    check(service.volume(4369)&&service.play(2,true,120),"loop launch");check(output->volume==4369&&output->offset==120&&output->loop,"volume/offset before playback");
    const auto oldReady=output->ready,oldComplete=output->complete;output->ready();events();check(service.snapshot().phase==MusicPhase::playing,"decoder ready");
    check(service.pause()&&output->paused&&service.snapshot().phase==MusicPhase::paused,"pause");check(service.seek(250)&&output->position==250,"seek");check(!service.seek(1000),"seek beyond track refused");
    check(service.resume()&&!output->paused,"resume");output->position=350;output->available_=false;output->lost("Fixture unplug");
    events();
    check(service.snapshot().phase==MusicPhase::unavailable&&service.snapshot().losses==1,"output loss");
    check(!service.play(3)&&output->starts==1,"unavailable play refused without replay context mutation");
    check(service.pause(),"pause intent during loss");oldReady();oldComplete();events();check(service.snapshot().phase==MusicPhase::unavailable&&!service.snapshot().completions,"stale callbacks ignored");
    output->available_=true;output->outputAvailable();output->outputAvailable();events();
    check(output->starts==2&&output->offset==350&&output->paused&&output->loop&&output->volume==4369&&service.snapshot().recoveries==1,"one recovery preserves position/pause/loop/volume");
    output->ready();events();check(service.snapshot().phase==MusicPhase::paused,"paused recovery ready");
    check(service.resume(),"recovered resume");const auto cancelled=output->complete;service.stop();cancelled();events();check(service.snapshot().phase==MusicPhase::stopped&&!service.snapshot().completions,"stop cancels queued completion");
    check(service.play(2,true),"end-boundary recovery launch");output->ready();events();output->position=1000;output->available_=false;output->lost("Fixture end-boundary unplug");events();
    check(service.snapshot().phase==MusicPhase::unavailable&&service.snapshot().positionMs==999,"end-boundary position remains playable");
    output->available_=true;output->outputAvailable();events();check(output->offset==999&&service.snapshot().recoveries==2,"end-boundary recovery resumes without replaying full track");service.stop();
    check(service.play(3),"second track");output->ready();events();const auto completed=output->complete;output->position=1000;completed();completed();events();
    check(service.snapshot().phase==MusicPhase::completed&&service.snapshot().completions==1,"single completion");
    check(service.play(2),"new generation after completion");const auto staleFailure=output->failed;service.stop();staleFailure("Late decoder failure");events();check(service.snapshot().phase==MusicPhase::stopped,"late failure ignored");
    output->failStart=true;check(!service.play(2)&&service.snapshot().phase==MusicPhase::failed,"start refusal");output->available_=false;const auto attempts=output->starts;output->available_=true;events();check(output->starts==attempts,"failed start never replayed by availability");output->failStart=false;
    service.changed=[&](const auto& snapshot){if(snapshot.phase==MusicPhase::loading)service.close();};
    check(!service.play(2)&&service.snapshot().phase==MusicPhase::closed&&!service.catalog(),"reentrant close cancels loading admission");service.changed={};
    check(service.initialize(root)&&service.snapshot().volume==4369,"reinitialize keeps volume");service.close();
}
static QJsonObject decode(const std::filesystem::path& root){
    MusicService service(makeQtMusicOutput(false));check(service.initialize(root),"Qt catalog initialize");
    check(service.play(2),"Qt track admission");QElapsedTimer clock;clock.start();
    while(service.snapshot().phase!=MusicPhase::completed&&service.snapshot().phase!=MusicPhase::failed&&clock.elapsed()<5000){events();QThread::msleep(2);}
    check(service.snapshot().phase==MusicPhase::completed,"Qt track completion");const auto complete=service.outputReport();check(complete.value("accepted").toBool()&&complete.value("audio_frames").toInteger()==44100,"full Qt PCM frame count");
    check(service.play(3,true),"Qt loop admission");clock.restart();
    while(service.outputReport().value("audio_frames").toInteger()<50000&&clock.elapsed()<5000){events();QThread::msleep(2);}
    check(service.snapshot().phase==MusicPhase::playing&&service.outputReport().value("audio_frames").toInteger()>=50000,"Qt looping across track boundary");
    check(service.pause(),"Qt pause");const auto position=service.snapshot().positionMs;QThread::msleep(20);events();check(service.snapshot().phase==MusicPhase::paused&&service.snapshot().positionMs==position,"paused position stable");
    check(service.seek(500)&&service.resume(),"Qt seek/resume");service.stop();events();check(service.snapshot().phase==MusicPhase::stopped,"Qt stop");service.close();events();return complete;
}
int main(int argc,char** argv)try{
    QCoreApplication app(argc,argv);check(argc==3,"arguments: catalog output.json");const std::filesystem::path root=argv[1];
    if(QString::fromLocal8Bit(argv[2])=="--catalog-check"){
        bool valid=true;QString error;try{MusicCatalog::load(root);}catch(const std::exception& failure){valid=false;error=QString::fromUtf8(failure.what());}
        std::cout<<QJsonDocument(QJsonObject{{"valid",valid},{"error",error}}).toJson(QJsonDocument::Compact).constData()<<'\n';return 0;
    }
    fake(root);const auto pcm=decode(root);
    QFile output(QString::fromLocal8Bit(argv[2]));check(output.open(QIODevice::WriteOnly|QIODevice::NewOnly),"report open");
    output.write(QJsonDocument(QJsonObject{{"success",true},{"fake_lifecycle",true},{"fake_device_recovery",true},{"stale_callbacks",true},{"cancelled_start",true},{"qt_decode",pcm}}).toJson());
    std::cout<<"Native music catalog/controller/decoder checks passed\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
