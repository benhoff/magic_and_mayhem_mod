#include "character-wire-fixture.hpp"
#include "live_realm_controller.hpp"
#include "../protocols/include/mnm/menu_v19.h"
#include <QApplication>
#include <QPushButton>
#include <iostream>
static void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
class RealmPeer final {
public:
    RealmPeer(){
        bridge=std::make_unique<MenuBridge>();
        check(dir.isValid()&&bridge->create(dir.filePath("realm.bin"),true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true),"V19 channel");
        file.setFileName(dir.filePath("realm.bin"));check(file.open(QIODevice::ReadWrite)&&file.size()==MNM_MENU_V19_SIZE,"peer mapping");
        bytes=file.map(0,file.size());check(bytes,"peer map");
        put(136,4);put(140,1);put(148,MNM_MENU_OK);put(152,9);const int lane=MNM_MENU_V19_REALM;
        for(const auto& value:std::array<std::pair<int,quint32>,10>{{{0,3},{4,3},{8,5},{12,1},{16,3},{20,0},{24,1},{28,UINT32_MAX},{32,6},{36,7}}})put(lane+value.first,value.second);
        for(int i=0;i<3;++i){const int at=lane+48+i*24;put(at,quint32(i+1));put(at+4,i?1:0);put(at+8,i?0:1);put(at+12,i==1?1:0);put(at+16,100+100*i);put(at+20,100);}
        publish();
    }
    ~RealmPeer(){if(bytes)file.unmap(bytes);}
    void put(int at,quint32 value){qToLittleEndian(value,bytes+at);}
    quint32 word(int at)const{return qFromLittleEndian<quint32>(bytes+at);}
    void publish(){put(132,++generation);put(128,word(128)+2);}
    void attach(LiveMenuSession& session){MenuBridge::State state;check(bridge->read(state),"Realm read");LiveMenuSessionTestAccess::attach(session,std::move(bridge),state);}
    void acknowledge(){put(144,word(28));publish();}
    QTemporaryDir dir;QFile file;uchar* bytes=nullptr;std::unique_ptr<MenuBridge> bridge;quint32 generation=0;
};
int main(int argc,char** argv)try{
    QApplication app(argc,argv);unsigned cases=0;
    {
        RealmPeer peer;LiveMenuSession session("/unused");session.realmMenusEnabled=session.characterMenusEnabled=true;peer.attach(session);
        RealmViewerWidget widget;LiveRealmController controller(session,widget);int failures=0,original=0,presented=0;
        session.failed=[&](const auto&){++failures;};session.originalViewportRequested=[&]{++original;};
        session.stateChanged=[&](const auto& state){QString error;check(controller.present(state,&error),"controller projection");++presented;};
        peer.publish();LiveMenuSessionTestAccess::poll(session);widget.show();app.processEvents();
        const auto model=widget.campaign();check(model.regions[0].size()==3&&model.regions[0][0].available&&model.regions[0][1].available&&!model.regions[0][2].available,"engine ownership and adjacency availability");
        check(!widget.findChild<QPushButton*>("realmViewerCancel")->isEnabled(),"no invented direct Cancel");
        widget.findChild<QPushButton*>("realmViewerEnter")->click();check(peer.word(32)==MNM_MENU_REALM_ACTIVATE&&peer.word(40)==1&&!widget.isEnabled(),"semantic activation disables pending page");
        check(!session.request(MNM_MENU_REALM_ACTIVATE,1),"duplicate transaction denied");peer.acknowledge();LiveMenuSessionTestAccess::poll(session);
        check(widget.isEnabled()&&!failures,"owned destination acknowledged and Realm retained");
        check(!session.request(MNM_MENU_REALM_ACTIVATE,3)&&!session.request(MNM_MENU_REALM_ACTIVATE,0),"blocked and zero regions refused");
        check(session.request(MNM_MENU_REALM_CHARACTER),"original Character entry requested");peer.put(136,0);peer.put(140,0);peer.acknowledge();LiveMenuSessionTestAccess::poll(session);
        check(original==1,"original auxiliary ingress owns transition");peer.put(136,4);peer.put(140,1);peer.publish();LiveMenuSessionTestAccess::poll(session);
        check(widget.isEnabled()&&presented>=4&&!failures&&session.request(MNM_MENU_REALM_ACTIVATE,1),"fresh engine-confirmed Realm return reacquires ownership");++cases;
    }
    for(bool wrongThread:{false,true}){
        RealmPeer peer;LiveMenuSession session("/unused");session.realmMenusEnabled=true;peer.attach(session);int failures=0;session.failed=[&](const auto&){++failures;};
        check(session.request(MNM_MENU_REALM_ACTIVATE,1),"request for bad ACK");peer.acknowledge();peer.put(wrongThread?152:144,wrongThread?10:2);LiveMenuSessionTestAccess::poll(session);
        check(failures==1&&!session.request(MNM_MENU_REALM_ACTIVATE,1),"wrong ACK/thread retires ownership");++cases;
    }
    for(const auto& mutation:std::array<std::pair<int,quint32>,8>{{{16,20},{12,0},{20,80},{32,1},{48,2},{56,2},{64,800},{504,1}}}){
        RealmPeer peer;MenuBridge::State state;check(peer.bridge->read(state),"valid wire before mutation");peer.put(MNM_MENU_V19_REALM+mutation.first,mutation.second);peer.publish();
        check(!peer.bridge->read(state),"malformed Realm rejected");++cases;
    }
    std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"live_validated\":false}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
