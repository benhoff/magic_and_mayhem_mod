#include "menu_bridge.hpp"
#include "../protocols/include/mnm/menu_v1.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QtEndian>
#include <cstdio>
#include <cstdlib>
static void require(bool ok){if(!ok){std::fprintf(stderr,"Menu bridge test failed\n");std::exit(1);}}
int main(int argc,char** argv){
    QCoreApplication app(argc,argv);QTemporaryDir root;require(root.isValid());
    const auto path=root.filePath("channel.bin");MenuBridge bridge;require(bridge.create(path));
    MenuBridge other;require(!other.create(path));
    QFile peer(path);require(peer.open(QIODevice::ReadWrite));auto* data=peer.map(0,128);require(data);
    auto* w=reinterpret_cast<quint32*>(data);
    auto put=[&](int index,quint32 value){__atomic_store_n(w+index,qToLittleEndian(value),__ATOMIC_RELEASE);};
    auto get=[&](int index){return qFromLittleEndian(__atomic_load_n(w+index,__ATOMIC_ACQUIRE));};
    auto state=[&](quint32 seq,quint32 generation,quint32 screen,quint32 ready,quint32 ack,quint32 status){
        put(16,seq-1);put(17,generation);put(18,screen);put(19,ready);put(20,ack);put(21,status);put(22,123);put(16,seq);
    };
    MenuBridge::State snapshot;require(!bridge.read(snapshot));
    state(2,1,3,1,0,0);require(bridge.read(snapshot)&&snapshot.thread==123);
    require(!bridge.request(MNM_MENU_BACK,snapshot));
    require(bridge.request(MNM_MENU_OPEN_QUICK,snapshot));require(get(7)==1&&get(8)==1&&get(9)==1);
    require(!bridge.request(MNM_MENU_OPEN_QUICK,snapshot)); // Outstanding request.
    auto beat=get(6);bridge.heartbeat();require(get(6)>beat&&get(7)==1&&get(8)==1);
    state(4,2,3,0,1,MNM_MENU_OK);require(bridge.read(snapshot)&&!bridge.request(MNM_MENU_OPEN_QUICK,snapshot));
    state(6,3,22,1,1,MNM_MENU_OK);require(!bridge.request(MNM_MENU_BACK,snapshot)); // Stale supplied snapshot.
    require(bridge.read(snapshot)&&!bridge.request(MNM_MENU_QUIT,snapshot));
    require(bridge.request(MNM_MENU_BACK,snapshot)&&get(7)==2);
    state(8,4,3,1,2,MNM_MENU_OK);require(bridge.read(snapshot));
    put(16,9);require(!bridge.read(snapshot));put(16,8);
    put(18,999);require(!bridge.read(snapshot));put(18,3);
    put(2,2);require(!bridge.read(snapshot));put(2,1);
    put(19,2);require(!bridge.read(snapshot));put(19,1);
    state(10,4,3,1,2,MNM_MENU_RETIRED);require(bridge.read(snapshot)&&!bridge.request(MNM_MENU_OPEN_QUICK,snapshot));
    state(12,4,3,1,2,MNM_MENU_OK);require(bridge.read(snapshot)&&bridge.request(MNM_MENU_QUIT,snapshot)&&get(7)==3&&get(8)==MNM_MENU_QUIT);bridge.retire();
    require(get(5)==0&&!bridge.request(MNM_MENU_OPEN_QUICK,snapshot));beat=get(6);bridge.heartbeat();require(get(6)==beat);
    peer.unmap(data);std::puts("Menu bridge checks passed");return 0;
}
