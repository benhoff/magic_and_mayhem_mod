#define MNM_DRAWING_SELFTEST
#define DllMain FixtureDllMain
#include "../runtime/drawing/session.c"
static u8* pages;
static u32 active,copies;
static char mode;
static u32 WIN lease_getter(void* object,u32 lease,u32* metadata,u32 size){
 if(object!=(void*)1||size!=64)ExitProcess(160);u32 old;
 if(!lease){if(active||!VirtualProtect(pages,960000,2,&old)||old!=4)ExitProcess(161);active=1;metadata[0]=64;metadata[1]=1;metadata[2]=19;return 0x80000001u;}
 if(lease!=1||!active||copies!=2||!VirtualProtect(pages,960000,4,&old)||old!=2)ExitProcess(162);active=0;return 0;
}
static u32 WIN bytes_getter(void* object,u32* before,u32* after,u8* out,u32 capacity){
 u32 memory[7];if(object!=(void*)1||capacity!=960000||!active||VirtualQuery(pages,memory,28)!=28||memory[5]!=2)ExitProcess(163);
 ++copies;u32 row[16]={64,1,19,7,0,0,1,800,600,800,1,0,0,0,7,0};copy(before,row,64);copy(after,row,64);copy(out,pages,960000);
 if(mode=='c'&&copies==2)out[0]^=1;if(mode=='m'&&copies==2){before[6]=2;after[6]=2;}return mode=='r'?2:0;
}
void start(void){
 if(!initialize())ExitProcess(164);char flag[2];GetEnvironmentVariableA("MNM_FIXTURE_MODE",flag,2);mode=flag[0];pages=VirtualAlloc(0,960000,0x3000,4);if(!pages)ExitProcess(165);for(u32 i=0;i<480000;++i)((u16*)pages)[i]=(u16)i;
 source_lease_getter=&lease_getter;source_bytes_getter=&bytes_getter;struct Context c={0};c.observed=1;c.r[1]=1;c.r[2]=7;c.r[29]=1;c.r[6]=GetCurrentThreadId();c.r[4]=2;c.r[3]=3;u32 rect[4]={0,0,800,600};c.args[2]=1;c.args[3]=(u32)rect;c.args[4]=16;
 source_lease_capture(&c,1);if(!active||copies!=1)ExitProcess(166);EnterCriticalSection(journal_lock);source_worker_lease_packet(1,10,1,0);source_worker_lease_packet(2,10,0,70);source_worker_lease_packet(3,10,0,70);LeaveCriticalSection(journal_lock);
 source_lease_capture(&c,2);if(active||copies!=2||source_held_first||source_held_planes!=2)ExitProcess(167);pages[0]=7;source_lease_close(1);ExitProcess(0);
}
