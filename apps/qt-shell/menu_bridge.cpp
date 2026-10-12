#include "menu_bridge.hpp"
#include "../../protocols/include/mnm/menu_v18.h"
#include <QtEndian>
#include <cstring>
#include <algorithm>
namespace {
quint32 load(const quint32* p){return qFromLittleEndian(__atomic_load_n(p,__ATOMIC_ACQUIRE));}
void store(quint32* p,quint32 v){__atomic_store_n(p,qToLittleEndian(v),__ATOMIC_RELEASE);}
}
MenuBridge::~MenuBridge(){retire();if(mapping_)file_.unmap(mapping_);}
bool MenuBridge::create(const QString& path,bool battle,bool spells,bool mini,bool results,bool preferences,bool region,bool enter,bool campaignMini,bool campaignQuit,bool campaignDefeat,bool campaignPreferences,bool storage,bool grimoire,bool multiplayer,bool sessions,bool lobby,bool character){
    character_=character;lobby=lobby||character; lobby_=lobby;sessions=sessions||lobby;sessions_=sessions;multiplayer=multiplayer||sessions;multiplayer_=multiplayer;grimoire=grimoire||multiplayer;grimoire_=grimoire;storage=storage||grimoire;storage_=storage;campaignPreferences=campaignPreferences||storage;campaignPreferences_=campaignPreferences;campaignDefeat=campaignDefeat||campaignPreferences;campaignDefeat_=campaignDefeat;campaignQuit=campaignQuit||campaignDefeat;campaignQuit_=campaignQuit;campaignMini=campaignMini||campaignQuit;campaignMini_=campaignMini;enter_=enter||campaignMini;enter=enter_;mini=mini||campaignMini;region_=region||enter;region=region_;preferences_=preferences||region;preferences=preferences_;results_=results||preferences;mini_=mini;spells_=spells||mini||results_;battle_=battle||spells_;const int size=character?MNM_MENU_V18_SIZE:lobby?MNM_MENU_V17_SIZE:sessions?MNM_MENU_V16_SIZE:multiplayer?MNM_MENU_V15_SIZE:grimoire?MNM_MENU_V14_SIZE:storage?MNM_MENU_V13_SIZE:campaignPreferences?MNM_MENU_V12_SIZE:campaignDefeat?MNM_MENU_V11_SIZE:campaignQuit?MNM_MENU_V10_SIZE:campaignMini?MNM_MENU_V9_SIZE:enter?MNM_MENU_V8_SIZE:region?MNM_MENU_V7_SIZE:preferences?MNM_MENU_V6_SIZE:results?MNM_MENU_V5_SIZE:mini?MNM_MENU_V4_SIZE:spells?MNM_MENU_V3_SIZE:battle?MNM_MENU_V2_SIZE:MNM_MENU_V1_SIZE;
    if(file_.isOpen())return false;
    file_.setFileName(path);
    if(!file_.open(QIODevice::ReadWrite|QIODevice::NewOnly)||!file_.resize(size))return false;
    QByteArray data(size,0);std::memcpy(data.data(),character?MNM_MENU_V18_MAGIC:lobby?MNM_MENU_V17_MAGIC:sessions?MNM_MENU_V16_MAGIC:multiplayer?MNM_MENU_V15_MAGIC:grimoire?MNM_MENU_V14_MAGIC:storage?MNM_MENU_V13_MAGIC:campaignPreferences?MNM_MENU_V12_MAGIC:campaignDefeat?MNM_MENU_V11_MAGIC:campaignQuit?MNM_MENU_V10_MAGIC:campaignMini?MNM_MENU_V9_MAGIC:enter?MNM_MENU_V8_MAGIC:region?MNM_MENU_V7_MAGIC:preferences?MNM_MENU_V6_MAGIC:results?MNM_MENU_V5_MAGIC:mini?MNM_MENU_V4_MAGIC:spells?MNM_MENU_V3_MAGIC:battle?MNM_MENU_V2_MAGIC:MNM_MENU_V1_MAGIC,8);
    qToLittleEndian<quint32>(character?18:lobby?17:sessions?16:multiplayer?15:grimoire?14:storage?13:campaignPreferences?12:campaignDefeat?11:campaignQuit?10:campaignMini?9:enter?8:region?7:preferences?6:results?5:mini?4:spells?3:battle?2:1,data.data()+8);qToLittleEndian<quint32>(size,data.data()+12);
    if(file_.write(data)!=data.size()||!file_.flush())return false;
    mapping_=file_.map(0,data.size());if(!mapping_)return false;
    heartbeat();return true;
}
bool MenuBridge::read(State& state) const {
    if(!mapping_||std::memcmp(mapping_,character_?MNM_MENU_V18_MAGIC:lobby_?MNM_MENU_V17_MAGIC:sessions_?MNM_MENU_V16_MAGIC:multiplayer_?MNM_MENU_V15_MAGIC:grimoire_?MNM_MENU_V14_MAGIC:storage_?MNM_MENU_V13_MAGIC:campaignPreferences_?MNM_MENU_V12_MAGIC:campaignDefeat_?MNM_MENU_V11_MAGIC:campaignQuit_?MNM_MENU_V10_MAGIC:campaignMini_?MNM_MENU_V9_MAGIC:enter_?MNM_MENU_V8_MAGIC:region_?MNM_MENU_V7_MAGIC:preferences_?MNM_MENU_V6_MAGIC:results_?MNM_MENU_V5_MAGIC:mini_?MNM_MENU_V4_MAGIC:spells_?MNM_MENU_V3_MAGIC:battle_?MNM_MENU_V2_MAGIC:MNM_MENU_V1_MAGIC,8))return false;
    auto* words=reinterpret_cast<quint32*>(mapping_);
    if(load(words+2)!=(character_?18u:lobby_?17u:sessions_?16u:multiplayer_?15u:grimoire_?14u:storage_?13u:campaignPreferences_?12u:campaignDefeat_?11u:campaignQuit_?10u:campaignMini_?9u:enter_?8u:region_?7u:preferences_?6u:results_?5u:mini_?4u:spells_?3u:battle_?2u:1u)||load(words+3)!=(character_?MNM_MENU_V18_SIZE:lobby_?MNM_MENU_V17_SIZE:sessions_?MNM_MENU_V16_SIZE:multiplayer_?MNM_MENU_V15_SIZE:grimoire_?MNM_MENU_V14_SIZE:storage_?MNM_MENU_V13_SIZE:campaignPreferences_?MNM_MENU_V12_SIZE:campaignDefeat_?MNM_MENU_V11_SIZE:campaignQuit_?MNM_MENU_V10_SIZE:campaignMini_?MNM_MENU_V9_SIZE:enter_?MNM_MENU_V8_SIZE:region_?MNM_MENU_V7_SIZE:preferences_?MNM_MENU_V6_SIZE:results_?MNM_MENU_V5_SIZE:mini_?MNM_MENU_V4_SIZE:spells_?MNM_MENU_V3_SIZE:battle_?MNM_MENU_V2_SIZE:MNM_MENU_V1_SIZE))return false;
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
        b.mode=load(m+6);
        if(campaignMini_){
            const quint32 offered=storage_?31u:campaignPreferences_?7u:campaignQuit_?5u:1u;
            if(load(m+7)||b.battle||b.confirmation>1||b.depth!=5||b.parentScreen!=2||b.context!=5||b.mode!=2||(b.actions&~offered)||(b.confirmation&&b.actions)||(next.ready&&(b.confirmation||b.actions!=offered)))return false;
        }else if(load(m+6)||load(m+7)||b.battle!=1||b.confirmation>1||b.depth<1||b.depth>15||b.parentScreen!=MNM_MENU_MINI_PARENT_SCREEN||b.context==5||(b.actions&~7u)||(b.confirmation&&b.actions)||(next.ready&&(b.confirmation||b.actions!=7)))return false;
    }
    if(campaignDefeat_&&next.screen==MNM_MENU_DEFEAT_SCREEN){
        auto* m=words+MNM_MENU_V11_DEFEAT/4;auto& d=next.defeat;
        d.actions=load(m);d.context=load(m+1);d.depth=load(m+2);d.outcome=load(m+3);
        if(d.actions!=1||d.context!=5||d.depth!=5||d.outcome||!name(MNM_MENU_V11_DEFEAT+16,256,d.title)||d.title.isEmpty())return false;
        for(int i=0;i<21;++i)if(!name(MNM_MENU_V11_DEFEAT+272+256*i,256,d.texts[i]))return false;
        if(d.texts[19].isEmpty())return false;
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
        if((b.actions&~3u)||(b.available&~16383u)||(b.parentScreen!=3&&!(campaignPreferences_&&b.parentScreen==17&&b.depth==6))||b.depth<1||b.depth>15||load(m+11)||
           b.values[0]<0||b.values[0]>15||b.values[1]<-2500||b.values[1]>0||b.values[2]<0||b.values[2]>1||
           b.values[3]<0||b.values[3]>1||b.values[4]<0||b.values[4]>2||b.values[5]<0||b.values[5]>2||b.values[6]<0||b.values[6]>1)return false;
    }
    if(region_&&next.screen==MNM_MENU_REGION_SCREEN){
        auto* m=words+MNM_MENU_V7_REGION/4;auto& b=next.region;
        b.caller=load(m);b.depth=load(m+1);b.different=load(m+2);b.difficulty=load(m+3);b.available=load(m+4);b.actions=load(m+5);b.region=load(m+6);
        if(b.caller!=4||b.depth!=4||b.different||b.difficulty>3||(b.available&~15u)||(b.actions&~((enter_?3u:1u)|(grimoire_?4u:0u)))||b.region!=1||load(m+7)||
           !name(MNM_MENU_V7_REGION+32,128,b.realm)||b.realm!="Celtic"||!name(MNM_MENU_V7_REGION+160,128,b.name)||b.name.isEmpty()||b.name.contains('\n')||b.name.contains('\r'))return false;
        for(int i=72;i<80;++i)if(load(m+i))return false;
    }
    if(storage_&&(next.screen==MNM_MENU_LOAD_SCREEN||next.screen==MNM_MENU_SAVE_SCREEN)){
        auto* m=words+MNM_MENU_V13_STORAGE/4;auto& b=next.storage;
        b.screen=load(m);b.parent=load(m+1);b.depth=load(m+2);b.context=load(m+3);b.modal=load(m+4);b.pending=load(m+5);const auto count=load(m+6);b.selected=load(m+7);
        if(b.screen!=next.screen||b.context!=5||b.modal>1||b.pending>1||count>MNM_MENU_V13_MAX_SAVES||b.selected>count||
           !((b.parent==3&&b.depth==3&&next.screen==MNM_MENU_LOAD_SCREEN)||(b.parent==17&&b.depth==6))||
           (next.ready&&(b.modal||b.pending))||!name(MNM_MENU_V13_STORAGE+32,128,b.fileName))return false;
        for(quint32 i=0;i<count;++i){QString stem;if(!name(MNM_MENU_V13_STORAGE+160+128*i,128,stem)||stem.isEmpty()||stem.contains('\n')||stem.contains('\r')||b.names.contains(stem))return false;b.names.append(stem);}
    }
    if(grimoire_&&next.screen==MNM_MENU_GRIMOIRE_SCREEN){
        auto* m=words+MNM_MENU_V14_GRIMOIRE/4;auto& b=next.grimoire;
        b.chapter=load(m);b.section=load(m+1);b.cursor=load(m+2);b.available=load(m+3);b.parent=load(m+4);b.depth=load(m+5);b.context=load(m+6);
        if(!((b.chapter==0&&b.section<=4)||(b.chapter==1&&b.section==1))||b.cursor!=1||b.available!=63||b.parent!=18||b.depth!=5||b.context!=5||load(m+7))return false;
    }
    if(multiplayer_&&(next.screen==15||next.screen==16)){
        auto* m=words+MNM_MENU_V15_MULTIPLAYER/4;auto& b=next.multiplayer;
        b.parent=load(m);b.depth=load(m+1);b.context=load(m+2);b.transport=load(m+3);b.available=load(m+4);b.actions=load(m+5);b.modal=load(m+6);b.pending=load(m+7);b.gameLimit=load(m+8);b.userLimit=load(m+9);
        if(b.parent!=22||b.depth!=4||b.context!=2||b.transport>2||(b.available&~7u)||(b.actions&~3u)||b.modal>1||b.pending>1||b.gameLimit!=(next.screen==15?256u:0u)||b.userLimit!=15||load(m+10)||load(m+11))return false;
        if(!name(MNM_MENU_V15_MULTIPLAYER+48,64,b.gameName)||!name(MNM_MENU_V15_MULTIPLAYER+112,16,b.userName)||(next.screen==16&&!b.gameName.isEmpty()))return false;
    }
    if(sessions_&&next.screen==21){
        auto* m=words+MNM_MENU_V16_SESSIONS/4;auto& b=next.sessions;
        b.parent=load(m);b.depth=load(m+1);b.context=load(m+2);b.transport=load(m+3);b.actions=load(m+4);b.active=load(m+5);b.busy=load(m+6);b.pending=load(m+7);const auto count=load(m+8);b.selected=load(m+9);
        if(b.parent!=16||b.depth!=5||b.context!=2||b.transport<1||b.transport>3||(b.actions&~3u)||b.active>1||b.busy>1||b.pending>1||count>100||b.selected>count)return false;
        for(quint32 i=0;i<count;++i){QString text;if(!name(MNM_MENU_V16_SESSIONS+40+128*i,128,text)||text.isEmpty())return false;b.names.append(text);}
    }
    if(character_&&next.screen==MNM_MENU_CHARACTER_SCREEN&&next.ready){
        auto* m=words+MNM_MENU_V18_CHARACTER/4;auto& b=next.character;
        b.parent=load(m);b.depth=load(m+1);b.context=load(m+2);b.wizard=load(m+3);b.experience=load(m+4);b.actions=load(m+5);b.adjustments=load(m+6);
        if(!((b.parent==4&&b.depth==4)||(b.parent==18&&b.depth==5))||b.context!=5||b.wizard>=80||b.experience>1000000000||b.actions!=3||(b.adjustments&~4095u)||load(m+7))return false;
        const quint32 limits[]={200,800,40,7,7,7},steps[]={5,20,1,1,1,1};
        for(int i=0;i<6;++i){auto* r=m+8+i*6;auto& row=b.rows[i];row={load(r),load(r+1),load(r+2),load(r+3),load(r+4),load(r+5)};
            if(row.maximum!=limits[i]||row.increment!=steps[i]||row.opening>row.value||row.value>row.maximum||(row.value-row.opening)%row.increment||row.decrease>1000000||row.increase>1000000)return false;
            if((b.adjustments&(1u<<(2*i)))&&(row.value<row.opening+row.increment||row.decrease>1000000000-b.experience))return false;
            if((b.adjustments&(1u<<(2*i+1)))&&(row.value+row.increment>row.maximum||row.increase>b.experience))return false;
        }
        if(!name(MNM_MENU_V18_CHARACTER+176,32,b.name)||b.name.isEmpty())return false;
        for(int i=208;i<256;i+=4)if(load(words+(MNM_MENU_V18_CHARACTER+i)/4))return false;
    }
    if(lobby_&&next.screen==24){
        auto* m=words+MNM_MENU_V17_LOBBY/4;auto& b=next.lobby;b.parent=load(m);b.depth=load(m+1);b.context=load(m+2);b.provider=load(m+3);b.actions=load(m+4);b.map=load(m+5);b.localSlot=load(m+6);
        if(b.parent!=15||b.depth!=5||b.context!=2||b.provider<1||b.provider>3||b.actions>1||b.map>10000||b.localSlot||load(m+7))return false;
        for(int i=0;i<13;++i){const auto value=load(m+8+i);if(value>10000)return false;b.rules[i]=int(value);}
        for(int i=0;i<4;++i){const int at=MNM_MENU_V17_LOBBY+84+i*48;auto* row=words+at/4;auto& player=b.players[i];const auto active=load(row);if(active!=quint32(i==0))return false;player.active=active;player.portrait=int(load(row+1));player.colour=int(load(row+2));player.handicap=int(load(row+3));if(player.handicap<0||player.handicap>50||(active&&(player.portrait<0||player.portrait>11||player.colour<0||player.colour>7)))return false;if(!name(at+16,32,player.name)||(active&&player.name.isEmpty()))return false;}
        if(!name(MNM_MENU_V17_LOBBY+276,128,b.gameName)||b.gameName.isEmpty()||!name(MNM_MENU_V17_LOBBY+404,128,b.mapName))return false;
        for(int i=0;i<3;++i)if(load(words+(MNM_MENU_V17_LOBBY+532)/4+i))return false;
    }
    __atomic_thread_fence(__ATOMIC_ACQUIRE);if(seq!=load(p))return false;
    if(next.ready>1||next.status>(battle_?MNM_MENU_INVALID:MNM_MENU_RETIRED)||next.handoff>(enter_?3u:2u)||(next.screen!=0&&next.screen!=3&&next.screen!=22&&!(battle_&&(next.screen==14||next.screen==25||(multiplayer_&&(next.screen==15||next.screen==16))||(sessions_&&next.screen==21)||(lobby_&&next.screen==24)||(character_&&next.screen==20)||(grimoire_&&next.screen==MNM_MENU_GRIMOIRE_SCREEN)||(spells_&&next.screen==7)||(mini_&&next.screen==MNM_MENU_MINI_SCREEN)||(results_&&next.screen==MNM_MENU_RESULT_SCREEN)||(campaignDefeat_&&next.screen==MNM_MENU_DEFEAT_SCREEN)||(preferences_&&next.screen==MNM_MENU_PREFERENCES_SCREEN)||(region_&&next.screen==MNM_MENU_REGION_SCREEN)||(storage_&&(next.screen==MNM_MENU_LOAD_SCREEN||next.screen==MNM_MENU_SAVE_SCREEN))))))return false;
    state=next;return true;
}
void MenuBridge::publish(bool alive,quint32 action,quint32 generation,quint32 argument,const std::array<int,17>* rules,const std::array<int,63>* assignments,const std::array<int,7>* preferences,const QByteArray* storageName,const QByteArray* multiplayerNames){
    if(!mapping_)return;
    auto* p=reinterpret_cast<quint32*>(mapping_)+MNM_MENU_V1_HOST_WORD;
    auto seq=load(p);store(p,seq+1);store(p+1,alive?1:0);store(p+2,++heartbeat_);
    store(p+3,request_);
    // Heartbeats preserve the last request fields; they never resubmit it.
    if(action){store(p+4,action);store(p+5,generation);if(battle_){store(p+6,argument);for(int i=0;i<17;++i)store(p+7+i,rules?quint32((*rules)[i]):0);}}
    if(spells_&&assignments)for(int i=0;i<63;++i)store(reinterpret_cast<quint32*>(mapping_)+MNM_MENU_V3_HOST_SLOTS/4+i,quint32((*assignments)[i]));
    if(preferences_&&preferences)for(int i=0;i<7;++i)store(reinterpret_cast<quint32*>(mapping_)+MNM_MENU_V6_HOST_PREFERENCES/4+i,quint32((*preferences)[i]));
    if(storage_&&storageName){std::memset(mapping_+MNM_MENU_V13_HOST_NAME,0,MNM_MENU_V13_NAME_SIZE);std::memcpy(mapping_+MNM_MENU_V13_HOST_NAME,storageName->constData(),storageName->size());}
    if(multiplayer_&&multiplayerNames){std::memset(mapping_+MNM_MENU_V15_HOST_NAMES,0,MNM_MENU_V15_NAMES_SIZE);std::memcpy(mapping_+MNM_MENU_V15_HOST_NAMES,multiplayerNames->constData(),multiplayerNames->size());}
    store(p,seq+2);
}
void MenuBridge::heartbeat(bool alive){if(!retired_)publish(alive);}
bool MenuBridge::request(quint32 action,const State& state,quint32 argument,const std::array<int,17>* rules){
    State current;
    if(!read(current))return false;
    const bool setupAction=action==MNM_MENU_SETUP_MAP||action==MNM_MENU_SETUP_START||action==MNM_MENU_SETUP_PLAYER||action==MNM_MENU_SETUP_APPLY;
    const bool miniAction=mini_&&current.screen==MNM_MENU_MINI_SCREEN&&(current.mini.battle||campaignMini_)&&!current.mini.confirmation&&((action==MNM_MENU_MINI_CANCEL&&(current.mini.actions&MNM_MENU_MINI_CAN_CANCEL))||(action==MNM_MENU_MINI_PREFERENCES&&(current.mini.actions&MNM_MENU_MINI_CAN_PREFERENCES))||(action==MNM_MENU_MINI_QUIT&&(current.mini.actions&MNM_MENU_MINI_CAN_QUIT))||(storage_&&action==MNM_MENU_MINI_LOAD&&(current.mini.actions&MNM_MENU_MINI_CAN_LOAD))||(storage_&&action==MNM_MENU_MINI_SAVE&&(current.mini.actions&MNM_MENU_MINI_CAN_SAVE)));
    const bool resultAction=results_&&current.screen==MNM_MENU_RESULT_SCREEN&&((action==MNM_MENU_RESULT_CONTINUE&&(current.results.actions&1))||(action==MNM_MENU_RESULT_QUIT&&(current.results.actions&2)));
    const bool regionAction=region_&&((action==MNM_MENU_NEW_GAME&&current.screen==3)||(current.screen==18&&
        ((grimoire_&&action==MNM_MENU_REGION_GRIMOIRE&&!argument&&(current.region.actions&4))||(enter_&&action==MNM_MENU_REGION_ENTER&&(current.region.actions&2))||(action==MNM_MENU_REGION_CANCEL&&(current.region.actions&1))||(action==MNM_MENU_REGION_DIFFICULTY&&argument<4&&(current.region.available&(1u<<argument))))));
    const bool bookAction=grimoire_&&current.screen==MNM_MENU_GRIMOIRE_SCREEN&&((action==MNM_MENU_GRIMOIRE_CLOSE&&!argument)||(action==MNM_MENU_GRIMOIRE_SELECT&&argument<6&&(current.grimoire.available&(1u<<argument))));
    const bool sessionAction=sessions_&&current.screen==21&&!current.sessions.busy&&!current.sessions.pending&&((action==MNM_MENU_SESSION_CANCEL&&!argument&&(current.sessions.actions&2))||(action==MNM_MENU_SESSION_JOIN&&argument&&argument<=quint32(current.sessions.names.size())&&current.sessions.active&&(current.sessions.actions&1)));
    const bool characterAction=character_&&current.screen==20&&((action==MNM_MENU_CHARACTER_ADJUST&&argument<12&&(current.character.adjustments&(1u<<argument)))||((action==MNM_MENU_CHARACTER_OK||action==MNM_MENU_CHARACTER_CANCEL)&&!argument));
    const bool allowed=characterAction||(lobby_&&current.screen==24&&action==MNM_MENU_LOBBY_CANCEL&&!argument&&(current.lobby.actions&1))||sessionAction||(multiplayer_&&!argument&&((current.screen==22&&(action==MNM_MENU_OPEN_CREATE||action==MNM_MENU_OPEN_JOIN))||((current.screen==15||current.screen==16)&&action==MNM_MENU_MULTIPLAYER_CANCEL&&(current.multiplayer.actions&2))))||bookAction||(storage_&&action==MNM_MENU_OPEN_LOAD&&current.screen==3)||(campaignDefeat_&&current.screen==MNM_MENU_DEFEAT_SCREEN&&action==MNM_MENU_DEFEAT_OK&&current.defeat.actions==1)||regionAction||(preferences_&&action==MNM_MENU_OPEN_PREFERENCES&&current.screen==3)||resultAction||miniAction||(action==MNM_MENU_OPEN_QUICK&&current.screen==3)||(action==MNM_MENU_BACK&&current.screen==22)||(action==MNM_MENU_QUIT&&current.screen==3)||
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

namespace {
bool storageStem(const QString& name){
    if(name.isEmpty()||name.size()>=MNM_MENU_V13_NAME_SIZE||name.front()==' '||name.back()==' '||name.back()=='.')return false;
    for(const auto c:name)if(c.unicode()<32||c.unicode()>=127||QStringLiteral("/\\:\"<>|?*").contains(c))return false;
    const auto key=name.section('.',0,0).toUpper();
    if(key=="CON"||key=="PRN"||key=="AUX"||key=="NUL"||(key.size()==4&&(key.startsWith("COM")||key.startsWith("LPT"))&&key[3]>='1'&&key[3]<='9'))return false;
    return true;
}
}
bool MenuBridge::requestStorage(quint32 action,const State& state,quint32 row,const QString& name){
    State current;if(!storage_||retired_||!read(current)||!current.ready||current.status==MNM_MENU_RETIRED||current.ack!=request_||
       current.generation!=state.generation||current.screen!=state.screen||request_==UINT32_MAX||(current.screen!=MNM_MENU_LOAD_SCREEN&&current.screen!=13))return false;
    if(action==MNM_MENU_STORAGE_CANCEL){if(row||!name.isEmpty())return false;}
    else if(action==MNM_MENU_STORAGE_ACCEPT||action==MNM_MENU_STORAGE_DELETE){
        if(current.screen==MNM_MENU_LOAD_SCREEN||action==MNM_MENU_STORAGE_DELETE){if((current.screen==MNM_MENU_LOAD_SCREEN&&action==MNM_MENU_STORAGE_DELETE)||!name.isEmpty()||!row||row>quint32(current.storage.names.size())||!storageStem(current.storage.names[int(row)-1]))return false;}
        else if(row||!storageStem(name))return false;
    }else return false;
    const auto bytes=name.toLatin1();++request_;publish(true,action,current.generation,row,nullptr,nullptr,nullptr,&bytes);return true;
}

bool MenuBridge::requestMultiplayer(quint32 action,const State& state,quint32 transport,const QString& gameName,const QString& userName){
    State current;if(!multiplayer_||retired_||!read(current)||!current.ready||current.status==MNM_MENU_RETIRED||current.ack!=request_||current.generation!=state.generation||current.screen!=state.screen||request_==UINT32_MAX||(current.screen!=15&&current.screen!=16)||current.multiplayer.modal||current.multiplayer.pending)return false;
    if(action==MNM_MENU_MULTIPLAYER_CANCEL)return !transport&&gameName.isEmpty()&&userName.isEmpty()&&request(action,state);
    auto ascii=[](const QString& name,int max){return !name.isEmpty()&&name.size()<=max&&std::all_of(name.begin(),name.end(),[](QChar c){return c.unicode()>=32&&c.unicode()<127;});};
    if(action!=MNM_MENU_MULTIPLAYER_SUBMIT||transport>2||!(current.multiplayer.available&(1u<<transport))||!(current.multiplayer.actions&1)||!ascii(userName,15)||(current.screen==15?!ascii(gameName,63):!gameName.isEmpty()))return false;
    QByteArray names(MNM_MENU_V15_NAMES_SIZE,0);const auto game=gameName.toLatin1(),user=userName.toLatin1();std::memcpy(names.data(),game.constData(),game.size());std::memcpy(names.data()+64,user.constData(),user.size());
    ++request_;publish(true,action,current.generation,transport,nullptr,nullptr,nullptr,nullptr,&names);return true;
}
