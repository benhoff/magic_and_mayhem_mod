#include "live_menu_test.hpp"
#include "live_menu_session.hpp"
#include "campaign_smoke_test.hpp"
#include "gl_viewport.hpp"
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QListWidget>
#include <QKeyEvent>
#include <QLibrary>
#include <QTimer>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QtEndian>
#include <memory>
#include <cstdio>

void installQuickBattleAppTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,
    const QString& path,std::function<CampaignPresentationProbe()> probe){
    struct Evidence {unsigned stage=0;quint32 thread=0;bool valid=true,world=false,actions=false,focusReleased=false,resized=false,escape=false;quint64 initialFrames=0,initialPaints=0;QElapsedTimer worldClock;QJsonArray states,inputs,probes;};
    auto e=std::make_shared<Evidence>();
    session.resultMenusEnabled=true;session.preferencesMenusEnabled=false;session.regionMenusEnabled=false;
    auto failure=session.failed;
    session.failed=[failure,e](const QString& error){e->valid=false;std::fprintf(stderr,"Quick Battle app refused: %s\n",qPrintable(error));if(failure)failure(error);};
    auto output=session.output;
    session.output=[output](const QString& text){if(output)output(text);std::fprintf(stderr,"%s",qPrintable(text));};
    auto input=[e,&session](const char* phase){
        QFile file(QDir(session.evidenceDirectory()).filePath("render-input.bin"));
        if(!file.open(QIODevice::ReadOnly))return false;
        const auto data=file.readAll();if(data.size()!=1088||data.left(8)!="MNMINK01")return false;
        auto word=[&](int index){return qFromLittleEndian<quint32>(data.constData()+index*4);};
        if(word(4)&1)return false;
        const bool right=word(16+39)&0x80000000u;
        e->inputs.append(QJsonObject{{"phase",phase},{"sequence",int(word(4))},{"active",bool(word(5))},{"right_down",right},{"width",int(word(8))},{"height",int(word(9))}});
        return std::string(phase)=="right-down"?right&&word(5):!right&&!word(5);
    };
    auto key=[&window](QEvent::Type type,int value,quint32 scan){
        auto* viewport=dynamic_cast<GlViewport*>(window.findChild<QWidget*>("nativeGameViewport"));if(!viewport||!viewport->isVisible())return false;
        QKeyEvent event(type,value,Qt::NoModifier,scan,0,0);event.ignore();QApplication::sendEvent(viewport,&event);return event.isAccepted();
    };
    auto presentation=session.stateChanged;
    session.stateChanged=[e,presentation,&window,path](const MenuBridge::State& state){
        if(presentation)presentation(state);
        if(!state.ready)return;
        const unsigned screens[]={3,22,14,25,14,7,26,22,3};
        if(e->stage>=9||state.screen!=screens[e->stage]||state.ack!=e->stage)return;
        if(e->thread&&e->thread!=state.thread)e->valid=false;
        e->thread=state.thread;
        QJsonObject row{{"screen",int(state.screen)},{"ack",int(state.ack)},{"thread",int(state.thread)},{"map",int(state.battle.map)}};
        if(state.screen==26){e->escape=true;QJsonArray players;for(const auto& p:state.results.players)players.append(QJsonObject{{"active",p.active},{"name",p.name},{"kills",p.kills},{"deaths",p.deaths},{"handicap",p.handicap},{"score",p.score}});row["results"]=players;}
        e->states.append(row);const unsigned stage=e->stage++;
        QTimer::singleShot(stage==6?750:0,&window,[e,&window,path,stage]{

            const char* buttons[]={"mainMenuAction2","quickBattleAction2","singlePlayerMap","mapSelectionOk","singlePlayerStart","spellboxOK","quickResultAction2","quickBattleAction3","mainMenuAction4"};
            if(stage==3){auto* list=window.findChild<QListWidget*>("mapSelectionList");if(!list||list->count()<2){e->valid=false;return;}list->setCurrentRow(1);}
            auto* button=window.findChild<QPushButton*>(buttons[stage]);if(!button||!button->isEnabled()){e->valid=false;return;}button->click();
            QTimer::singleShot(100,&window,[&window,path,stage]{window.grab().save(QFileInfo(path).dir().filePath(QString("qt-journey-%1.png").arg(stage)));});
        });
    };
    auto handoff=session.battleStarted;
    session.battleStarted=[e,handoff,probe](quint32 target){if(handoff)handoff(target);if(target==1)return;e->world=target==2;e->valid&=e->world;const auto p=probe();e->initialFrames=p.frames;e->initialPaints=p.paints;e->worldClock.start();};
    auto* timer=new QTimer(&window);timer->setInterval(50);
    QObject::connect(timer,&QTimer::timeout,&window,[e,probe,&window,path,key,input]{
        if(!e->world||e->actions||e->escape)return;
        const auto p=probe();
        if(p.fallback||!p.error.isEmpty()){e->valid=false;e->actions=true;return;}
        auto* viewport=dynamic_cast<GlViewport*>(window.findChild<QWidget*>("nativeGameViewport"));
        if(!viewport||!viewport->isVisible()||!p.active||p.frames<e->initialFrames+3||p.paints<e->initialPaints+3||e->worldClock.elapsed()<6000)return;
        e->actions=true;e->probes.append(QJsonObject{{"frames",double(p.frames)},{"paints",double(p.paints)},{"active",p.active},{"fallback",p.fallback},{"recoveries",int(p.recoveries)},{"renderer",p.renderer}});
        window.activateWindow();window.raise();viewport->setFocus(Qt::OtherFocusReason);
        // Isolated Xvfb has no window manager to honor activation requests.
        // Focus the Qt host itself; all game keys still pass its event filter.
        QLibrary x11("X11");
        auto open=reinterpret_cast<void*(*)(const char*)>(x11.resolve("XOpenDisplay"));
        auto focus=reinterpret_cast<int(*)(void*,unsigned long,int,unsigned long)>(x11.resolve("XSetInputFocus"));
        auto flush=reinterpret_cast<int(*)(void*)>(x11.resolve("XFlush"));
        auto close=reinterpret_cast<int(*)(void*)>(x11.resolve("XCloseDisplay"));
        if(!open||!focus||!flush||!close){e->valid=false;return;}
        if(auto* display=open(nullptr)){focus(display,window.winId(),2,0);flush(display);close(display);}else{e->valid=false;return;}
        QTimer::singleShot(250,&window,[e,key,input]{e->valid&=key(QEvent::KeyPress,Qt::Key_Right,114)&&input("right-down");});
        QTimer::singleShot(700,&window,[e,&window,input]{
            auto* v=dynamic_cast<GlViewport*>(window.findChild<QWidget*>("nativeGameViewport"));if(!v){e->valid=false;return;}v->clearFocus();e->focusReleased=input("focus-release");e->valid&=e->focusReleased;v->setFocus(Qt::OtherFocusReason);window.resize(1180,900);e->resized=window.size()==QSize(1180,900);
        });
        QTimer::singleShot(1800,&window,[e,&window,path,key]{window.grab().save(QFileInfo(path).dir().filePath("qt-native-world.png"));e->valid&=key(QEvent::KeyPress,Qt::Key_Escape,9);});
        QTimer::singleShot(1920,&window,[key]{key(QEvent::KeyRelease,Qt::Key_Escape,9);});
    });timer->start();
    auto finished=session.finished;
    session.finished=[e,finished,probe,path,&app]{
        if(finished)finished();
        const auto p=probe();
        const bool success=e->valid&&e->stage==9&&e->world&&e->actions&&e->focusReleased&&e->resized&&e->escape&&!p.fallback&&p.error.isEmpty()&&p.recoveries==0;
        QFile file(path);bool saved=file.open(QIODevice::WriteOnly|QIODevice::NewOnly);
        if(saved)saved=file.write(QJsonDocument(QJsonObject{{"success",success},{"states",e->states},{"inputs",e->inputs},{"world_presentation",e->probes},{"native_frames",double(p.frames)},{"native_paints",double(p.paints)},{"native_fallback",p.fallback},{"native_error",p.error},{"recoveries",int(p.recoveries)},{"focus_release",e->focusReleased},{"resize",e->resized},{"escape_results",e->escape},{"input_via_qt_viewport",true},{"scope","Bounded native Qt menu/command Quick Battle journey with Qt-forwarded input, focus-release, viewport resize, Results and normal Quit; original CPU raster remains active, no independent pixel comparison or complete replacement claim"}}).toJson())>0&&file.flush();
        app.exit(success&&saved?0:1);
    };
    session.winePrefix=QFileInfo(path).dir().filePath("wineprefix");session.smokeSeconds=0;
    QTimer::singleShot(180000,&window,[e,probe,path,&app]{
        const auto p=probe();QFile file(path);
        if(file.open(QIODevice::WriteOnly|QIODevice::NewOnly))file.write(QJsonDocument(QJsonObject{{"success",false},{"error","Quick Battle app deadline exceeded"},{"stage",int(e->stage)},{"world",e->world},{"actions",e->actions},{"states",e->states},{"inputs",e->inputs},{"world_presentation",e->probes},{"native_frames",double(p.frames)},{"native_paints",double(p.paints)},{"native_active",p.active},{"native_fallback",p.fallback},{"native_error",p.error}}).toJson());
        std::fputs("Quick Battle app deadline exceeded.\n",stderr);app.exit(1);
    });
    QTimer::singleShot(0,&window,[&window]{if(auto* b=window.findChild<QPushButton*>("launchGame"))b->click();});
}
