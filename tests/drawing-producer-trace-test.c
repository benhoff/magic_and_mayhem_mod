/* Actual PE32 Win32 mutex/thread fixture for recorder ownership only. */
#include "../runtime/shadow/win32_min.h"
API u32 WIN GetCurrentThreadId(void);
API HANDLE WIN CreateThread(void*,u32,u32(WIN*)(void*),void*,u32,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
API void WIN Sleep(u32);
static u32 depth;
#include "../runtime/scene/producer_trace_guard.h"
static u32 ready,release_child,shared,child_id;
static void require(int okay,u32 code){if(!okay)ExitProcess(code);}
static void increment(u32 owner){
 PRODUCER_TRACE_SCOPE;
 u32* nesting=producer_trace_depth(owner);require(*nesting==1,51);
 ++shared;SetLastError(0xabcd);
}
static u32 WIN child(void* unused){
 (void)unused;child_id=GetCurrentThreadId();
 {PRODUCER_TRACE_SCOPE;u32* nesting=producer_trace_depth(child_id);require(!*nesting,52);++*nesting;}
 __atomic_store_n(&ready,1,__ATOMIC_RELEASE);
 for(u32 i=0;i<1000;++i){increment(child_id);require(GetLastError()==0xabcd,53);}
 while(!__atomic_load_n(&release_child,__ATOMIC_ACQUIRE))Sleep(0);
 {PRODUCER_TRACE_SCOPE;u32* nesting=producer_trace_depth(child_id);require(*nesting==1,54);--*nesting;}
 return 0;
}
void start(void){
 char test[16];u32 n=GetEnvironmentVariableA("MNM_TRACE_TEST",test,sizeof(test));
 if(n&&test[0]=='i'){require(!producer_trace_configure(),40);ExitProcess(0);}
 require(producer_trace_configure()&&producer_trace_mode,41);
 u32 owner=GetCurrentThreadId();
 if(n&&test[0]=='z'){PRODUCER_TRACE_SCOPE;(void)producer_trace_depth(0);ExitProcess(55);}
 if(n&&test[0]=='e'){PRODUCER_TRACE_SCOPE;for(u32 i=1;i<=33;++i)(void)producer_trace_depth(i);ExitProcess(42);}
 {PRODUCER_TRACE_SCOPE;u32* nesting=producer_trace_depth(owner);require(!*nesting,43);++*nesting;}
 /* No recorder mutex is held across the simulated original body. If it is,
  * the child cannot enter and this actual bounded wait refuses. */
 HANDLE thread=CreateThread(0,0,child,0,0,0);require(thread!=0,44);
 for(u32 i=0;i<5000&&!__atomic_load_n(&ready,__ATOMIC_ACQUIRE);++i)Sleep(1);
 require(__atomic_load_n(&ready,__ATOMIC_ACQUIRE),45);
 for(u32 i=0;i<1000;++i){increment(owner);require(GetLastError()==0xabcd,46);}
 {PRODUCER_TRACE_SCOPE;require(*producer_trace_depth(owner)==1,47);--*producer_trace_depth(owner);}
 __atomic_store_n(&release_child,1,__ATOMIC_RELEASE);
 require(WaitForSingleObject(thread,5000)==0&&CloseHandle(thread),48);
 {PRODUCER_TRACE_SCOPE;require(shared==2000&&!depth&&!(*producer_trace_depth(owner))&&!(*producer_trace_depth(child_id)),49);}
 ExitProcess(0);
}
