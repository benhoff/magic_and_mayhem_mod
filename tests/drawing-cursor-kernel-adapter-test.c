#include "../runtime/shadow/win32_min.h"
API HANDLE WIN CreateMutexA(void*,u32,const char*);
API u32 WIN DuplicateHandle(HANDLE,HANDLE,HANDLE,HANDLE*,u32,u32,u32);
API u32 WIN WaitForSingleObject(HANDLE,u32);
API u32 WIN ReleaseMutex(HANDLE);
API u32 WIN GetCurrentThreadId(void);
#include "../runtime/cursor-mutex/cursor_kernel_adapter.h"
static MnmCursorKernelAdapter adapter;
static void check(int ok,u32 code){if(!ok)ExitProcess(code);}
static unsigned wait(void* h,unsigned n){return WaitForSingleObject(h,n);}
static unsigned release(void* h){return ReleaseMutex(h);}
void start(void){
 unsigned owner=GetCurrentThreadId();
 HANDLE outer=CreateMutexA(0,0,0),inner=CreateMutexA(0,0,0),a=0,b=0,process=GetCurrentProcess();
 check(outer&&inner&&DuplicateHandle(process,outer,process,&a,0,0,2)&&DuplicateHandle(process,inner,process,&b,0,0,2),201);
 check(mnm_cursor_kernel_publish(&adapter,0,outer,a,1)&&mnm_cursor_kernel_publish(&adapter,1,inner,b,2),202);
 MnmCursorKernelLease* lease=mnm_cursor_kernel_reserve(&adapter,owner,outer,inner);check(lease!=0,203);unsigned token=lease->token->token;
 check(mnm_cursor_gate_acquire(&lease->gate,a,b,wait,release),204);
 check(!mnm_cursor_kernel_return(&adapter,owner+1,token)&&!mnm_cursor_kernel_return(&adapter,owner,token),205);
 check(mnm_cursor_kernel_retire(&adapter,0,outer,1)&&mnm_cursor_kernel_retire(&adapter,1,inner,2),206);
 check(!mnm_cursor_kernel_reserve(&adapter,owner+1,outer,inner),207);
 check(!mnm_cursor_kernel_close_begin(&adapter,0)&&!mnm_cursor_kernel_close_begin(&adapter,1),208);
 check(CloseHandle(outer)&&CloseHandle(inner),209);
 check(mnm_cursor_gate_release(&lease->gate,release)&&mnm_cursor_kernel_return(&adapter,owner,token),210);
 check(!mnm_cursor_kernel_return(&adapter,owner,token),211);
 check(mnm_cursor_kernel_close_begin(&adapter,0)==a,212);
 check(mnm_cursor_kernel_close_end(&adapter,0,a,1,0)&&adapter.outer.duplicate==a,213);
 check(mnm_cursor_kernel_close_begin(&adapter,0)==a&&CloseHandle(a)&&mnm_cursor_kernel_close_end(&adapter,0,a,1,1),214);
 check(mnm_cursor_kernel_close_begin(&adapter,1)==b&&CloseHandle(b)&&mnm_cursor_kernel_close_end(&adapter,1,b,2,1),215);
 check(!adapter.outer.duplicate&&!adapter.inner.duplicate&&!adapter.outer.leases&&!adapter.inner.leases&&!adapter.fault,216);
 ExitProcess(0);
}
