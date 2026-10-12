#define main sprite_reference_unused_main
#define u32 sprite_word
#include "sprite-binary-reference.cpp"
#undef u32
#undef main
#include <array>
#include <cstdlib>
using u8=unsigned char;using u32=unsigned;
#define THIS __attribute__((thiscall))
using ActionFn=u32 (THIS *)(void*,u32);
static u32 get(const void* p){u32 n;std::memcpy(&n,p,4);return n;}
static void put(void* p,u32 n){std::memcpy(p,&n,4);}
static int equal(const void* a,const void* b,u32 n){return !std::memcmp(a,b,n);}
static std::array<u8,800*600> hitMap{};
static int readable(const void* p,u32 n){auto at=reinterpret_cast<std::uintptr_t>(p);
    const auto h=reinterpret_cast<std::uintptr_t>(hitMap.data());return (at>=0x400000&&at+n>=at&&at+n<=0x6fb000)||(at>=h&&at+n>=at&&at+n<=h+hitMap.size());}
static int battle_string(u8* out,u32 capacity,const void* source,u32 limit){
    auto* p=static_cast<const u8*>(source);for(u32 i=0;i<limit&&i<capacity;++i){if(!readable(p+i,1))return 0;out[i]=p[i];if(!p[i])return 1;}return 0;
}
static u32 callbackCount;
static void record(u32 event,void*,u32,u32){if(event==3)++callbackCount;}
#include "../runtime/menu/character.h"
#include "character_adapter_fixture.h"
static void require(bool ok,const char* text){if(!ok){std::cerr<<text<<'\n';std::exit(1);}}
static void THIS fade(void* object){static_cast<u8*>(object)[0xc]=1;}
static void reset(){
    std::memset(reinterpret_cast<void*>(0x6c5148),0,0x120);std::memset(reinterpret_cast<void*>(0x659408),0,0x1e25);
    std::memset(reinterpret_cast<void*>(0x65b250),0,80*0x93a);std::memset(reinterpret_cast<void*>(0x6e8000),0,0x1100);
    const auto p=reinterpret_cast<u8*>(0x6c5148);global(0x6c5148,0x5c66b4);global(0x6c514c,20);global(0x6c5150,1);
    global(0x68991c,0);global(0x689920,5);global(0x6f349c,4);global(0x6f34a8,0x657ce0);global(0x6f34ac,0x659408);global(0x6f34b0,0x6c5148);global(0x6f34e0,0x6c5148);
    global(0x657ce0,0x5c63e4);global(0x657ce4,3);
    global(0x659408,0x5c6a60);global(0x65940c,4);global(0x65941a,0x659e51);std::strcpy(reinterpret_cast<char*>(0x659e51),"Celtic");
    global(0x659f55,9);global(0x659c2b,8);global(0x659461,reinterpret_cast<std::uintptr_t>(hitMap.data()));
    std::strcpy(reinterpret_cast<char*>(0x65a69d),"Cornelius");
    global(0x65b250,1);global(0x65b2a8,50);global(0x6fa208,1);
    global(0x6c51b7,0x6e8d00);global(0x6e8d00,0x5c66e4);global(0x6e8d04,0x4b5470);global(0x6e8d08,0x6c5148);
    global(0x6c51bb,0x6e8400);global(0x6c51cf,0x6e8500);global(0x6c51c7,0x6e8000);global(0x6c51cb,0x6e8100);global(0x6c51ef,50);
    for(u32 i=0;i<2;++i){auto* b=reinterpret_cast<u8*>(0x6e8400+i*0x59);put(b,0x5c6c6c);put(b+8,1);put(b+0x25,0x6e8d00);put(b+0x2d,i);}
    for(u32 i=0;i<12;++i){auto* b=reinterpret_cast<u8*>(0x6e8500+i*0x7a);put(b,0x5c6bd0);put(b+8,1);put(b+0x25,0x6e8d00);put(b+0x2d,25+i);}
    const u32 values[]={40,160,4,2,2,2},limits[]={200,800,40,7,7,7},steps[]={5,20,1,1,1,1};
    for(u32 i=0;i<6;++i){auto* cell=reinterpret_cast<u8*>((i<3?0x6e8000:0x6e8100)+(i%3)*0x2c);
        put(cell+(i<3?0x24:0x18),values[i]);put(cell+(i<3?0x28:0x1c),values[i]);put(cell+(i<3?0x20:0x14),limits[i]);
        global(0x65b250+(i<3?0x3c+i*4:0xf0+(i-3)*4),values[i]);put(p+(i<3?0xab+i*0x10:0xdb),steps[i]);
    }
    // Exact simple price: initial0, linear1, exponent1 => current index/next index.
    const float one=1;for(u32 i=0;i<4;++i){std::memcpy(p+0xb3+i*0x10,&one,4);std::memcpy(p+0xb7+i*0x10,&one,4);}
    unsigned short cw=0x27f;asm volatile("fninit; fldcw %0" : : "m"(cw));
    reinterpret_cast<void (THIS *)(void*)>(0x4b5f90)(p);callbackCount=0;
}
int main(int argc,char** argv)try{
    require(argc==2,"PE required");map_image(read(argv[1]));
    require(equal(reinterpret_cast<void*>(0x557510),fade_bytes,sizeof(fade_bytes)),"fade expected bytes");
    *reinterpret_cast<u8*>(0x557510)=0xe9;global(0x557511,unsigned(reinterpret_cast<std::uintptr_t>(&fade)-0x557515));
    auto* p=reinterpret_cast<u8*>(0x6c5148);reset();require(character_snapshot(p,character_payload),"valid settled snapshot");
    require(get(character_payload+24)==2730,"opening adjustment mask");
    require(character_dispatch(p,MNM_MENU_CHARACTER_ADJUST,0)==MNM_MENU_UNAVAILABLE&&!callbackCount,"baseline decrease refusal");
    require(character_dispatch(p,MNM_MENU_CHARACTER_ADJUST,12)==MNM_MENU_INVALID&&!callbackCount,"index bounds");
    require(character_dispatch(p,MNM_MENU_CHARACTER_OK,1)==MNM_MENU_INVALID&&!callbackCount,"grant argument refusal");
    require(character_dispatch(p,MNM_MENU_CHARACTER_ADJUST,1)==MNM_MENU_OK&&callbackCount==1,"real original increase");
    require(character_snapshot(p,character_payload)&&get(character_payload+36)==45&&get(character_payload+16)==41,"original edited value/budget");
    require(character_dispatch(p,MNM_MENU_CHARACTER_ADJUST,0)==MNM_MENU_OK&&callbackCount==2,"real original refund");
    require(character_snapshot(p,character_payload)&&get(character_payload+36)==40&&get(character_payload+16)==50,"original refund restored");
    require(character_dispatch(p,MNM_MENU_CHARACTER_CANCEL,0)==MNM_MENU_OK&&!character_snapshot(p,character_payload),"original Cancel return denies further actions");
    // Exercise the full original getter range, with original price/OK callbacks.
    reset();global(0x65b250,0);global(0x659f55,80);
    const u32 late=0x65b250+79*0x93a;global(late,1);global(late+0x58,50);
    const u32 values[]={40,160,4,2,2,2};
    for(u32 i=0;i<6;++i)global(late+(i<3?0x3c+i*4:0xf0+(i-3)*4),values[i]);
    std::strcpy(reinterpret_cast<char*>(0x65a69d+79*16),"Last wizard");
    require(character_snapshot(p,character_payload)&&get(character_payload+12)==79,"last wizard snapshot");
    require(character_dispatch(p,MNM_MENU_CHARACTER_ADJUST,1)==MNM_MENU_OK,"last wizard original increase");
    require(character_dispatch(p,MNM_MENU_CHARACTER_OK,0)==MNM_MENU_OK&&get(reinterpret_cast<void*>(late+0x3c))==45&&get(reinterpret_cast<void*>(late+0x58))==41,"last wizard original commit");
    unsigned guards=0;
    const std::array<std::array<u32,2>,15> bad{{{0x6f34e0,0},{0x6f349c,3},{0x689920,2},{0x6c5148,0},{0x6c5150,0},
        {0x6c517b,1},{0x6c518b,1},{0x6c5263,1},{0x6e8d04,0},{0x6e8500+0x2d,99},{0x6c51ef,51},
        {0x6e8000+0x28,41},{0x6e8000+0x20,201},{0x659e33,1},{0x659e4c,1}}};
    for(const auto& b:bad){reset();global(b[0],b[1]);require(!character_snapshot(p,character_payload),"invalid identity/state admitted");
        require(character_dispatch(p,MNM_MENU_CHARACTER_OK,0)==MNM_MENU_UNAVAILABLE&&!callbackCount,"invalid state dispatched");++guards;}
    reset();global(0x6f349c,5);global(0x6f34b0,0x6578c0);global(0x6f34b4,0x6c5148);global(0x6578c0,0x5c667c);global(0x6578c4,18);global(0x65793c,4);global(0x65792f,2);
    require(character_snapshot(p,character_payload)&&get(character_payload)==18,"Region caller snapshot");global(0x65792f,1);require(!character_snapshot(p,character_payload),"fresh first-region refusal");++guards;
    std::cout<<"{\"success\":true,\"guard_cases\":"<<guards<<",\"original_adjust_refund_cancel\":true,\"live_validated\":false}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
