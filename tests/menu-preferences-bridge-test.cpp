#include "menu_bridge.hpp"
#include "../protocols/include/mnm/menu_v6.h"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QtEndian>
#include <cstdio>
#include <cstdlib>
static void check(bool ok,int line){if(!ok){std::fprintf(stderr,"Preferences bridge failed at %d\n",line);std::exit(1);}}
#define require(ok) check((ok),__LINE__)
int main(int argc,char** argv){
    QCoreApplication app(argc,argv);QTemporaryDir root;MenuBridge bridge;
    const auto path=root.filePath("preferences.bin");require(bridge.create(path,false,false,false,false,true));
    QFile peer(path);require(peer.open(QIODevice::ReadWrite));auto* bytes=peer.map(0,MNM_MENU_V6_SIZE);require(bytes);
    auto put=[&](int word,quint32 value){qToLittleEndian(value,bytes+word*4);};
    auto get=[&](int word){return qFromLittleEndian<quint32>(bytes+word*4);};
    constexpr int m=MNM_MENU_V6_PREFERENCES/4;quint32 sequence=0;
    const std::array<int,7> values{0,-250,0,0,1,0,0};
    auto state=[&](quint32 generation,quint32 ack,quint32 screen=10,quint32 ready=1){
        put(32,++sequence);put(33,generation);put(34,screen);put(35,ready);put(36,ack);put(37,0);put(38,123);put(39,0);
        put(m,3);put(m+1,16383);put(m+2,3);put(m+3,3);put(m+11,0);
        for(int i=0;i<7;++i)put(m+4+i,quint32(values[i]));
        put(32,++sequence);
    };
    MenuBridge::State s;state(1,0,3);require(bridge.read(s)&&bridge.request(MNM_MENU_OPEN_PREFERENCES,s));
    state(2,1);require(bridge.read(s)&&s.preferences.values==values);
    auto draft=values;draft[1]=-500;draft[4]=2;
    require(bridge.requestPreferences(MNM_MENU_PREFERENCES_PREVIEW,s,draft,1));
    require(get(7)==2&&get(8)==MNM_MENU_PREFERENCES_PREVIEW&&get(10)==1);
    for(int i=0;i<7;++i)require(int(get(MNM_MENU_V6_HOST_PREFERENCES/4+i))==draft[i]);
    require(!bridge.requestPreferences(MNM_MENU_PREFERENCES_CANCEL,s,values)); // One outstanding request.
    state(3,2);require(!bridge.requestPreferences(MNM_MENU_PREFERENCES_OK,s,draft));require(bridge.read(s));
    const auto host=QByteArray(reinterpret_cast<const char*>(bytes+16),112);
    auto invalid=draft;invalid[6]=2;require(!bridge.requestPreferences(MNM_MENU_PREFERENCES_OK,s,invalid));
    require(host==QByteArray(reinterpret_cast<const char*>(bytes+16),112));
    invalid=draft;invalid[0]=-1;require(!bridge.requestPreferences(MNM_MENU_PREFERENCES_OK,s,invalid));
    invalid=draft;invalid[1]=-2501;require(!bridge.requestPreferences(MNM_MENU_PREFERENCES_OK,s,invalid));
    require(!bridge.requestPreferences(MNM_MENU_PREFERENCES_PREVIEW,s,draft,2));
    put(m+1,16383^(1<<13));require(bridge.read(s));require(!bridge.requestPreferences(MNM_MENU_PREFERENCES_PREVIEW,s,draft,1));
    require(!bridge.requestPreferences(MNM_MENU_PREFERENCES_OK,s,draft));
    draft[1]=values[1];put(m+1,16383^(1<<6));require(bridge.read(s));require(!bridge.requestPreferences(MNM_MENU_PREFERENCES_OK,s,draft));
    state(4,2,10,0);require(bridge.read(s)&&!bridge.requestPreferences(MNM_MENU_PREFERENCES_OK,s,values));
    for(int field=0;field<12;++field){state(5,2);put(m+field,field==1?16384:99);require(!bridge.read(s));}
    state(6,2);put(32,sequence+1);require(!bridge.read(s));
    state(6,2);put(2,5);require(!bridge.read(s));put(2,6);
    state(7,2);require(bridge.read(s));require(bridge.requestPreferences(MNM_MENU_PREFERENCES_CANCEL,s,invalid)); // Cancel restores engine entry, ignores draft.
    state(8,3);require(bridge.read(s));bridge.retire();require(!bridge.requestPreferences(MNM_MENU_PREFERENCES_OK,s,values));
    peer.unmap(bytes);
    MenuBridge legacy;require(legacy.create(root.filePath("legacy.bin"),true,true,false,true));
    require(!legacy.requestPreferences(MNM_MENU_PREFERENCES_OK,s,values));
    std::puts("Preferences bridge checks passed");
}
