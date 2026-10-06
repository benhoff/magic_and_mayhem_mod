#include "live_preferences_menu_controller.hpp"
#include "live_mini_menu_controller.hpp"
#include "../protocols/include/mnm/menu_v12.h"
#include <QApplication>
#include <QTemporaryDir>
#include <QDir>
#include <QFileInfo>
#include <QPushButton>
#include <QSlider>
#include <QtEndian>
#include <cstdio>
#include <cstdlib>
static void check(bool v,int line){if(!v){std::fprintf(stderr,"Campaign Preferences failed at %d\n",line);std::exit(1);}}
#define require(v) check((v),__LINE__)
struct LiveMenuSessionTestAccess {
 static MenuBridge& initialize(LiveMenuSession& s,const QString& path){s.bridge_=std::make_unique<MenuBridge>();require(s.bridge_->create(path,false,false,false,false,false,false,false,false,false,false,true));s.root_=QFileInfo(path).absolutePath();s.preferencesStore_.begin(s.root_+"/accepted.json","missing");s.active_=true;s.clock_.start();return *s.bridge_;}
 static void poll(LiveMenuSession& s){s.poll();}
 static bool gameplay(const LiveMenuSession& s){return s.inBattle_;}
};
int main(int argc,char** argv){
 QApplication app(argc,argv);QTemporaryDir root;LiveMenuSession session("/unused");auto path=root.filePath("channel.bin");auto& bridge=LiveMenuSessionTestAccess::initialize(session,path);
 QFile peer(path);require(peer.open(QIODevice::ReadWrite));auto* bytes=peer.map(0,MNM_MENU_V12_SIZE);require(bytes);
 auto put=[&](int word,quint32 v){qToLittleEndian(v,bytes+word*4);};quint32 seq=0;
 auto state=[&](quint32 gen,quint32 ack,int screen,bool ready,quint32 thread=123){put(32,++seq);put(33,gen);put(34,screen);put(35,ready);put(36,ack);put(37,MNM_MENU_OK);put(38,thread);put(39,0);
  if(screen==17){int m=MNM_MENU_V4_MINI/4;const quint32 words[]={0,0,5,2,7,5,2,0};for(int i=0;i<8;++i)put(m+i,words[i]);}
  if(screen==10){int m=MNM_MENU_V6_PREFERENCES/4;const quint32 words[]={3,16383,17,6,0,quint32(-250),0,0,1,0,0,0};for(int i=0;i<12;++i)put(m+i,words[i]);}put(32,++seq);};
 PreferencesWidget preferences;LivePreferencesMenuController controller(session,preferences);MiniMenuWidget mini;LiveMiniMenuController miniController(session,mini);int native=0,viewport=0;
 session.stateChanged=[&](const auto& s){if(s.ready)++native;require(controller.present(s));require(miniController.present(s,nullptr));};session.originalViewportRequested=[&]{++viewport;};
 state(1,0,17,true);LiveMenuSessionTestAccess::poll(session);require(mini.findChild<QPushButton*>("miniMenuButton3")->isEnabled());require(session.requestMini(MNM_MENU_MINI_PREFERENCES));
 state(2,1,17,false);LiveMenuSessionTestAccess::poll(session);require(viewport==1&&LiveMenuSessionTestAccess::gameplay(session));
 state(3,1,10,true,124);LiveMenuSessionTestAccess::poll(session);require(native==1&&LiveMenuSessionTestAccess::gameplay(session));
 state(3,1,10,true);LiveMenuSessionTestAccess::poll(session);require(native==2&&!LiveMenuSessionTestAccess::gameplay(session));
 auto values=std::array<int,7>{0,-500,0,0,2,0,0};require(session.requestPreferences(MNM_MENU_PREFERENCES_PREVIEW,values,1));state(4,2,10,true);put(MNM_MENU_V6_PREFERENCES/4+5,quint32(-500));LiveMenuSessionTestAccess::poll(session);require(native==3&&viewport==1);
 require(session.requestPreferences(MNM_MENU_PREFERENCES_CANCEL,values));state(5,3,10,false);LiveMenuSessionTestAccess::poll(session);require(viewport==2&&LiveMenuSessionTestAccess::gameplay(session));
 state(6,3,17,false);LiveMenuSessionTestAccess::poll(session);require(native==3);state(7,3,2,false);LiveMenuSessionTestAccess::poll(session);require(native==3);state(8,3,17,true);LiveMenuSessionTestAccess::poll(session);require(native==4&&!LiveMenuSessionTestAccess::gameplay(session));
 require(session.requestMini(MNM_MENU_MINI_PREFERENCES));state(9,4,17,false);LiveMenuSessionTestAccess::poll(session);state(10,4,10,true);LiveMenuSessionTestAccess::poll(session);require(native==5);
 MenuBridge::State s;require(bridge.read(s));put(MNM_MENU_V6_PREFERENCES/4+3,5);require(!bridge.read(s));put(MNM_MENU_V6_PREFERENCES/4+3,6);require(bridge.read(s));
 session.campaignPreferencesEnabled=false;QString error;require(!controller.present(s,&error));session.campaignPreferencesEnabled=true;
 // Restrict OK while preserving Cancel and audio availability from the engine.
 put(MNM_MENU_V6_PREFERENCES/4,2);put(MNM_MENU_V6_PREFERENCES/4+1,16383^(1u<<12));require(bridge.read(s)&&controller.present(s));require(!preferences.findChild<QPushButton*>("preferencesOk")->isEnabled());require(!preferences.findChild<QSlider*>("preferencesSlider1")->isEnabled());
 require(!QFile::exists(root.filePath("accepted.json")));
 state(11,4,10,true);LiveMenuSessionTestAccess::poll(session);require(session.requestPreferences(MNM_MENU_PREFERENCES_OK,values));
 require(!QFile::exists(root.filePath("accepted.json")));QDir().mkpath(root.filePath("game/CFG"));QFile cfg(root.filePath("game/CFG/prefs.cfg"));require(cfg.open(QIODevice::WriteOnly));cfg.write("[VIDEO]\nIsHighRes=TRUE\nCutDownAnims=FALSE\nDialogSpeed=2\nMaxFramesPerSec=20\nWindowSize=0\n[SOUND]\nMusicVolume=0\nSFXVolume=-500\n");cfg.close();
 state(12,5,10,false);LiveMenuSessionTestAccess::poll(session);require(viewport==4&&LiveMenuSessionTestAccess::gameplay(session)&&QFile::exists(root.filePath("accepted.json")));
 peer.unmap(bytes);std::puts("Campaign Preferences bridge/controller/session checks passed");
}
