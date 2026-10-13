#include "campaign-menu-wire-fixture.hpp"
#include "live_campaign_defeat_controller.hpp"
#include <QApplication>
#include <QLabel>
#include <iostream>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
int main(int argc,char** argv)try{
    QApplication app(argc,argv);unsigned cases=0;
    for(quint32 outcome:{0u,1u}){
        CampaignMenuWireFixture peer;peer.report(outcome);LiveMenuSession session("/unused");session.progressedMenusEnabled=session.realmMenusEnabled=true;peer.attach(session);
        BattleResultWidget widget;LiveCampaignDefeatController controller(session,widget);int failures=0,returns=0;
        session.failed=[&](const auto&){++failures;};session.originalViewportRequested=[&]{++returns;};
        session.stateChanged=[&](const auto& state){check(controller.present(state),"original report model");};
        peer.publish();LiveMenuSessionTestAccess::poll(session);
        check(widget.findChild<QLabel*>("battleResultTitle")->text()==(outcome?"Original victory title":"Original defeat title"),"verbatim original title");
        for(int i=0;i<21;++i)check(widget.findChild<QLabel*>(QString("battleResultText%1").arg(i+1))->text()==(i==19?"":"Original score text"),"verbatim score fields including empty reason");
        widget.continueRequested();check(peer.word(32)==MNM_MENU_DEFEAT_OK,"original OK requested");
        check(!session.requestDefeatContinue(),"duplicate report OK refused");peer.original();peer.acknowledge();LiveMenuSessionTestAccess::poll(session);check(returns==1&&!failures,"original cleanup handoff");
        peer.realm();LiveMenuSessionTestAccess::poll(session);check(session.request(MNM_MENU_REALM_ACTIVATE,1)&&!failures,"fresh Realm accepts command after report");++cases;
    }
    for(bool wrongThread:{false,true}){
        CampaignMenuWireFixture peer;peer.report(1);LiveMenuSession session("/unused");session.progressedMenusEnabled=true;peer.attach(session);int failures=0;session.failed=[&](const auto&){++failures;};
        check(session.requestDefeatContinue(),"report request");peer.original();peer.acknowledge();peer.put(wrongThread?152:144,wrongThread?10:2);LiveMenuSessionTestAccess::poll(session);check(failures==1,"report identity mismatch refused");++cases;
    }
    for(const auto& mutation:std::array<std::pair<int,quint32>,4>{{{0,2},{4,4},{8,4},{12,2}}}){
        CampaignMenuWireFixture peer;peer.report(1);MenuBridge::State state;check(peer.bridge().read(state),"valid report wire");peer.put(MNM_MENU_V11_DEFEAT+mutation.first,mutation.second);peer.publish();check(!peer.bridge().read(state),"invalid report metadata refused");++cases;
    }
    std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"live_validated\":false}\n";
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
