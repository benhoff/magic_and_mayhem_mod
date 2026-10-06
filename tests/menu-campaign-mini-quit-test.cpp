#include "menu_bridge.hpp"
#include "live_mini_menu_controller.hpp"
#include "../protocols/include/mnm/menu_v10.h"
#include <QApplication>
#include <QPushButton>
#include <QTemporaryDir>
#include <QtEndian>
#include <cstdio>
#include <cstdlib>
#define require(ok) do{if(!(ok)){std::fprintf(stderr,"Campaign Mini check failed at %d\n",__LINE__);std::exit(1);}}while(0)
int main(int argc,char** argv){
 QApplication app(argc,argv);QTemporaryDir root;MenuBridge bridge;auto path=root.filePath("mini10.bin");require(bridge.create(path,false,false,false,false,false,false,false,true,true));
 QFile peer(path);require(peer.open(QIODevice::ReadWrite));auto* b=peer.map(0,MNM_MENU_V10_SIZE);require(b);auto put=[&](int at,quint32 v){qToLittleEndian(v,b+at);};quint32 seq=0;
 auto state=[&](quint32 generation,quint32 ack,quint32 ready=1){put(128,++seq);put(132,generation);put(136,17);put(140,ready);put(144,ack);put(148,0);put(152,77);put(156,3);const quint32 mini[]={0,0,5,2,5,5,2,0};for(int i=0;i<8;++i)put(MNM_MENU_V4_MINI+4*i,mini[i]);put(128,++seq);};
 MenuBridge::State s;state(1,0);require(bridge.read(s)&&s.mini.mode==2&&!s.mini.battle);require(!bridge.request(MNM_MENU_MINI_PREFERENCES,s));require(bridge.request(MNM_MENU_MINI_QUIT,s)&&!bridge.request(MNM_MENU_MINI_QUIT,s));
 state(2,1);require(!bridge.request(MNM_MENU_MINI_CANCEL,s)&&bridge.read(s));
 for(int i=0;i<8;++i){state(3,1);put(MNM_MENU_V4_MINI+4*i,99);require(!bridge.read(s));}
 state(3,1,0);put(MNM_MENU_V4_MINI+4,1);put(MNM_MENU_V4_MINI+16,0);require(bridge.read(s)&&!bridge.request(MNM_MENU_MINI_CANCEL,s));
 state(4,1);require(bridge.read(s));LiveMenuSession session("/unused");MiniMenuWidget widget;LiveMiniMenuController controller(session,widget);require(controller.present(s,nullptr)&&widget.mode()==MiniMenuWidget::Mode::Campaign);
 for(int i=1;i<=5;++i)require(widget.findChild<QPushButton*>(QString("miniMenuButton%1").arg(i))->isEnabled()==(i==4||i==5));
 session.campaignMiniEnabled=false;QString error;require(!controller.present(s,&error)&&!error.isEmpty());
 session.campaignMiniEnabled=true;session.campaignQuitEnabled=false;require(controller.present(s,nullptr));require(!widget.findChild<QPushButton*>("miniMenuButton4")->isEnabled());
 bridge.retire();require(!bridge.request(MNM_MENU_MINI_CANCEL,s));peer.unmap(b);std::puts("Campaign Mini bridge/controller checks passed");
}
