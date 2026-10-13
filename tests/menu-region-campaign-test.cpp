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
    for(const auto& mutation:std::array<std::pair<int,quint32>,6>{{{0,22},{4,5},{8,2},{24,20},{28,2},{20,32}}}){
        CampaignMenuWireFixture peer;MenuBridge::State state;check(peer.bridge().read(state),"valid Region before mutation");peer.put(MNM_MENU_V7_REGION+mutation.first,mutation.second);peer.publish();check(!peer.bridge().read(state),"invalid V20 Region wire");++cases;
    }
    std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"live_validated\":false}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
