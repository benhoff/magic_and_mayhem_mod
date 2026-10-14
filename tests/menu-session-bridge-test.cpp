#include "menu_bridge.hpp"
#include "../../protocols/include/mnm/menu_v16.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QtEndian>
#include <cstdio>
#include <cstdlib>
#define require(ok) do {if(!(ok)){std::fprintf(stderr,"Session wire failure at%d\n",__LINE__);std::exit(1);}} while(0)
int main(int argc,char** argv){QCoreApplication app(argc,argv);QTemporaryDir dir;MenuBridge b;const auto path=dir.filePath("sessions.bin");require(b.create(path,true,true,true,true,true,true,true,true,true,true,true,true,true,true,true));QFile f(path);require(f.open(QIODevice::ReadWrite)&&f.size()==MNM_MENU_V16_SIZE);auto* bytes=f.map(0,f.size());require(bytes);auto put=[&](int at,quint32 v){qToLittleEndian(v,bytes+at);};
 put(128,2);put(132,10);put(136,21);put(140,1);put(148,MNM_MENU_OK);put(152,42);const int at=MNM_MENU_V16_SESSIONS;const quint32 expected[]={16,5,2,2,3,1,0,0,0,0};for(int i=0;i<10;++i)put(at+4*i,expected[i]);MenuBridge::State state;require(b.read(state)&&state.sessions.names.isEmpty());require(!b.request(MNM_MENU_SESSION_JOIN,state,1)&&!b.request(MNM_MENU_SESSION_CANCEL,state,1));
 const quint32 bad[]={0,0,0,0,4,2,2,2,101,1};for(int i=0;i<10;++i){put(at+4*i,bad[i]);require(!b.read(state));put(at+4*i,expected[i]);}require(b.read(state));auto stale=state;--stale.generation;require(!b.request(MNM_MENU_SESSION_CANCEL,stale));
 put(at+32,2);bytes[at+40]='a';bytes[at+168]='b';require(b.read(state)&&state.sessions.names.size()==2);require(!b.request(MNM_MENU_SESSION_JOIN,state,3));put(at+24,1);require(b.read(state)&&!b.request(MNM_MENU_SESSION_JOIN,state,1));put(at+24,0);put(at+28,1);require(b.read(state)&&!b.request(MNM_MENU_SESSION_JOIN,state,1));put(at+28,0);require(b.read(state));
 require(b.request(MNM_MENU_SESSION_JOIN,state,2)&&!b.request(MNM_MENU_SESSION_JOIN,state,1));require(qFromLittleEndian<quint32>(bytes+40)==2);put(144,1);put(132,11);require(b.read(state)&&b.request(MNM_MENU_SESSION_CANCEL,state));b.retire();require(!b.request(MNM_MENU_SESSION_CANCEL,state));f.unmap(bytes);
 MenuBridge old;const auto oldpath=dir.filePath("v15.bin");require(old.create(oldpath,true,true,true,true,true,true,true,true,true,true,true,true,true,true));QFile oldf(oldpath);require(oldf.open(QIODevice::ReadWrite)&&oldf.size()==MNM_MENU_V15_SIZE);auto* ob=oldf.map(0,oldf.size());require(ob);qToLittleEndian<quint32>(2,ob+128);qToLittleEndian<quint32>(21,ob+136);require(!old.read(state));oldf.unmap(ob);std::puts("V16 bounded sessions, stale/duplicate/busy/deferred/retired guards and V15 rejection passed");}
