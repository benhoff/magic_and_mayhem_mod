#include "../runtime/shadow/win32_min.h"
extern void producer_enter_26(void),producer_enter_52(void),resident_leaf_control(void);
extern void world_bypass_test_call(u32,u32,u32,u32,void*,u32);
u32 producer_trampolines[53];
static u32 calls;
static __attribute__((aligned(16))) u32 actual[140],expected[140];
int producer_enter_observe(u32* registers,u32 tag,u8* fx){
 if((tag!=26 && tag!=52) || !fx || ((u32)fx&15) || registers[7]!=0x12345678)ExitProcess(91);
 ++calls;registers[7]=0;__asm__ __volatile__("fninit; fldz");return 1;
}
void producer_leave_observe(u32* registers){(void)registers;ExitProcess(92);}
void start(void){
 world_bypass_test_call((u32)producer_enter_26,0x11223344,0x55667788,8,actual,4);
 world_bypass_test_call((u32)resident_leaf_control,0x11223344,0x55667788,8,expected,4);
 for(u32 i=0;i<8;++i)if(actual[i]!=expected[i])ExitProcess(93+i);
 for(u32 i=12;i<140;++i)if(actual[i]!=expected[i])ExitProcess(104);
 world_bypass_test_call((u32)producer_enter_52,0x11223344,0x55667788,8,actual,4);
 for(u32 i=0;i<8;++i)if(actual[i]!=expected[i])ExitProcess(93+i);
 for(u32 i=12;i<140;++i)if(actual[i]!=expected[i])ExitProcess(104);
 if(calls!=2)ExitProcess(105);
 HANDLE file=CreateFileA("leaf-abi.bin",0x40000000,0,0,1,0x80,0);u32 done=0;
 if(file==(HANDLE)-1 || !WriteFile(file,actual,sizeof(actual),&done,0) || done!=sizeof(actual) || !CloseHandle(file))ExitProcess(106);
 ExitProcess(0);
}
