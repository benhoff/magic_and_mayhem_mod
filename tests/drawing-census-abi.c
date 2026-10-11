/* Actual observation callbacks and assembly against synthetic PE32 callees.
 * No original instructions or rendering execute in this fixture. */
#include "../runtime/drawing-census/observer.c"
extern void canvas_test_call(u32,u32,u32,u32,void*);
static volatile u8 reserve[0x310000];
static u8 before[560] __attribute__((aligned(16))),after[560] __attribute__((aligned(16)));
__attribute__((naked)) static void pop0(void){__asm__ volatile("ret");}
__attribute__((naked)) static void pop8(void){__asm__ volatile("ret $8");}
__attribute__((naked)) static void pop24(void){__asm__ volatile("ret $24");}
__attribute__((naked)) static void tail8(void){__asm__ volatile("jmp _census_enter_1");}
static void identical(void){for(u32 i=0;i<560;++i)if(before[i]!=after[i])ExitProcess(20);}
void start(void){
 reserve[sizeof(reserve)-1]=1;
 static u32 host[16]={0};host[4]=2;host[5]=2;host[6]=1;control=host;
 u32 callees[4]={(u32)pop0,(u32)pop8,(u32)pop24,(u32)pop8};
 for(u32 tag=0;tag<4;++tag){
  used=sequence=serial=lost=active=io_failed=0;zero(frames,sizeof(frames));
  census_trampolines[tag]=callees[tag];
  SetLastError(1234);canvas_test_call(callees[tag],0x12340000,0xabcdef00,hooks[tag].pop,before);
  SetLastError(1234);canvas_test_call((u32)hooks[tag].entry,0x12340000,0xabcdef00,hooks[tag].pop,after);
  identical();if(GetLastError()!=1234||lost||active||serial!=1)ExitProcess(21);
  u32* row=(u32*)buffer;
  if(row[13]!=1234||row[4]!=2||row[6]!=hooks[tag].address)ExitProcess(22);
  if(hooks[tag].site){if(sequence!=1||row[1]!=3||row[7])ExitProcess(23);}
  else if(sequence!=2||row[1]!=1||row[33]!=2||row[45]!=1234||row[34]!=row[2])ExitProcess(24);
 }
 /* A tail call shares SP/thread with its parent; the newest frame returns first. */
 used=sequence=serial=lost=active=0;zero(frames,sizeof(frames));
 census_trampolines[4]=(u32)tail8;census_trampolines[1]=(u32)pop8;
 SetLastError(1234);canvas_test_call((u32)hooks[4].entry,0x12340000,0xabcdef00,8,after);
 identical();if(lost||active||serial!=2||sequence!=4||GetLastError()!=1234)ExitProcess(26);
 u32* rows=(u32*)buffer;
 if(rows[3]!=4||rows[35]!=1||rows[67]!=1||rows[99]!=4)ExitProcess(27);
 /* Exhaustion forwards the original and records loss rather than guessing. */
 for(u32 i=0;i<2048;++i)frames[i].sp=i+1;
 SetLastError(1234);canvas_test_call((u32)hooks[1].entry,0x12340000,0xabcdef00,8,after);
 identical();if(lost!=1||active||GetLastError()!=1234)ExitProcess(25);
 ExitProcess(0);
}
