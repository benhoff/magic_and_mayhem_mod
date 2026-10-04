#include "menu_audio_controller.hpp"
#include "menu_preview.hpp"
#include "main_menu_widget.hpp"
#include "quick_battle_menu_widget.hpp"
#include "audio-catalog-fixture.hpp"
#include "grimoire-fixtures.hpp"
#include <QApplication>
#include <QKeyEvent>
#include <QPushButton>
#include <QSlider>
#include <QStackedWidget>
#include <cstring>
#include <iostream>
namespace r=mnm::reconstruction::audio;using audioFixture::check;
namespace {
void save(const QString& path,const QByteArray& data){grimoire_fixture::write(path,data);}
void fixtures(const QString& root){
    check(QDir().mkpath(root+"/CFG"),"strings folder");
    QByteArray strings("[STRINGS]\n");for(int i=0;i<90;++i)strings+=QString("STR_%1=Fixture %2\n").arg(i,2,10,QLatin1Char('0')).arg(i).toLatin1();
    save(root+"/CFG/interface screens text.cfg",strings);
    QImage background(800,600,QImage::Format_RGB32);background.fill(QColor(80,110,140));
    const auto image=[&](const QString& folder){check(QDir().mkpath(folder+"/800x600"),"menu folder");check(background.save(folder+"/800x600/Fixture 800-600.JPG","JPG"),"menu image");};
    const auto main=root+"/Interface/MainScreen",quick=root+"/Interface/QuickBattleMainMenu",prefs=root+"/Interface/BattleOptionsScreen";
    image(main);image(quick);image(prefs);
    QByteArray layout("[GLOBALS]\nBackgroundFile=Fixture\n[TEXT_1]\nRect2=600,565,790,590\n");
    for(int i=0;i<6;++i)layout+=QString("[TEXTBUTTON_%1]\nRect2=150,%2,650,%3\nText=%4\n").arg(i+1).arg(260+i*45).arg(300+i*45).arg(i).toLatin1();
    save(main+"/screen (MainMenu).cfg",layout);
    layout="[GLOBALS]\nBackgroundFile=Fixture\n[TEXT_1]\nRect2=100,75,700,125\nText=2\n";
    for(int i=0;i<4;++i)layout+=QString("[TEXTBUTTON_%1]\nRect2=200,%2,600,%3\nText=%4\n").arg(i+1).arg(200+i*70).arg(250+i*70).arg(i).toLatin1();
    save(quick+"/Screen (Quick Battle Main Menu).cfg",layout);
    layout="[GLOBALS]\nBackgroundFile=Fixture\n";
    for(int i=0;i<11;++i)layout+=QString("[TEXT_%1]\nRect2=100,%2,700,%3\nFont=%4\nTextFlags=LEFT\nText=%5\n").arg(i+1).arg(30+i*25).arg(55+i*25).arg(i==0?"LARGE":"SMALL").arg(i).toLatin1();
    for(int i=0;i<12;++i)layout+=QString("[RADIOBUTTON_%1]\nRect2=100,%2,500,%3\nFont=SMALL\nText=%4\n").arg(i+1).arg(240+i*15).arg(265+i*15).arg(i+12).toLatin1();
    for(int i=0;i<2;++i)layout+=QString("[SLIDERBAR_%1]\nRect2=100,%2,325,%3\nminValue=-10000\nmaxValue=0\n").arg(i+1).arg(145+i*50).arg(175+i*50).toLatin1();
    layout+="[TEXTBUTTON_1]\nRect2=40,530,280,570\nFont=LARGE\nText=10\n[TEXTBUTTON_2]\nRect2=530,530,760,570\nFont=LARGE\nText=11\n";
    save(prefs+"/screen (Battle Options).cfg",layout);grimoire_fixture::create(root);
    check(QDir().mkpath(root+"/Sounds"),"sounds folder");const auto sounds=std::filesystem::path(root.toStdString())/"Sounds";
    audioFixture::write(sounds/"Click.wav",audioFixture::wave(4000));audioFixture::write(sounds/"Page.wav",audioFixture::wave(-2000));
    audioFixture::profile(sounds,"[Sounds]\n822=Click\n830=Page\n840=Logical\n[Randomised]\n840=830\n[Optimisation]\nMaxSimultaneousSounds=4\n");
}
class Writer final:public QIODevice {
public:
    Writer(){open(QIODevice::WriteOnly|QIODevice::Unbuffered);}
    QByteArray bytes;qint64 limit=100000;
    qint64 readData(char*,qint64) override{return -1;}
    qint64 writeData(const char* data,qint64 count) override{const auto size=std::min(count,limit);bytes.append(data,qsizetype(size));return size;}
};
class Output final:public AudioSessionOutput {
public:
    Writer writer;mnm::audio::Device* device=nullptr;std::unique_ptr<mnm::audio::PcmQueue> queue;bool refuse=false;int starts=0;
    std::uint32_t rate(QString&) override{return 48000;}
    bool start(mnm::audio::Device& d,QString& error) override{++starts;if(refuse){error="Synthetic sink failure";return false;}device=&d;queue=std::make_unique<mnm::audio::PcmQueue>(d);return true;}
    void stop() override{if(device)check(device->count()>0,"sink discarded before samples destroyed");queue.reset();device=nullptr;}
    bool running() const override{return bool(queue);}
    QString error() const override{return {};}
    std::int16_t sample(){writer.bytes.clear();writer.limit=100000;check(queue && queue->pump(writer,4),"pump cue");std::int16_t sample=0;check(writer.bytes.size()==4,"stereo frame");std::memcpy(&sample,writer.bytes.constData(),2);return sample;}
};
void click(QWidget& widget,const char* name){auto* button=widget.findChild<QPushButton*>(name);check(button,"button found");button->click();}
}
int main(int argc,char** argv){QApplication app(argc,argv);try{
    QTemporaryDir temporary;check(temporary.isValid(),"fixture root");const auto root=temporary.path();fixtures(root);
    MenuPreview preview;QString error;check(preview.loadAssets(root,false,&error),qPrintable(error));preview.show();app.processEvents();
    auto output=std::make_unique<Output>();auto* sink=output.get();MenuAudioController audio(std::move(output));audio.attach(preview);
    int failures=0;audio.failed=[&](const QString&){++failures;};check(audio.start(root+"/Sounds",r::NativeSourcePathPolicy::literal),"shared audio startup");auto* device=sink->device;
    click(preview,"mainMenuAction0");auto* action=preview.findChild<QPushButton*>("mainMenuAction0");
    QKeyEvent down(QEvent::KeyPress,Qt::Key_Space,Qt::NoModifier),up(QEvent::KeyRelease,Qt::Key_Space,Qt::NoModifier);
    QApplication::sendEvent(action,&down);QApplication::sendEvent(action,&up);
    check(sink->sample()==2530,"mouse and keyboard activations overlap through recovered duplicates");
    click(preview,"mainMenuAction2");check(preview.findChild<QStackedWidget*>()->currentWidget()==preview.findChild<QuickBattleMenuWidget*>(),"real click navigates to quick menu");
    check(sink->device==device && sink->sample()==1265,"one click at default accepted volume after transition cleanup");
    sink->writer.limit=1;check(sink->queue->pump(sink->writer,16) && sink->queue->pendingBytes()==15,"queued partial cue before navigation");
    auto* quick=preview.findChild<QuickBattleMenuWidget*>();QKeyEvent escape(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);QApplication::sendEvent(quick,&escape);
    check(sink->device==device && !sink->queue->pendingBytes() && sink->sample()==1265,"keyboard cancel discards old cue, emits once and shares device");
    click(preview,"mainMenuAction3");auto* prefs=preview.findChild<PreferencesWidget*>();check(prefs && preview.findChild<QStackedWidget*>()->currentWidget()==prefs,"lazy preferences loaded and bound");
    auto* slider=prefs->findChild<QSlider*>("preferencesSlider2");slider->setValue(-2000);click(*prefs,"preferencesOk");
    check(audio.soundLevel()== -2000 && sink->sample()==400,"accepted volume applies to action in destination screen");
    click(preview,"mainMenuAction3");slider->setValue(0);click(*prefs,"preferencesCancel");
    check(audio.soundLevel()== -2000 && sink->sample()==400,"cancelled draft does not change shared volume");
    click(preview,"mainMenuAction3");prefs->findChild<QSlider*>("preferencesSlider1")->setValue(0);click(*prefs,"preferencesOk");
    check(audio.soundLevel()== -2000 && sink->sample()==400,"music preference does not alter effects gain");
    check(preview.openGrimoire(root,&error),qPrintable(error));check(sink->sample()==0,"programmatic screen open clears old sounds and does not invent click");
    auto* book=preview.findChild<GrimoireWidget*>();click(*book,"grimoireNext");check(sink->sample()== -200,"accepted page change routes page cue through mixer");
    // Failed navigation emits no page-change cue. Re-entering an existing screen
    // must not duplicate audio connections.
    check(!book->setLocation({99,0,0,0},&error),"invalid page rejected");
    click(*book,"grimoireClose");check(sink->sample()==400,"book close routes one activation cue");
    for(int i=0;i<3;++i){click(preview,"mainMenuAction2");check(sink->sample()==400,"no duplicate binding on revisit");click(preview,"quickBattleAction3");check(sink->sample()==400,"cancel on revisit");}
    check(preview.openGrimoire(root,&error),qPrintable(error));click(*book,"grimoireNext");check(sink->sample()== -200,"book reopen binds page cue once");click(*book,"grimoireClose");
    click(preview,"mainMenuAction3");slider->setValue(-10000);click(*prefs,"preferencesOk");check(sink->sample()==0 && audio.running(),"preview mute suppresses new cues without ending session");
    click(preview,"mainMenuAction3");slider->setValue(0);click(*prefs,"preferencesOk");check(sink->sample()==4000,"unmute uses accepted level");
    sink->refuse=true;click(preview,"mainMenuAction2");check(!audio.running() && !sink->device && failures==1,"transition sink failure cleans audio but keeps menu usable");
    sink->refuse=false;check(audio.start(root+"/Sounds",r::NativeSourcePathPolicy::literal),"session recovers after output failure");
    check(preview.openGrimoire(root,&error),qPrintable(error));click(*book,"grimoireNext");preview.close();check(!audio.running() && !sink->device,"window close clears audio even with active cue");
    auto missingOutput=std::make_unique<Output>();auto* missing=missingOutput.get();MenuAudioController unavailable(std::move(missingOutput),{999,830});
    check(!unavailable.start(root+"/Sounds",r::NativeSourcePathPolicy::literal) && !unavailable.running() && !missing->device && !unavailable.lastError().isEmpty(),"unavailable mapping aborts audio instead of substitute");
    auto groupOutput=std::make_unique<Output>();auto* group=groupOutput.get();MenuAudioController randomized(std::move(groupOutput),{840,830});
    MenuPreview second;check(second.loadAssets(root,false,&error),qPrintable(error));randomized.attach(second);check(randomized.start(root+"/Sounds",r::NativeSourcePathPolicy::literal),"randomized cue mapping accepted");click(second,"mainMenuAction2");check(group->sample()== -632,"group cue resolves through manager admission");
    second.close();check(!randomized.running(),"independent preview closes its own session");
    std::cout<<"Native menu audio passed; real widget actions, exact PCM, settings, shared ownership, transitions, page cues and failures; no game or physical sink\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
