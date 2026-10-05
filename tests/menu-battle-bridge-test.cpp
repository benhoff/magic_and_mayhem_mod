#include "menu_bridge.hpp"
#include "../protocols/include/mnm/menu_v2.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QtEndian>
#include <cstring>
#include <cstdio>
#include <cstdlib>
static void require(bool ok){if(!ok){std::fprintf(stderr,"Battle wire checks failed\n");std::exit(1);}}
int main(int argc,char** argv){
    QCoreApplication app(argc,argv);QTemporaryDir root;MenuBridge bridge;
    require(root.isValid()&&bridge.create(root.filePath("battle.bin"),true));
    QFile peer(root.filePath("battle.bin"));require(peer.open(QIODevice::ReadWrite));auto* data=peer.map(0,MNM_MENU_V2_SIZE);require(data);
    auto* w=reinterpret_cast<quint32*>(data);
    auto put=[&](int offset,quint32 value){__atomic_store_n(w+offset/4,qToLittleEndian(value),__ATOMIC_RELEASE);};
    auto get=[&](int offset){return qFromLittleEndian(__atomic_load_n(w+offset/4,__ATOMIC_ACQUIRE));};
    put(128,1);put(132,10);put(136,14);put(140,1);put(152,77);
    put(MNM_MENU_V2_MAP,2);put(MNM_MENU_V2_MAP_COUNT,2);
    std::memcpy(data+MNM_MENU_V2_MAP_NAME,"Second map",11);
    std::memcpy(data+MNM_MENU_V2_MAP_NAMES,"First map",10);
    std::memcpy(data+MNM_MENU_V2_MAP_NAMES+128,"Second map",11);
    for(int i=0;i<13;++i)put(MNM_MENU_V2_RULES+i*4,50);
    for(int i=0;i<4;++i){const int offset=MNM_MENU_V2_PLAYERS+i*48;put(offset,i<2);put(offset+4,i<2?i:UINT32_MAX);put(offset+8,i<2?i:UINT32_MAX);std::memcpy(data+offset+16,i<2?"M\x80rlin":"No Player",i<2?8:10);}
    MenuBridge::State state;require(!bridge.read(state));put(128,2);
    require(bridge.read(state)&&state.battle.map==2&&state.battle.maps.size()==2&&state.battle.players[0].name==QString::fromUtf8("M€rlin"));
    require(!bridge.request(MNM_MENU_SETUP_START,state)); // Full settings required.
    std::array<int,17> rules{};for(int i=0;i<13;++i)rules[i]=50;
    rules[0]=-1;require(!bridge.request(MNM_MENU_SETUP_MAP,state,0,&rules));rules[0]=75;
    require(bridge.request(MNM_MENU_SETUP_MAP,state,0,&rules)&&get(28)==1&&get(32)==MNM_MENU_SETUP_MAP&&get(44)==75);
    const auto beat=get(24);bridge.heartbeat();require(get(24)>beat&&get(28)==1&&get(44)==75);
    require(!bridge.request(MNM_MENU_SETUP_START,state,0,&rules)); // Outstanding transaction.
    put(128,3);put(132,11);put(136,25);put(144,1);put(148,MNM_MENU_OK);put(128,4);
    require(!bridge.request(MNM_MENU_MAP_OK,state,2));require(bridge.read(state));
    require(!bridge.request(MNM_MENU_SETUP_CANCEL,state));
    require(bridge.request(MNM_MENU_MAP_OK,state,2)&&get(40)==2&&get(28)==2);
    put(128,5);put(132,12);put(136,14);put(144,2);put(128,6);require(bridge.read(state));
    put(MNM_MENU_V2_MAP_COUNT,129);require(!bridge.read(state));put(MNM_MENU_V2_MAP_COUNT,2);
    std::memset(data+MNM_MENU_V2_MAP_NAMES,'X',128);require(!bridge.read(state));data[MNM_MENU_V2_MAP_NAMES+127]=0;
    put(MNM_MENU_V2_PLAYERS,2);require(!bridge.read(state));put(MNM_MENU_V2_PLAYERS,1);
    put(156,3);require(!bridge.read(state));put(156,1);require(bridge.read(state)&&state.handoff==1);
    require(bridge.request(MNM_MENU_SETUP_START,state,0,&rules));bridge.retire();require(get(20)==0&&!bridge.request(MNM_MENU_SETUP_START,state,0,&rules));
    peer.unmap(data);std::puts("Battle wire snapshots and transactions passed");return 0;
}
