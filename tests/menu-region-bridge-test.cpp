#include "menu_bridge.hpp"
#include "../protocols/include/mnm/menu_v7.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QtEndian>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#define require(ok) do{if(!(ok)){std::fprintf(stderr,"Region bridge failed at %d\n",__LINE__);std::exit(1);}}while(0)
int main(int argc,char** argv){
 QCoreApplication app(argc,argv);QTemporaryDir root;MenuBridge bridge;auto path=root.filePath("region.bin");require(bridge.create(path,false,false,false,false,false,true));
 QFile peer(path);require(peer.open(QIODevice::ReadWrite));auto* b=peer.map(0,MNM_MENU_V7_SIZE);require(b);
 auto put=[&](int at,quint32 v){qToLittleEndian(v,b+at);};auto get=[&](int at){return qFromLittleEndian<quint32>(b+at);};quint32 seq=0;
 auto state=[&](quint32 screen,quint32 generation,quint32 ack,quint32 ready=1){
  put(128,++seq);put(132,generation);put(136,screen);put(140,ready);put(144,ack);put(148,0);put(152,77);put(156,0);
  const quint32 v[]={4,4,0,0,15,1,1,0};for(int i=0;i<8;++i)put(MNM_MENU_V7_REGION+i*4,v[i]);
  std::memset(b+MNM_MENU_V7_REGION+32,0,288);std::memcpy(b+MNM_MENU_V7_REGION+32,"Celtic",7);std::memcpy(b+MNM_MENU_V7_REGION+160,"Forest of Pain",15);put(128,++seq);
 };
 MenuBridge::State s;state(3,1,0);require(bridge.read(s)&&bridge.request(MNM_MENU_NEW_GAME,s));
 state(18,2,1);require(bridge.read(s)&&s.region.name=="Forest of Pain");require(!bridge.request(MNM_MENU_REGION_DIFFICULTY,s,4));
 require(bridge.request(MNM_MENU_REGION_DIFFICULTY,s,3)&&get(40)==3);require(!bridge.request(MNM_MENU_REGION_CANCEL,s));
 state(18,3,2);require(!bridge.request(MNM_MENU_REGION_CANCEL,s));require(bridge.read(s));put(MNM_MENU_V7_REGION+16,7);require(!bridge.request(MNM_MENU_REGION_DIFFICULTY,s,3));
 put(MNM_MENU_V7_REGION+20,0);require(!bridge.request(MNM_MENU_REGION_CANCEL,s));
 state(18,4,2,0);require(bridge.read(s)&&!bridge.request(MNM_MENU_REGION_CANCEL,s));
 for(int field=0;field<8;++field){state(18,5,2);put(MNM_MENU_V7_REGION+field*4,99);require(!bridge.read(s));}
 state(18,6,2);put(MNM_MENU_V7_REGION+288,1);require(!bridge.read(s));
 state(18,6,2);std::memset(b+MNM_MENU_V7_REGION+160,'X',128);require(!bridge.read(s));
 state(18,6,2);put(128,seq+1);require(!bridge.read(s));
 state(18,7,2);require(bridge.read(s)&&bridge.request(MNM_MENU_REGION_CANCEL,s));state(18,8,3);require(bridge.read(s));bridge.retire();require(!bridge.request(MNM_MENU_REGION_CANCEL,s));peer.unmap(b);
 MenuBridge legacy;require(legacy.create(root.filePath("v6.bin"),false,false,false,false,true));require(!legacy.request(MNM_MENU_NEW_GAME,s));
 std::puts("Region bridge checks passed");
}
