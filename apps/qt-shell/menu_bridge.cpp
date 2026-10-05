#include "menu_bridge.hpp"
#include "../../protocols/include/mnm/menu_v2.h"
#include <QtEndian>
#include <cstring>
#include <algorithm>
namespace {
quint32 load(const quint32* p){return qFromLittleEndian(__atomic_load_n(p,__ATOMIC_ACQUIRE));}
void store(quint32* p,quint32 v){__atomic_store_n(p,qToLittleEndian(v),__ATOMIC_RELEASE);}
}
MenuBridge::~MenuBridge(){retire();if(mapping_)file_.unmap(mapping_);}
bool MenuBridge::create(const QString& path,bool battle){
    battle_=battle;const int size=battle?MNM_MENU_V2_SIZE:MNM_MENU_V1_SIZE;
    if(file_.isOpen())return false;
    file_.setFileName(path);
    if(!file_.open(QIODevice::ReadWrite|QIODevice::NewOnly)||!file_.resize(size))return false;
    QByteArray data(size,0);std::memcpy(data.data(),battle?MNM_MENU_V2_MAGIC:MNM_MENU_V1_MAGIC,8);
    qToLittleEndian<quint32>(battle?2:1,data.data()+8);qToLittleEndian<quint32>(size,data.data()+12);
    if(file_.write(data)!=data.size()||!file_.flush())return false;
    mapping_=file_.map(0,data.size());if(!mapping_)return false;
    heartbeat();return true;
}
bool MenuBridge::read(State& state) const {
    if(!mapping_||std::memcmp(mapping_,battle_?MNM_MENU_V2_MAGIC:MNM_MENU_V1_MAGIC,8))return false;
    auto* words=reinterpret_cast<quint32*>(mapping_);
    if(load(words+2)!=(battle_?2u:1u)||load(words+3)!=(battle_?MNM_MENU_V2_SIZE:MNM_MENU_V1_SIZE))return false;
    auto* p=words+(battle_?MNM_MENU_V2_ENGINE_WORD:MNM_MENU_V1_ENGINE_WORD);const auto seq=load(p);if(!seq||(seq&1))return false;
    State next;next.generation=load(p+1);next.screen=load(p+2);next.ready=load(p+3);next.ack=load(p+4);next.status=load(p+5);next.thread=load(p+6);next.sequence=seq;
    if(battle_){
        next.handoff=load(p+7);
        if(next.ready&&(next.screen==14||next.screen==25)){
            auto number=[&](int offset){return load(words+offset/4);};
            auto name=[&](int offset,int length,QString& text){
                const auto bytes=QByteArray(reinterpret_cast<const char*>(mapping_)+offset,length);const int end=bytes.indexOf('\0');if(end<0)return false;
                static const ushort cp[32]={0x20ac,0x81,0x201a,0x192,0x201e,0x2026,0x2020,0x2021,0x2c6,0x2030,0x160,0x2039,0x152,0x8d,0x17d,0x8f,0x90,0x2018,0x2019,0x201c,0x201d,0x2022,0x2013,0x2014,0x2dc,0x2122,0x161,0x203a,0x153,0x9d,0x17e,0x178};
                for(int i=0;i<end;++i){auto c=static_cast<unsigned char>(bytes[i]);text+=QChar(c>=128&&c<160?cp[c-128]:c);}return true;
            };
            auto& b=next.battle;b.map=number(MNM_MENU_V2_MAP);const auto count=number(MNM_MENU_V2_MAP_COUNT);
            if(count>MNM_MENU_V2_MAX_MAPS||b.map>10000||!name(MNM_MENU_V2_MAP_NAME,128,b.mapName))return false;
            for(int i=0;i<13;++i){auto value=number(MNM_MENU_V2_RULES+4*i);if(value>10000)return false;b.rules[i]=int(value);}
            for(int i=0;i<4;++i){auto& player=b.players[i];int offset=MNM_MENU_V2_PLAYERS+i*48;
                auto active=number(offset);if(active>1)return false;player.active=active;
                player.portrait=int(number(offset+4));player.colour=int(number(offset+8));player.handicap=int(number(offset+12));
                if(player.handicap<0||player.handicap>50||(player.active&&(player.portrait<0||player.portrait>11||player.colour<0||player.colour>7))||!name(offset+16,32,player.name))return false;
            }
            for(quint32 i=0;i<count;++i){QString text;if(!name(MNM_MENU_V2_MAP_NAMES+i*128,128,text)||text.isEmpty())return false;b.maps.append(text);}
        }
    }
    __atomic_thread_fence(__ATOMIC_ACQUIRE);if(seq!=load(p))return false;
    if(next.ready>1||next.status>(battle_?MNM_MENU_INVALID:MNM_MENU_RETIRED)||next.handoff>2||(next.screen!=0&&next.screen!=3&&next.screen!=22&&!(battle_&&(next.screen==14||next.screen==25))))return false;
    state=next;return true;
}
void MenuBridge::publish(bool alive,quint32 action,quint32 generation,quint32 argument,const std::array<int,17>* rules){
    if(!mapping_)return;
    auto* p=reinterpret_cast<quint32*>(mapping_)+MNM_MENU_V1_HOST_WORD;
    auto seq=load(p);store(p,seq+1);store(p+1,alive?1:0);store(p+2,++heartbeat_);
    store(p+3,request_);
    // Heartbeats preserve the last request fields; they never resubmit it.
    if(action){store(p+4,action);store(p+5,generation);if(battle_){store(p+6,argument);for(int i=0;i<17;++i)store(p+7+i,rules?quint32((*rules)[i]):0);}}
    store(p,seq+2);
}
void MenuBridge::heartbeat(bool alive){if(!retired_)publish(alive);}
bool MenuBridge::request(quint32 action,const State& state,quint32 argument,const std::array<int,17>* rules){
    State current;
    if(!read(current))return false;
    const bool setupAction=action==MNM_MENU_SETUP_MAP||action==MNM_MENU_SETUP_START||action==MNM_MENU_SETUP_PLAYER||action==MNM_MENU_SETUP_APPLY;
    const bool allowed=(action==MNM_MENU_OPEN_QUICK&&current.screen==3)||(action==MNM_MENU_BACK&&current.screen==22)||(action==MNM_MENU_QUIT&&current.screen==3)||
        (battle_&&((action==MNM_MENU_OPEN_SINGLE&&current.screen==22)||((setupAction||action==MNM_MENU_SETUP_CANCEL)&&current.screen==14)||((action==MNM_MENU_MAP_OK||action==MNM_MENU_MAP_CANCEL)&&current.screen==25)));
    if(!allowed||(setupAction&&!rules)||(rules&&std::any_of(rules->begin(),rules->end(),[](int value){return value<0||value>10000;})))return false;
    if(retired_||!current.ready||current.status==MNM_MENU_RETIRED||current.ack!=request_||
       current.generation!=state.generation||current.screen!=state.screen||request_==UINT32_MAX )return false;
    ++request_;publish(true,action,current.generation,argument,rules);return true;
}
void MenuBridge::retire(){if(!retired_){publish(false);retired_=true;}}
