#include "live_storage_menu_controller.hpp"
#include "../protocols/include/mnm/menu_v13.h"
#include <QApplication>
#include <QTemporaryDir>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QtEndian>
#include <cstdio>
#include <cstdlib>
#include <cstring>
static void check(bool ok,int line){if(!ok){std::fprintf(stderr,"Storage bridge failed at %d\n",line);std::exit(1);}}
#define require(v) check((v),__LINE__)
struct LiveMenuSessionTestAccess {
 static MenuBridge& initialize(LiveMenuSession& s,const QString& path){s.bridge_=std::make_unique<MenuBridge>();require(s.bridge_->create(path,false,false,false,false,false,false,false,false,false,false,false,true));s.active_=true;s.clock_.start();return *s.bridge_;}
 static void poll(LiveMenuSession& s){s.poll();}
 static bool original(const LiveMenuSession& s){return s.inBattle_;}
};
int main(int argc,char** argv){
 QApplication app(argc,argv);QTemporaryDir root;LiveMenuSession session("/unused");const auto path=root.filePath("channel.bin");auto& bridge=LiveMenuSessionTestAccess::initialize(session,path);
 QFile peer(path);require(peer.open(QIODevice::ReadWrite));auto* bytes=peer.map(0,MNM_MENU_V13_SIZE);require(bytes);
 auto put=[&](int word,quint32 v){qToLittleEndian(v,bytes+4*word);};auto get=[&](int word){return qFromLittleEndian<quint32>(bytes+4*word);};quint32 sequence=0;
 auto state=[&](quint32 generation,quint32 ack,quint32 screen,bool ready,quint32 modal=0){
  put(32,++sequence);put(33,generation);put(34,screen);put(35,ready);put(36,ack);put(37,MNM_MENU_OK);put(38,123);put(39,0);
  if(screen==MNM_MENU_LOAD_SCREEN||screen==13){auto* m=bytes+MNM_MENU_V13_STORAGE;std::memset(m,0,MNM_MENU_V13_STORAGE_SIZE);const quint32 fields[]={screen,screen==MNM_MENU_LOAD_SCREEN?3u:17u,screen==MNM_MENU_LOAD_SCREEN?3u:6u,5,modal,0,2,2};for(int i=0;i<8;++i)put(MNM_MENU_V13_STORAGE/4+i,fields[i]);std::memcpy(m+32,"second",7);std::memcpy(m+160,"first",6);std::memcpy(m+288,"second",7);}
  put(32,++sequence);
 };
 LoadGameWidget load;SaveGameWidget save;LiveStorageMenuController controller(session,load,save);int native=0,viewport=0;
 session.stateChanged=[&](const auto& s){require(controller.present(s));if(s.ready)++native;};session.originalViewportRequested=[&]{++viewport;};
 state(1,0,3,true);LiveMenuSessionTestAccess::poll(session);require(session.request(MNM_MENU_OPEN_LOAD));require(!session.request(MNM_MENU_OPEN_LOAD));
 state(2,1,MNM_MENU_LOAD_SCREEN,true);MenuBridge::State probe;require(bridge.read(probe));require(probe.storage.selected==2&&probe.storage.names.size()==2);LiveMenuSessionTestAccess::poll(session);require(load.selectedSaveId()=="2");
 load.findChild<QPushButton*>("loadGameLoad")->click();require(get(8)==MNM_MENU_STORAGE_ACCEPT&&get(10)==2);
 state(3,2,MNM_MENU_LOAD_SCREEN,false);LiveMenuSessionTestAccess::poll(session);require(viewport==1&&LiveMenuSessionTestAccess::original(session));require(!session.requestStorage(MNM_MENU_STORAGE_ACCEPT,1));
 state(4,2,MNM_MENU_LOAD_SCREEN,true);LiveMenuSessionTestAccess::poll(session);require(!LiveMenuSessionTestAccess::original(session));
 MenuBridge::State observed;require(bridge.read(observed));auto stale=observed;--stale.generation;require(!bridge.requestStorage(MNM_MENU_STORAGE_ACCEPT,stale,1));
 require(!bridge.requestStorage(MNM_MENU_STORAGE_DELETE,observed,1)&&!bridge.requestStorage(MNM_MENU_STORAGE_ACCEPT,observed,3)&&!bridge.requestStorage(MNM_MENU_STORAGE_ACCEPT,observed,1,"first"));
 const int base=MNM_MENU_V13_STORAGE/4;put(base+6,101);require(!bridge.read(observed));put(base+6,2);put(base+7,3);require(!bridge.read(observed));put(base+7,2);
 put(base+2,4);require(!bridge.read(observed));put(base+2,3);put(base+4,1);require(!bridge.read(observed));put(base+4,0);
 state(5,2,13,true);LiveMenuSessionTestAccess::poll(session);auto* edit=save.findChild<QLineEdit*>("saveGameFileName");edit->setText("draft");state(5,2,13,true);LiveMenuSessionTestAccess::poll(session);require(edit->text()=="draft");
 require(bridge.read(observed));for(const auto& invalid:{QString(),QString("../escape"),QString("CON"),QString("com1.sav"),QString("trailing."),QString(" spaced"),QString("bad:stream"),QString(128,'a'),QString::fromUtf8("caf\xc3\xa9")})require(!bridge.requestStorage(MNM_MENU_STORAGE_ACCEPT,observed,0,invalid));
 require(session.requestStorage(MNM_MENU_STORAGE_ACCEPT,0,"new-save"));require(std::strcmp(reinterpret_cast<char*>(bytes+MNM_MENU_V13_HOST_NAME),"new-save")==0);
 state(6,3,13,false,1);LiveMenuSessionTestAccess::poll(session);require(viewport==2&&LiveMenuSessionTestAccess::original(session));
 state(7,3,13,true);LiveMenuSessionTestAccess::poll(session);require(!LiveMenuSessionTestAccess::original(session)&&edit->text()=="second");
 require(session.requestStorage(MNM_MENU_STORAGE_DELETE,2));state(8,4,13,true);LiveMenuSessionTestAccess::poll(session);require(get(8)==MNM_MENU_STORAGE_DELETE&&get(10)==2);
 require(session.requestStorage(MNM_MENU_STORAGE_CANCEL));state(9,5,13,false);LiveMenuSessionTestAccess::poll(session);state(10,5,3,true);LiveMenuSessionTestAccess::poll(session);require(!LiveMenuSessionTestAccess::original(session));
 bridge.retire();require(!session.request(MNM_MENU_OPEN_LOAD));require(native>=6);peer.unmap(bytes);std::puts("Storage wire/controller/session transaction guards passed");return 0;
}
