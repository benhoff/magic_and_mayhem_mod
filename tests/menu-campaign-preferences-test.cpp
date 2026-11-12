#include "live_preferences_menu_controller.hpp"
#include "live_mini_menu_controller.hpp"
#include "../protocols/include/mnm/menu_v12.h"
#include <QApplication>
#include <QTemporaryDir>
#include <QDir>
#include <QFileInfo>
#include <QPushButton>
#include <QSlider>
#include <QEventLoop>
#include <QTimer>
#include <QElapsedTimer>
#include <QtEndian>
#include <future>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
static void check(bool v,int line){if(!v){std::fprintf(stderr,"Campaign Preferences failed at %d\n",line);std::exit(1);}}
#define require(v) check((v),__LINE__)
struct LiveMenuSessionTestAccess {
 static MenuBridge& initialize(LiveMenuSession& s,const QString& path){s.bridge_=std::make_unique<MenuBridge>();require(s.bridge_->create(path,false,false,false,false,false,false,false,false,false,false,true));s.root_=QFileInfo(path).absolutePath();s.preferencesStore_.begin(s.root_+"/accepted.json","missing");s.active_=true;s.clock_.start();return *s.bridge_;}
 static void poll(LiveMenuSession& s){s.poll();}
 static void persist(LiveMenuSession& s){s.preferencesStore_.wait();}
 static bool gameplay(const LiveMenuSession& s){return s.inBattle_;}
 static bool active(const LiveMenuSession& s){return s.active_;}
};
static void delayedPersistence(){
 using namespace std::chrono_literals;
 QTemporaryDir root;const auto path=root.filePath("channel.bin");EnginePreferencesStore disk;disk.begin(root.filePath("accepted.json"),"missing");
 std::promise<void> blocked,allowed;auto entered=blocked.get_future();auto gate=allowed.get_future().share();
 LiveMenuSession session("/unused",nullptr,[&](const QByteArray& bytes,const auto& expected,QString* error){blocked.set_value();gate.wait();return disk.acceptSnapshot(bytes,expected,error);});
 LiveMenuSessionTestAccess::initialize(session,path);
 QFile peer(path);require(peer.open(QIODevice::ReadWrite));auto* bytes=peer.map(0,MNM_MENU_V12_SIZE);require(bytes);
 auto put=[&](int word,quint32 value){qToLittleEndian(value,bytes+word*4);};quint32 seq=0;
 auto state=[&](quint32 gen,quint32 ack,int screen){put(32,++seq);put(33,gen);put(34,screen);put(35,1);put(36,ack);put(37,MNM_MENU_OK);put(38,123);put(39,0);
  if(screen==10){const quint32 values[]={3,16383,3,3,0,quint32(-500),0,0,2,0,0,0};for(int i=0;i<12;++i)put(MNM_MENU_V6_PREFERENCES/4+i,values[i]);}put(32,++seq);};
 unsigned failures=0,saved=0;session.failed=[&](const QString&){++failures;};session.output=[&](const QString& message){if(message.contains("saved for future"))++saved;};
 state(1,0,3);LiveMenuSessionTestAccess::poll(session);require(session.request(MNM_MENU_OPEN_PREFERENCES));
 state(2,1,10);LiveMenuSessionTestAccess::poll(session);const std::array<int,7> values{0,-500,0,0,2,0,0};require(session.requestPreferences(MNM_MENU_PREFERENCES_OK,values));
 QDir().mkpath(root.filePath("game/CFG"));QFile profile(root.filePath("game/CFG/prefs.cfg"));require(profile.open(QIODevice::WriteOnly));
 profile.write("[VIDEO]\nIsHighRes=TRUE\nCutDownAnims=FALSE\nDialogSpeed=2\nMaxFramesPerSec=20\nWindowSize=0\n[SOUND]\nMusicVolume=0\nSFXVolume=-500\n");profile.close();
 state(3,2,3);LiveMenuSessionTestAccess::poll(session);require(entered.wait_for(1s)==std::future_status::ready&&!QFile::exists(root.filePath("accepted.json")));
 QEventLoop loop;QTimer heartbeat;heartbeat.setInterval(20);unsigned pulses=0;bool released=false,deadline=false;
 QObject::connect(&heartbeat,&QTimer::timeout,&loop,[&]{++pulses;state(3,2,3);LiveMenuSessionTestAccess::poll(session);if(saved)loop.quit();});
 QTimer::singleShot(MNM_MENU_V1_LEASE_MS+300,&loop,[&]{released=true;allowed.set_value();});
 QTimer::singleShot(MNM_MENU_V1_LEASE_MS+2500,&loop,[&]{deadline=true;if(!released){released=true;allowed.set_value();}loop.quit();});
 QElapsedTimer clock;clock.start();heartbeat.start();loop.exec();heartbeat.stop();LiveMenuSessionTestAccess::persist(session);
 require(!deadline&&clock.elapsed()>=MNM_MENU_V1_LEASE_MS&&pulses>20&&saved==1&&failures==0&&LiveMenuSessionTestAccess::active(session));
 require(QFile::exists(root.filePath("accepted.json")));peer.unmap(bytes);
 std::printf("Delayed preference persistence preserves GUI/menu heartbeat: %u ticks\n",pulses);
}
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
 require(session.requestMini(MNM_MENU_MINI_PREFERENCES));state(9,4,10,true);LiveMenuSessionTestAccess::poll(session);require(native==5&&viewport==2&&!LiveMenuSessionTestAccess::gameplay(session));
 MenuBridge::State s;require(bridge.read(s));put(MNM_MENU_V6_PREFERENCES/4+3,5);require(!bridge.read(s));put(MNM_MENU_V6_PREFERENCES/4+3,6);require(bridge.read(s));
 session.campaignPreferencesEnabled=false;QString error;require(!controller.present(s,&error));session.campaignPreferencesEnabled=true;
 // Restrict OK while preserving Cancel and audio availability from the engine.
 put(MNM_MENU_V6_PREFERENCES/4,2);put(MNM_MENU_V6_PREFERENCES/4+1,16383^(1u<<12));require(bridge.read(s)&&controller.present(s));require(!preferences.findChild<QPushButton*>("preferencesOk")->isEnabled());require(!preferences.findChild<QSlider*>("preferencesSlider1")->isEnabled());
 require(!QFile::exists(root.filePath("accepted.json")));
 state(11,4,10,true);LiveMenuSessionTestAccess::poll(session);require(session.requestPreferences(MNM_MENU_PREFERENCES_OK,values));
 require(!QFile::exists(root.filePath("accepted.json")));QDir().mkpath(root.filePath("game/CFG"));QFile cfg(root.filePath("game/CFG/prefs.cfg"));require(cfg.open(QIODevice::WriteOnly));cfg.write("[VIDEO]\nIsHighRes=TRUE\nCutDownAnims=FALSE\nDialogSpeed=2\nMaxFramesPerSec=20\nWindowSize=0\n[SOUND]\nMusicVolume=0\nSFXVolume=-500\n");cfg.close();
 state(12,5,10,false);LiveMenuSessionTestAccess::poll(session);LiveMenuSessionTestAccess::persist(session);require(viewport==3&&LiveMenuSessionTestAccess::gameplay(session)&&QFile::exists(root.filePath("accepted.json")));
 // Original-controlled entry can also publish a fresh Preferences visit without
 // a native Mini request. Keep that previously supported same-owner admission.
 state(13,5,2,false);LiveMenuSessionTestAccess::poll(session);
 state(14,5,10,true);LiveMenuSessionTestAccess::poll(session);require(native==7&&!LiveMenuSessionTestAccess::gameplay(session));
 // A ready opening ACK cannot acquire ownership under a different request
 // sequence or menu thread, even when its Preferences payload is otherwise valid.
 for(int invalid=0;invalid<2;++invalid){
  LiveMenuSession rejected("/unused");auto rejectedPath=root.filePath(QString("rejected-%1.bin").arg(invalid));LiveMenuSessionTestAccess::initialize(rejected,rejectedPath);
  QFile badPeer(rejectedPath);require(badPeer.open(QIODevice::ReadWrite));auto* bad=badPeer.map(0,MNM_MENU_V12_SIZE);require(bad);
  std::memcpy(bad+128,bytes+128,MNM_MENU_V12_SIZE-128);
  auto word=[&](int at,quint32 value){qToLittleEndian(value,bad+at*4);};
  word(32,2);word(33,1);word(34,17);word(35,1);word(36,0);word(37,MNM_MENU_OK);word(38,123);word(39,0);
  int failures=0,presentations=0;rejected.failed=[&](const QString& reason){require(reason.contains("acknowledgement identity"));++failures;};rejected.stateChanged=[&](const auto& ready){if(ready.ready)++presentations;};
  LiveMenuSessionTestAccess::poll(rejected);require(rejected.requestMini(MNM_MENU_MINI_PREFERENCES));
  word(32,4);word(33,2);word(34,10);word(35,1);word(36,invalid?1:2);word(38,invalid?124:123);
  LiveMenuSessionTestAccess::poll(rejected);require(failures==1&&presentations==1&&!LiveMenuSessionTestAccess::gameplay(rejected));badPeer.unmap(bad);
 }
 peer.unmap(bytes);delayedPersistence();std::puts("Campaign Preferences bridge/controller/session checks passed");
}
