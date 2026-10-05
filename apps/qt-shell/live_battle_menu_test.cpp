#include "live_menu_test.hpp"
#include "live_menu_session.hpp"
#include "window_host.hpp"
#include "spellbox_widget.hpp"
#include <QApplication>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QSlider>
#include <QListWidget>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QScreen>
#include <QLibrary>
#include <memory>
#include <cstdio>
void installLiveBattleMenuTest(QApplication& app,QMainWindow& window,LiveMenuSession& session,const QString& path){
    struct Evidence {int stage=0;bool valid=true,started=false,presented=false;int battles=0,returns=0;quint32 thread=0;MenuBridge::Battle initial;QJsonArray states;QJsonArray assignments,spellInitial,spellShelves;};
    auto e=std::make_shared<Evidence>();const bool spells=qEnvironmentVariable("MNM_LIVE_MENU_TEST_BATTLE")=="spells";
    const bool results=qEnvironmentVariable("MNM_LIVE_MENU_TEST_BATTLE")=="results";
    session.resultMenusEnabled=results;
    const bool repeat=qEnvironmentVariableIsSet("MNM_LIVE_MENU_TEST_REPEAT");
    auto input=[](uint8_t type,uint8_t detail,uint16_t state,QPoint point){
        WindowHost host;const auto desktops=host.desktops({});
        if(desktops.size()!=1)return false;
        const auto target=host.inputWindow(desktops.front(),QSize(800,600));
        Q_UNUSED(state);
        static QLibrary x11("X11"),xtst("Xtst");
        auto open=reinterpret_cast<void*(*)(const char*)>(x11.resolve("XOpenDisplay"));
        auto root=reinterpret_cast<unsigned long(*)(void*)>(x11.resolve("XDefaultRootWindow"));
        auto translate=reinterpret_cast<int(*)(void*,unsigned long,unsigned long,int,int,int*,int*,unsigned long*)>(x11.resolve("XTranslateCoordinates"));
        auto focus=reinterpret_cast<int(*)(void*,unsigned long,int,unsigned long)>(x11.resolve("XSetInputFocus"));
        auto flush=reinterpret_cast<int(*)(void*)>(x11.resolve("XFlush"));
        auto close=reinterpret_cast<int(*)(void*)>(x11.resolve("XCloseDisplay"));
        auto motion=reinterpret_cast<int(*)(void*,int,int,int,unsigned long)>(xtst.resolve("XTestFakeMotionEvent"));
        auto key=reinterpret_cast<int(*)(void*,unsigned int,int,unsigned long)>(xtst.resolve("XTestFakeKeyEvent"));
        auto button=reinterpret_cast<int(*)(void*,unsigned int,int,unsigned long)>(xtst.resolve("XTestFakeButtonEvent"));
        if(!target||!open||!root||!translate||!focus||!flush||!close||!motion||!key||!button)return false;
        void* display=open(nullptr);if(!display)return false;
        focus(display,target,2,0);int x=0,y=0;unsigned long child=0;
        bool ok=translate(display,target,root(display),point.x(),point.y(),&x,&y,&child);
        if(ok){
            if(type==XCB_MOTION_NOTIFY)ok=motion(display,-1,x,y,0);
            else if(type==XCB_KEY_PRESS||type==XCB_KEY_RELEASE)ok=key(display,detail,type==XCB_KEY_PRESS,0);
            else ok=button(display,detail,type==XCB_BUTTON_PRESS,0);
        }
        flush(display);close(display);return ok;
    };
    auto output=session.output;session.output=[output](const QString& text){if(output)output(text);std::fprintf(stderr,"%s",qPrintable(text));};
    auto failure=session.failed;session.failed=[failure,e](const QString& text){e->valid=false;if(failure)failure(text);std::fprintf(stderr,"Battle fallback: %s\n",qPrintable(text));};
    auto presentation=session.stateChanged;
    session.stateChanged=[presentation,e,&window,&session,path,spells,repeat,results,input](const MenuBridge::State& s){
        if(presentation){presentation(s);}
        if(!s.ready)return;
        const quint32 screens[]={3,22,14,22,14,25,14,25,14,14,14,14,results?26u:spells?7u:22u,results?26u:spells?22u:14u,22};
        if(e->stage>=(results?15:spells?14:repeat?15:12)||s.screen!=screens[e->stage]||s.ack!=quint32(e->stage))return;
        if((results&&e->stage==14)||(!results&&!spells&&(e->stage==12||e->stage==14))||(spells&&e->stage==13))++e->returns;
        if(e->thread&&e->thread!=s.thread){e->valid=false;}
        e->thread=s.thread;
        e->states.append(QJsonObject{{"screen",int(s.screen)},{"ack",int(s.ack)},{"generation",int(s.generation)},
            {"result_actions",int(s.results.actions)},{"map",int(s.battle.map)},{"map_name",s.battle.mapName},{"mana",s.battle.rules[0]},{"lives",s.battle.rules[8]},
            {"magic_items",s.battle.rules[2]},{"human_handicap",s.battle.players[0].handicap},{"human_name",s.battle.players[0].name},
            {"human_portrait",s.battle.players[0].portrait},{"human_colour",s.battle.players[0].colour},{"fourth_active",s.battle.players[3].active}});
        if(results&&s.screen==26){
            for(int i=0;i<4;++i){const auto& row=s.results.players[i];const QString cells[]={row.name,row.kills,row.deaths,row.handicap,row.score};
                for(int c=0;c<5;++c){auto* label=window.findChild<QLabel*>(QString("quickResultText%1").arg(7+i+c*4));e->valid&=label&&label->text()==(row.active?cells[c]:QString());}}
            QJsonArray rows;for(const auto& row:s.results.players)rows.append(QJsonObject{{"active",row.active},{"name",row.name},{"kills",row.kills},{"deaths",row.deaths},{"handicap",row.handicap},{"score",row.score}});auto obj=e->states.last().toObject();obj["results"]=rows;e->states.replace(e->states.size()-1,obj);}
        const int stage=e->stage++;
        if(spells&&stage==12){for(int item:s.spells.assignments)e->spellInitial.append(item);for(int item:s.spells.shelves)e->spellShelves.append(item);}
        if(stage==4)e->initial=s.battle;
        if(stage==5||stage==6||stage==7||stage==8){
            e->valid&=s.battle.rules[0]==150&&s.battle.rules[8]==3&&s.battle.players[0].handicap==10;
            if(!spells)e->valid&=s.battle.rules[2]==0;
        }
        if(stage==8)e->valid&=s.battle.map==2;
        if(stage==9)e->valid&=s.battle.players[0].portrait!=e->initial.players[0].portrait;
        if(stage==10)e->valid&=s.battle.players[0].colour!=e->initial.players[0].colour;
        if(stage==11)e->valid&=!s.battle.players[3].active;
        QTimer::singleShot(0,&window,[&window,&session,path,stage,e,spells,results,input]{
            window.grab().save(QFileInfo(path).dir().filePath(QString("qt-battle-stage-%1.png").arg(stage)));
            auto click=[&](const char* name){auto* b=window.findChild<QPushButton*>(name);if(!b||!b->isEnabled()){e->valid=false;return;}b->click();};
            if(results&&stage>=12){
                if(stage==12){
                    click("quickResultAction1");
                    QTimer::singleShot(3000,&window,[&window,path]{if(auto* screen=QGuiApplication::primaryScreen())screen->grabWindow(window.winId()).save(QFileInfo(path).dir().filePath("original-results-continue.png"));});
                    QTimer::singleShot(5000,&window,[input,e]{e->valid&=input(XCB_KEY_PRESS,9,0,{400,300})&&input(XCB_KEY_RELEASE,9,0,{400,300});});
                }
                if(stage==13){if(qEnvironmentVariable("MNM_LIVE_MENU_TEST_EXIT")=="results")window.close();else click("quickResultAction2");}
                if(stage==14)e->valid&=session.requestExit();
                return;
            }
            switch(stage){
            case 0:click("mainMenuAction2");break;
            case 1:case 3:click("quickBattleAction2");break;
            case 2:click("singlePlayerCancel");break;
            case 4:
                window.findChild<QSlider*>("singlePlayerSlider1")->setValue(150);
                window.findChild<QSlider*>("singlePlayerSlider9")->setValue(3);
                window.findChild<QSlider*>("singlePlayerSlider14")->setValue(10);
                if(!spells)window.findChild<QSlider*>("singlePlayerSlider3")->setValue(0);
                click("singlePlayerMap");break;
            case 5:click("mapSelectionCancel");break;
            case 6:click("singlePlayerMap");break;
            case 7:{auto* list=window.findChild<QListWidget*>("mapSelectionList");if(!list||list->count()<2){e->valid=false;return;}list->setCurrentRow(1);click("mapSelectionOk");break;}
            case 8:click("singlePlayerPortrait0");break;
            case 9:click("singlePlayerColour0");break;
            case 10:click("singlePlayerRemove3");break;
            case 11:click("singlePlayerStart");break;
            case 13:if(spells)e->valid&=session.requestExit();else click("singlePlayerStart");break;
            case 12:
                if(spells){
                    auto* w=window.findChild<SpellboxWidget*>("liveSpellSelection");
                    if(!w||w->inventory().items.isEmpty()||w->inventory().talismans.isEmpty()){e->valid=false;return;}
                    auto model=w->inventory();
                    e->valid&=w->assignItem(model.items[0].id,model.talismans[0].id);
                    click("spellboxCancel");
                    e->valid&=w->draftRequest().assignments[0].itemId==model.talismans[0].itemId;
                    for(const auto& t:model.talismans)e->valid&=w->removeItem(t.id);
                    for(int i=0;i<qMin(2,qMin(model.items.size(),model.talismans.size()));++i)
                        e->valid&=w->assignItem(model.items[i].id,model.talismans[i].id);
                    for(const auto& a:w->draftRequest().assignments)e->assignments.append(QJsonObject{{"slot",a.talismanId.toInt()},{"item",a.itemId.isEmpty()?-1:a.itemId.toInt()}});
                    click("spellboxOK");
                }else click("quickBattleAction2");break;
            case 14:e->valid&=session.requestExit();break;
            }
        });
    };
    auto handoff=session.battleStarted;
    session.battleStarted=[e,handoff,&window,path,spells,results,input](quint32 destination){
        if(handoff){handoff(destination);}
        e->started=true;++e->battles;e->valid&=destination==2u;
        for(int delay:{10000,25000,34000,45000})QTimer::singleShot(delay,&window,[e,&window,path,delay,cycle=e->battles]{
            auto* container=window.findChild<QWidget*>("legacyGameContainer");e->presented|=container&&container->isVisible();
            if(auto* screen=QGuiApplication::primaryScreen())screen->grabWindow(window.winId()).save(QFileInfo(path).dir().filePath(QString("original-battle-%1-%2.png").arg(cycle).arg(delay)));
        });
        {
            QTimer::singleShot(30000,&window,[input,e]{e->valid&=input(XCB_KEY_PRESS,9,0,{400,300});});
            QTimer::singleShot(30100,&window,[input,e]{e->valid&=input(XCB_KEY_RELEASE,9,0,{400,300});});
            if(!results)QTimer::singleShot(36000,&window,[input,e]{e->valid&=input(XCB_MOTION_NOTIFY,0,0,{400,300})&&input(XCB_BUTTON_PRESS,1,0,{400,300});});
            if(!results)QTimer::singleShot(36100,&window,[input,e]{e->valid&=input(XCB_BUTTON_RELEASE,1,0x100,{400,300});});
            if(!results)QTimer::singleShot(42000,&window,[input,e]{e->valid&=input(XCB_MOTION_NOTIFY,0,0,{537,550})&&input(XCB_BUTTON_PRESS,1,0,{537,550});});
            if(!results)QTimer::singleShot(42100,&window,[input,e]{e->valid&=input(XCB_BUTTON_RELEASE,1,0x100,{537,550});});
        }
    };
    auto finished=session.finished;
    session.finished=[e,finished,&app,path,spells,repeat,results]{
        if(finished){finished();}
        const bool success=e->valid&&e->stage==(results?15:spells?14:repeat?15:12)&&e->started&&e->presented&&(!spells||(e->battles==1&&e->returns==1))&&(!repeat||(e->battles==2&&e->returns==2));
        QFile file(path);bool saved=file.open(QIODevice::WriteOnly|QIODevice::NewOnly);
        if(saved)saved=file.write(QJsonDocument(QJsonObject{{"success",success},{"states",e->states},{"start_accepted",e->started},
            {"original_viewport_presented",e->presented},{"result_menu",results},{"spell_selection",spells},{"spell_assignments",e->assignments},{"spell_initial_assignments",e->spellInitial},{"spell_initial_shelves",e->spellShelves},{"battles",e->battles},{"native_returns",e->returns},{"scope","Engine setup/map/control callbacks, snapshot agreement, Start handoff; original viewport screenshots require inspection"}}).toJson())>0&&file.flush();
        app.exit(success&&saved?0:1);
    };
    session.winePrefix=QFileInfo(path).dir().filePath("wineprefix");session.smokeSeconds=(repeat||spells||results)?0:90;
    if(repeat||spells||results){
        auto launched=session.launched;
        session.launched=[launched,e,&app,&window]{
            if(launched)launched();
            QTimer::singleShot(180000,&window,[e,&app]{e->valid=false;std::fprintf(stderr,"Repeated battle deadline exceeded.\n");app.exit(1);});
        };
    }
    QTimer::singleShot(0,&window,[&window]{window.findChild<QPushButton*>("launchGame")->click();});
}
