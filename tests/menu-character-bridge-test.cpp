#include "live_character_controller.hpp"
#include "character-wire-fixture.hpp"
#include <QApplication>
#include <QPushButton>
#include <cstdio>
static void check(bool ok,const char* reason){CharacterWireFixture::check(ok,reason);}
static void transactions(quint32 parent){
    CharacterWireFixture peer;peer.begin(parent);
    LiveMenuSession session("/unused");session.characterMenusEnabled=true;peer.attach(session);
    CharacterScreenWidget widget;LiveCharacterController controller(session,widget);
    auto button=[&](const char* name){return widget.findChild<QPushButton*>(name);};
    auto adjustment=[&](int row,bool up){return widget.findChild<QPushButton*>(QString("characterAdjustment%1").arg(row*2+!up));};
    int presented=0,returned=0;
    session.stateChanged=[&](const auto& state){check(controller.present(state),"presentation");++presented;};
    session.originalViewportRequested=[&]{++returned;MenuBridge::State outside;check(controller.present(outside),"handoff reset");};
    peer.publish();LiveMenuSessionTestAccess::poll(session);check(presented==1&&widget.isEnabled(),"opening");
    for(int i=0;i<6;++i){
        const auto sequence=peer.word(28);adjustment(i,true)->click();
        check(!widget.isEnabled()&&widget.draftRequest().values[i]==int(CharacterWireFixture::initial[i]),"waiting has no optimistic edit");
        adjustment(i,true)->click();check(peer.word(28)==sequence+1,"duplicate click suppressed");
        check(peer.acknowledge(),"buy acknowledgement");LiveMenuSessionTestAccess::poll(session);
        check(widget.isEnabled()&&widget.draftRequest().values[i]==int(CharacterWireFixture::initial[i]+CharacterWireFixture::increment[i])&&widget.draftRequest().remainingExperience==39,"authoritative purchase");
        adjustment(i,false)->click();check(peer.acknowledge(),"refund acknowledgement");LiveMenuSessionTestAccess::poll(session);
        check(widget.draftRequest().values[i]==int(CharacterWireFixture::initial[i])&&widget.draftRequest().remainingExperience==50,"authoritative refund");
        check(!adjustment(i,false)->isEnabled(),"cannot refund baseline");
    }
    adjustment(0,true)->click();check(peer.acknowledge(),"cancel draft buy");LiveMenuSessionTestAccess::poll(session);
    button("characterCancel")->click();check(peer.acknowledge(),"Cancel acknowledgement");LiveMenuSessionTestAccess::poll(session);
    check(returned==1&&!widget.isEnabled()&&!session.request(MNM_MENU_CHARACTER_OK),"parent owns return");
    peer.begin(parent);LiveMenuSessionTestAccess::poll(session);
    check(widget.isEnabled()&&widget.draftRequest().values[0]==40&&widget.draftRequest().remainingExperience==50,"Cancel reopen rolled back");
    adjustment(0,true)->click();check(peer.acknowledge(),"commit draft buy");LiveMenuSessionTestAccess::poll(session);
    button("characterOk")->click();check(peer.acknowledge(),"OK acknowledgement");LiveMenuSessionTestAccess::poll(session);check(returned==2,"OK handoff");
    peer.begin(parent);LiveMenuSessionTestAccess::poll(session);
    check(widget.draftRequest().values[0]==45&&widget.draftRequest().remainingExperience==39&&!adjustment(0,false)->isEnabled(),"OK reopen uses new baseline");
}
static void acknowledgementRefusals(){
    for(const auto action:{MNM_MENU_CHARACTER_ADJUST,MNM_MENU_CHARACTER_OK,MNM_MENU_CHARACTER_CANCEL})for(bool wrongThread:{false,true}){
        CharacterWireFixture peer;LiveMenuSession session("/unused");session.characterMenusEnabled=true;peer.attach(session);
        int failures=0,returns=0;session.failed=[&](const auto&){++failures;};session.originalViewportRequested=[&]{++returns;};
        check(session.request(action,action==MNM_MENU_CHARACTER_ADJUST?1:0),"request before invalid acknowledgement");
        check(peer.acknowledge(),"fixture accepted");
        if(wrongThread)peer.put(152,99);else peer.put(144,peer.word(144)+1);
        peer.put(128,peer.word(128)+2);LiveMenuSessionTestAccess::poll(session);
        check(failures==1&&!returns&&!session.request(action),"wrong request/thread fails closed");
    }
    CharacterWireFixture peer;LiveMenuSession session("/unused");session.characterMenusEnabled=true;peer.attach(session);
    check(session.request(MNM_MENU_CHARACTER_ADJUST,1),"unacknowledged request");
    peer.publish();LiveMenuSessionTestAccess::poll(session);
    check(!session.request(MNM_MENU_CHARACTER_OK)&&peer.word(28)==1,"unchanged acknowledgement stays pending");
}
static void wireBounds(){
    CharacterWireFixture peer;LiveMenuSession session("/unused");session.characterMenusEnabled=true;peer.attach(session);
    const int at=MNM_MENU_V18_CHARACTER;
    for(int i=0;i<8;++i){const auto saved=peer.word(at+i*4);peer.put(at+i*4,i==4?1000000001:i==6?4096:99);
        check(!session.request(MNM_MENU_CHARACTER_OK),"malformed wire refuses action");peer.put(at+i*4,saved);}
    peer.put(at+208,1);check(!session.request(MNM_MENU_CHARACTER_OK),"reserved data refused");peer.put(at+208,0);
    peer.put(at+36,201);check(!session.request(MNM_MENU_CHARACTER_OK),"value beyond maximum refused");peer.put(at+36,40);
    check(!session.request(MNM_MENU_CHARACTER_ADJUST,0)&&!session.request(MNM_MENU_CHARACTER_ADJUST,12)&&!session.request(MNM_MENU_CHARACTER_OK,1),"action bounds");
    peer.begin();check(!session.request(MNM_MENU_CHARACTER_ADJUST,1),"stale generation refused");
    QTemporaryDir dir;MenuBridge old;check(old.create(dir.filePath("old.bin"),true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true),"V17 compatibility create");
}
int main(int argc,char** argv){
    QApplication app(argc,argv);
    try{wireBounds();transactions(4);transactions(18);acknowledgementRefusals();
        std::puts("Character bounds, six-row transactions, both caller reopen journeys and exact acknowledgements passed");return 0;
    }catch(const std::exception& error){std::fprintf(stderr,"Character bridge: %s\n",error.what());return 1;}
}
