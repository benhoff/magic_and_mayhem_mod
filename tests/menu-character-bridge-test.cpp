#include "live_character_controller.hpp"
#include "../../protocols/include/mnm/menu_v18.h"
#include <QApplication>
#include <QTemporaryDir>
#include <QPushButton>
#include <QtEndian>
#include <cstdio>
#include <cstdlib>
#define require(ok) do{if(!(ok)){std::fprintf(stderr,"Character bridge failed at%d\n",__LINE__);std::exit(1);}}while(0)
struct LiveMenuSessionTestAccess {
    static void attach(LiveMenuSession& s,std::unique_ptr<MenuBridge> b,const MenuBridge::State& state){
        s.bridge_=std::move(b);s.active_=true;s.clock_.start();s.state_=state;s.sequence_=state.sequence;s.lastState_=0;
    }
    static void poll(LiveMenuSession& s){s.poll();}
};
int main(int argc,char** argv){
    QApplication app(argc,argv);QTemporaryDir dir;const auto path=dir.filePath("character.bin");
    auto bridge=std::make_unique<MenuBridge>();require(bridge->create(path,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true));
    QFile file(path);require(file.open(QIODevice::ReadWrite)&&file.size()==MNM_MENU_V18_SIZE);auto* bytes=file.map(0,file.size());require(bytes);
    auto put=[&](int at,quint32 n){qToLittleEndian(n,bytes+at);};const int at=MNM_MENU_V18_CHARACTER;
    put(128,2);put(132,7);put(136,20);put(140,1);put(148,MNM_MENU_OK);put(152,42);
    const quint32 fields[]={4,4,5,0,50,3,2730,0};for(int i=0;i<8;++i)put(at+4*i,fields[i]);
    const quint32 values[]={40,160,4,2,2,2},maximum[]={200,800,40,7,7,7},steps[]={5,20,1,1,1,1};
    for(int i=0;i<6;++i){const quint32 row[]={values[i],values[i],maximum[i],steps[i],7,11};for(int j=0;j<6;++j)put(at+32+i*24+j*4,row[j]);}
    bytes[at+176]='C';MenuBridge::State state;require(bridge->read(state)&&state.character.name=="C");
    auto stale=state;--stale.generation;require(!bridge->request(MNM_MENU_CHARACTER_ADJUST,stale,1));
    require(!bridge->request(MNM_MENU_CHARACTER_ADJUST,state,0)&&!bridge->request(MNM_MENU_CHARACTER_ADJUST,state,12));
    require(!bridge->request(MNM_MENU_CHARACTER_OK,state,1));
    for(int i=0;i<8;++i){put(at+4*i,i==4?1000000001:i==6?4096:99);require(!bridge->read(state));put(at+4*i,fields[i]);}
    put(at+208,1);require(!bridge->read(state));put(at+208,0);
    put(at+36,201);require(!bridge->read(state));put(at+36,40);require(bridge->read(state));
    LiveMenuSession session("/unused");session.characterMenusEnabled=true;
    LiveMenuSessionTestAccess::attach(session,std::move(bridge),state);
    CharacterScreenWidget widget;LiveCharacterController controller(session,widget);require(controller.present(state));
    require(widget.draftRequest().values[0]==40&&widget.draftRequest().remainingExperience==50);
    widget.findChild<QPushButton*>("characterAdjustment0")->click();
    require(widget.draftRequest().values[0]==40&&widget.draftRequest().remainingExperience==50); // No optimistic edit.
    require(qFromLittleEndian<quint32>(bytes+32)==MNM_MENU_CHARACTER_ADJUST&&qFromLittleEndian<quint32>(bytes+40)==1);
    widget.findChild<QPushButton*>("characterAdjustment0")->click();require(qFromLittleEndian<quint32>(bytes+28)==1); // Once only while pending.
    put(144,1);put(132,8);put(128,4);put(at+16,39);put(at+36,45);put(at+24,2731);
    session.stateChanged=[&](const auto& next){require(controller.present(next));};LiveMenuSessionTestAccess::poll(session);
    require(widget.draftRequest().values[0]==45&&widget.draftRequest().remainingExperience==39);
    widget.findChild<QPushButton*>("characterAdjustment1")->click();require(qFromLittleEndian<quint32>(bytes+28)==2&&qFromLittleEndian<quint32>(bytes+40)==0);
    put(144,2);put(132,9);put(128,6);put(at+16,50);put(at+36,40);put(at+24,2730);LiveMenuSessionTestAccess::poll(session);
    require(widget.draftRequest().values[0]==40&&widget.draftRequest().remainingExperience==50);
    bool returned=false;session.originalViewportRequested=[&]{returned=true;};widget.findChild<QPushButton*>("characterCancel")->click();
    require(qFromLittleEndian<quint32>(bytes+32)==MNM_MENU_CHARACTER_CANCEL);put(144,3);put(132,10);put(128,8);put(136,0);put(140,0);LiveMenuSessionTestAccess::poll(session);require(returned);
    MenuBridge old;require(old.create(dir.filePath("old.bin"),true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true));
    file.unmap(bytes);std::puts("Character wire bounds, authoritative UI, stale/once-only requests and caller handoff passed");
}
