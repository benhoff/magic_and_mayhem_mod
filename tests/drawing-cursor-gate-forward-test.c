#include "../runtime/shadow/win32_min.h"
#include "../runtime/drawing/cursor_source_gate.h"
#include "../runtime/drawing/cursor_gate_forward.h"
static u8 input[512]__attribute__((aligned(16))),output[512]__attribute__((aligned(16))),actual[512]__attribute__((aligned(16)));
static unsigned waits,releases,forwards,outer_result,inner_result,release_mask;
static void require_forward(int ok,u32 code){if(!ok)ExitProcess(code);}
static void save(void* out){__asm__ volatile("fxsave (%0)"::"r"(out):"memory");}
static void restore(void* in){__asm__ volatile("fxrstor (%0)"::"r"(in):"memory");}
static unsigned get_error(void){return GetLastError();}
static void set_error(unsigned value){SetLastError(value);}
static int same(const u8* a,const u8* b){for(unsigned i=0;i<512;++i)if(a[i]!=b[i])return 0;return 1;}
static void clobber(void){u16 control=0x017f;__asm__ volatile("fldcw %0"::"m"(control));SetLastError(0xdeadbeef);}
static unsigned wait(void* handle,unsigned timeout){require_forward(!timeout&&handle==(waits?(void*)2:(void*)1),210);++waits;clobber();return waits==1?outer_result:inner_result;}
static unsigned release(void* handle){require_forward(handle==(void*)1||handle==(void*)2,211);++releases;clobber();return (release_mask&(handle==(void*)2?2u:1u))!=0;}
static unsigned original(void* argument){save(actual);require_forward(argument==(void*)3&&GetLastError()==0xabc01234&&same(input,actual),212);++forwards;u16 control=0x037f;__asm__ volatile("fldcw %0"::"m"(control));save(output);SetLastError(0xdef01234);return 0xfedcba98;}
void start(void){
 const u32 values[4]={0,0x80,0x102,0xffffffff};
 for(unsigned a=0;a<4;++a)for(unsigned b=0;b<4;++b)for(unsigned mask=0;mask<4;++mask){
  MnmCursorSourceGate gate={0};outer_result=values[a];inner_result=values[b];release_mask=mask;waits=releases=forwards=0;
  for(unsigned i=0;i<512;++i)input[i]=output[i]=actual[i]=0;
  u16 control=0x027f;__asm__ volatile("fldcw %0"::"m"(control));save(input);SetLastError(0xabc01234);
  unsigned result=mnm_cursor_gate_forward(&gate,(void*)1,(void*)2,wait,release,original,(void*)3,get_error,set_error,save,restore);
  save(actual);require_forward(result==0xfedcba98&&GetLastError()==0xdef01234&&same(output,actual)&&forwards==1,213);
  unsigned owned=(a<2?1u:0u)|(a==0&&b<2?2u:0u);if(owned&~mask)require_forward(gate.failure==6,215);require_forward(gate.acquired==(owned&~mask)&&waits==(a==0?2u:1u)&&releases==(unsigned)((owned&1)!=0)+(unsigned)((owned&2)!=0),214);
 }
 ExitProcess(0);
}
