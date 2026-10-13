// Production Realm admission plus a normally scheduled unchanged original tick.
#define main realm_selection_reference_unused_main
#include "realm-selection-contract-reference.cpp"
#undef main
#define WIN __attribute__((stdcall))
namespace adapter {
using u8=unsigned char;using u32=unsigned;
static u32 get(const void* p){u32 n;std::memcpy(&n,p,4);return n;}
static void put(void* p,u32 n){std::memcpy(p,&n,4);}
static bool equal(const void* a,const void* b,u32 n){return !std::memcmp(a,b,n);}
static bool readable(const void* p,u32 n){
    const auto at=reinterpret_cast<std::uintptr_t>(p),map=reinterpret_cast<std::uintptr_t>(hitMap.data());
    return (at>=0x400000&&at<=0x700000&&n<=0x700000-at)||(at>=map&&at<=map+hitMap.size()&&n<=map+hitMap.size()-at);
}
static void record(u32,void*,u32,u32){}
#include "../runtime/menu/realm.h"
static void fixture(bool high=true){
    RegionSelectionInput in{1,true,false,false,0,false,false,false,false};reset(0,1,in,0,high);
    global(0x657ce0,0x5c63e4);global(0x657ce4,3);global(realm,0x5c6a60);global(realm+4,4);
    global(0x6f349c,3);global(0x6f34a8,0x657ce0);global(0x6f34ac,realm);global(0x6f34e0,realm);
    global(0x68991c,0);global(0x689920,5);global(realm+0x823,3);
    global(0x65ae1d,1);global(0x65af5d,0xffffffffu);
    global(0x65b1dd,0);global(0x65b1e1,0);global(0x65b1e5,1);
    global(realm+0x1df+3*4,1);hitMap.fill(0);
    for(unsigned i=1;i<=3;++i)hitMap[10*800+i*10]=static_cast<u8>(i);
    global(realm+0xa23,0x6e8100);global(0x6e8100,0x5c75a0);global(0x6e8104,0x54fd20);global(0x6e8108,0);global(0x6e810c,realm);
    realm_map_object=realm_activation=0;
}
static void stub(unsigned at,const unsigned char* bytes,void* target){
    check(!std::memcmp(reinterpret_cast<void*>(at),bytes,8),"dependency expected bytes");
    *reinterpret_cast<unsigned char*>(at)=0xe9;global(at+1,unsigned(reinterpret_cast<std::uintptr_t>(target)-at-5));
}
static void __attribute__((thiscall)) any_cue(unsigned id){check(id==1||id==22,"original cue");}
}
int main(int argc,char** argv){
    using namespace adapter;check(argc==2,"Pinned PE required");map_image(read(argv[1]));
    const auto* tickStart=reinterpret_cast<const u8*>(0x5517a0);
    const Bytes tickBytes(tickStart,tickStart+0xaa0);
    const auto* callbackStart=reinterpret_cast<const u8*>(0x54fd20);const Bytes callbackBytes(callbackStart,callbackStart+0x50);
#define PATCH(name,target) stub(name##_address,name##_bytes,reinterpret_cast<void*>(&target))
    PATCH(input_poll,no_stack);PATCH(hover_clear,no_stack);PATCH(hover_text,one_stack);PATCH(cue,any_cue);
    PATCH(sample,sample);PATCH(kind,kind);PATCH(movement,no_stack);PATCH(draw,one_stack);PATCH(draw_prepare,no_stack);
    PATCH(render,render);PATCH(draw_finish,no_stack);PATCH(push,push);PATCH(wizard,wizard);PATCH(tutorial,tutorial);PATCH(free_tutorial,free_tutorial);
    unsigned admissions=0,guards=0,actions=0;u8 state[512];
    for(bool high:{false,true})for(unsigned selected:{2u,3u}){
        fixture(high);check(realm_snapshot(reinterpret_cast<void*>(realm),state),"settled map admission");++admissions;
        check(get(state+16)==3&&get(state+32)==14&&get(state+28)==0xffffffffu,"engine region masks and no-destination sentinel");
        const auto x=word(0x641ab8),y=word(0x641abc),button=word(0x641ac4);
        check(realm_dispatch(reinterpret_cast<void*>(realm),MNM_MENU_REALM_ACTIVATE,selected)==MNM_MENU_OK,"activate admitted");
        check(realm_input_begin(reinterpret_cast<void*>(realm))==1,"release ingress");
        reinterpret_cast<void(__attribute__((thiscall))*)(void*)>(0x5517a0)(reinterpret_cast<void*>(realm));realm_input_end();
        check(word(0x641ab8)==x&&word(0x641abc)==y&&word(0x641ac4)==button,"physical input restored");
        if(selected==2)check(word(0x65af5d)==2&&word(realm+0xa2b)==0,"original owned destination branch");
        else check(word(realm+0xa2b)==1&&word(realm+0xa33)==3&&word(realm+0xa2f)==1,"original adjacent Region Entry branch");
        check(!*reinterpret_cast<u8*>(0x6f2d3d),"original consumes latch");++actions;
    }
    const std::array<std::pair<unsigned,unsigned>,18> mutations{{
        {realm,0},{realm+4,3},{realm+0xa2b,1},{realm+0xa37,1},{realm+0xa44,1},
        {realm+0x12,0},{realm+0x823,20},{realm+0xb49,9},{realm+0xb4d,81},
        {0x6f349c,4},{0x6f34ac,0},{0x6f34e0,0},{0x657ce0,0},
        {0x689920,2},{0x6e0080,1},{0x6c54dc,1},{0x6f2d28,1},{0x65af5d,20}}};
    for(const auto& m:mutations){fixture();global(m.first,m.second);check(!realm_snapshot(reinterpret_cast<void*>(realm),state),"mutated admission refused");++guards;}
    fixture();hitMap[0]=4;check(!realm_snapshot(reinterpret_cast<void*>(realm),state),"out-of-range hit-map identity");++guards;
    fixture();global(realm+0x1df+3*4,0);check(realm_dispatch(reinterpret_cast<void*>(realm),MNM_MENU_REALM_ACTIVATE,3)==MNM_MENU_UNAVAILABLE,"unowned nonadjacent denied");++guards;
    fixture();check(realm_dispatch(reinterpret_cast<void*>(realm),MNM_MENU_REALM_ACTIVATE,0)==MNM_MENU_INVALID,"region zero denied");++guards;
    fixture();global(0x641ac4,1);check(realm_dispatch(reinterpret_cast<void*>(realm),MNM_MENU_REALM_ACTIVATE,2)==MNM_MENU_UNAVAILABLE,"physical press not overwritten");++guards;
    fixture();check(realm_dispatch(reinterpret_cast<void*>(realm),MNM_MENU_REALM_ACTIVATE,2)==MNM_MENU_OK,"pending activation");global(realm+0xa44,1);
    check(realm_input_begin(reinterpret_cast<void*>(realm))==-1&&!realm_activation,"changed admission consumes request without mutation");++guards;
    for(unsigned action:{MNM_MENU_REALM_CHARACTER,MNM_MENU_REALM_GRIMOIRE,MNM_MENU_REALM_SPELLBOX}){
        fixture();check(realm_dispatch(reinterpret_cast<void*>(realm),action,0)==MNM_MENU_OK,"original auxiliary dispatch");
        const unsigned flag=action==MNM_MENU_REALM_CHARACTER?0x6f2d34:action==MNM_MENU_REALM_GRIMOIRE?0x6f2d2c:0x6f2d28;
        check(word(flag)==1,"original callback flag");++actions;
    }
    check(!std::memcmp(tickStart,tickBytes.data(),tickBytes.size()),"original tick body unchanged");
    check(!std::memcmp(callbackStart,callbackBytes.data(),callbackBytes.size()),"original auxiliary callback unchanged");
    check(sizeof(realm_payload)==512,"production lane size");
    std::cout<<"{\"success\":true,\"admissions\":"<<admissions<<",\"actions\":"<<actions<<",\"guards\":"<<guards<<",\"original_tick_unchanged\":true,\"live_validated\":false}\n";
}
