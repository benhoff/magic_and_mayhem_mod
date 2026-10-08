#include "../runtime/shadow/win32_min.h"
extern void canvas_test_call(u32,u32,u32,u32,void*);
extern void world_primitive_11(void),world_primitive_12(void);
extern int world_additive_test_setup(void),world_additive_test_check(void),world_colour_test_setup(void),world_colour_test_check(void);
extern u32 world_trampolines[13];
u8* world_stream_begin(u32* sample){(void)sample;return 0;}
void world_stream_publish(u32 a,u32 b,u32 c,u32 d,u32 e,u32 f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;}
void world_stream_fail(u32 reason){(void)reason;}
static volatile u8 reserve[0x310000];
static u8 before[560] __attribute__((aligned(16))),after[560] __attribute__((aligned(16)));
__attribute__((naked)) static void resume(void){__asm__ volatile("ret $8");}
void start(void){
    reserve[sizeof(reserve)-1]=1;
    world_trampolines[11]=(u32)resume;
    SetLastError(1234);canvas_test_call((u32)resume,0x600030,0xabcdef00,8,before);
    if(!world_additive_test_setup())ExitProcess(2);
    SetLastError(1234);canvas_test_call((u32)world_primitive_11,0x600030,0xabcdef00,8,after);
    if(GetLastError()!=1234||!world_additive_test_check())ExitProcess(3);
    for(u32 i=0;i<560;++i)if(before[i]!=after[i])ExitProcess(4);
    world_trampolines[12]=(u32)resume;
    SetLastError(1234);canvas_test_call((u32)resume,0x60001c,0xabcdef00,8,before);
    if(!world_colour_test_setup())ExitProcess(5);
    SetLastError(1234);canvas_test_call((u32)world_primitive_12,0x60001c,0xabcdef00,8,after);
    if(GetLastError()!=1234||!world_colour_test_check())ExitProcess(6);
    for(u32 i=0;i<560;++i)if(before[i]!=after[i])ExitProcess(7);
    ExitProcess(0);
}
