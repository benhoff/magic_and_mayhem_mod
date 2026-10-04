#include "menu_audio_controller.hpp"
#include "menu_preview.hpp"
#include "main_menu_widget.hpp"
#include "quick_battle_menu_widget.hpp"
#include "audio-catalog-fixture.hpp"
#include "grimoire-fixtures.hpp"
#include <QApplication>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QStackedWidget>
#include <QSettings>
#include <QFile>
#include <QProcess>
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
    void fail(const QString& message){stop();if(failed)failed(message);}
    void changedDevices(){if(available)available();}
    std::int16_t sample(){writer.bytes.clear();writer.limit=100000;check(queue && queue->pump(writer,4),"pump cue");std::int16_t sample=0;check(writer.bytes.size()==4,"stereo frame");std::memcpy(&sample,writer.bytes.constData(),2);return sample;}
};
void click(QWidget& widget,const char* name){auto* button=widget.findChild<QPushButton*>(name);check(button,"button found");button->click();}
}
int main(int argc,char** argv){QApplication app(argc,argv);try{
    if(argc==3 && QString::fromLocal8Bit(argv[1])=="--restore-preferences"){
        const auto root=QString::fromLocal8Bit(argv[2]);QSettings store(root+"/preferences.ini",QSettings::IniFormat);
        auto output=std::make_unique<Output>();auto* sink=output.get();MenuAudioController controller(std::move(output),{},nullptr,&store);
        MenuPreview preview;QString error;check(preview.loadAssets(root,false,&error),qPrintable(error));controller.attach(preview);
        check(controller.start(root+"/Sounds",r::NativeSourcePathPolicy::literal),"fresh process audio starts");
        click(preview,"mainMenuAction0");check(sink->sample()==400,"fresh process first cue uses saved volume");
        click(preview,"mainMenuAction3");auto* prefs=preview.findChild<PreferencesWidget*>();
        check(prefs && prefs->settings().soundLevel== -2000,"fresh process lazy Preferences restores saved volume");
        preview.close();return 0;
    }

    QTemporaryDir temporary;check(temporary.isValid(),"fixture root");const auto root=temporary.path();fixtures(root);
    MenuPreview preview;QString error;check(preview.loadAssets(root,false,&error),qPrintable(error));preview.show();app.processEvents();
    const auto settingsPath=root+"/preferences.ini";
    QSettings settings(settingsPath,QSettings::IniFormat);
    auto output=std::make_unique<Output>();auto* sink=output.get();MenuAudioController audio(std::move(output),{},nullptr,&settings);audio.attach(preview);
    check(audio.soundLevel()== -1000 && !QFile::exists(settingsPath),"missing settings use default without writing on startup");
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
    QSettings acceptedSettings(settingsPath,QSettings::IniFormat);
    check(acceptedSettings.value("audio/v1/effectsLevel").toInt()== -2000,"accepted volume synced to independent INI reader");
    QFile saved(settingsPath);check(saved.open(QIODevice::ReadOnly),"saved settings readable");const auto acceptedBytes=saved.readAll();saved.close();
    click(preview,"mainMenuAction3");slider->setValue(0);click(*prefs,"preferencesCancel");
    check(saved.open(QIODevice::ReadOnly) && saved.readAll()==acceptedBytes,"cancel leaves saved file byte-identical");saved.close();
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
    auto* retry=preview.findChild<QPushButton*>("menuAudioRetry");auto* audioLabel=preview.findChild<QLabel*>("menuAudioStatus");
    check(retry && retry->isVisible() && retry->isEnabled() && audioLabel->text().contains("Synthetic sink failure"),"persistent failure status and retry affordance");
    sink->refuse=false;retry->click();check(audio.running() && sink->sample()==0 && audioLabel->text()=="Audio ready" && !retry->isVisible(),"visible retry restores silent output");
    click(preview,"quickBattleAction3");click(preview,"mainMenuAction3");slider->setValue(-2000);click(*prefs,"preferencesOk");
    sink->fail("Output disconnected");for(int i=0;i<3;++i)app.processEvents();
    check(!audio.running() && failures==2 && retry->isVisible() && audioLabel->text().contains("Output disconnected"),"asynchronous failure reported without another menu action");
    click(preview,"mainMenuAction3");slider->setValue(-2000);click(*prefs,"preferencesOk");check(audio.soundLevel()== -2000,"accepted volume changes while output absent");
    QSettings offlineSettings(settingsPath,QSettings::IniFormat);check(offlineSettings.value("audio/v1/effectsLevel").toInt()== -2000,"offline accepted volume is persisted");
    sink->changedDevices();for(int i=0;i<3;++i)app.processEvents();
    check(audio.running() && sink->sample()==0 && audio.soundLevel()== -2000,"device availability automatically restores accepted settings without old cues");
    click(preview,"mainMenuAction0");check(sink->sample()==400,"new action after recovery uses latest gain");
    check(preview.openGrimoire(root,&error),qPrintable(error));click(*book,"grimoireNext");preview.close();check(!audio.running() && !sink->device,"window close clears audio even with active cue");
    QProcess restart;restart.start(QCoreApplication::applicationFilePath(),{"--restore-preferences",root});
    check(restart.waitForFinished(10000) && restart.exitStatus()==QProcess::NormalExit && restart.exitCode()==0,qPrintable("fresh process restart: "+restart.readAllStandardError()));
    {
        QSettings reopened(settingsPath,QSettings::IniFormat);
        auto restartOutput=std::make_unique<Output>();auto* restartSink=restartOutput.get();
        MenuAudioController restarted(std::move(restartOutput),{},nullptr,&reopened);
        MenuPreview restartedPreview;check(restartedPreview.loadAssets(root,false,&error),qPrintable(error));restarted.attach(restartedPreview);
        check(restarted.start(root+"/Sounds",r::NativeSourcePathPolicy::literal),"new session with disk settings starts");
        click(restartedPreview,"mainMenuAction0");check(restartSink->sample()==400,"first cue before opening Preferences uses restored gain");
        click(restartedPreview,"mainMenuAction3");auto* restoredPrefs=restartedPreview.findChild<PreferencesWidget*>();
        check(restoredPrefs && restoredPrefs->settings().soundLevel== -2000 && restoredPrefs->findChild<QSlider*>("preferencesSlider2")->value()== -2000,"lazy Preferences shows restored effects level");
        click(*restoredPrefs,"preferencesCancel");check(restarted.soundLevel()== -2000,"restart cancel keeps restored setting");
        restartedPreview.close();
    }
    // Corrupt values never coerce to full volume. Startup does not repair/write
    // the file; only an accepted preference changes this versioned key.
    const QStringList values={"garbage","-2000.5","1","-10001","999999999999999999999","","true","@Invalid()","-10000","0","-2000"};
    for(int i=0;i<values.size();++i){
        const auto path=root+QString("/validation-%1.ini").arg(i);
        save(path,"[audio]\nv1\\effectsLevel="+values[i].toUtf8()+"\n");
        QFile before(path);check(before.open(QIODevice::ReadOnly),"invalid fixture readable");const auto bytes=before.readAll();before.close();
        QSettings candidate(path,QSettings::IniFormat);
        auto validationOutput=std::make_unique<Output>();auto* validationSink=validationOutput.get();
        MenuAudioController controller(std::move(validationOutput),{},nullptr,&candidate);
        const int expected=i<8? -1000:values[i].toInt();check(controller.soundLevel()==expected,"strict persisted integer and gain range validation");
        MenuPreview candidatePreview;check(candidatePreview.loadAssets(root,false,&error),qPrintable(error));controller.attach(candidatePreview);
        check(controller.start(root+"/Sounds",r::NativeSourcePathPolicy::literal),"validated persisted gain starts");
        click(candidatePreview,"mainMenuAction0");const int sample=expected== -10000?0:expected==0?4000:expected== -2000?400:1265;
        check(validationSink->sample()==sample,"restored boundary/default gain applied before first cue");
        check(before.open(QIODevice::ReadOnly) && before.readAll()==bytes,"load and cues do not rewrite stored settings");before.close();candidatePreview.close();
    }
    {
        // An existing directory is an unwritable INI destination even as root.
        QSettings unwritable(root+"/Sounds",QSettings::IniFormat);
        auto failedStoreOutput=std::make_unique<Output>();auto* failedStoreSink=failedStoreOutput.get();
        MenuAudioController controller(std::move(failedStoreOutput),{},nullptr,&unwritable);
        MenuPreview failedStorePreview;check(failedStorePreview.loadAssets(root,false,&error),qPrintable(error));controller.attach(failedStorePreview);
        QString failure;controller.failed=[&](const QString& message){failure=message;};
        check(controller.start(root+"/Sounds",r::NativeSourcePathPolicy::literal),"write failure fixture starts");
        click(failedStorePreview,"mainMenuAction3");auto* failedPrefs=failedStorePreview.findChild<PreferencesWidget*>();
        failedPrefs->findChild<QSlider*>("preferencesSlider2")->setValue(-2000);click(*failedPrefs,"preferencesOk");
        check(failure.contains("Could not save effects volume") && controller.running() && failedStoreSink->sample()==400,"write error reported without losing accepted session gain");failedStorePreview.close();
    }
    auto missingOutput=std::make_unique<Output>();auto* missing=missingOutput.get();MenuAudioController unavailable(std::move(missingOutput),{999,830});
    check(!unavailable.start(root+"/Sounds",r::NativeSourcePathPolicy::literal) && !unavailable.running() && !missing->device && !unavailable.lastError().isEmpty(),"unavailable mapping aborts audio instead of substitute");
    auto groupOutput=std::make_unique<Output>();auto* group=groupOutput.get();MenuAudioController randomized(std::move(groupOutput),{840,830});
    MenuPreview second;check(second.loadAssets(root,false,&error),qPrintable(error));randomized.attach(second);check(randomized.start(root+"/Sounds",r::NativeSourcePathPolicy::literal),"randomized cue mapping accepted");click(second,"mainMenuAction2");check(group->sample()== -632,"group cue resolves through manager admission");
    second.close();check(!randomized.running(),"independent preview closes its own session");
    std::cout<<"Native menu audio passed; real widget actions, exact PCM, persisted settings, shared ownership, transitions, page cues and failures; no game or physical sink\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
