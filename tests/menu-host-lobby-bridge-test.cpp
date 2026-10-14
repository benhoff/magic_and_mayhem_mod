#include "menu_bridge.hpp"
#include "../../protocols/include/mnm/menu_v17.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QtEndian>
#include <cstdio>
#include <cstdlib>
#define require(ok) do {if(!(ok)){std::fprintf(stderr,"Host lobby wire failure at%d\n",__LINE__);std::exit(1);}} while(0)
int main(int argc,char** argv){QCoreApplication app(argc,argv);QTemporaryDir dir;MenuBridge b;const auto path=dir.filePath("lobby.bin");require(b.create(path,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true));QFile f(path);require(f.open(QIODevice::ReadWrite)&&f.size()==MNM_MENU_V17_SIZE);auto* bytes=f.map(0,f.size());require(bytes);auto put=[&](int at,quint32 v){qToLittleEndian(v,bytes+at);};put(128,2);put(132,10);put(136,24);put(140,1);put(148,MNM_MENU_OK);put(152,42);const int at=MNM_MENU_V17_LOBBY;const quint32 expected[]={15,5,2,2,1,0,0,0};for(int i=0;i<8;++i)put(at+4*i,expected[i]);put(at+84,1);bytes[at+100]='u';bytes[at+276]='g';bytes[at+404]='m';MenuBridge::State state;require(b.read(state)&&state.lobby.players[0].active);
 const quint32 bad[]={0,0,0,0,2,10001,1,1};for(int i=0;i<8;++i){put(at+4*i,bad[i]);require(!b.read(state));put(at+4*i,expected[i]);}put(at+132,1);require(!b.read(state));put(at+132,0);put(at+532,1);require(!b.read(state));put(at+532,0);put(at+88,12);require(!b.read(state));put(at+88,0);require(b.read(state));auto stale=state;--stale.generation;require(!b.request(MNM_MENU_LOBBY_CANCEL,stale)&&!b.request(MNM_MENU_LOBBY_CANCEL,state,1)&&!b.request(MNM_MENU_SESSION_CANCEL,state));
 require(b.request(MNM_MENU_LOBBY_CANCEL,state)&&!b.request(MNM_MENU_LOBBY_CANCEL,state));put(144,1);put(132,11);require(b.read(state));b.retire();require(!b.request(MNM_MENU_LOBBY_CANCEL,state));f.unmap(bytes);
 MenuBridge old;const auto oldpath=dir.filePath("v16.bin");require(old.create(oldpath,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true));QFile oldf(oldpath);require(oldf.open(QIODevice::ReadWrite)&&oldf.size()==MNM_MENU_V16_SIZE);auto* ob=oldf.map(0,oldf.size());require(ob);qToLittleEndian<quint32>(2,ob+128);qToLittleEndian<quint32>(24,ob+136);require(!old.read(state));oldf.unmap(ob);std::puts("V17 host-only wire bounds/stale/duplicate/retired checks and V16 rejection passed");}
