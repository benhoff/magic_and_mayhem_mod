#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#include "region_campaign_fixture_bytes.h"
#include <array>
#include <cstdlib>
#define THIS __attribute__((thiscall))
namespace region_adapter {
using u8=unsigned char;using u32=unsigned;
using ActionFn=u32(THIS*)(void*,u32);
static std::array<u8,800*600> map{};
static void check(bool ok,const char* why){if(!ok){std::cerr<<why<<'\n';std::exit(1);}}
static u32 get(const void* p){u32 n;std::memcpy(&n,p,4);return n;}
static void put(void* p,u32 n){std::memcpy(p,&n,4);}
static void word(u32 at,u32 n){put(reinterpret_cast<void*>(at),n);}
static bool readable(const void* p,u32 n){const auto at=reinterpret_cast<std::uintptr_t>(p),m=reinterpret_cast<std::uintptr_t>(map.data());
    return (at>=0x400000&&at<=0x700000&&n<=0x700000-at)||(at>=m&&at<=m+map.size()&&n<=m+map.size()-at);}
static bool equal(const void* a,const void* b,u32 n){return !std::memcmp(a,b,n);}
static bool battle_string(u8* out,u32 capacity,const void* text,u32 bound){
    if(!readable(text,bound))return false;
    for(u32 i=0;i<bound&&i<capacity;++i){out[i]=static_cast<const u8*>(text)[i];if(!out[i])return true;}return false;
}
static void record(u32,void*,u32,u32){}
#include "../runtime/menu/region_entry.h"
static void THIS transition(void* object){check(object==reinterpret_cast<void*>(0x6578c0),"original transition receiver");static_cast<u8*>(object)[0xc]=1;put(static_cast<u8*>(object)+0xd,0);}
static int THIS kind(const char* name){return !std::strcmp(name,"Celtic")?1:!std::strcmp(name,"Greek")?2:!std::strcmp(name,"Medieval")?3:0;}
static int wizard(){return 0;}
static void THIS sample(void*,int id,int,int,int,int,int,int){check(id==99999,"original capacity cue");}
static void stub(u32 at,const u8* bytes,void* target){check(!std::memcmp(reinterpret_cast<void*>(at),bytes,8),"expected dependency bytes");*reinterpret_cast<u8*>(at)=0xe9;word(at+1,u32(reinterpret_cast<std::uintptr_t>(target)-at-5));}
static void fixture(const char* name="Celtic",u32 occupants=0,bool special=false){
    std::memset(reinterpret_cast<void*>(0x6578c0),0,0x100);std::memset(reinterpret_cast<void*>(0x659408),0,0x1e25);
    std::memset(reinterpret_cast<void*>(0x6e3000),0,0x1000);
    word(0x6578c0,0x5c667c);word(0x6578c4,18);word(0x6578c8,1);
    word(0x6578c0+0x4b,0x6e3500);word(0x6578c0+0x4f,0x6e3700);word(0x6578c0+0x53,0x6e3100);
    word(0x6578c0+0x5b,0x6e3000);word(0x6578c0+0x5f,0x6e3400);word(0x6578c0+0x63,1);word(0x6578c0+0x67,4);word(0x6578c0+0x6b,4);
    word(0x6578c0+0x6f,3);word(0x6578c0+0x73,0x6e3900);word(0x6578c0+0x78,0x6e3980);word(0x6578c0+0x7c,4);*reinterpret_cast<u8*>(0x6578c0+0x77)=!special;
    std::strcpy(reinterpret_cast<char*>(0x6e3900),name);std::strcpy(reinterpret_cast<char*>(0x6e3980),"Original supplied region");
    word(0x6e3000,0x5c66ac);word(0x6e3004,0x4b3420);word(0x6e3008,0x6578c0);
    for(u32 i=0;i<4;++i){u32 at=0x6e3100+i*0x7e;word(at,0x5c6d40);word(at+8,1);word(at+0x2d,i);word(at+0x39,i==1);word(0x6e3400+i*4,at);}
    for(u32 i=0;i<2;++i){u32 at=0x6e3500+i*0x59;word(at,0x5c6c6c);word(at+8,1);word(at+0x25,0x6e3000);word(at+0x2d,i);}
    for(u32 i=0;i<3;++i){u32 at=0x6e3700+i*0x7a;word(at,0x5c6bd0);word(at+8,1);word(at+0x25,0x6e3000);word(at+0x2d,2+i);}
    word(0x657ce0,0x5c63e4);word(0x657ce4,3);word(0x659408,0x5c6a60);word(0x65940c,4);
    word(0x659408+0x12,0x659408+0xa49);word(0x659408+0x59,u32(reinterpret_cast<std::uintptr_t>(map.data())));word(0x659408+0x823,3);
    std::strcpy(reinterpret_cast<char*>(0x659408+0xa49),name);word(0x659f51,0);word(0x659f55,9);
    word(0x6f349c,4);word(0x6f34a8,0x657ce0);word(0x6f34ac,0x659408);word(0x6f34b0,0x6578c0);word(0x6f34e0,0x6578c0);word(0x689920,5);
    word(0x65b1dd,0);word(0x65b1e5,1);word(0x65af5d,0xffffffffu);
    for(u32 i=0;i<9;++i){word(0x65ab9d+i*4,i&&i<=occupants);word(0x65ae1d+i*4,i&&i<=occupants?3:2);}
    word(0x65ae1d,special?3:1);if(special)word(0x65ae21,3);
    region_campaign_enabled=region_grimoire_enabled=1;region_enter_handoff=0;
}
}
int main(int argc,char** argv){
    using namespace region_adapter;check(argc==2,"pinned original image");map_image(read(argv[1]));
    const auto* actionStart=reinterpret_cast<const u8*>(0x4b3420);const Bytes actionBytes(actionStart,actionStart+0x1a0);
    const auto* admissionStart=reinterpret_cast<const u8*>(0x54eec0);const Bytes admissionBytes(admissionStart,admissionStart+0x90);
#define PATCH(name) stub(name##_address,name##_bytes,reinterpret_cast<void*>(&name))
    PATCH(transition);PATCH(kind);PATCH(wizard);PATCH(sample);
    unsigned entries=0,refusals=0,auxiliary=0,cancels=0;u8 state[320];
    for(const char* name:{"Celtic","Greek","Medieval"})for(unsigned n:{0u,3u,4u,8u})for(bool special:{false,true}){
        fixture(name,n,special);check(region_snapshot(reinterpret_cast<void*>(0x6578c0),state,1),"settled Region admission");
        const bool allowed=special||n<4;check(bool(get(state+20)&2)==allowed,"capacity/special availability");
        check(get(state+28)==(allowed?(special?3:4):0),"separate original enter handoff");
        const auto result=region_dispatch(reinterpret_cast<void*>(0x6578c0),MNM_MENU_REGION_ENTER,0,1);
        if(!allowed){check(result==MNM_MENU_UNAVAILABLE&&!*reinterpret_cast<u8*>(0x6578cc),"capacity refusal no callback");++refusals;continue;}
        check(result==MNM_MENU_OK&&get(reinterpret_cast<void*>(0x689924))==1&&get(reinterpret_cast<void*>(0x657903))==1,"original difficulty and return");
        if(special)check(*reinterpret_cast<u8*>(0x659e50)==1&&get(reinterpret_cast<void*>(0x659e33))==4&&get(reinterpret_cast<void*>(0x6f2d0c))==3,"original battle request");
        else check(get(reinterpret_cast<void*>(0x65af5d))==3&&!*reinterpret_cast<u8*>(0x659e50),"original destination without battle");
        ++entries;
    }
    for(const char* name:{"Celtic","Greek","Medieval"})for(unsigned action:{MNM_MENU_REGION_GRIMOIRE,MNM_MENU_REGION_SPELLBOX,MNM_MENU_REGION_CHARACTER}){
        fixture(name);check(region_dispatch(reinterpret_cast<void*>(0x6578c0),action,0,1)==MNM_MENU_OK,"original auxiliary");
        const unsigned expected=action==MNM_MENU_REGION_GRIMOIRE?0x6e0088:action==MNM_MENU_REGION_SPELLBOX?0x6f2aa0:0x6c5148;
        check(get(reinterpret_cast<void*>(0x6578f3))==expected,"original destination identity");++auxiliary;
    }
    for(unsigned moved:{0u,1u}){fixture();*reinterpret_cast<u8*>(0x657937)=moved;word(0x65ae1d,moved?1:3);check(region_dispatch(reinterpret_cast<void*>(0x6578c0),MNM_MENU_REGION_CANCEL,0,1)==MNM_MENU_OK,"original Cancel");check(get(reinterpret_cast<void*>(0x659e3f))==(moved?0u:1u),"original caller-dependent Realm exit");++cancels;}
    fixture();region_campaign_enabled=0;check(!region_snapshot(reinterpret_cast<void*>(0x6578c0),state,1),"legacy nonfirst scope retained");++refusals;
    for(const auto& change:std::array<std::pair<unsigned,unsigned>,7>{{{0x65792f,4},{0x65793c,22},{0x6f349c,5},{0x689920,2},{0x659408+0x823,20},{0x659408+0xb49,9},{0x6e3004,0}}}){
        fixture();word(change.first,change.second);check(!region_snapshot(reinterpret_cast<void*>(0x6578c0),state,1),"invalid state refused");++refusals;
    }
    fixture();word(0x6e3700+0x7a+8,0);check(region_dispatch(reinterpret_cast<void*>(0x6578c0),MNM_MENU_REGION_SPELLBOX,0,1)==MNM_MENU_UNAVAILABLE,"disabled original auxiliary");++refusals;
    check(!std::memcmp(actionStart,actionBytes.data(),actionBytes.size())&&!std::memcmp(admissionStart,admissionBytes.data(),admissionBytes.size()),"original callback/admission unchanged");
    check(sizeof(region_payload)==320,"production Region lane");
    std::cout<<"{\"success\":true,\"entries\":"<<entries<<",\"refusals\":"<<refusals<<",\"auxiliary\":"<<auxiliary<<",\"cancels\":"<<cancels<<",\"live_validated\":false}\n";
}
