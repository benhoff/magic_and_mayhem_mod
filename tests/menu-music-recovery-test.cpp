#include "menu_music_controller.hpp"
#include "menu_music_devices.hpp"
#include "menu_preview.hpp"
#include "menu-audio-fixture.hpp"
#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QSlider>
#include <cmath>
#include <iostream>
#include <thread>
using audioFixture::check;
namespace {
void events(){for(int i=0;i<6;++i)QCoreApplication::processEvents();}
bool near(float a,float b){return std::abs(a-b)<0.000001f;}
class Output final:public MenuMusicOutput {
public:
    bool active=false,refuse=false,loop=false,insideCallback=false;
    QByteArray current="A",selected;QString file;int starts=0,stops=0;
    float volume=1,startVolume=1;
    bool start(const QString& path,bool repeat,QString& error) override{
        ++starts;file=path;loop=repeat;selected=current;startVolume=volume;
        if(current.isEmpty() || refuse){error="Synthetic music device unavailable";return false;}
        active=true;return true;
    }
    void stop() override{check(!insideCallback,"decoder teardown deferred off notification stack");++stops;active=false;}
    void setVolume(float value) override{volume=value;}
    void lost(const QString& reason){insideCallback=true;if(failed)failed(reason);insideCallback=false;}
    void changed(const std::vector<QByteArray>& ids,const QByteArray& next){
        current=next;
        const auto change=musicDeviceChange(active,selected,ids,current);
        if(change!=MenuMusicDeviceChange::none)lost(change==MenuMusicDeviceChange::disconnected?QString("Music output disconnected"):QString("Default music output changed"));
        insideCallback=true;if(!next.isEmpty() && available)available();insideCallback=false;
    }
    void signalAvailable(){insideCallback=true;if(available)available();insideCallback=false;}
};
void click(QWidget& w,const char* name){auto* button=w.findChild<QPushButton*>(name);check(button,"button exists");button->click();}
}
int main(int argc,char** argv){QApplication app(argc,argv);try{
    check(musicDeviceChange(false,"A",{},"")==MenuMusicDeviceChange::none,"inactive outputs ignore list changes");
    check(musicDeviceChange(true,"A",{"A","B"},"A")==MenuMusicDeviceChange::none,"unrelated devices do not interrupt music");
    check(musicDeviceChange(true,"A",{"B"},"B")==MenuMusicDeviceChange::disconnected,"selected ID removal detects disconnect");
    check(musicDeviceChange(true,"A",{"A","B"},"B")==MenuMusicDeviceChange::defaultChanged,"default change while selected remains detects switch");
    check(musicDeviceChange(true,"A",{},"")==MenuMusicDeviceChange::disconnected,"all outputs removed detects loss");
    QTemporaryDir temporary;check(temporary.isValid(),"fixture directory");const auto root=temporary.path();menuAudioFixture::fixtures(root);
    const auto track=root+"/Sounds/Click.wav";
    QSettings settings(root+"/recovery.ini",QSettings::IniFormat);settings.setValue("audio/v1/musicLevel",-2000);settings.sync();
    auto output=std::make_unique<Output>();auto* sink=output.get();MenuMusicController music(std::move(output),&settings);
    MenuPreview preview;QString error;check(preview.loadAssets(root,false,&error),qPrintable(error));preview.show();events();music.attach(preview);
    int failures=0;QString diagnostic;music.failed=[&](const QString& message){++failures;diagnostic=message;};
    check(music.start(track),"initial track starts");sink->ready();events();
    auto* retry=preview.findChild<QPushButton*>("menuMusicRetry");auto* status=preview.findChild<QLabel*>("menuMusicStatus");
    check(retry && !retry->isVisible() && status->text()=="Music playing" && near(sink->startVolume,0.1f),"saved gain and music status initialized");
    const auto initialStarts=sink->starts;sink->changed({"A","B"},"A");events();check(sink->starts==initialStarts,"unrelated output notification does not restart");
    auto staleReady=sink->ready;auto staleFailed=sink->failed;auto staleAvailable=sink->available;
    sink->changed({},"");sink->lost("Repeated disconnect");
    check(sink->active && !music.canRecover(),"failure gates retry immediately but defers backend destruction");events();
    check(music.canRecover() && music.state()==MenuMusicState::failed && !sink->active && failures==1 && diagnostic=="Music output disconnected","loss coalesced with original diagnostic");
    check(retry->isVisible() && retry->isEnabled() && status->text().contains(diagnostic),"recoverable fault shows separate Retry music");
    click(preview,"mainMenuAction3");auto* prefs=preview.findChild<PreferencesWidget*>();prefs->findChild<QSlider*>("preferencesSlider1")->setValue(-1000);click(*prefs,"preferencesOk");
    check(music.musicLevel()== -1000 && settings.value("audio/v1/musicLevel").toInt()== -1000,"accepted volume saved during output loss");
    retry->click();check(music.canRecover() && failures==2 && sink->starts==initialStarts+1 && retry->isVisible(),"manual retry fails clearly when no output exists");
    const auto failedStarts=sink->starts;events();check(sink->starts==failedStarts,"failed open does not poll/retry on a timer");
    sink->changed({"B"},"B");sink->signalAvailable();sink->signalAvailable();events();
    check(music.state()==MenuMusicState::recovering && sink->starts==failedStarts+1 && sink->file==track && sink->selected=="B" && sink->loop && near(sink->startVolume,0.31622777f),"coalesced availability reopens retained track from beginning with latest gain");
    check(!retry->isVisible() && status->text().contains("Restarting"),"retry hidden while replacement loads");
    staleReady();staleFailed("Old track failure");staleAvailable();events();check(music.state()==MenuMusicState::recovering && failures==2,"old generation cannot alter recovery");
    sink->ready();events();check(music.state()==MenuMusicState::playing && status->text()=="Music playing","replacement accepted before reporting playing");
    const auto beforeSwitch=sink->starts;sink->changed({"B","C"},"C");sink->signalAvailable();events();
    check(music.state()==MenuMusicState::recovering && sink->starts==beforeSwitch+1 && sink->selected=="C" && failures==3,"same-turn default change and availability preserved through deferred loss cleanup");
    sink->ready();events();
    sink->refuse=true;sink->changed({"C","D"},"D");events();
    check(music.state()==MenuMusicState::failed && music.canRecover() && retry->isVisible() && !sink->active,"failed automatic reopen remains manually recoverable");
    const int noLoop=sink->starts;events();check(sink->starts==noLoop,"automatic open failure creates no retry loop");
    sink->refuse=false;retry->click();check(music.state()==MenuMusicState::recovering && sink->starts==noLoop+1 && near(sink->startVolume,0.31622777f),"explicit retry retains accepted gain");sink->ready();events();
    bool threadRejected=false;std::thread worker([&]{try{music.recover();}catch(const std::logic_error&){threadRejected=true;}});worker.join();check(threadRejected,"recovery constrained to owning Qt thread");
    auto lateAvailable=sink->available;sink->changed({},"");events();check(music.canRecover(),"loss before close");
    sink->current="D";sink->signalAvailable();preview.close();lateAvailable();events();
    check(music.state()==MenuMusicState::stopped && !music.canRecover() && !sink->active && !retry->isVisible(),"closure cancels queued recovery and retained track");
    {
        auto fake=std::make_unique<Output>();auto* missing=fake.get();missing->current.clear();MenuMusicController controller(std::move(fake));
        check(!controller.start(track) && controller.canRecover(),"initial missing device retains validated track");
        missing->changed({"A"},"A");events();check(controller.state()==MenuMusicState::recovering && missing->starts==2,"initial missing device recovers on availability");
        missing->ready();events();missing->lost("Reentrant close test");events();
        controller.changed=[&](MenuMusicState state,const QString&){if(state==MenuMusicState::recovering)controller.stop();};
        const auto starts=missing->starts;check(!controller.recover() && missing->starts==starts && !controller.canRecover(),"closure in recovering notification cancels before decoder opens");controller.changed={};
        check(!controller.start(root+"/missing.wav") && !controller.canRecover(),"invalid initial track is not a recoverable device request");
    }
    {
        auto fake=std::make_unique<Output>();auto* backend=fake.get();MenuMusicController controller(std::move(fake));
        check(controller.start(track),"deleted track fixture starts");backend->lost("Device lost");events();
        check(QFile::remove(track) && !controller.recover() && controller.canRecover() && controller.lastError().contains("readable"),"recovery revalidates file and retains previously selected request");
        menuAudioFixture::save(track,QByteArray("Replacement fixture for fake decoder"));check(controller.recover(),"restored track can be retried");
        auto availability=backend->available;backend->lost("Queued before destruction");
        controller.stop();availability();events();check(controller.state()==MenuMusicState::stopped,"stop invalidates queued loss and availability");
    }
    std::cout<<"Music device loss/default switch/retry, coalescing, retained track/gain and cancellation passed; synthetic outputs only\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}}
