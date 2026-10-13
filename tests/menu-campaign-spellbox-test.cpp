#include "campaign-menu-wire-fixture.hpp"
#include "live_spell_menu_controller.hpp"
#include <QApplication>
#include <QPushButton>
#include <QKeyEvent>
#include <iostream>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static void enable(LiveMenuSession& s){s.progressedMenusEnabled=s.realmMenusEnabled=s.campaignSpellboxEnabled=true;}
int main(int argc,char** argv)try{
    QApplication app(argc,argv);unsigned cases=0;
    for(quint32 parent:{4u,18u})for(bool escape:{false,true}){
        CampaignMenuWireFixture peer(true);peer.spells(parent);LiveMenuSession session("/unused");enable(session);peer.attach(session);SpellboxWidget widget;LiveSpellMenuController controller(session,widget);int failures=0,returns=0,battles=0;
        session.failed=[&](const auto&){++failures;};session.originalViewportRequested=[&]{++returns;};session.battleStarted=[&](auto){++battles;};
        session.stateChanged=[&](const auto& state){QString error;check(controller.present(state,&error),"campaign inventory model");};peer.publish();LiveMenuSessionTestAccess::poll(session);
        check(widget.inventory().ownerId=="79"&&widget.findChild<QPushButton*>("spellboxOK")->text()=="Done","campaign owner and Done");QString error;check(widget.assignItem("0","0",&error),"draft assignment");
        if(escape){QKeyEvent event(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);QApplication::sendEvent(&widget,&event);}else widget.findChild<QPushButton*>("spellboxOK")->click();
        check(peer.word(32)==MNM_MENU_SPELL_FINISH&&peer.word(MNM_MENU_V3_HOST_SLOTS)==0,"Done/Escape preserves selected assignment");peer.original(4);peer.acknowledge();LiveMenuSessionTestAccess::poll(session);check(!failures&&returns==1&&!battles,"campaign Done uses original cleanup without battle start");
        if(parent==4){peer.realm();LiveMenuSessionTestAccess::poll(session);check(session.request(MNM_MENU_REALM_ACTIVATE,1),"Realm reopened");}else {peer.region();LiveMenuSessionTestAccess::poll(session);check(session.request(MNM_MENU_REGION_CHARACTER),"Region reopened");}++cases;
    }
    for(bool wrongThread:{false,true}){
        CampaignMenuWireFixture peer(true);peer.spells();LiveMenuSession session("/unused");enable(session);peer.attach(session);int failures=0;session.failed=[&](const auto&){++failures;};std::array<int,63> draft;draft.fill(-1);check(session.finishSpells(draft),"request before bad ACK");peer.original(4);peer.acknowledge();peer.put(wrongThread?152:144,wrongThread?10:2);LiveMenuSessionTestAccess::poll(session);check(failures==1,"bad spell ACK refused");++cases;
    }
    for(const auto& mutation:std::array<std::pair<int,quint32>,6>{{{0,2},{4,3},{8,6},{12,1},{16,1},{28,1}}}){
        CampaignMenuWireFixture peer(true);peer.spells();MenuBridge::State state;check(peer.bridge().read(state),"valid V21 inventory");peer.put(MNM_MENU_V21_SPELL_CONTEXT+mutation.first,mutation.second);peer.publish();check(!peer.bridge().read(state),"invalid campaign spell metadata refused");++cases;
    }
    std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"live_validated\":false}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
