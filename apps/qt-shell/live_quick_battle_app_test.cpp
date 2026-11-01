#include "live_menu_test.hpp"
#include "live_menu_session.hpp"
#include "campaign_smoke_test.hpp"
#include "gl_viewport.hpp"
#include "host_timing_trace.hpp"
#include "acceptance_input_ack.hpp"
#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QListWidget>
#include <QComboBox>
#include <QKeyEvent>
#include <QScreen>
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
    struct Evidence {int mapRow=1;QString mapId,mapName;unsigned stage=0;unsigned worldVisits=0;unsigned originalInputRows=0;quint32 thread=0;bool done=false,valid=true,world=false,actions=false,focusReleased=false,resized=false,escape=false;quint64 initialFrames=0,initialPaints=0;QElapsedTimer worldClock;QJsonArray states,inputs,probes,display;};
    auto e=std::make_shared<Evidence>();
    if(qEnvironmentVariableIntValue("MNM_ACCEPTANCE_PILOT")==1){
        if(auto* scaling=window.findChild<QComboBox*>("viewportScaling"))scaling->setCurrentIndex(2);
        else e->valid=false;
    }
    if(qEnvironmentVariableIsSet("MNM_QUICK_BATTLE_MAP_ROW")){bool ok=false;e->mapRow=qEnvironmentVariableIntValue("MNM_QUICK_BATTLE_MAP_ROW",&ok);e->valid=ok&&e->mapRow>=0&&e->mapRow<32;}
    session.skipMoviesEnabled=qEnvironmentVariableIntValue("MNM_ACCEPTANCE_PILOT")!=1 || qEnvironmentVariableIntValue("MNM_ACCEPTANCE_SKIP_INTROS")==1; // Pilot can retain enabled default intros.
    session.resultMenusEnabled=true;session.preferencesMenusEnabled=false;session.regionMenusEnabled=false;
    auto fail=[e,probe,path,&app](const QString& error){
        if(e->done)return;
        e->done=true;e->valid=false;
        const auto p=probe();QFile file(path);
        if(file.open(QIODevice::WriteOnly|QIODevice::NewOnly))file.write(QJsonDocument(QJsonObject{{"success",false},{"error",error},{"selected_map_row",e->mapRow},{"selected_map_id",e->mapId},{"selected_map_name",e->mapName},{"stage",int(e->stage)},{"world",e->world},{"actions",e->actions},{"states",e->states},{"inputs",e->inputs},{"world_presentation",e->probes},{"native_frames",double(p.frames)},{"native_paints",double(p.paints)},{"native_active",p.active},{"native_fallback",p.fallback},{"native_error",p.error}}).toJson());
        std::fprintf(stderr,"Quick Battle app refused: %s\n",qPrintable(error));app.exit(1);
    };
    auto failure=session.failed;
    session.failed=[failure,fail](const QString& error){if(failure)failure(error);fail(error);};
    auto output=session.output;
    session.output=[output](const QString& text){if(output)output(text);std::fprintf(stderr,"%s",qPrintable(text));};
    auto input=[e,&session,&window](const char* phase){
        QFile file(QDir(session.evidenceDirectory()).filePath("render-input.bin"));
        if(!file.open(QIODevice::ReadOnly))return false;
        const auto data=file.readAll();if(data.size()!=1088)return false;
        auto word=[&](int index){return qFromLittleEndian<quint32>(data.constData()+index*4);};
        const bool client=data.left(8)=="MNMINK01"&&word(2)==1;
        const bool desktop=data.left(8)=="MNMINK02"&&word(2)==2&&word(10)==1;
        if((!client&&!desktop)||word(3)!=1088||(word(4)&1))return false;
        const bool right=word(16+39)&0x80000000u;
        auto* viewport=window.findChild<GlViewport*>("nativeGameViewport");
        e->inputs.append(QJsonObject{{"presented_world",QString::number(viewport?viewport->swappedWorldGeneration():0)},{"presented_paint",QString::number(viewport?viewport->swappedPaint():0)},{"phase",phase},{"sequence",int(word(4))},{"active",bool(word(5))},{"right_down",right},{"width",int(word(8))},{"height",int(word(9))}});
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
        const unsigned screens[]={3,22,14,25,14,7,26,26,22,3};
        if(e->stage>=10||state.screen!=screens[e->stage]||(e->stage<=6&&state.ack!=e->stage))return;
        if(e->thread&&e->thread!=state.thread)e->valid=false;
        e->thread=state.thread;
        QJsonObject row{{"screen",int(state.screen)},{"ack",int(state.ack)},{"thread",int(state.thread)},{"map",int(state.battle.map)}};
        if(state.screen==26){e->escape=true;QJsonArray players;for(const auto& p:state.results.players)players.append(QJsonObject{{"active",p.active},{"name",p.name},{"kills",p.kills},{"deaths",p.deaths},{"handicap",p.handicap},{"score",p.score}});row["results"]=players;}
        e->states.append(row);const unsigned stage=e->stage++;
        QTimer::singleShot(0,&window,[e,&window,path,stage]{

            const char* buttons[]={"mainMenuAction2","quickBattleAction2","singlePlayerMap","mapSelectionOk","singlePlayerStart","spellboxOK","quickResultAction1","quickResultAction2","quickBattleAction3","mainMenuAction4"};
            if(stage==3){auto* list=window.findChild<QListWidget*>("mapSelectionList");if(!e->valid||!list||list->count()<=e->mapRow){e->valid=false;return;}const auto* selected=list->item(e->mapRow);e->mapId=selected->data(Qt::UserRole).toString();e->mapName=selected->text();list->setCurrentRow(e->mapRow);}
            auto* button=window.findChild<QPushButton*>(buttons[stage]);if(!button||!button->isEnabled()){e->valid=false;return;}button->click();
            if(stage==6||stage==7){button->click();button->click();} // Rapid duplicate actions.
            QTimer::singleShot(100,&window,[&window,path,stage]{window.grab().save(QFileInfo(path).dir().filePath(QString("qt-journey-%1.png").arg(stage)));});
        });
    };
    auto handoff=session.battleStarted;
    session.battleStarted=[e,handoff,probe](quint32 target){if(handoff)handoff(target);if(target==1)return;e->world=target==2;e->valid&=e->world;const auto p=probe();e->initialFrames=p.frames;e->initialPaints=p.paints;e->worldClock.start();++e->worldVisits;};
    auto returned=session.originalViewportRequested;
    session.originalViewportRequested=[e,returned,probe]{
        if(returned)returned();
        if(e->stage==7){const auto p=probe();e->actions=false;e->escape=false;e->initialFrames=p.frames;e->initialPaints=p.paints;e->worldClock.restart();++e->worldVisits;}
    };
    auto* timer=new QTimer(&window);timer->setInterval(50);
    QObject::connect(timer,&QTimer::timeout,&window,[e,probe,&window,&session,path,key,input,fail]{
        if(e->done)return;
        const auto p=probe();
        if(p.fallback||!p.error.isEmpty()){fail(p.error.isEmpty()?QString("Native presentation fallback"):p.error);return;}
        if(!e->world||e->actions||e->escape)return;
        auto* viewport=dynamic_cast<GlViewport*>(window.findChild<QWidget*>("nativeGameViewport"));
        if(!viewport||!viewport->isVisible()||!p.active||p.frames<e->initialFrames+3||p.paints<e->initialPaints+3||e->worldClock.elapsed()<(qEnvironmentVariableIntValue("MNM_ACCEPTANCE_PILOT")==1?2000:6000))return;
        if(!qEnvironmentVariable("MNM_ACCEPTANCE_CAPTURE_DIRECTORY").isEmpty() && !viewport->swappedWorldGeneration())return;
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
        auto releaseInput=[e,&window,input]{
            HostTimingTrace trace("quick-focus-resize");
            auto* v=dynamic_cast<GlViewport*>(window.findChild<QWidget*>("nativeGameViewport"));if(!v){e->valid=false;return;}v->clearFocus();e->focusReleased=input("focus-release");e->valid&=e->focusReleased;v->setFocus(Qt::OtherFocusReason);window.resize(1180,900);e->resized=window.size()==QSize(1180,900);
        };
        auto afterInput=[e,&window,path,key]{
            if(qEnvironmentVariableIntValue("MNM_ACCEPTANCE_NORMAL")==1){
                auto toggle=[e,&window,key](bool full){
                    HostTimingTrace trace("quick-fullscreen-key");
                    auto* v=window.findChild<GlViewport*>("nativeGameViewport");
                    const auto before=v?v->swappedPaint():0;
                    const bool accepted=key(QEvent::KeyPress,Qt::Key_F9,75)&&key(QEvent::KeyRelease,Qt::Key_F9,75);
                    e->valid &= accepted&&window.isFullScreen()==full;
                    e->display.append(QJsonObject{{"fullscreen",window.isFullScreen()},{"accepted",accepted},{"screen_width",window.screen()->geometry().width()},{"screen_height",window.screen()->geometry().height()},{"paint_before",QString::number(before)},{"width",window.width()},{"height",window.height()}});
                };
                QTimer::singleShot(200,&window,[toggle]{toggle(true);});
                QTimer::singleShot(700,&window,[toggle]{toggle(false);});
            }
            QTimer::singleShot(1100,&window,[e,&window,path,key]{HostTimingTrace trace("quick-screenshot");window.grab().save(QFileInfo(path).dir().filePath("qt-native-world.png"));e->valid&=key(QEvent::KeyPress,Qt::Key_Escape,9);});
            QTimer::singleShot(1700,&window,[key]{key(QEvent::KeyRelease,Qt::Key_Escape,9);});
        };
        if(qEnvironmentVariableIntValue("MNM_ACCEPTANCE_PILOT")==1){
            const auto receipt=QDir(session.evidenceDirectory()).filePath("render-input.bin.acceptance");
            QTimer::singleShot(250,&window,[e,&window,key,input,releaseInput,afterInput,receipt,fail]{
                if(e->done)return;
                if(!key(QEvent::KeyPress,Qt::Key_Right,114)||!input("right-down")){fail("Qt Right publication failed");return;}
                const auto published=quint32(e->inputs.last().toObject().value("sequence").toInt());
                AcceptanceInputAck::wait(&window,receipt,e->originalInputRows,published,true,
                    [e,&window,releaseInput,afterInput,receipt,fail](unsigned row){
                        if(e->done)return;
                        releaseInput();
                        if(!e->valid){fail("Qt focus release failed");return;}
                        const auto released=quint32(e->inputs.last().toObject().value("sequence").toInt());
                        AcceptanceInputAck::wait(&window,receipt,row,released,false,
                            [e,afterInput](unsigned releasedRow){if(e->done)return;e->originalInputRows=releasedRow;afterInput();},
                            [fail]{fail("Original Right release receipt did not arrive within three seconds");});
                    },[fail]{fail("Original Right press receipt did not arrive within three seconds");});
            });
        }else{
            QTimer::singleShot(250,&window,[e,key,input]{e->valid&=key(QEvent::KeyPress,Qt::Key_Right,114)&&input("right-down");});
            QTimer::singleShot(700,&window,releaseInput);
            QTimer::singleShot(700,&window,afterInput);
        }
    });timer->start();
    auto finished=session.finished;
    session.finished=[e,finished,probe,path,&app]{
        if(finished)finished();
        if(e->done)return;
        e->done=true;
        const auto p=probe();
        const bool success=e->valid&&e->stage==10&&e->worldVisits==2&&e->world&&e->actions&&e->focusReleased&&e->resized&&e->escape&&!p.fallback&&p.error.isEmpty()&&p.recoveries==0;
        QFile file(path);bool saved=file.open(QIODevice::WriteOnly|QIODevice::NewOnly);
        if(saved)saved=file.write(QJsonDocument(QJsonObject{{"success",success},{"selected_map_row",e->mapRow},{"selected_map_id",e->mapId},{"selected_map_name",e->mapName},{"states",e->states},{"inputs",e->inputs},{"world_presentation",e->probes},{"native_frames",double(p.frames)},{"native_paints",double(p.paints)},{"native_fallback",p.fallback},{"native_error",p.error},{"recoveries",int(p.recoveries)},{"focus_release",e->focusReleased},{"resize",e->resized},{"escape_results",e->escape},{"input_via_qt_viewport",true},{"original_input_wait",qEnvironmentVariableIntValue("MNM_ACCEPTANCE_PILOT")==1},{"original_input_rows",int(e->originalInputRows)},{"original_input_wait_limit_ms",AcceptanceInputAck::DeadlineMs},{"world_visits",int(e->worldVisits)},{"results_delay_ms",0},{"escape_hold_ms",600},{"rapid_duplicate_clicks",true},{"fullscreen_transitions",e->display},{"scope","Bounded native Qt menu/command Quick Battle journey with Qt-forwarded input, focus-release, viewport resize, Results and normal Quit; original CPU raster remains active, no independent pixel comparison or complete replacement claim"}}).toJson())>0&&file.flush();
        app.exit(success&&saved?0:1);
    };
    if(session.winePrefix.isEmpty())session.winePrefix=QFileInfo(path).dir().filePath("wineprefix");
    session.smokeSeconds=0;
    QTimer::singleShot(qEnvironmentVariableIntValue("MNM_ACCEPTANCE_PILOT")==1?300000:180000,&window,[fail]{fail("Quick Battle app deadline exceeded");});
    QTimer::singleShot(0,&window,[&window]{if(auto* b=window.findChild<QPushButton*>("launchGame"))b->click();});
}
