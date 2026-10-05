#include "menu_bridge.hpp"
#include "../protocols/include/mnm/menu_v5.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QtEndian>
#include <cstdio>
#include <cstdlib>
#include <cstring>
static void require(bool ok){if(!ok){std::fprintf(stderr,"Results bridge check failed\n");std::exit(1);}}
int main(int argc,char** argv){
    QCoreApplication app(argc,argv);QTemporaryDir root;MenuBridge bridge;
    const auto path=root.filePath("results.bin");require(bridge.create(path,true,true,false,true));
    QFile peer(path);require(peer.open(QIODevice::ReadWrite));auto* bytes=peer.map(0,MNM_MENU_V5_SIZE);require(bytes);
    auto put=[&](int word,quint32 value){qToLittleEndian(value,bytes+word*4);};
    constexpr int m=MNM_MENU_V5_RESULTS/4;quint32 sequence=0;
    auto state=[&](quint32 generation,quint32 ack,quint32 ready){
        put(32,++sequence);put(33,generation);put(34,26);put(35,ready);put(36,ack);put(37,0);put(38,123);put(39,0);
        put(m,3);put(m+1,1);put(m+2,1);put(m+3,0);
        for(int i=0;i<4;++i){put(m+4+i*162,0);put(m+5+i*162,UINT32_MAX);}put(32,++sequence);
    };
    MenuBridge::State s;state(1,0,1);require(bridge.read(s)&&s.results.context==1);
    require(!bridge.request(MNM_MENU_BACK,s)&&bridge.request(MNM_MENU_RESULT_CONTINUE,s)&&!bridge.request(MNM_MENU_RESULT_CONTINUE,s));
    state(2,1,1);require(!bridge.request(MNM_MENU_RESULT_QUIT,s));require(bridge.read(s)&&bridge.request(MNM_MENU_RESULT_QUIT,s));
    state(3,2,1);put(m,1);require(bridge.read(s)&&!bridge.request(MNM_MENU_RESULT_QUIT,s));
    state(4,2,0);require(bridge.read(s)&&!bridge.request(MNM_MENU_RESULT_CONTINUE,s));
    for(auto field:{m,m+1,m+2,m+3,m+4,m+5}){state(5,2,1);put(field,99);require(!bridge.read(s));}
    state(6,2,1);put(m+4,1);require(!bridge.read(s)); // Active row needs a name.
    state(6,2,1);std::memset(bytes+MNM_MENU_V5_RESULTS+24,'x',128);require(!bridge.read(s));
    state(6,2,1);std::memset(bytes+MNM_MENU_V5_RESULTS+24,0,128);put(32,sequence+1);require(!bridge.read(s));
    state(7,2,1);require(bridge.read(s));bridge.retire();require(!bridge.request(MNM_MENU_RESULT_CONTINUE,s));
    peer.unmap(bytes);std::puts("Results bridge checks passed");
}
