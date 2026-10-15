#define MNM_DRAWING_SELFTEST
#define DllMain FixtureDllMain
#include "../runtime/drawing/session.c"
API HANDLE WIN CreateThread(void*,u32,u32(WIN*)(void*),void*,u32,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
static u32 callbacks;static u32 command[25];
static void require(int value,u32 code){if(!value)ExitProcess(code);}
static u32 WIN reenter(void* unused){(void)unused;EnterCriticalSection(journal_lock);LeaveCriticalSection(journal_lock);return 0;}
static void __attribute__((cdecl)) callback(void* context,const void* data){
 require(context==(void*)1&&data==command&&GetLastError()==0xbeef,111);HANDLE thread=CreateThread(0,0,reenter,0,0,0);require(thread&&WaitForSingleObject(thread,5000)==0,112);CloseHandle(thread);++callbacks;SetLastError(0xcafe);u16 cw=0x0b7f;__asm__ volatile("fldcw %0; fnclex"::"m"(cw));
}
void start(void){
 require(initialize(),110);wine_blit_worker_original=(void*)&callback;command[0]=42;command[1]=2;command[9]=3;command[2]=4;command[10]=5;
 for(u32 i=3;i<9;++i)command[i]=i*7;for(u32 i=11;i<25;++i)command[i]=i*11;
 for(u32 i=0;i<2;++i){void* object=(void*)(2+i);u32 r[24]={0};wine_lifetime_entry(r,1,0);wine_lifetime_return(r,0,0,&object);}
 for(u32 i=0;i<3;++i){SetLastError(0xbeef);wine_blit_worker((void*)1,command);u16 cw;__asm__ volatile("fnstcw %0":"=m"(cw));require(callbacks==i+1&&GetLastError()==0xcafe&&cw==0x0b7f&&!wine_blit_worker_pending,113);}
 wine_blit_worker_close(1);wine_lifetime_close(1);ExitProcess(0);
}
