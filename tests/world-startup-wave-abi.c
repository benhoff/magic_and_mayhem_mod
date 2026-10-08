#include "../runtime/shadow/win32_min.h"
extern void canvas_test_call(u32,u32,u32,u32,void*);
extern void world_wave_leave(void),world_wave_test_setup(void);
extern int world_wave_test_check(void),world_palette_test(void);
extern u32 world_wave_return;
u8* world_stream_begin(u32* sample){(void)sample;return 0;}
void world_stream_publish(u32 a,u32 b,u32 c,u32 d,u32 e,u32 f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;}
void world_stream_fail(u32 reason){(void)reason;}
static volatile u8 reserve[0x310000];
static u8 before[560] __attribute__((aligned(16))),after[560] __attribute__((aligned(16)));
__attribute__((naked)) static void resume(void){__asm__ volatile("ret");}
void start(void){
    reserve[sizeof(reserve)-1]=1;
    for(u32 i=0;i<256;++i)((u32*)0x5f14d0)[i]=i*13;
    world_wave_return=(u32)resume;
    SetLastError(1234);canvas_test_call((u32)resume,0x12340000,0xabcdef00,0,before);
    world_wave_test_setup();
    SetLastError(1234);canvas_test_call((u32)world_wave_leave,0x12340000,0xabcdef00,0,after);
    if(GetLastError()!=1234||!world_wave_test_check())ExitProcess(2);
    for(u32 i=0;i<560;++i)if(before[i]!=after[i])ExitProcess(3);
    if(!world_palette_test())ExitProcess(4);
    ExitProcess(0);
}
