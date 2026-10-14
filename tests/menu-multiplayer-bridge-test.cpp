#include "menu_bridge.hpp"
#include "../../protocols/include/mnm/menu_v15.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QtEndian>
#include <cstdio>
#include <cstdlib>
#define require(ok) do {if(!(ok)){std::fprintf(stderr,"Multiplayer wire check failed at %d\n",__LINE__);std::exit(1);}} while(0)
int main(int argc,char** argv){
 QCoreApplication app(argc,argv);QTemporaryDir dir;MenuBridge b;const auto path=dir.filePath("mp.bin");require(b.create(path,true,true,true,true,true,true,true,true,true,true,true,true,true,true));
 QFile f(path);require(f.open(QIODevice::ReadWrite)&&f.size()==MNM_MENU_V15_SIZE);auto* bytes=f.map(0,f.size());require(bytes);auto put=[&](int at,quint32 value){qToLittleEndian(value,bytes+at);};
 put(128,2);put(132,10);put(136,15);put(140,1);put(148,MNM_MENU_OK);put(152,42);
 const int at=MNM_MENU_V15_MULTIPLAYER;const quint32 expected[]={22,4,2,0,7,3,0,0,256,15,0,0};for(int i=0;i<12;++i)put(at+4*i,expected[i]);
 MenuBridge::State state;require(b.read(state)&&state.multiplayer.available==7);
 const quint32 bad[]={0,0,0,3,8,4,2,2,0,16,1,1};for(int i=0;i<12;++i){put(at+4*i,bad[i]);require(!b.read(state));put(at+4*i,expected[i]);}
 require(b.read(state));auto stale=state;--stale.generation;require(!b.requestMultiplayer(MNM_MENU_MULTIPLAYER_SUBMIT,stale,0,"game","user"));
 require(!b.requestMultiplayer(MNM_MENU_MULTIPLAYER_SUBMIT,state,3,"game","user"));
 require(!b.requestMultiplayer(MNM_MENU_MULTIPLAYER_SUBMIT,state,0,"","user")&&!b.requestMultiplayer(MNM_MENU_MULTIPLAYER_SUBMIT,state,0,"game",""));
 for(auto name:{QString(16,'u'),QString("line\n"),QString::fromUtf8("é"),QString(QChar(0))})require(!b.requestMultiplayer(MNM_MENU_MULTIPLAYER_SUBMIT,state,0,"game",name));
 require(!b.requestMultiplayer(MNM_MENU_MULTIPLAYER_SUBMIT,state,0,QString(64,'g'),"user"));
 put(at+16,5);require(b.read(state)&&!b.requestMultiplayer(MNM_MENU_MULTIPLAYER_SUBMIT,state,1,"game","user"));put(at+16,7);
 put(140,0);require(!b.requestMultiplayer(MNM_MENU_MULTIPLAYER_SUBMIT,state,0,"game","user"));put(140,1);require(b.read(state));
 require(b.requestMultiplayer(MNM_MENU_MULTIPLAYER_SUBMIT,state,2," game ",QString(15,'u'))&&!b.requestMultiplayer(MNM_MENU_MULTIPLAYER_SUBMIT,state,0,"game","user"));
 require(qFromLittleEndian<quint32>(bytes+40)==2&&QByteArray(reinterpret_cast<char*>(bytes+MNM_MENU_V15_HOST_NAMES))==" game "&&QByteArray(reinterpret_cast<char*>(bytes+MNM_MENU_V15_HOST_NAMES+64))==QByteArray(15,'u'));
 put(144,1);put(132,11);put(136,16);put(at+32,0);require(b.read(state)&&!b.requestMultiplayer(MNM_MENU_MULTIPLAYER_SUBMIT,state,0,"game","user"));
 require(b.requestMultiplayer(MNM_MENU_MULTIPLAYER_SUBMIT,state,0,"","user"));put(144,2);put(132,12);require(b.read(state));require(b.requestMultiplayer(MNM_MENU_MULTIPLAYER_CANCEL,state));b.retire();require(!b.requestMultiplayer(MNM_MENU_MULTIPLAYER_CANCEL,state));f.unmap(bytes);
 MenuBridge old;require(old.create(dir.filePath("old.bin"),true,true,true,true,true,true,true,true,true,true,true,true,true));QFile oldf(dir.filePath("old.bin"));require(oldf.open(QIODevice::ReadWrite)&&oldf.size()==MNM_MENU_V14_SIZE);auto* ob=oldf.map(0,oldf.size());require(ob);qToLittleEndian<quint32>(2,ob+128);qToLittleEndian<quint32>(15,ob+136);require(!old.read(state));oldf.unmap(ob);
 std::puts("V15 multiplayer names/provider/stale/duplicate/retired guards and V14 compatibility passed");
}
