#include "menu_bridge.hpp"
#include "../../protocols/include/mnm/menu_v14.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QtEndian>
#include <cstdio>
#include <cstdlib>
#define require(ok) do {if(!(ok)){std::fprintf(stderr,"Grimoire wire check failed at %d\n",__LINE__);std::exit(1);}} while(0)
int main(int argc,char** argv){
 QCoreApplication app(argc,argv);QTemporaryDir dir;MenuBridge bridge;
 const auto path=dir.filePath("book.bin");require(bridge.create(path,true,true,true,true,true,true,true,true,true,true,true,true,true));
 QFile file(path);require(file.open(QIODevice::ReadWrite)&&file.size()==MNM_MENU_V14_SIZE);auto* bytes=file.map(0,file.size());require(bytes);
 auto put=[&](int offset,quint32 v){qToLittleEndian(v,bytes+offset);};
 put(128,2);put(132,10);put(136,MNM_MENU_GRIMOIRE_SCREEN);put(140,1);put(148,MNM_MENU_OK);put(152,42);
 const int at=MNM_MENU_V14_GRIMOIRE;const quint32 expected[]={0,0,1,63,18,5,5,0};for(int i=0;i<8;++i)put(at+4*i,expected[i]);
 MenuBridge::State state;require(bridge.read(state)&&state.grimoire.available==63&&state.grimoire.parent==18);
 const quint32 bad[]={6,124,991,1999,2999,3999,UINT32_MAX};for(auto id:bad)require(!bridge.request(MNM_MENU_GRIMOIRE_SELECT,state,id));
 require(!bridge.request(MNM_MENU_GRIMOIRE_CLOSE,state,1)&&!bridge.request(MNM_MENU_REGION_GRIMOIRE,state));
 auto stale=state;stale.generation--;require(!bridge.request(MNM_MENU_GRIMOIRE_CLOSE,stale));
 const quint32 invalid[]={1,5,0,0,0,0,0,1};for(int i=0;i<8;++i){put(at+4*i,invalid[i]);require(!bridge.read(stale));put(at+4*i,expected[i]);}
 put(140,0);require(!bridge.request(MNM_MENU_GRIMOIRE_CLOSE,state));put(140,1);
 require(bridge.request(MNM_MENU_GRIMOIRE_SELECT,state,4)&&!bridge.request(MNM_MENU_GRIMOIRE_SELECT,state,3));
 require(qFromLittleEndian<quint32>(bytes+40)==4);put(144,1);put(132,11);put(at+4,4);require(bridge.read(state)&&state.grimoire.section==4);
 require(bridge.request(MNM_MENU_GRIMOIRE_CLOSE,state));bridge.retire();require(!bridge.request(MNM_MENU_GRIMOIRE_CLOSE,state));file.unmap(bytes);
 // Explicit V13 compatibility never accepts semantic book screen27.
 MenuBridge old;const auto oldpath=dir.filePath("storage.bin");require(old.create(oldpath,true,true,true,true,true,true,true,true,true,true,true,true));
 QFile oldfile(oldpath);require(oldfile.open(QIODevice::ReadWrite)&&oldfile.size()==MNM_MENU_V13_SIZE);auto* oldbytes=oldfile.map(0,oldfile.size());require(oldbytes);
 qToLittleEndian<quint32>(2,oldbytes+128);qToLittleEndian<quint32>(27,oldbytes+136);require(!old.read(state));oldfile.unmap(oldbytes);
 std::puts("V14 Grimoire wire bounds, stale/duplicate/retired guards and V13 rejection passed");
}
