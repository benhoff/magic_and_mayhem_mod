#include "../runtime/shadow/win32_min.h"
API HANDLE WIN CreateMutexA(void*,u32,const char*);
API u32 WIN DuplicateHandle(HANDLE,HANDLE,HANDLE,HANDLE*,u32,u32,u32);
API u32 WIN WaitForSingleObject(HANDLE,u32);
API u32 WIN ReleaseMutex(HANDLE);
API u32 WIN GetCurrentThreadId(void);
API HANDLE WIN CreateEventA(void*,u32,u32,const char*);
API u32 WIN SetEvent(HANDLE);
API HANDLE WIN CreateThread(void*,u32,u32(WIN*)(void*),void*,u32,u32*);
API void WIN InitializeCriticalSection(void*);
API void WIN EnterCriticalSection(void*);
API void WIN LeaveCriticalSection(void*);
#include "../runtime/cursor-mutex/cursor_kernel_adapter.h"
static MnmCursorKernelAdapter adapter;
static unsigned lifetime_lock[6];
static HANDLE ready,go,closed,source_outer,source_inner;
static void check(int ok,u32 code){if(!ok)ExitProcess(code);}
static unsigned wait(void* h,unsigned n){return WaitForSingleObject(h,n);}
static unsigned release(void* h){return ReleaseMutex(h);}
static u32 WIN retiring_thread(void* ignored){
 (void)ignored;check(SetEvent(ready)&&WaitForSingleObject(go,10000)==0,220);
 EnterCriticalSection(lifetime_lock);
 check(mnm_cursor_kernel_retire(&adapter,0,source_outer,1)&&mnm_cursor_kernel_retire(&adapter,1,source_inner,2),221);
 LeaveCriticalSection(lifetime_lock);
 check(CloseHandle(source_outer)&&CloseHandle(source_inner)&&SetEvent(closed),222);return 0;
}
void start(void){
 InitializeCriticalSection(lifetime_lock);ready=CreateEventA(0,0,0,0);go=CreateEventA(0,0,0,0);closed=CreateEventA(0,0,0,0);check(ready&&go&&closed,223);
 unsigned owner=GetCurrentThreadId();
 HANDLE outer=CreateMutexA(0,0,0),inner=CreateMutexA(0,0,0),a=0,b=0,process=GetCurrentProcess();
 check(outer&&inner&&DuplicateHandle(process,outer,process,&a,0,0,2)&&DuplicateHandle(process,inner,process,&b,0,0,2),201);
 check(mnm_cursor_kernel_publish(&adapter,0,outer,a,1),202);
 check(!mnm_cursor_kernel_publish(&adapter,1,inner,a,2)&&!adapter.inner.duplicate,229);
 check(!mnm_cursor_kernel_publish(&adapter,1,outer,b,2)&&!adapter.inner.duplicate,230);
 check(!mnm_cursor_kernel_publish(&adapter,1,a,b,2)&&!adapter.inner.duplicate,231);
 check(!mnm_cursor_kernel_publish(&adapter,1,inner,outer,2)&&!adapter.inner.duplicate,232);
 check(!mnm_cursor_kernel_publish(&adapter,1,inner,b,1)&&!adapter.inner.duplicate,233);
 check(!mnm_cursor_kernel_publish(&adapter,2,inner,b,2)&&!adapter.inner.duplicate,234);
 check(mnm_cursor_kernel_publish(&adapter,1,inner,b,2),235);
 MnmCursorKernelLease* lease=mnm_cursor_kernel_reserve(&adapter,owner,outer,inner);check(lease!=0,203);unsigned token=lease->token->token;
 check(mnm_cursor_gate_acquire(&lease->gate,a,b,wait,release),204);
 check(!mnm_cursor_kernel_return(&adapter,owner+1,token)&&!mnm_cursor_kernel_return(&adapter,owner,token),205);
 source_outer=outer;source_inner=inner;
 HANDLE thread=CreateThread(0,0,retiring_thread,0,0,0);check(thread&&WaitForSingleObject(ready,10000)==0,224);
 check(SetEvent(go)&&WaitForSingleObject(closed,10000)==0,225);
 EnterCriticalSection(lifetime_lock);check(adapter.outer.retired&&adapter.inner.retired,227);LeaveCriticalSection(lifetime_lock);
 check(WaitForSingleObject(thread,10000)==0&&CloseHandle(thread),228);
 check(!mnm_cursor_kernel_reserve(&adapter,owner+1,outer,inner),207);
 check(!mnm_cursor_kernel_close_begin(&adapter,0)&&!mnm_cursor_kernel_close_begin(&adapter,1),208);

 check(mnm_cursor_gate_release(&lease->gate,release)&&mnm_cursor_kernel_return(&adapter,owner,token),210);
 check(!mnm_cursor_kernel_return(&adapter,owner,token),211);
 check(mnm_cursor_kernel_close_begin(&adapter,0)==a,212);
 check(mnm_cursor_kernel_close_end(&adapter,0,a,1,0)&&adapter.outer.duplicate==a,213);
 check(mnm_cursor_kernel_close_begin(&adapter,0)==a&&CloseHandle(a)&&mnm_cursor_kernel_close_end(&adapter,0,a,1,1),214);
 check(mnm_cursor_kernel_close_begin(&adapter,1)==b&&CloseHandle(b)&&mnm_cursor_kernel_close_end(&adapter,1,b,2,1),215);
 check(!adapter.outer.duplicate&&!adapter.inner.duplicate&&!adapter.outer.leases&&!adapter.inner.leases&&!adapter.fault,216);
 check(CloseHandle(ready)&&CloseHandle(go)&&CloseHandle(closed),226);ExitProcess(0);
}
