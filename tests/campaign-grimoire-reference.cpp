#define main sprite_reference_unused_main
#define u32 sprite_word
#include "sprite-binary-reference.cpp"
#undef u32
#undef main
#include "campaign_grimoire_bytes.h"
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
static void record(u32,void*,u32,u32){}
#include "../runtime/menu/grimoire.h"
static void require(bool ok,const char* why){if(!ok){std::cerr<<why<<'\n';std::exit(1);}}
static void word(u32 at,u32 n){put(reinterpret_cast<void*>(at),n);}
static u32 calls,lastChapter,lastSection;
static void THIS selector(void* p,u32 chapter,u32 section,u32 extra){require(!extra,"original selector arg");++calls;lastChapter=chapter;lastSection=section;put(static_cast<u8*>(p)+0x42,chapter);put(static_cast<u8*>(p)+0x46,section);}
static void THIS fade(void*){}
extern "C" __attribute__((naked)) u32 original_knowledge(u32,u32,u32){
    __asm__ volatile("push %ebp\n push %edi\n sub $0x20,%esp\n mov 0x2c(%esp),%edi\n mov 0x30(%esp),%ebp\n mov 0x34(%esp),%eax\n call *%eax\n mov 0xc(%esp),%eax\n add $0x20,%esp\n pop %edi\n pop %ebp\n ret\n");
}
static void reset(const char* name,u32 parent,u32 owner){
    std::memset(reinterpret_cast<void*>(0x659408),0,0x1e25);std::memset(reinterpret_cast<void*>(0x6578c0),0,0x81);std::memset(reinterpret_cast<void*>(0x6e0088),0,0x1a3e);std::memset(reinterpret_cast<void*>(0x6e3000),0,0x10000);std::memset(reinterpret_cast<void*>(0x65b250),0,80*0x93a);
    auto* p=reinterpret_cast<u8*>(0x6e0088);word(0x6e0088,0x5c6b70);put(p+0x16,0x6e5000);put(p+0x1a,0x6e3100);put(p+0xa00,0x6e3000);put(p+0x1a3a,0x6e30f0);put(p+0x4a,1);word(0x6e3000,0x5c6b8c);word(0x6e3004,0x4c77c0);word(0x6e3008,0x6e0088);
    auto* realm=reinterpret_cast<u8*>(0x659408);put(realm,0x5c6a60);put(realm+4,4);put(realm+0x12,0x659408+0xa49);std::strcpy(reinterpret_cast<char*>(realm+0xa49),name);put(realm+0x823,19);put(realm+0xb4d,80);put(realm+0x59,0x400000);
    word(0x657ce0,0x5c63e4);word(0x657ce4,3);word(0x68991c,0);word(0x689920,5);word(0x6f34e0,0x6e0088);word(0x6f349c,parent==4?4:5);word(0x6f34a8,0x657ce0);word(0x6f34ac,0x659408);word(0x6f34b0,parent==4?0x6e0088:0x6578c0);word(0x6f34b4,0x6e0088);word(0x6578c0,0x5c667c);word(0x6578c4,18);word(0x6578c0+0x7c,4);word(0x6578c0+0x6f,3);
    word(0x65b250+owner*0x93a,1);word(0x65b250+owner*0x93a+0x936,0x6e9800);word(0x6e1f20,0);grimoire_campaign_enabled=1;grimoire_payload[0]=0;calls=0;
    std::strcpy(reinterpret_cast<char*>(0x6e9000),"Static level zero");std::strcpy(reinterpret_cast<char*>(0x6e9100),"Static level two");std::strcpy(reinterpret_cast<char*>(0x6e9200),"~D unresolved");std::strcpy(reinterpret_cast<char*>(0x6e9300),"~NP second page");
    for(u32 c=0;c<8;++c)for(u32 i=0;i<4;++i){u32 e=0x6e7000+(c*4+i)*32;word(0x6e5000+c*0x1fc+12+i*4,e);word(e,0x6e9000);word(e+4,i);word(e+12,i==2?0x6e9200:i==3?0x6e9300:0x6e9000);if(i==1)word(e+20,0x6e9100);}
}
int main(int argc,char** argv){
    require(argc==2,"pinned PE");map_image(read(argv[1]));auto stub=[](u32 at,const u8* expected,void* target){require(!std::memcmp(reinterpret_cast<void*>(at),expected,8),"dependency bytes");*reinterpret_cast<u8*>(at)=0xe9;word(at+1,u32(reinterpret_cast<std::uintptr_t>(target)-at-5));};stub(0x4c72d0,selector_bytes,reinterpret_cast<void*>(&selector));stub(0x557510,fade_bytes,reinterpret_cast<void*>(&fade));
    require(!std::memcmp(reinterpret_cast<void*>(0x4c55c5),endpoint_bytes,8),"original knowledge endpoint");*reinterpret_cast<u8*>(0x4c55c5)=0xc3;unsigned knowledgeCases=0,cases=0,guards=0;
    for(u32 owner:{0u,79u})for(u32 overrideValue:{0u,1u})for(u32 level=0;level<5;++level)for(u32 chapter=0;chapter<8;++chapter){reset("Greek",4,owner);word(0x6e1f20,overrideValue);*reinterpret_cast<u8*>(0x6e9800+chapter*124+1)=u8(level);require(original_knowledge(chapter,1,0x4c557d)==(overrideValue?4:level),"unchanged original knowledge indexing/getter");++knowledgeCases;}
    for(u32 overrideValue:{0u,1u}){reset("Greek",4,79);word(0x6e1f20,overrideValue);word(0x65b250+79*0x93a+0x936,0);require(original_knowledge(7,123,0x4c55ac)==(overrideValue?4:0),"original null table and override");++knowledgeCases;}
    for(const char* name:{"Celtic","Greek","Medieval"})for(u32 parent:{4u,18u})for(u32 owner:{0u,79u}){reset(name,parent,owner);u8 state[32];require(grimoire_snapshot(reinterpret_cast<void*>(0x6e0088),state)&&get(state+16)==parent&&get(grimoire_campaign_payload)==owner&&get(grimoire_campaign_payload+8)==16,"bounded broader book projection");
        *reinterpret_cast<u8*>(0x6e9800+7*124+1)=3;require(grimoire_dispatch(reinterpret_cast<void*>(0x6e0088),MNM_MENU_GRIMOIRE_SELECT,7*124+1)==MNM_MENU_OK&&calls==1&&lastChapter==7&&lastSection==1,"unchanged packed original dispatcher");require(grimoire_snapshot(reinterpret_cast<void*>(0x6e0088),state)&&get(grimoire_campaign_payload+4)==2,"nearest stored lower text level");require(grimoire_dispatch(reinterpret_cast<void*>(0x6e0088),MNM_MENU_GRIMOIRE_SELECT,7*124+2)==MNM_MENU_UNAVAILABLE&&calls==1,"dynamic page not replaced");require(grimoire_dispatch(reinterpret_cast<void*>(0x6e0088),MNM_MENU_GRIMOIRE_CLOSE,0)==MNM_MENU_OK&&get(reinterpret_cast<void*>(0x6e0088+0x1e))==1,"original Close callback");++cases;
    }
    reset("Celtic",18,0);word(0x659408+0x59,0);word(0x659408+0x823,0);word(0x659408+0xb4d,9);word(0x6578c0+0x6f,1);word(0x6e1aba,1);word(0x6e1abe,1);u8 freshState[32];require(grimoire_snapshot(reinterpret_cast<void*>(0x6e0088),freshState),"retained unloaded fresh Celtic first caller");++cases;
    const std::array<std::pair<u32,u32>,10> bad{{{0x6f349c,3},{0x6f34b0,0},{0x689920,2},{0x68991c,1},{0x659408+0xa44,1},{0x659408+0xb4d,79},{0x6e0088+0x4a,3},{0x6e0088+0x42,8},{0x6e3004,0},{0x6e7024,124}}};
    for(const auto& value:bad){reset("Greek",4,79);word(value.first,value.second);u8 state[32];require(!grimoire_snapshot(reinterpret_cast<void*>(0x6e0088),state),"bad broader caller/catalog refused");++guards;}
    reset("Greek",4,79);grimoire_campaign_enabled=0;u8 state[32];require(!grimoire_snapshot(reinterpret_cast<void*>(0x6e0088),state),"legacy admission retained");++guards;
    std::cout<<"{\"success\":true,\"knowledge_cases\":"<<knowledgeCases<<",\"cases\":"<<cases<<",\"guards\":"<<guards<<",\"live_validated\":false}\n";
}
