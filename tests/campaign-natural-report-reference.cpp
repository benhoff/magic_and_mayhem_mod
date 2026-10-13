#define main sprite_reference_unused_main
#define u32 sprite_word
#include "sprite-binary-reference.cpp"
#undef u32
#undef main
#include "campaign_natural_report_bytes.h"
#include <array>
#include <cstdlib>
using u8=unsigned char;using u32=unsigned;
#define THIS __attribute__((thiscall))
using ActionFn=u32(THIS*)(void*,u32);
static u32 get(const void* p){u32 n;std::memcpy(&n,p,4);return n;}
static void put(void* p,u32 n){std::memcpy(p,&n,4);}
static bool readable(const void* p,u32 n){const auto at=reinterpret_cast<std::uintptr_t>(p);return at>=0x400000&&at+n>=at&&at+n<=0x6fb000;}
static bool battle_string(u8* out,u32 capacity,const void* source,u32 limit){
    if(!readable(source,limit))return false;
    for(u32 i=0;i<limit&&i<capacity;++i){out[i]=static_cast<const u8*>(source)[i];if(!out[i])return true;}return false;
}
static u32 callbacks;
static void record(u32 event,void*,u32,u32){if(event==3)++callbacks;}
#include "../runtime/menu/campaign_defeat.h"
static void require(bool ok,const char* why){if(!ok){std::cerr<<why<<'\n';std::exit(1);}}
static void THIS cue(unsigned){}
static void THIS focus(void*){}
static void word(u32 at,u32 value){put(reinterpret_cast<void*>(at),value);}
static void reset(u32 outcome){
    std::memset(reinterpret_cast<void*>(0x6db967),0,0x2b1);std::memset(reinterpret_cast<void*>(0x6e3000),0,0x5000);
    auto* p=reinterpret_cast<u8*>(0x6db967);word(0x6db967,0x5c5ebc);word(0x6db96b,6);word(0x6db96f,1);p[0x11]=1;put(p+0x53,outcome);
    put(p+0x47,0x6e3800);put(p+0x4b,0x6e3900);put(p+0x4f,0x6e3820);
    std::strcpy(reinterpret_cast<char*>(p+0x57),outcome?"Original victory":"Original natural defeat");
    word(0x6e3800,0x6e3840);word(0x6e3840+0x25,0x6e3820);word(0x6e3840+0x2d,21);
    word(0x6e3820,0x5c5eec);word(0x6e3824,0x4747a0);word(0x6e3828,0x6db967);
    for(u32 i=0;i<21;++i){const u32 label=0x6e4000+i*16,text=0x6e5000+i*256;
        word(0x6e3900+i*4,label);word(label+8,text);std::strcpy(reinterpret_cast<char*>(text),i==19?"":"Original engine text");}
    word(0x68991c,0);word(0x689920,5);word(0x6f349c,5);word(0x6f34e0,0x6db967);
    word(0x6f34a8,0x657ce0);word(0x6f34ac,0x659408);word(0x6f34b0,0x6cbb78);word(0x6f34b4,0x6db967);
    word(0x657ce0,0x5c63e4);word(0x657ce4,3);word(0x659408,0x5c6a60);word(0x65940c,4);word(0x6cbb78,0x5c5dd8);word(0x6cbb7c,2);
    campaign_reports_enabled=1;defeat_quit_pending=0;*reinterpret_cast<u8*>(0x6dbc19)=0;callbacks=0;
}
int main(int argc,char** argv){
    require(argc==2,"pinned PE");map_image(read(argv[1]));
    auto stub=[](u32 at,const u8* expected,void* target){require(!std::memcmp(reinterpret_cast<void*>(at),expected,8),"expected private dependency bytes");*reinterpret_cast<u8*>(at)=0xe9;word(at+1,u32(reinterpret_cast<std::uintptr_t>(target)-at-5));};
    stub(0x4a39a0,cue_bytes,reinterpret_cast<void*>(&cue));stub(0x474b50,focus_bytes,reinterpret_cast<void*>(&focus));
    const auto* original=reinterpret_cast<const u8*>(0x4747a0);const Bytes originalBytes(original,original+0x15);
    u8 data[MNM_MENU_V11_DEFEAT_SIZE];unsigned outcomes=0,guards=0;
    for(u32 outcome:{0u,1u}){
        reset(outcome);auto* p=reinterpret_cast<u8*>(0x6db967);require(defeat_snapshot(p,data)&&get(data+12)==outcome&&data[16],"natural original outcome admitted");
        require(defeat_dispatch(p,MNM_MENU_DEFEAT_OK)==MNM_MENU_OK&&callbacks==1&&get(p+0x43)==1&&p[0xc]==1,"unchanged original OK/fade");++outcomes;
        reset(outcome);campaign_reports_enabled=0;require(!defeat_snapshot(p,data),"older provenance/outcome boundary retained");++guards;
    }
    const std::array<std::pair<u32,u32>,10> bad{{{0x6db967,0},{0x6db96b,3},{0x6db967+0x53,2},{0x689920,2},{0x68991c,1},{0x6f349c,4},{0x6f34b0,0},{0x6f34b4,0},{0x6e3824,0},{0x6e3840+0x2d,0}}};
    for(const auto& value:bad){reset(1);word(value.first,value.second);require(!defeat_snapshot(reinterpret_cast<void*>(0x6db967),data),"invalid report owner/control refused");++guards;}
    reset(1);std::memset(reinterpret_cast<void*>(0x6db967+0x57),'x',256);require(!defeat_snapshot(reinterpret_cast<void*>(0x6db967),data),"unterminated title refused");++guards;
    require(!std::memcmp(original,originalBytes.data(),originalBytes.size()),"original OK body unchanged");require(sizeof(defeat_payload)==5648,"report lane bounds");
    std::cout<<"{\"success\":true,\"outcomes\":"<<outcomes<<",\"guards\":"<<guards<<",\"live_validated\":false}\n";
}
