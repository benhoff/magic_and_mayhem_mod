#include "../runtime/shadow/win32_min.h"
static u32 sleeps,clock_calls;
static void test_sleep(u32 ms){if(ms!=0||GetLastError()!=0x77)ExitProcess(11);++sleeps;SetLastError(0x99);}
static u32 WIN test_tick(void){if(GetLastError()!=0x77)ExitProcess(12);++clock_calls;SetLastError(0x88);return 0xabcdef01;}
static int same(const void* a,const void* b,u32 n){const u8* x=a;const u8* y=b;while(n--)if(*x++!=*y++)return 0;return 1;}
#define MNM_PACER_POLICY_TEST
#define PACER_FIRST_YIELD(caller) ((void)(caller))
#define Sleep test_sleep
#include "../runtime/render/pacer_yield.h"
void start(void){
    u8 load[6]={0x8b,0x35,0x64,0x51,0x5c,0};
    u8 loop[12]={0xff,0xd6,0x2b,0x05,0x00,0x81,0x6e,0x00,0x3b,0xc7,0x72,0xf4};
    if(!pacer_entry_matches(load,loop,0x1234,0x1234,0x400000))ExitProcess(1);
    for(u32 i=0;i<6;++i){load[i]^=1;if(pacer_entry_matches(load,loop,0x1234,0x1234,0x400000))ExitProcess(2);load[i]^=1;}
    for(u32 i=0;i<12;++i){loop[i]^=1;if(pacer_entry_matches(load,loop,0x1234,0x1234,0x400000))ExitProcess(3);loop[i]^=1;}
    if(pacer_entry_matches(load,loop,0x1235,0x1234,0x400000)||pacer_entry_matches(load,loop,0,0,0x400000)||pacer_entry_matches(load,loop,0x1234,0x1234,0x410000))ExitProcess(4);
    pacer_original_tick=test_tick;
    for(u32 mode=0;mode<4;++mode){pacer_enabled=mode!=0;SetLastError(0x77);
        u32 result=pacer_read(mode==2?0x4e3f7f:0x4e3f8b,mode!=1);
        if(result!=0xabcdef01||GetLastError()!=0x88)ExitProcess(5);
    }
    if(sleeps!=1||clock_calls!=4||!pacer_yield_seen)ExitProcess(6);ExitProcess(0);
}
