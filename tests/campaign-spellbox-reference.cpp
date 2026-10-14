#define main sprite_reference_unused_main
#define u32 sprite_word
#include "sprite-binary-reference.cpp"
#undef u32
#undef main
#include "campaign_spellbox_bytes.h"
#include <array>
#include <cstdlib>
using u8=unsigned char;using u32=unsigned;
#define THIS __attribute__((thiscall))
using ActionFn=u32(THIS*)(void*,u32);
static u32 get(const void* p){u32 n;std::memcpy(&n,p,4);return n;}
static void put(void* p,u32 n){std::memcpy(p,&n,4);}
static bool readable(const void* p,u32 n){auto at=reinterpret_cast<std::uintptr_t>(p);return at>=0x400000&&at+n>=at&&at+n<=0x6fb000;}
static void copy(void* to,const void* from,u32 n){std::memcpy(to,from,n);}
static bool equal(const void* a,const void* b,u32 n){return !std::memcmp(a,b,n);}
static bool battle_string(u8* out,u32 capacity,const void* source,u32 limit){if(!readable(source,limit))return false;for(u32 i=0;i<limit&&i<capacity;++i){out[i]=static_cast<const u8*>(source)[i];if(!out[i])return true;}return false;}
static void record(u32,void*,u32,u32){}
#include "../runtime/menu/spells.h"
static void require(bool ok,const char* why){if(!ok){std::cerr<<why<<'\n';std::exit(1);}}
static void THIS no3(void*,u32,u32,u32){}
static void THIS no1(void*,u32){}
static void THIS no0(void*){}
static void __attribute__((stdcall)) no7(u32,u32,u32,u32,u32,u32,u32){}
static char* THIS text(void*,u32){return reinterpret_cast<char*>(0x6e7000);}
static void word(u32 at,u32 n){put(reinterpret_cast<void*>(at),n);}
static void reset(const char* realmName,u32 parent,u32 slot){
    std::memset(reinterpret_cast<void*>(0x659408),0,0x1e25);std::memset(reinterpret_cast<void*>(0x6578c0),0,0x81);std::memset(reinterpret_cast<void*>(0x6f2aa0),0,0x26a);std::memset(reinterpret_cast<void*>(0x6e3000),0,0x9000);
    u8* p=reinterpret_cast<u8*>(0x6f2aa0),*r=reinterpret_cast<u8*>(0x659408);put(p,0x5c775c);put(p+4,7);put(p+8,1);put(p+0x90,0x65b250+slot*0x93a);put(p+0x94,UINT32_MAX);put(p+0x98,0x6f4d48);
    put(r,0x5c6a60);put(r+4,4);put(r+0x12,0x659408+0xa49);std::strcpy(reinterpret_cast<char*>(r+0xa49),realmName);put(r+0x823,19);put(r+0xb4d,80);put(r+0x59,0x400000); // Authored readable resource range; no hitmap semantics exercised.
    word(0x657ce0,0x5c63e4);word(0x657ce4,3);word(0x68991c,0);word(0x689920,5);word(0x6f34e0,0x6f2aa0);word(0x6f349c,parent==4?4:5);word(0x6f34a8,0x657ce0);word(0x6f34ac,0x659408);word(0x6f34b0,parent==4?0x6f2aa0:0x6578c0);word(0x6f34b4,0x6f2aa0);
    word(0x6578c0,0x5c667c);word(0x6578c4,18);word(0x6578c0+0x7c,4);word(0x6578c0+0x6f,3);word(0x65b250+slot*0x93a,1);word(0x65b250+slot*0x93a+8,0);
    word(0x6f5288,0);word(0x6f5284,30);spell_campaign_enabled=1;spell_valid=0;std::strcpy(reinterpret_cast<char*>(0x6e7000),"Original service stub");
    word(0x6e4000+16,u32(reinterpret_cast<std::uintptr_t>(&no0)));
    for(u32 k=0;k<2;++k){u32 cb=0x6e4100+k*16;word(cb,0x5c77b4);word(cb+4,k?0x5765d0:0x576380);word(cb+12,0x6f2aa0);}
    for(u32 a=0;a<3;++a){put(p+0x84+4*a,2);put(p+0x218+4*a,0x6e5000+a*7*0x86);
        for(u32 i=0;i<7;++i){u32 cell=0x6e5000+(a*7+i)*0x86;word(cell,0x6e4000);word(cell+0x25,0x6e4100);word(cell+0x2d,0x7fe + a*7+i);word(cell+0x49,1);word(cell+0x7a,a);word(cell+0x7e,UINT32_MAX);}}
    put(p+0x214,0x6e6000);for(u32 i=0;i<25;++i){u32 cell=0x6e6000+i*0x82;word(cell,0x6e4000);word(cell+0x25,0x6e4110);word(cell+0x2d,0x7d3+i);word(cell+0x7a,i<2?i:UINT32_MAX);}
    word(0x6e5000+(2*7+1)*0x86+0x7e,2);for(u32 i=0;i<63;++i)word(0x6f4c08+4*i,i);
}
int main(int argc,char** argv){
    require(argc==2,"pinned PE");map_image(read(argv[1]));
    auto stub=[](u32 at,const u8* expected,void* target){require(!std::memcmp(reinterpret_cast<void*>(at),expected,8),"private dependency expected bytes");*reinterpret_cast<u8*>(at)=0xe9;word(at+1,u32(reinterpret_cast<std::uintptr_t>(target)-at-5));};
    stub(0x4cb7d0,set_bytes,reinterpret_cast<void*>(&no3));stub(0x4cb7f0,show_bytes,reinterpret_cast<void*>(&no3));stub(0x579500,talisman_bytes,reinterpret_cast<void*>(&no1));stub(0x5799c0,shelf_bytes,reinterpret_cast<void*>(&no1));stub(0x56f000,audio_bytes,reinterpret_cast<void*>(&no7));stub(0x58a680,text_bytes,reinterpret_cast<void*>(&text));stub(0x557510,fade_bytes,reinterpret_cast<void*>(&no0));
    unsigned cases=0,guards=0;std::array<u32,63> slots;slots.fill(UINT32_MAX);slots[0]=0;slots[21]=1;
    for(const char* name:{"Celtic","Greek","Medieval"})for(u32 parent:{4u,18u})for(u32 owner:{0u,79u}){
        reset(name,parent,owner);require(spell_snapshot(reinterpret_cast<void*>(0x6f2aa0),spell_payload),"campaign inventory snapshot");require(get(spell_context_payload)==0&&get(spell_context_payload+4)==parent&&get(spell_payload)==owner&&get(spell_payload+4)==UINT32_MAX,"mode caller owner and no timer");
        require(spell_finish(reinterpret_cast<void*>(0x6f2aa0),slots.data())==MNM_MENU_OK,"original assignment/Done");require(get(reinterpret_cast<void*>(0x6e5000+0x7e))==0&&get(reinterpret_cast<void*>(0x6e5000+7*0x86+0x7e))==1&&get(reinterpret_cast<void*>(0x6f2aa0+0x3e))==1,"unchanged original controls and OK");
        reinterpret_cast<void(THIS*)(void*)>(0x575890)(reinterpret_cast<void*>(0x6f2aa0));require(get(reinterpret_cast<void*>(0x6f2aa0+0xb4))==0&&get(reinterpret_cast<void*>(0x6f2aa0+0xb4+21*4))==1,"original teardown snapshot only");++cases;
    }
    const std::array<std::pair<u32,u32>,10> bad{{{0x6f2aa0+0x250,2},{0x6f349c,3},{0x6f34b0,0},{0x689920,2},{0x68991c,1},{0x659408+0xa44,1},{0x659408+0xb4d,79},{0x6f2aa0+0x264,1},{0x6f2aa0+0x94,0},{0x6e4104,0}}};
    for(const auto& value:bad){reset("Greek",4,79);word(value.first,value.second);require(!spell_snapshot(reinterpret_cast<void*>(0x6f2aa0),spell_payload),"invalid campaign refused");++guards;}
    reset("Greek",18,79);spell_campaign_enabled=0;require(!spell_snapshot(reinterpret_cast<void*>(0x6f2aa0),spell_payload),"legacy mode0 refused");++guards;
    std::cout<<"{\"success\":true,\"cases\":"<<cases<<",\"guards\":"<<guards<<",\"live_validated\":false}\n";
}
