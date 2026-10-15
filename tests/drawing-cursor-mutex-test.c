#include "../runtime/shadow/win32_min.h"
API u32 WIN GetCurrentThreadId(void);
API u32 WIN GetTickCount(void);
API void WIN InitializeCriticalSection(void*);
API void WIN EnterCriticalSection(void*);
API void WIN LeaveCriticalSection(void*);
#define MNM_CURSOR_MUTEX_SELFTEST 1
#define DllMain fixture_unused_dll_main
#include "../runtime/cursor-mutex/observer.c"
#undef DllMain
static u32 callback_calls,expected_operation,expected_parameter,expected_result,expected_error;
static HANDLE expected_handle;
static u8 fp_input[512]__attribute__((aligned(16))),fp_output[512]__attribute__((aligned(16))),fp_actual[512]__attribute__((aligned(16)));
static void require_mutex(int ok,u32 code){if(!ok)ExitProcess(code);}
static int same_mutex(const void* a,const void* b,u32 n){const u8* x=a;const u8* y=b;while(n--)if(*x++!=*y++)return 0;return 1;}
static void original_enter(u32 op,HANDLE handle,u32 parameter){
 mutex_fp_save(fp_actual);require_mutex(same_mutex(fp_input,fp_actual,512),211);
 require_mutex(op==expected_operation&&handle==expected_handle&&parameter==expected_parameter&&GetLastError()==expected_error,212);++callback_calls;
}
static void original_leave(void){u16 control=0x037f;__asm__ volatile("fldcw %0"::"m"(control));mutex_fp_save(fp_output);SetLastError(0xdef01234);}
static HANDLE WIN fake_create(void* security,u32 owned,const char* name){
 require_mutex(security==(void*)1&&name==(const char*)2,213);original_enter(1,0,owned);original_leave();return (HANDLE)expected_result;
}
static u32 WIN fake_wait(HANDLE handle,u32 timeout){original_enter(2,handle,timeout);original_leave();return expected_result;}
static u32 WIN fake_release(HANDLE handle){original_enter(3,handle,0);original_leave();return expected_result;}
static u32 WIN fake_close(HANDLE handle){original_enter(4,handle,0);original_leave();return expected_result;}
static u32 WIN fake_point(u32 owner,void* output,u32 size){
 require_mutex(size==64&&owner==GetCurrentThreadId(),214);u32 row[16]={1,64,0x40209ca7,1,owner,20,24,0,0,0,0,0,0,0,0,0};mutex_copy(output,row,64);
 // Deliberately clobber diagnostic state; the outer wrapper must restore it.
 u16 control=0x017f;__asm__ volatile("fldcw %0"::"m"(control));SetLastError(0xdeadbeef);return 1;
}
static void prepare_mutex(u32 op,HANDLE handle,u32 parameter,u32 result){
 expected_operation=op;expected_handle=handle;expected_parameter=parameter;expected_result=result;expected_error=0xabc01234;
 for(u32 i=0;i<512;++i)fp_input[i]=fp_output[i]=fp_actual[i]=0;
 u16 control=0x027f;__asm__ volatile("fldcw %0"::"m"(control));mutex_fp_save(fp_input);SetLastError(expected_error);
}
static void verify_mutex(u32 result){mutex_fp_save(fp_actual);require_mutex(result==expected_result&&GetLastError()==0xdef01234&&same_mutex(fp_actual,fp_output,512),215);}
void start(void){
 InitializeCriticalSection(mutex_lock);original_create=&fake_create;original_wait=&fake_wait;original_release=&fake_release;original_close=&fake_close;original_point=&fake_point;
 char name[1024];u32 length=GetEnvironmentVariableA("MNM_DRAW_CURSOR_MUTEX_FILE",name,sizeof(name));require_mutex(length&&length<sizeof(name),201);
 mutex_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);require_mutex(mutex_file!=(HANDLE)-1,202);
 prepare_mutex(1,0,0,0x12345678);HANDLE created=observe_create((void*)1,0,(const char*)2);verify_mutex((u32)created);
 for(u32 timeout=0;timeout<3;++timeout)for(u32 result=0;result<4;++result){
  const u32 values[4]={0,0x80,0x102,0xffffffff},delays[3]={0,10,0xffffffff};prepare_mutex(2,created,delays[timeout],values[result]);verify_mutex(observe_wait(created,delays[timeout]));
 }
 for(u32 result=0;result<2;++result){prepare_mutex(3,created,0,result);verify_mutex(observe_release(created));}
 prepare_mutex(4,created,0,0);verify_mutex(observe_close(created));require_mutex(mutex_ids[0].live,216);
 prepare_mutex(4,created,0,1);verify_mutex(observe_close(created));require_mutex(!mutex_ids[0].live,217);
 prepare_mutex(1,0,1,0x12345678);created=observe_create((void*)1,1,(const char*)2);verify_mutex((u32)created);require_mutex(mutex_ids[0].id==2,218);
 prepare_mutex(4,created,0,1);verify_mutex(observe_close(created));
 prepare_mutex(2,(HANDLE)0x87654321,10,0);verify_mutex(observe_wait((HANDLE)0x87654321,10));
 require_mutex(callback_calls==20&&mutex_calls==20&&mutex_count==40&&!mutex_pending&&!mutex_dropped&&!identity_overflow,219);
 mutex_finish();ExitProcess(0);
}
