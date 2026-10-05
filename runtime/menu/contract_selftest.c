#include "../shadow/win32_min.h"
#include "menu_fixture_bytes.h"
API int WIN MenuInstallForTest(void);
#define THIS __attribute__((thiscall))
typedef u32 (THIS *TickFn)(void*);
u32 saved_esp,callback_result,abi_failure;
static u32 helper_count,helper_object;
u32 invoke_action(u32,void*,u32);
static void put(void* p,u32 value){u8* b=p;b[0]=value;b[1]=value>>8;b[2]=value>>16;b[3]=value>>24;}
static u32 get(const void* p){const u8* b=p;return b[0]|((u32)b[1]<<8)|((u32)b[2]<<16)|((u32)b[3]<<24);}
static void copy(void* out,const void* in,u32 n){u8* d=out;const u8* s=in;while(n--)*d++=*s++;}
static void require(int ok,u32 code){if(!ok)ExitProcess(code);}
// Reserve original callback addresses in the PE image before Wine maps files.
#pragma section(".fixture", read, write, execute)
__declspec(allocate(".fixture")) volatile u8 fixture_space[0x300000];
static u8 object[128];
static void reset(u32 id,u32 table){
    for(u32 i=0;i<128;++i)object[i]=0xa5;
    put(object,table);put(object+4,id);put(object+8,1);put(object+0x33,0);put(object+0x43,0);
    object[0xc]=0;put(object+0xd,17);helper_count=helper_object=abi_failure=0;
    put((void*)0x6f34e0,(u32)object);put((void*)0x6f349c,1);
}
static void check(u32 site,u32 action,u32 next,u32 returning){
    u32 id=site==0x4a75c0?3:22,table=id==3?0x5c63e4:0x5c641c;
    reset(id,table);SetLastError(0xabc123);
    require(invoke_action(site,object,action)==0,11);
    require(!abi_failure&&helper_count==1&&helper_object==(u32)object,12);
    require(GetLastError()==0xabc123,13);
    require(get(object+0x33)==next&&get(object+0x43)==returning,14);
    require(object[0xc]==1&&get(object+0xd)==0&&get(object+8)==1&&object[0x32]==0xa5&&object[0x47]==0xa5,15);
}
#include "channel_selftest.h"
#include "battle_selftest.h"
void start(void){
    u32 old;
    fixture_space[0]=1;
    require(VirtualProtect((void*)0x4a0000,0x260000,0x40,&old),1);
    copy((void*)0x4a75c0,main_callback,sizeof(main_callback));copy((void*)0x4a83a0,quick_callback,sizeof(quick_callback));
    battle_fixture_init();
    // The real common helper depends on engine services. Substitute only it.
    u8 helper[]={0xff,0x05,0,0,0,0,0x89,0x0d,0,0,0,0,0xc6,0x41,0x0c,1,0xc7,0x41,0x0d,0,0,0,0,0xc3};
    put(helper+2,(u32)&helper_count);put(helper+8,(u32)&helper_object);copy((void*)0x557510,helper,sizeof(helper));
    const u8 tick[]={0xb8,0xdf,0x9b,0x57,0x13,0xc3};copy((void*)0x5595d0,tick,sizeof(tick));
    put((void*)0x5c63f4,0x5595d0);put((void*)0x5c642c,0x5595d0);
    FlushInstructionCache(GetCurrentProcess(),0,0);
    // Each case executes the original callback bytes/jump tables at original VAs.
    for(u32 pass=0;pass<2;++pass){
        if(pass){
            *(u8*)0x4a75c0=0x90;require(!MenuInstallForTest(),21);*(u8*)0x4a75c0=main_callback[0];
            require(get((void*)0x5c63f4)==0x5595d0&&get((void*)0x5c642c)==0x5595d0,22);
            put((void*)0x5c642c,0x5595d1);require(!MenuInstallForTest(),23);put((void*)0x5c642c,0x5595d0);
            SetLastError(0xabc123);require(MenuInstallForTest()&&GetLastError()==0xabc123,24);
            require(get((void*)0x5c63f4)!=0x5595d0&&get((void*)0x5c63f4)==get((void*)0x5c642c),25);
        }
        *(u8*)0x6e2030=0;check(0x4a75c0,4,0,1);
        check(0x4a75c0,2,0x6e0020,0);check(0x4a75c0,0xffffffff,0,0);
        check(0x4a83a0,0,0x6de750,0);check(0x4a83a0,1,0x6a5018,0);
        check(0x4a83a0,2,0x658970,0);check(0x4a83a0,3,0,1);check(0x4a83a0,0xffffffff,0,0);
    }
    reset(3,0x5c63e4);SetLastError(0xabc123);
    TickFn observed_tick=(TickFn)get((void*)0x5c63f4);
    for(u32 i=0;i<300;++i)require(observed_tick(object)==0x13579bdf&&GetLastError()==0xabc123,31);
    // Original code/slot checks reject repeat installation with no new writes.
    require(!MenuInstallForTest(),32);
    if(battle_fixture_enabled())battle_fixture_tests(observed_tick);else channel_tests(observed_tick);
    ExitProcess(0);
}
