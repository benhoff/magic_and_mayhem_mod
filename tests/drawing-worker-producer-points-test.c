#define MNM_DRAWING_SELFTEST
#define DllMain FixtureDllMain
#include "../runtime/drawing/session.c"
API HANDLE WIN CreateThread(void*,u32,u32(WIN*)(void*),void*,u32,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
static u32 callbacks,getters,refused,close_inside;static u32 command[25];
static u8 before_fp[512] __attribute__((aligned(16))),after_fp[512] __attribute__((aligned(16)));
static void require(int value,u32 code){if(!value)ExitProcess(code);}
static u32 WIN reenter(void* unused){(void)unused;EnterCriticalSection(journal_lock);LeaveCriticalSection(journal_lock);return 0;}
static void reentry(void){HANDLE t=CreateThread(0,0,reenter,0,0,0);require(t&&WaitForSingleObject(t,5000)==0&&CloseHandle(t),112);}
static u32 WIN point(void* object,u32* out,u32 size){
 require(!object&&size==64,114);++getters;reentry();u32 r[16]={64,1,3,callbacks*10,callbacks,0,0,0,0,0,0,0,0,0,0,0};r[14]=GetCurrentThreadId();copy(out,r,64);SetLastError(0xdead);__asm__ volatile("fninit; fldpi":::"memory");return !refused;
}
static void __attribute__((cdecl)) callback(void* context,const void* data){
 u8 actual[512] __attribute__((aligned(16)));memset(actual,0,512);snapshot_fp(actual);
 require(context==(void*)1&&data==command,121);require(GetLastError()==0xbeef,122);
 if(!equal(actual,before_fp,512)){char name[600];path(name,"fp-entry-mismatch.bin");HANDLE file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(file!=(HANDLE)-1){write(file,before_fp,512);write(file,actual,512);CloseHandle(file);}ExitProcess(123);}reentry();++callbacks;
 if(close_inside){EnterCriticalSection(journal_lock);worker_producer_close();LeaveCriticalSection(journal_lock);close_inside=0;}
 SetLastError(0xcafe);u16 cw=0x0b7f;__asm__ volatile("fldcw %0; fnclex"::"m"(cw));snapshot_fp(after_fp);
}
static void invoke(void){
 __asm__ volatile("fninit; fld1":::"memory");snapshot_fp(before_fp);SetLastError(0xbeef);wine_blit_worker((void*)1,command);
 u8 actual[512] __attribute__((aligned(16)));memset(actual,0,512);snapshot_fp(actual);require(GetLastError()==0xcafe&&equal(actual,after_fp,512)&&!wine_blit_worker_pending,113);
}
void start(void){
 char mode[32];u32 n=GetEnvironmentVariableA("MNM_WORKER_POINT_CASE",mode,sizeof(mode));require(n&&n<sizeof(mode),115);
 if(mode[0]=='i'){require(!initialize(),116);ExitProcess(0);}
 require(initialize(),110);wine_blit_worker_original=(void*)&callback;command[0]=42;command[1]=2;command[9]=3;command[2]=4;command[10]=5;
 for(u32 i=3;i<9;++i)command[i]=i*7;for(u32 i=11;i<25;++i)command[i]=i*11;
 for(u32 i=0;i<2;++i){void* object=(void*)(2+i);u32 r[24]={0};wine_lifetime_entry(r,1,0);wine_lifetime_return(r,0,0,&object);}
 if(mode[0]!='u')worker_producer_getter=&point;
 refused=mode[0]=='r';close_inside=mode[0]=='m';
 if(mode[0]=='b')worker_producer_rows=MNM_WORKER_PRODUCER_MAX_ROWS-1;
 for(u32 i=0;i<3;++i){invoke();require(callbacks==i+1,117);}
 EnterCriticalSection(journal_lock);wine_submission_close(1);LeaveCriticalSection(journal_lock);
 invoke();require(callbacks==4&&!worker_producer_file&&!worker_producer_enabled,118);
 if(mode[0]=='d'||mode[0]=='u')require(!getters,119);
 else require(getters==(mode[0]=='m'?1u:6u),120);
 ExitProcess(0);
}
