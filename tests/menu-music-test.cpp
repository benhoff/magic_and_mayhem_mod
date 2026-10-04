#include "menu_music_controller.hpp"
#include "menu_audio_controller.hpp"
#include "menu_preview.hpp"
#include "main_menu_widget.hpp"
#include "menu-audio-fixture.hpp"
#include <QApplication>
#include <QFile>
#include <QLabel>
#include <QProcess>
#include <QPushButton>
#include <QSettings>
#include <QSlider>
#include <cmath>
#include <iostream>
using audioFixture::check;
namespace {
class Music final:public MenuMusicOutput {
public:
    bool active=false,loop=false,refuse=false,synchronousReady=false,synchronousFailure=false;
    int starts=0,stops=0;float volume=1,startVolume=1;QString file;
    bool start(const QString& path,bool repeat,QString& error) override{
        ++starts;file=path;loop=repeat;startVolume=volume;active=!refuse;
        if(synchronousReady && ready)ready();
        if(synchronousFailure && failed)failed("Decoder failed during start");
        if(refuse)error="Synthetic music startup failure";
        return !refuse;
    }
    void stop() override{++stops;active=false;}
    void setVolume(float value) override{volume=value;}
};
class Effects final:public AudioSessionOutput {
public:
    bool active=false;
    std::uint32_t rate(QString&) override{return 48000;}
    bool start(mnm::audio::Device&,QString&) override{active=true;return true;}
    void stop() override{active=false;}
    bool running() const override{return active;}
    QString error() const override{return {};}
};
void click(QWidget& widget,const char* name){auto* button=widget.findChild<QPushButton*>(name);check(button,"button exists");button->click();}
void events(){for(int i=0;i<3;++i)QCoreApplication::processEvents();}
bool near(float actual,float expected){return std::abs(actual-expected)<0.000001f;}
QByteArray bytes(const QString& file){QFile input(file);check(input.open(QIODevice::ReadOnly),"settings readable");return input.readAll();}
}
int main(int argc,char** argv){QApplication app(argc,argv);try{
    if(argc==4 && QString::fromLocal8Bit(argv[1])=="--restore"){
        const auto root=QString::fromLocal8Bit(argv[2]);const int level=QString::fromLocal8Bit(argv[3]).toInt();
        QSettings store(root+"/music.ini",QSettings::IniFormat);
        auto output=std::make_unique<Music>();auto* backend=output.get();MenuMusicController controller(std::move(output),&store);
        MenuPreview preview;QString error;check(preview.loadAssets(root,false,&error),qPrintable(error));controller.attach(preview);
        check(controller.start(root+"/Sounds/Click.wav") && controller.musicLevel()==level,"fresh process restores music level");
        check(near(backend->startVolume,float(std::pow(10.0,double(level)/2000.0))),"fresh process gain precedes decoder start");
        click(preview,"mainMenuAction3");check(preview.findChild<PreferencesWidget*>()->settings().musicLevel==level,"fresh process seeds lazy Preferences");preview.close();return 0;
    }
    QTemporaryDir temporary;check(temporary.isValid(),"fixture directory");const auto root=temporary.path();menuAudioFixture::fixtures(root);
    const auto track=root+"/Sounds/Click.wav",path=root+"/music.ini";
    QSettings store(path,QSettings::IniFormat);
    auto output=std::make_unique<Music>();auto* backend=output.get();MenuMusicController music(std::move(output),&store);
    check(music.musicLevel()== -1500 && !QFile::exists(path),"missing music setting uses default without writing");
    MenuPreview preview;QString error;check(preview.loadAssets(root,false,&error),qPrintable(error));preview.show();events();music.attach(preview);
    MenuAudioController effects(std::make_unique<Effects>(),{},nullptr,&store);effects.attach(preview);
    check(effects.start(root+"/Sounds",mnm::reconstruction::audio::NativeSourcePathPolicy::literal),"shared Preferences effects session starts");
    int failures=0;QString lastFailure;music.failed=[&](const QString& value){++failures;lastFailure=value;};
    check(music.start(track) && music.state()==MenuMusicState::loading && backend->loop && backend->file==track,"explicit local track requests an infinite loop");
    check(near(backend->startVolume,0.17782794f),"default attenuation applied before decoder starts");
    auto* label=preview.findChild<QLabel*>("menuMusicStatus");check(label && label->text().contains("Loading"),"music has its own loading status");
    backend->ready();check(music.state()==MenuMusicState::loading,"decoder notifications deferred out of backend stack");events();
    check(music.state()==MenuMusicState::playing && label->text()=="Music playing","ready notification publishes playback status");
    const int starts=backend->starts,stops=backend->stops;
    click(preview,"mainMenuAction2");click(preview,"quickBattleAction3");
    check(preview.openGrimoire(root,&error),qPrintable(error));click(*preview.findChild<GrimoireWidget*>(),"grimoireNext");click(*preview.findChild<GrimoireWidget*>(),"grimoireClose");
    check(backend->starts==starts && backend->stops==stops && backend->active,"music survives menu/page transitions without replay");
    click(preview,"mainMenuAction3");auto* prefs=preview.findChild<PreferencesWidget*>();
    auto* musicSlider=prefs->findChild<QSlider*>("preferencesSlider1");auto* effectsSlider=prefs->findChild<QSlider*>("preferencesSlider2");
    musicSlider->setValue(-2000);effectsSlider->setValue(-3000);click(*prefs,"preferencesOk");
    check(music.musicLevel()== -2000 && near(backend->volume,0.1f) && effects.soundLevel()== -3000,"accepted independent levels reach separate controllers");
    const auto accepted=bytes(path);QSettings reader(path,QSettings::IniFormat);
    check(reader.value("audio/v1/musicLevel").toInt()== -2000 && reader.value("audio/v1/effectsLevel").toInt()== -3000,"both versioned keys saved without replacing each other");
    click(preview,"mainMenuAction3");musicSlider->setValue(0);effectsSlider->setValue(0);click(*prefs,"preferencesCancel");
    check(bytes(path)==accepted && near(backend->volume,0.1f) && effects.soundLevel()== -3000,"Cancel changes neither saved bytes nor active gains");
    click(preview,"mainMenuAction3");musicSlider->setValue(-10000);click(*prefs,"preferencesOk");
    check(backend->volume==0 && backend->active && backend->starts==starts,"mute adjusts gain without stopping/replaying music");
    click(preview,"mainMenuAction3");musicSlider->setValue(0);click(*prefs,"preferencesOk");check(backend->volume==1,"full volume endpoint");
    preview.setAudioStatus("Audio paused: effects failure",true);
    auto oldReady=backend->ready;auto oldFailed=backend->failed;backend->failed("Synthetic decoder failure");backend->failed("Duplicate decoder failure");events();
    check(music.state()==MenuMusicState::failed && !backend->active && failures==1 && lastFailure=="Synthetic decoder failure","decoder failure stops once, coalesces and reports first diagnostic");
    check(label->text().contains(lastFailure) && preview.findChild<QLabel*>("menuAudioStatus")->text().contains("effects failure"),"music error does not overwrite effects status");
    click(preview,"mainMenuAction3");musicSlider->setValue(-1000);click(*prefs,"preferencesOk");
    check(music.musicLevel()== -1000 && effects.soundLevel()== -3000,"accepted music value updates while music is failed");
    backend->synchronousReady=true;check(music.start(track),"explicit restart after decoder failure");events();
    check(music.state()==MenuMusicState::playing && near(backend->startVolume,0.31622777f),"restart restores latest gain before playback");
    oldReady();oldFailed("Late notification from old track");events();check(music.state()==MenuMusicState::playing && failures==1,"old generation cannot change restarted music");
    QProcess restart;restart.start(QCoreApplication::applicationFilePath(),{"--restore",root,"-1000"});
    check(restart.waitForFinished(10000) && restart.exitStatus()==QProcess::NormalExit && restart.exitCode()==0,qPrintable("fresh process restore: "+restart.readAllStandardError()));
    auto closeReady=backend->ready;auto closeFailed=backend->failed;backend->failed("Queued before close");preview.close();closeReady();closeFailed("Late after close");events();
    check(music.state()==MenuMusicState::stopped && !backend->active && !effects.running() && failures==1,"closure stops channels and invalidates queued/future notifications");
    check(!music.start("") && !music.start(root) && !music.start(root+"/missing.wav") && !music.start("https://example.invalid/music.wav"),"empty/directory/missing/remote tracks rejected before decoder");
    check(backend->starts==starts+1,"invalid tracks never reach output start");
    backend->refuse=true;check(!music.start(track) && music.lastError()=="Synthetic music startup failure" && !backend->active,"synchronous startup failure unwinds and cancels queued ready");events();check(music.state()==MenuMusicState::failed,"failed startup cannot become playing");
    backend->refuse=false;backend->synchronousFailure=true;check(music.start(track),"asynchronous failure accepted for delivery");events();check(music.state()==MenuMusicState::failed && music.lastError()=="Decoder failed during start","initial decoder failure delivered safely");
    backend->synchronousFailure=false;
    music.changed=[&](MenuMusicState state,const QString&){if(state==MenuMusicState::loading)music.stop();};
    const int beforeCancel=backend->starts;check(!music.start(track) && backend->starts==beforeCancel && music.state()==MenuMusicState::stopped,"reentrant close during loading cancels before decoder starts");music.changed={};
    {
        std::function<void()> lateReady;std::function<void(const QString&)> lateFailed;
        {auto fake=std::make_unique<Music>();auto* sink=fake.get();MenuMusicController controller(std::move(fake));
         check(controller.start(track),"notification lifetime fixture starts");lateReady=sink->ready;lateFailed=sink->failed;sink->ready();}
        lateReady();lateFailed("Notification after controller destruction");events();
    }
    {
        auto fake=std::make_unique<Music>();auto* sink=fake.get();MenuMusicController controller(std::move(fake));
        MenuAudioController cues(std::make_unique<Effects>());
        auto window=std::make_unique<MenuPreview>();check(window->loadAssets(root,false,&error),qPrintable(error));controller.attach(*window);cues.attach(*window);
        check(controller.start(track) && cues.start(root+"/Sounds",mnm::reconstruction::audio::NativeSourcePathPolicy::literal),"destruction without close fixture starts both channels");
        sink->ready();window.reset();events();
        check(controller.state()==MenuMusicState::stopped && !sink->active && !cues.running(),"direct widget destruction stops channels without accessing destroyed presentation");
    }
    const QStringList invalid={"garbage","-1500.5","1","-10001","999999999999999999","","true"};
    for(int i=0;i<invalid.size();++i){
        const auto candidatePath=root+QString("/invalid-music-%1.ini").arg(i);
        menuAudioFixture::save(candidatePath,"[audio]\nv1\\musicLevel="+invalid[i].toUtf8()+"\n");
        QSettings candidate(candidatePath,QSettings::IniFormat);auto fake=std::make_unique<Music>();auto* sink=fake.get();
        MenuMusicController controller(std::move(fake),&candidate);const auto before=bytes(candidatePath);
        check(controller.musicLevel()== -1500 && controller.start(track) && near(sink->startVolume,0.17782794f) && bytes(candidatePath)==before,"invalid music settings default without startup write");
    }
    {
        QSettings unwritable(root+"/Sounds",QSettings::IniFormat);auto fake=std::make_unique<Music>();auto* sink=fake.get();
        MenuMusicController controller(std::move(fake),&unwritable);MenuPreview window;check(window.loadAssets(root,false,&error),qPrintable(error));controller.attach(window);
        QString failure;controller.failed=[&](const QString& value){failure=value;};check(controller.start(track),"unwritable store does not prevent playback");
        click(window,"mainMenuAction3");auto* settings=window.findChild<PreferencesWidget*>();settings->findChild<QSlider*>("preferencesSlider1")->setValue(-2000);click(*settings,"preferencesOk");
        check(failure.contains("Could not save music volume") && near(sink->volume,0.1f) && sink->active,"save failure retains accepted gain and playback");window.close();
    }
    std::cout<<"Menu music lifecycle, independent persisted gains, fresh-process restore and failures passed; fake backend, no game or physical audio\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
