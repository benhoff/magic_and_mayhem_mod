#define MNM_DRAWING_SELFTEST
#define DllMain FixtureDllMain
#include "../runtime/drawing/session.c"
static u32 calls,status;
static void require(int value,u32 code){if(!value)ExitProcess(code);}
static u32 WIN getter(void* object,u32* before,u32* after,u8* out,u32 capacity){
 require(object==(void*)1&&capacity==960000,141);++calls;u32 row[16]={64,1,19,7,0,0,1,800,600,800,1,0,0,0,7,0};copy(before,row,64);copy(after,row,64);if(!status)for(u32 i=0;i<480000;++i)((u16*)out)[i]=(u16)i;return status;
}
void start(void){
 require(initialize(),140);source_bytes_getter=&getter;struct Context c={0};c.observed=1;c.r[1]=1;c.r[2]=7;c.r[29]=1;c.r[6]=GetCurrentThreadId();c.r[4]=2;c.r[3]=3;u32 rect[4]={0,0,800,600};c.args[2]=1;c.args[3]=(u32)rect;c.args[4]=16;
 SetLastError(0xbeef);source_bytes_capture(&c,1);source_bytes_capture(&c,2);require(calls==2&&source_bytes_planes==2,142);
 c.r[29]=2;status=2;source_bytes_capture(&c,1);source_bytes_capture(&c,2);require(calls==4&&source_bytes_planes==2,143);
 source_bytes_close(1);ExitProcess(0);
}
