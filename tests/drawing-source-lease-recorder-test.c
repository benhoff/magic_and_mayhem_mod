#define MNM_DRAWING_SELFTEST
#define DllMain FixtureDllMain
#include "../runtime/drawing/session.c"
static u32 calls;
static u32 WIN getter(void* object,u32 lease,u32* metadata,u32 size){
 if(object!=(void*)1||size!=64)ExitProcess(161);++calls;metadata[0]=64;metadata[1]=1;metadata[2]=19;return lease?0:0x80000001u;
}
void start(void){
 if(!initialize())ExitProcess(160);source_lease_getter=&getter;struct Context c={0};c.observed=1;c.r[1]=1;c.r[2]=7;c.r[29]=1;c.r[6]=GetCurrentThreadId();c.r[4]=2;c.args[2]=1;c.args[4]=16;
 source_lease_capture(&c,1);if(c.source_lease!=1)ExitProcess(162);source_lease_capture(&c,2);if(calls!=2)ExitProcess(163);source_lease_close(1);ExitProcess(0);
}
