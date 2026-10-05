#include "menu_bridge.hpp"
#include "../../protocols/include/mnm/menu_v6.h"
#include <QtEndian>
#include <cstring>
#include <algorithm>
namespace {
quint32 load(const quint32* p){return qFromLittleEndian(__atomic_load_n(p,__ATOMIC_ACQUIRE));}
void store(quint32* p,quint32 v){__atomic_store_n(p,qToLittleEndian(v),__ATOMIC_RELEASE);}
}
MenuBridge::~MenuBridge(){retire();if(mapping_)file_.unmap(mapping_);}
bool MenuBridge::create(const QString& path,bool battle,bool spells,bool mini,bool results,bool preferences){
    preferences_=preferences;results_=results||preferences;mini_=mini;spells_=spells||mini||results_;battle_=battle||spells_;const int size=preferences?MNM_MENU_V6_SIZE:results?MNM_MENU_V5_SIZE:mini?MNM_MENU_V4_SIZE:spells?MNM_MENU_V3_SIZE:battle?MNM_MENU_V2_SIZE:MNM_MENU_V1_SIZE;
    if(file_.isOpen())return false;
    file_.setFileName(path);
    if(!file_.open(QIODevice::ReadWrite|QIODevice::NewOnly)||!file_.resize(size))return false;
    QByteArray data(size,0);std::memcpy(data.data(),preferences?MNM_MENU_V6_MAGIC:results?MNM_MENU_V5_MAGIC:mini?MNM_MENU_V4_MAGIC:spells?MNM_MENU_V3_MAGIC:battle?MNM_MENU_V2_MAGIC:MNM_MENU_V1_MAGIC,8);
    qToLittleEndian<quint32>(preferences?6:results?5:mini?4:spells?3:battle?2:1,data.data()+8);qToLittleEndian<quint32>(size,data.data()+12);
    if(file_.write(data)!=data.size()||!file_.flush())return false;
    mapping_=file_.map(0,data.size());if(!mapping_)return false;
    heartbeat();return true;
}
bool MenuBridge::read(State& state) const {
    if(!mapping_||std::memcmp(mapping_,preferences_?MNM_MENU_V6_MAGIC:results_?MNM_MENU_V5_MAGIC:mini_?MNM_MENU_V4_MAGIC:spells_?MNM_MENU_V3_MAGIC:battle_?MNM_MENU_V2_MAGIC:MNM_MENU_V1_MAGIC,8))return false;
    auto* words=reinterpret_cast<quint32*>(mapping_);
    if(load(words+2)!=(preferences_?6u:results_?5u:mini_?4u:spells_?3u:battle_?2u:1u)||load(words+3)!=(preferences_?MNM_MENU_V6_SIZE:results_?MNM_MENU_V5_SIZE:mini_?MNM_MENU_V4_SIZE:spells_?MNM_MENU_V3_SIZE:battle_?MNM_MENU_V2_SIZE:MNM_MENU_V1_SIZE))return false;
    auto* p=words+(battle_?MNM_MENU_V2_ENGINE_WORD:MNM_MENU_V1_ENGINE_WORD);const auto seq=load(p);if(!seq||(seq&1))return false;
    State next;next.generation=load(p+1);next.screen=load(p+2);next.ready=load(p+3);next.ack=load(p+4);next.status=load(p+5);next.thread=load(p+6);next.sequence=seq;
    auto name=[&](int offset,int length,QString& text){
                const auto bytes=QByteArray(reinterpret_cast<const char*>(mapping_)+offset,length);const int end=bytes.indexOf('\0');if(end<0)return false;
                static const ushort cp[32]={0x20ac,0x81,0x201a,0x192,0x201e,0x2026,0x2020,0x2021,0x2c6,0x2030,0x160,0x2039,0x152,0x8d,0x17d,0x8f,0x90,0x2018,0x2019,0x201c,0x201d,0x2022,0x2013,0x2014,0x2dc,0x2122,0x161,0x203a,0x153,0x9d,0x17e,0x178};
                for(int i=0;i<end;++i){auto c=static_cast<unsigned char>(bytes[i]);text+=QChar(c>=128&&c<160?cp[c-128]:c);}return true;
            };
    if(battle_){
        next.handoff=load(p+7);
        if(next.ready&&(next.screen==14||next.screen==25)){
            auto number=[&](int offset){return load(words+offset/4);};
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
    if(spells_&&next.ready&&next.screen==7){
        auto number=[&](int off){return int(load(words+(MNM_MENU_V3_SPELL+off)/4));};auto& b=next.spells;
        b.owner=number(0);b.seconds=number(4);b.offered=quint32(number(8));if(b.owner<0||b.owner>3||b.seconds<-1||b.seconds>10000||(b.offered>>21))return false;
        quint32 seen=0;
        auto item=[&](int v){if(v==-1)return true;if(v<0||v>=21||(seen&(1u<<v)))return false;
        seen|=1u<<v;return true;};
        for(int a=0;a<3;++a){b.counts[a]=number(MNM_MENU_V3_COUNTS+4*a);if(b.counts[a]<0||b.counts[a]>7)return false;
            for(int i=0;i<21;++i){int v=number(MNM_MENU_V3_SLOTS+4*(a*21+i));if(!item(v)||(i>=b.counts[a]&&v!=-1))return false;b.assignments[a*21+i]=v;}}
        for(int i=0;i<25;++i){b.shelves[i]=number(MNM_MENU_V3_SHELVES+4*i);if(!item(b.shelves[i]))return false;}
        if(seen!=b.offered)return false;
        for(int i=0;i<21;++i){if(!(b.offered&(1u<<i)))continue;
            if(!name(MNM_MENU_V3_SPELL+MNM_MENU_V3_ITEM_NAMES+i*128,128,b.items[i])||b.items[i].isEmpty())return false;
            for(int a=0;a<3;++a){int at=i*3+a;b.recipes[at]=number(MNM_MENU_V3_RECIPES+at*4);if(b.recipes[at]<0||b.recipes[at]>92||!name(MNM_MENU_V3_SPELL+MNM_MENU_V3_SPELL_NAMES+at*128,128,b.names[at])||b.names[at].isEmpty())return false;}
        }
    }
    if(mini_&&next.screen==MNM_MENU_MINI_SCREEN){
        auto* m=words+MNM_MENU_V4_MINI/4;auto& b=next.mini;
        b.battle=load(m);b.confirmation=load(m+1);b.depth=load(m+2);b.parentScreen=load(m+3);b.actions=load(m+4);b.context=load(m+5);
        if(load(m+6)||load(m+7)||b.battle!=1||b.confirmation>1||b.depth<1||b.depth>15||b.parentScreen!=MNM_MENU_MINI_PARENT_SCREEN||b.context==5||(b.actions&~7u)||(b.confirmation&&b.actions)||(next.ready&&(b.confirmation||b.actions!=7)))return false;
    }
    if(results_&&next.screen==MNM_MENU_RESULT_SCREEN){
        const int at=MNM_MENU_V5_RESULTS;auto* r=words+at/4;auto& b=next.results;
        b.actions=load(r);b.context=load(r+1);b.depth=load(r+2);
        if(!b.actions||(b.actions&~3u)||b.context!=1||b.depth<1||b.depth>15||load(r+3))return false;
        for(int i=0;i<4;++i){auto& row=b.players[i];const int off=at+16+i*648;const auto active=load(words+off/4);
            if(active>1||load(words+(off+4)/4)!=UINT32_MAX)return false;
            row.active=active;
            QString* cells[]={&row.name,&row.kills,&row.deaths,&row.handicap,&row.score};
            for(int c=0;c<5;++c)if(!name(off+8+c*128,128,*cells[c]))return false;
            if(row.active!=!row.name.isEmpty())return false;
        }
    }
    if(preferences_&&next.screen==MNM_MENU_PREFERENCES_SCREEN){
        auto* m=words+MNM_MENU_V6_PREFERENCES/4;auto& b=next.preferences;
        b.actions=load(m);b.available=load(m+1);b.parentScreen=load(m+2);b.depth=load(m+3);
        for(int i=0;i<7;++i)b.values[i]=int(load(m+4+i));
        if((b.actions&~3u)||(b.available&~16383u)||b.parentScreen!=3||b.depth<1||b.depth>15||load(m+11)||
           b.values[0]<0||b.values[0]>15||b.values[1]<-2500||b.values[1]>0||b.values[2]<0||b.values[2]>1||
           b.values[3]<0||b.values[3]>1||b.values[4]<0||b.values[4]>2||b.values[5]<0||b.values[5]>2||b.values[6]<0||b.values[6]>1)return false;
    }
    __atomic_thread_fence(__ATOMIC_ACQUIRE);if(seq!=load(p))return false;
    if(next.ready>1||next.status>(battle_?MNM_MENU_INVALID:MNM_MENU_RETIRED)||next.handoff>2||(next.screen!=0&&next.screen!=3&&next.screen!=22&&!(battle_&&(next.screen==14||next.screen==25||(spells_&&next.screen==7)||(mini_&&next.screen==MNM_MENU_MINI_SCREEN)||(results_&&next.screen==MNM_MENU_RESULT_SCREEN)||(preferences_&&next.screen==MNM_MENU_PREFERENCES_SCREEN)))))return false;
    state=next;return true;
}
void MenuBridge::publish(bool alive,quint32 action,quint32 generation,quint32 argument,const std::array<int,17>* rules,const std::array<int,63>* assignments,const std::array<int,7>* preferences){
    if(!mapping_)return;
    auto* p=reinterpret_cast<quint32*>(mapping_)+MNM_MENU_V1_HOST_WORD;
    auto seq=load(p);store(p,seq+1);store(p+1,alive?1:0);store(p+2,++heartbeat_);
    store(p+3,request_);
    // Heartbeats preserve the last request fields; they never resubmit it.
    if(action){store(p+4,action);store(p+5,generation);if(battle_){store(p+6,argument);for(int i=0;i<17;++i)store(p+7+i,rules?quint32((*rules)[i]):0);}}
    if(spells_&&assignments)for(int i=0;i<63;++i)store(reinterpret_cast<quint32*>(mapping_)+MNM_MENU_V3_HOST_SLOTS/4+i,quint32((*assignments)[i]));
    if(preferences_&&preferences)for(int i=0;i<7;++i)store(reinterpret_cast<quint32*>(mapping_)+MNM_MENU_V6_HOST_PREFERENCES/4+i,quint32((*preferences)[i]));
    store(p,seq+2);
}
void MenuBridge::heartbeat(bool alive){if(!retired_)publish(alive);}
bool MenuBridge::request(quint32 action,const State& state,quint32 argument,const std::array<int,17>* rules){
    State current;
    if(!read(current))return false;
    const bool setupAction=action==MNM_MENU_SETUP_MAP||action==MNM_MENU_SETUP_START||action==MNM_MENU_SETUP_PLAYER||action==MNM_MENU_SETUP_APPLY;
    const bool miniAction=mini_&&current.screen==MNM_MENU_MINI_SCREEN&&current.mini.battle&&!current.mini.confirmation&&((action==MNM_MENU_MINI_CANCEL&&(current.mini.actions&MNM_MENU_MINI_CAN_CANCEL))||(action==MNM_MENU_MINI_PREFERENCES&&(current.mini.actions&MNM_MENU_MINI_CAN_PREFERENCES))||(action==MNM_MENU_MINI_QUIT&&(current.mini.actions&MNM_MENU_MINI_CAN_QUIT)));
    const bool resultAction=results_&&current.screen==MNM_MENU_RESULT_SCREEN&&((action==MNM_MENU_RESULT_CONTINUE&&(current.results.actions&1))||(action==MNM_MENU_RESULT_QUIT&&(current.results.actions&2)));
    const bool allowed=(preferences_&&action==MNM_MENU_OPEN_PREFERENCES&&current.screen==3)||resultAction||miniAction||(action==MNM_MENU_OPEN_QUICK&&current.screen==3)||(action==MNM_MENU_BACK&&current.screen==22)||(action==MNM_MENU_QUIT&&current.screen==3)||
        (battle_&&((action==MNM_MENU_OPEN_SINGLE&&current.screen==22)||((setupAction||action==MNM_MENU_SETUP_CANCEL)&&current.screen==14)||((action==MNM_MENU_MAP_OK||action==MNM_MENU_MAP_CANCEL)&&current.screen==25)));
    if(!allowed||(setupAction&&!rules)||(rules&&std::any_of(rules->begin(),rules->end(),[](int value){return value<0||value>10000;})))return false;
    if(retired_||!current.ready||current.status==MNM_MENU_RETIRED||current.ack!=request_||
       current.generation!=state.generation||current.screen!=state.screen||request_==UINT32_MAX )return false;
    ++request_;publish(true,action,current.generation,argument,rules);return true;
}
bool MenuBridge::finishSpells(const State& state,const std::array<int,63>& assignments){
    State current;if(!spells_||retired_||!read(current)||current.screen!=7||!current.ready||current.status==MNM_MENU_RETIRED||current.generation!=state.generation||current.ack!=request_||request_==UINT32_MAX)return false;
    quint32 seen=0;for(int a=0;a<3;++a)for(int i=0;i<21;++i){int v=assignments[a*21+i];
        if(v==-1)continue;
        if(i>=current.spells.counts[a]||v<0||v>=21||(seen&(1u<<v))||!(current.spells.offered&(1u<<v)))return false;
        seen|=1u<<v;}
    ++request_;publish(true,MNM_MENU_SPELL_FINISH,current.generation,0,nullptr,&assignments);return true;
}
bool MenuBridge::requestPreferences(quint32 action,const State& state,const std::array<int,7>& values,quint32 slider){
    State current;if(!preferences_||retired_||!read(current)||current.screen!=10||!current.ready||current.status==MNM_MENU_RETIRED||current.generation!=state.generation||state.screen!=10||current.ack!=request_||request_==UINT32_MAX)return false;
    const auto& b=current.preferences;
    if(action==MNM_MENU_PREFERENCES_CANCEL){if(!(b.actions&2))return false;}
    else if(action==MNM_MENU_PREFERENCES_OK||action==MNM_MENU_PREFERENCES_PREVIEW){
        if(values[0]<0||values[0]>15||values[1]<-2500||values[1]>0||values[2]<0||values[2]>1||values[3]<0||values[3]>1||values[4]<0||values[4]>2||values[5]<0||values[5]>2||values[6]<0||values[6]>1)return false;
        if(action==MNM_MENU_PREFERENCES_PREVIEW){if(slider>1||!(b.available&(1u<<(12+slider))))return false;}
        else {if(!(b.actions&1))return false;const int first[5]={0,2,4,7,10};
            for(int i=0;i<7;++i)if(values[i]!=b.values[i]){const int bit=i<2?12+i:first[i-2]+(i==6?!values[i]:values[i]);if(!(b.available&(1u<<bit)))return false;}}
    }else return false;
    ++request_;publish(true,action,current.generation,slider,nullptr,nullptr,&values);return true;
}
void MenuBridge::retire(){if(!retired_){publish(false);retired_=true;}}
