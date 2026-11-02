#include "campaign-menu-wire-fixture.hpp"
#include "live_region_entry_controller.hpp"
#include <QApplication>
#include <QPushButton>
#include <iostream>
static void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
static void enable(LiveMenuSession& s){s.progressedMenusEnabled=s.realmMenusEnabled=s.characterMenusEnabled=true;}
int main(int argc,char** argv)try{
    QApplication app(argc,argv);unsigned cases=0;
    for(const QByteArray& name:{QByteArray("Celtic"),QByteArray("Greek"),QByteArray("Medieval")}){
        CampaignMenuWireFixture peer;peer.region(name);LiveMenuSession session("/unused");enable(session);peer.attach(session);
        RegionEntryWidget widget;LiveRegionEntryController controller(session,widget);int failures=0,original=0;
        session.failed=[&](const auto&){++failures;};session.originalViewportRequested=[&]{++original;};
        session.stateChanged=[&](const auto& state){widget.setEnabled(state.screen==18&&state.ready);QString error;check(controller.present(state,&error),"loaded Region presentation");};
        peer.publish();LiveMenuSessionTestAccess::poll(session);widget.show();app.processEvents();
        check(widget.region().id==QString::fromLatin1(name)+":3"&&widget.region().artworkNumber==3&&widget.region().auxiliaryAvailable==std::array<bool,3>{true,true,true},"engine region and auxiliary model");
        for(auto action:{MNM_MENU_REGION_GRIMOIRE,MNM_MENU_REGION_SPELLBOX,MNM_MENU_REGION_CHARACTER}){
            const quint32 sequence=peer.word(28);
            widget.auxiliaryRequested(action==MNM_MENU_REGION_GRIMOIRE?RegionEntryWidget::AuxiliaryAction::Grimoire:action==MNM_MENU_REGION_SPELLBOX?RegionEntryWidget::AuxiliaryAction::Spellbox:RegionEntryWidget::AuxiliaryAction::Character);
            check(peer.word(28)==sequence+1&&peer.word(32)==quint32(action),"controller auxiliary request");peer.original();peer.acknowledge();LiveMenuSessionTestAccess::poll(session);
            peer.region(name);LiveMenuSessionTestAccess::poll(session);check(widget.isEnabled()&&!failures,"fresh Region return");
        }
        check(original==3&&session.request(MNM_MENU_REGION_CANCEL),"all three caller handoffs and Cancel");peer.original();peer.acknowledge();LiveMenuSessionTestAccess::poll(session);
        peer.realm();LiveMenuSessionTestAccess::poll(session);check(!failures&&session.request(MNM_MENU_REALM_ACTIVATE,1),"different-region Cancel returns to native Realm");++cases;
    }
    for(quint32 handoff:{3u,4u}){
        CampaignMenuWireFixture peer;peer.region("Celtic",3,handoff==4,31,handoff);LiveMenuSession session("/unused");enable(session);peer.attach(session);
        int failures=0,battles=0,original=0;session.failed=[&](const auto&){++failures;};session.battleStarted=[&](quint32 value){check(value==3,"battle handoff value");++battles;};session.originalViewportRequested=[&]{++original;};
        check(session.request(MNM_MENU_REGION_ENTER),"Enter requested");peer.original(handoff);peer.acknowledge();LiveMenuSessionTestAccess::poll(session);
        check(!failures&&battles==int(handoff==3)&&original==int(handoff==4),"battle differs from campaign return");
        if(handoff==4){peer.realm();LiveMenuSessionTestAccess::poll(session);check(session.request(MNM_MENU_REALM_ACTIVATE,1),"destination return restores Realm");}++cases;
    }
    for(bool wrongThread:{false,true}){
        CampaignMenuWireFixture peer;LiveMenuSession session("/unused");enable(session);peer.attach(session);int failures=0;session.failed=[&](const auto&){++failures;};
        check(session.request(MNM_MENU_REGION_CHARACTER),"request for bad ACK");peer.original();peer.acknowledge();peer.put(wrongThread?152:144,wrongThread?10:2);LiveMenuSessionTestAccess::poll(session);
        check(failures==1,"bad auxiliary ACK identity refused");++cases;
    }
    for(int mutation=0;mutation<5;++mutation){
        CampaignMenuWireFixture peer;peer.region("Celtic",3,true,31,mutation==3?3:4);
        const auto opening=peer.word(132);LiveMenuSession session("/unused");enable(session);peer.attach(session);
        int failures=0,battles=0,original=0,returned=0;
        session.failed=[&](const auto&){++failures;};session.battleStarted=[&](quint32){++battles;};session.originalViewportRequested=[&]{++original;};
        session.stateChanged=[&](const auto& s){if(s.screen==4&&s.ready)++returned;};
        check(session.request(MNM_MENU_REGION_ENTER),"fast completed parent request");
        peer.realm();peer.put(MNM_MENU_V19_REALM+12,mutation==2?2:1);peer.put(MNM_MENU_V19_REALM+28,mutation==4?2:3);peer.acknowledge();
        if(mutation==1)peer.put(132,opening);
        LiveMenuSessionTestAccess::poll(session);
        if(!mutation){check(!failures&&!battles&&!original&&returned==1,"already-ready same Realm consumes Enter ACK without World or repeat");
            check(session.request(MNM_MENU_REALM_CHARACTER),"completed parent restores command ownership");}
        else check(failures==1&&!battles&&!returned,"stale/wrong Realm or handoff/destination refused");
        ++cases;
    }
    for(const auto& mutation:std::array<std::pair<int,quint32>,6>{{{0,22},{4,5},{8,2},{24,20},{28,2},{20,32}}}){
        CampaignMenuWireFixture peer;MenuBridge::State state;check(peer.bridge().read(state),"valid Region before mutation");peer.put(MNM_MENU_V7_REGION+mutation.first,mutation.second);peer.publish();check(!peer.bridge().read(state),"invalid V20 Region wire");++cases;
    }
    for(bool mainReturn:{false,true}){
        CampaignMenuWireFixture peer;peer.realm();LiveMenuSession session("/unused");enable(session);peer.attach(session);
        int original=0,failures=0;session.originalViewportRequested=[&]{++original;};session.failed=[&](const auto&){++failures;};
        check(session.beginOriginalRealmControls()&&original==1&&peer.word(28)==0,"Realm original controls handoff has no wire dispatch");
        check(!session.beginOriginalRealmControls()&&!session.request(MNM_MENU_REALM_ACTIVATE,1),"native requests blocked during original controls");
        peer.original();LiveMenuSessionTestAccess::poll(session);
        if(mainReturn){peer.put(136,3);peer.put(140,1);peer.put(156,0);peer.publish();}
        else peer.realm();
        LiveMenuSessionTestAccess::poll(session);
        check(!failures&&session.request(mainReturn?MNM_MENU_QUIT:MNM_MENU_REALM_CHARACTER),"fresh original parent restores native requests");++cases;
    }
    for(int mutation=0;mutation<10;++mutation){
        CampaignMenuWireFixture peer;peer.realm();if(mutation==8)peer.region();LiveMenuSession session("/unused");enable(session);peer.attach(session);
        int original=0;session.originalViewportRequested=[&]{++original;};
        if(mutation==0)session.progressedMenusEnabled=false;
        if(mutation==1)session.realmMenusEnabled=false;
        if(mutation==2)session.originalViewportRequested={};
        if(mutation==3){peer.put(140,0);peer.publish();}
        if(mutation==4)peer.realm();
        if(mutation==5){peer.put(152,10);peer.publish();}
        if(mutation==6){peer.put(144,1);peer.publish();}
        if(mutation==7)session.setMediaActive(true);
        if(mutation==9){peer.put(MNM_MENU_V19_REALM+36,0);peer.publish();}
        check(!session.beginOriginalRealmControls()&&!original&&peer.word(28)==0,"disabled/stale/foreign/unready Realm handoff refused without dispatch");++cases;
    }
    std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"live_validated\":false}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
