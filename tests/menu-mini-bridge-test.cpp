#include "menu_bridge.hpp"
#include "../protocols/include/mnm/menu_v4.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QtEndian>
#include <cstdio>
#include <cstdlib>
static void require(bool ok){if(!ok){std::fprintf(stderr,"Mini bridge check failed\n");std::exit(1);}}
int main(int argc,char** argv){
    QCoreApplication app(argc,argv);QTemporaryDir root;MenuBridge bridge;
    const auto path=root.filePath("mini.bin");require(bridge.create(path,true,true,true));
    QFile peer(path);require(peer.open(QIODevice::ReadWrite));auto* bytes=peer.map(0,MNM_MENU_V4_SIZE);require(bytes);
    auto put=[&](int word,quint32 value){qToLittleEndian(value,bytes+word*4);};
    constexpr int m=MNM_MENU_V4_MINI/4;quint32 sequence=0;
    auto state=[&](quint32 generation,quint32 ack,quint32 ready){
        put(32,++sequence);put(33,generation);put(34,17);put(35,ready);put(36,ack);put(37,0);put(38,123);put(39,0);
        put(m,1);put(m+1,0);put(m+2,1);put(m+3,2);put(m+4,7);put(m+5,3);put(m+6,0);put(m+7,0);put(32,++sequence);
    };
    MenuBridge::State s;state(1,0,1);require(bridge.read(s)&&s.mini.parentScreen==2);
    require(!bridge.request(MNM_MENU_BACK,s)&&bridge.request(MNM_MENU_MINI_CANCEL,s)&&!bridge.request(MNM_MENU_MINI_CANCEL,s));
    state(2,1,1);require(!bridge.request(MNM_MENU_MINI_PREFERENCES,s));require(bridge.read(s)&&bridge.request(MNM_MENU_MINI_PREFERENCES,s));
    state(3,2,1);require(bridge.read(s)&&bridge.request(MNM_MENU_MINI_QUIT,s));
    state(4,3,0);put(m+1,1);put(m+4,0);require(bridge.read(s)&&!bridge.request(MNM_MENU_MINI_QUIT,s));
    // Reject malformed/unsupported states without authorizing any request.
    for(auto field:{m,m+1,m+2,m+3,m+4,m+5,m+6,m+7}){
        state(5,3,1);put(field,field==m+5?5u:99u);require(!bridge.read(s));
    }
    state(6,3,1);put(m+1,1);require(!bridge.read(s));
    state(6,3,1);put(32,sequence+1);require(!bridge.read(s));
    state(7,3,1);require(bridge.read(s));bridge.retire();require(!bridge.request(MNM_MENU_MINI_CANCEL,s));
    peer.unmap(bytes);std::puts("Mini bridge checks passed");
}
