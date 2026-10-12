#include "../runtime/shadow/win32_min.h"
API HANDLE WIN CreateMutexA(void*,u32,const char*);
API u32 WIN GetCurrentThreadId(void);
API u32 WIN WaitForSingleObject(HANDLE,u32);
API u32 WIN ReleaseMutex(HANDLE);
API u32 WIN SetEnvironmentVariableA(const char*,const char*);
static u8* owned;
#define MNM_CURSOR_GATE_BASE 0x18000000u
#define MNM_CURSOR_GATE_CREATE_INNER ((u32)owned+0x100)
#define MNM_CURSOR_GATE_CREATE_OUTER ((u32)owned+0x200)
#define MNM_CURSOR_GATE_CLOSE_INNER ((u32)owned+0x300)
#define MNM_CURSOR_GATE_CLOSE_OUTER ((u32)owned+0x400)
#define MNM_CURSOR_GATE_GLOBAL_INNER ((u32)owned+0x500)
#define MNM_CURSOR_GATE_GLOBAL_OUTER ((u32)owned+0x504)
#define MNM_CURSOR_GATE_IAT ((u32)owned+0x1000)
#define DllMain unused_module_main
#include "../runtime/cursor-source-gate/module.c"
#undef DllMain
static void check(int ok,u32 code){if(!ok)ExitProcess(code);}
static u8 fp_before[512]__attribute__((aligned(16))),fp_after[512]__attribute__((aligned(16)));
static void prepare(void){for(u32 i=0;i<512;++i)fp_before[i]=fp_after[i]=0;u16 control=0x027f;__asm__ volatile("fldcw %0"::"m"(control));cursor_gate_fx_save(fp_before);SetLastError(0xabcdef01);}
static void verify(void){cursor_gate_fx_save(fp_after);check(GetLastError()==0xabcdef01&&cursor_gate_equal(fp_after,fp_before,512),230);}
static unsigned fake_creates,fake_closes,expected_error,fake_result;
static HANDLE WIN fake_create(void* security,unsigned initial_owner,const char* name){
 cursor_gate_fx_save(fp_after);check(GetLastError()==expected_error&&cursor_gate_equal(fp_after,fp_before,512),240);
 check(security==(void*)1&&initial_owner==7&&name==(const char*)2,241);++fake_creates;
 u16 control=0x037f;__asm__ volatile("fldcw %0"::"m"(control));cursor_gate_fx_save(fp_before);SetLastError(0xdef01234);return (HANDLE)fake_result;
}
static unsigned WIN fake_close(HANDLE handle){
 cursor_gate_fx_save(fp_after);check(GetLastError()==expected_error&&cursor_gate_equal(fp_after,fp_before,512)&&handle==(HANDLE)0x12345678,242);++fake_closes;
 u16 control=0x037f;__asm__ volatile("fldcw %0"::"m"(control));cursor_gate_fx_save(fp_before);SetLastError(0xdef01234);return fake_result;
}
static void check_forward(void){
 CursorCreate saved_create=cursor_gate_original_create;CursorHandleCall saved_close=cursor_gate_original_close;
 cursor_gate_original_create=fake_create;cursor_gate_original_close=fake_close;
 for(unsigned i=0;i<2;++i){
  fake_result=i?0x12345678:0;prepare();expected_error=0xabcdef01;
  HANDLE result=cursor_gate_create_at((void*)1,7,(const char*)2,0);cursor_gate_fx_save(fp_after);
  check(result==(HANDLE)fake_result&&GetLastError()==0xdef01234&&cursor_gate_equal(fp_after,fp_before,512)&&fake_creates==i+1,243);
  fake_result=i?0x87654321:0;prepare();expected_error=0xabcdef01;
  unsigned closed=cursor_gate_close((HANDLE)0x12345678);cursor_gate_fx_save(fp_after);
  check(closed==fake_result&&GetLastError()==0xdef01234&&cursor_gate_equal(fp_after,fp_before,512)&&fake_closes==i+1,244);
 }
 cursor_gate_original_create=saved_create;cursor_gate_original_close=saved_close;
}
void start(void){
 owned=VirtualAlloc(0,65536,0x3000,0x40);check(owned!=0,201);
 cursor_gate_copy((void*)MNM_CURSOR_GATE_CREATE_INNER,cursor_gate_create_inner_pin,sizeof(cursor_gate_create_inner_pin));
 cursor_gate_copy((void*)MNM_CURSOR_GATE_CREATE_OUTER,cursor_gate_create_outer_pin,sizeof(cursor_gate_create_outer_pin));
 cursor_gate_copy((void*)MNM_CURSOR_GATE_CLOSE_INNER,cursor_gate_close_inner_pin,sizeof(cursor_gate_close_inner_pin));
 cursor_gate_copy((void*)MNM_CURSOR_GATE_CLOSE_OUTER,cursor_gate_close_outer_pin,sizeof(cursor_gate_close_outer_pin));
 *(void**)MNM_CURSOR_GATE_IAT=(void*)CloseHandle;*(void**)(MNM_CURSOR_GATE_IAT+0x18)=(void*)CreateMutexA;
 *(void**)(MNM_CURSOR_GATE_IAT+0xa0)=(void*)WaitForSingleObject;*(void**)(MNM_CURSOR_GATE_IAT+0x9c)=(void*)ReleaseMutex;
 check(SetEnvironmentVariableA("MNM_DRAW_CURSOR_SOURCE_GATE","1"),202);
 const unsigned addresses[4]={MNM_CURSOR_GATE_CREATE_INNER,MNM_CURSOR_GATE_CREATE_OUTER,MNM_CURSOR_GATE_CLOSE_INNER,MNM_CURSOR_GATE_CLOSE_OUTER};
 for(unsigned i=0;i<4;++i){*(u8*)addresses[i]^=1;check(!cursor_gate_install()&&!cursor_gate_enabled,203);*(u8*)addresses[i]^=1;}
 void* expected=*(void**)(MNM_CURSOR_GATE_IAT+0x18);*(void**)(MNM_CURSOR_GATE_IAT+0x18)=0;check(!cursor_gate_install()&&!cursor_gate_enabled,245);*(void**)(MNM_CURSOR_GATE_IAT+0x18)=expected;
 check(cursor_gate_install()&&cursor_gate_enabled,204);
 HANDLE outer=cursor_gate_create_at(0,0,0,MNM_CURSOR_GATE_CREATE_OUTER+6);
 HANDLE inner=cursor_gate_create_at(0,0,0,MNM_CURSOR_GATE_CREATE_INNER+6);
 check(outer&&inner&&cursor_gate_adapter.outer.duplicate&&cursor_gate_adapter.inner.duplicate,205);
 *(void**)MNM_CURSOR_GATE_GLOBAL_OUTER=outer;*(void**)MNM_CURSOR_GATE_GLOBAL_INNER=inner;
 unsigned point[32],owner=GetCurrentThreadId();prepare();unsigned token=CursorSourceGateBeginV1(owner,point,128);verify();
 check(token&&point[5]==token&&point[13]==3&&point[9]==0&&point[10]==0,206);
 prepare();check(!CursorSourceGateEndV1(owner+1,token,point,128),207);verify();
 check(cursor_gate_close(outer)&&cursor_gate_close(inner),208);
 prepare();check(!CursorSourceGateBeginV1(owner,point,128)&&point[4]==7,209);verify();
 prepare();check(CursorSourceGateEndV1(owner,token,point,128)&&point[13]==0&&point[11]==1&&point[12]==1,210);verify();
 check(!cursor_gate_adapter.outer.duplicate&&!cursor_gate_adapter.inner.duplicate&&!cursor_gate_adapter.outer.leases&&!cursor_gate_adapter.inner.leases,211);
 prepare();check(!CursorSourceGateEndV1(owner,token,point,128),212);verify();
 prepare();check(!CursorSourceGateBeginV1(owner,0,128)&&!CursorSourceGateBeginV1(owner,point,127),213);verify();
 check_forward();
 prepare();check(CursorSourceGateShutdownV1(owner,point,128)&&point[13]==0&&point[14]==0,214);verify();
 ExitProcess(0);
}
