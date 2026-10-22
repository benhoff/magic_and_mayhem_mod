#define MNM_DRAWING_SELFTEST
#define DllMain FixtureDllMain
#include "../runtime/drawing/session.c"
API HANDLE WIN CreateThread(void*,u32,void*,void*,u32,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
static volatile u32 worker_done;
static u32 WIN worker(void* unused){
 (void)unused;Sleep(40);EnterCriticalSection(journal_lock);source_worker_lease_packet(2,10,0,70);source_worker_lease_packet(3,10,0,70);worker_done=1;LeaveCriticalSection(journal_lock);return 0;
}
static u32 WIN getter(void* object,u32 lease,u32* metadata,u32 size){(void)object;(void)size;metadata[0]=64;metadata[1]=1;metadata[2]=19;return lease?0:0x80000001u;}
void start(void){
 if(!initialize())ExitProcess(160);source_lease_getter=&getter;struct Context c={0};c.observed=1;c.r[1]=1;c.r[2]=7;c.r[29]=1;c.r[6]=GetCurrentThreadId();c.r[4]=2;c.args[2]=1;c.args[4]=16;source_lease_capture(&c,1);
 char mode[2];GetEnvironmentVariableA("MNM_FIXTURE_MODE",mode,2);HANDLE thread=0;
 if(mode[0]=='a'){source_worker_lease.api=0;c.r[29]=0;source_worker_lease_begin(&c);ExitProcess(164);}
 if(mode[0]=='l'){source_worker_lease.api=0;c.source_lease=0;source_worker_lease_begin(&c);ExitProcess(165);}
 if(mode[0]=='b'){source_worker_lease.api=0;source_worker_lease_records=16384;source_worker_lease_close(1);ExitProcess(166);}EnterCriticalSection(journal_lock);
 /* An unrelated pending command never satisfies this API's completion. */
 source_worker_lease_packet(1,11,2,0);
 if(mode[0]!='n')source_worker_lease_packet(1,10,1,0);
 if(mode[0]=='u')source_worker_lease_packet(2,0,0,71);
 if(mode[0]=='e'){source_worker_lease_packet(2,10,0,70);source_worker_lease_packet(3,10,0,70);}
 LeaveCriticalSection(journal_lock);
 if(mode[0]=='d'){u32 id;thread=CreateThread(0,0,(void*)&worker,0,0,&id);if(!thread)ExitProcess(161);}
 u32 started=GetTickCount();source_lease_capture(&c,2);if(mode[0]!='e'&&mode[0]!='d')ExitProcess(162);
 if(mode[0]=='d'&&(!worker_done||GetTickCount()-started<20||WaitForSingleObject(thread,1000)))ExitProcess(163);
 source_lease_close(1);ExitProcess(0);
}
