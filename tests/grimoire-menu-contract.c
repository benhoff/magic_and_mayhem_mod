/* Private original callback execution; page/render/resource helpers are stubbed. */
#include "../runtime/shadow/win32_min.h"
#include "grimoire_fixture_bytes.h"
#define THIS __attribute__((thiscall))
API HANDLE WIN GetStdHandle(u32);
#pragma section(".fixture", read, write, execute)
__declspec(allocate(".fixture")) volatile u8 fixture_space[0x300000];
u32 saved_esp, callback_result, abi_failure;
u32 invoke_action(u32, void*, u32);
static u32 fades, previous, next, selections, chapter, section, forced;
static u32 get(const void* p) {
    const u8* b=p; return b[0]|((u32)b[1]<<8)|((u32)b[2]<<16)|((u32)b[3]<<24);
}
static void put(void* p,u32 v) { u8* b=p; b[0]=v; b[1]=v>>8; b[2]=v>>16; b[3]=v>>24; }
static void copy(void* d,const void* s,u32 n) { u8* a=d; const u8* b=s; while(n--)*a++=*b++; }
void* memcpy(void* d,const void* s,unsigned n) { copy(d,s,n); return d; }
static void zero(void* p,u32 n) { u8* b=p; while(n--)*b++=0; }
static void require(int ok,u32 code) { if(!ok)ExitProcess(code); }
static void owner(void* p) { require(p==(void*)0x6e0088,10); }
static void THIS fade(void* p) { owner(p); ++fades; }
static void THIS back(void* p) { owner(p); ++previous; }
static void THIS forward(void* p) { owner(p); ++next; }
static void THIS select(void* p,u32 c,u32 s,u32 force) {
    owner(p); ++selections; chapter=c; section=s; forced=force;
}
static void stub(u32 address,void* target) { ((u8*)address)[0]=0xe9; put((void*)(address+1),(u32)target-address-5); }
void start(void) {
    fixture_space[0]=1; u32 old;
    require(VirtualProtect((void*)0x470000,0x290000,0x40,&old),1);
    copy((void*)0x4c77c0,grimoire_callback_bytes,sizeof(grimoire_callback_bytes));
    stub(0x557510,(void*)&fade); stub(0x4c6db0,(void*)&back);
    stub(0x4c6910,(void*)&forward); stub(0x4c72d0,(void*)&select);
    const u32 actions[]={3999,1999,2999,0,1,124,991,0xffffffffu};
    u8* p=(u8*)0x6e0088; u32 cases=0;
    for(u32 animation=0;animation<4;++animation)for(u32 i=0;i<8;++i) {
        zero(p,0x1a40); fades=previous=next=selections=0;
        put(p+0x26,11); put(p+0x2a,22); put(p+0x2e,33); put(p+0x32,44);
        put(p+0x3a,animation&1); put(p+0x3e,animation>>1);
        put(p+0x1e,7); put(p+0x1a2e,99);
        const u32 action=actions[i];
        require(invoke_action(0x4c77c0,p,action)==action&&!abi_failure,20);
        require(get(p+0x1a2e)==0,21);
        require(get(p+0x26)==(animation?33:11)&&get(p+0x2a)==(animation?44:22)&&
                get(p+0x2e)==(animation?11:33)&&get(p+0x32)==(animation?22:44)&&
                !get(p+0x3a)&&!get(p+0x3e),22);
        require(fades==(action==3999)&&previous==(action==1999)&&next==(action==2999),23);
        require(get(p+0x1e)==(action==3999?1:7),24);
        require(selections==(i>=3),25);
        if(i>=3)require(chapter==action/124&&section==action%124&&!forced,26);
        ++cases;
    }
    require(cases==32,27);
    const char result[]="{\"success\":true,\"cases\":32,\"real_game_launched\":false,\"page_and_render_helpers_stubbed\":true,\"calling_convention_verified\":true}\n";
    u32 wrote; WriteFile(GetStdHandle(0xfffffff5),result,sizeof(result)-1,&wrote,0); ExitProcess(0);
}
