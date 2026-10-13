#include "campaign-menu-wire-fixture.hpp"
#include "grimoire-fixtures.hpp"
#include "live_grimoire_controller.hpp"
#include <QApplication>
#include <QPushButton>
#include <QListWidget>
#include <iostream>
static void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
static void enable(LiveMenuSession& s){s.progressedMenusEnabled=s.realmMenusEnabled=s.campaignBookEnabled=true;}
int main(int argc,char** argv)try{
    QApplication app(argc,argv);QTemporaryDir assets;grimoire_fixture::create(assets.path());unsigned cases=0;
    for(quint32 parent:{4u,18u}){
        CampaignMenuWireFixture peer(true,true);peer.book(parent);LiveMenuSession session("/unused");enable(session);peer.attach(session);GrimoireWidget widget;QString error;check(widget.loadAssets(assets.path(),&error),"authored Grimoire assets");LiveGrimoireController controller(session,widget);int failures=0,returns=0;
        session.failed=[&](const auto&){++failures;};session.originalViewportRequested=[&]{++returns;};session.stateChanged=[&](const auto& state){check(controller.present(state,&error),"broader book model");};peer.publish();LiveMenuSessionTestAccess::poll(session);
        check(widget.findChild<QPushButton*>("grimoireTab7")->isEnabled()&&!widget.findChild<QPushButton*>("grimoireArtwork")->isEnabled(),"engine supported chapter and retained artwork boundary");check(widget.findChild<QListWidget*>("grimoireEntries")->count()==1,"only engine supported static row shown");
        widget.findChild<QPushButton*>("grimoireTab7")->click();check(peer.word(32)==MNM_MENU_GRIMOIRE_SELECT&&peer.word(40)==7*124,"checked packed chapter action");peer.book(parent,7,0);peer.acknowledge();LiveMenuSessionTestAccess::poll(session);
        auto* list=widget.findChild<QListWidget*>("grimoireEntries");list->itemActivated(list->item(0));check(peer.word(40)==7*124+1,"typed section action");peer.book(parent,7,1);peer.acknowledge();LiveMenuSessionTestAccess::poll(session);check(widget.location().chapter==7&&widget.location().section==1&&widget.location().level==0,"engine confirmed selected level");
        widget.closed();peer.original();peer.acknowledge();LiveMenuSessionTestAccess::poll(session);check(returns==1&&!failures,"original book Close cleanup");
        if(parent==4){peer.realm();LiveMenuSessionTestAccess::poll(session);check(session.request(MNM_MENU_REALM_ACTIVATE,1),"book returns to Realm");}else {peer.region();LiveMenuSessionTestAccess::poll(session);check(session.request(MNM_MENU_REGION_CHARACTER),"book returns to Region");}++cases;
    }
    for(bool wrongThread:{false,true}){
        CampaignMenuWireFixture peer(true,true);peer.book();LiveMenuSession session("/unused");enable(session);peer.attach(session);int failures=0;session.failed=[&](const auto&){++failures;};check(session.request(MNM_MENU_GRIMOIRE_CLOSE),"book request");peer.original();peer.acknowledge();peer.put(wrongThread?152:144,wrongThread?10:2);LiveMenuSessionTestAccess::poll(session);check(failures==1,"book identity mismatch refused");++cases;
    }
    for(const auto& mutation:std::array<std::pair<int,quint32>,5>{{{0,80},{4,5},{8,15},{12,1},{4092,1}}}){
        CampaignMenuWireFixture peer(true,true);peer.book();MenuBridge::State state;check(peer.bridge().read(state),"valid broader wire");peer.put(MNM_MENU_V22_GRIMOIRE+mutation.first,mutation.second);peer.publish();check(!peer.bridge().read(state),"bad broader metadata refused");++cases;
    }
    {
        CampaignMenuWireFixture peer(true,true);peer.book(4,7,1);MenuBridge::State state;check(peer.bridge().read(state),"static page");LiveMenuSession session("/unused");enable(session);GrimoireWidget widget;QString error;check(widget.loadAssets(assets.path(),&error),"fresh authored book");LiveGrimoireController controller(session,widget);check(controller.present(state,&error),"static page accepted");state.grimoire.section=2;state.grimoire.level=1;state.grimoire.entries.insert(7*124+2,1);check(!controller.present(state,&error)&&widget.location().section==1,"dynamic/multipage cannot replace original values with preview placeholders");++cases;
    }
    std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"live_validated\":false}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
