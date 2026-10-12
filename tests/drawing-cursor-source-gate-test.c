#include "../runtime/shadow/win32_min.h"
API HANDLE WIN CreateMutexA(void*,u32,const char*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
API u32 WIN ReleaseMutex(HANDLE);
API HANDLE WIN CreateEventA(void*,u32,u32,const char*);
API u32 WIN SetEvent(HANDLE);
API HANDLE WIN CreateThread(void*,u32,u32(WIN*)(void*),void*,u32,u32*);
API u32 WIN GetExitCodeThread(HANDLE,u32*);
#include "../runtime/drawing/cursor_source_gate.h"
static HANDLE outer,inner,ready,stop;
static u32 selected;
static void require_gate(int ok,u32 code){if(!ok)ExitProcess(code);}
static unsigned gate_wait(void* handle,unsigned timeout){return WaitForSingleObject(handle,timeout);}
static unsigned gate_release(void* handle){return ReleaseMutex(handle);}
static u32 WIN owner_thread(void* ignored){
 (void)ignored;HANDLE mutex=selected?inner:outer;require_gate(WaitForSingleObject(mutex,10000)==0,210);require_gate(SetEvent(ready),211);
 require_gate(WaitForSingleObject(stop,10000)==0,212);require_gate(ReleaseMutex(mutex),213);return 0;
}
static u32 WIN abandoned_thread(void* ignored){(void)ignored;require_gate(WaitForSingleObject(selected?inner:outer,10000)==0,214);return 0;}
static u32 WIN probe_thread(void* ignored){
 HANDLE mutex=ignored;u32 result=WaitForSingleObject(mutex,0);if(result==0||result==0x80)require_gate(ReleaseMutex(mutex),215);return result;
}
static u32 probe_handle(HANDLE mutex){HANDLE thread=CreateThread(0,0,&probe_thread,mutex,0,0);require_gate(thread!=0&&WaitForSingleObject(thread,10000)==0,216);u32 result=0;require_gate(GetExitCodeThread(thread,&result)&&CloseHandle(thread),217);return result;}
static unsigned responses[2],release_mask,wait_count,release_count,order[2];
static unsigned scripted_wait(void* handle,unsigned timeout){
 require_gate(timeout==0&&handle==(wait_count?inner:outer)&&wait_count<2,230);
 return responses[wait_count++];
}
static unsigned scripted_release(void* handle){
 require_gate(release_count<2,231);unsigned bit=handle==inner?2:1;
 order[release_count++]=bit;return (release_mask&bit)!=0;
}
static void scripted_matrix(void){
 const unsigned values[4]={0,0x80,0x102,0xffffffff};
 for(unsigned a=0;a<4;++a)for(unsigned b=0;b<4;++b)for(unsigned mask=0;mask<4;++mask){
  MnmCursorSourceGate gate={0};responses[0]=values[a];responses[1]=values[b];release_mask=mask;wait_count=release_count=0;
  int accepted=mnm_cursor_gate_acquire(&gate,outer,inner,scripted_wait,scripted_release);
  unsigned owned=(a<2?1:0)|(a==0&&b<2?2:0);
  require_gate(accepted==(a==0&&b==0)&&wait_count==(a==0?2u:1u),232);
  if(accepted)mnm_cursor_gate_release(&gate,scripted_release);
  require_gate(gate.acquired==(owned&~mask),233);
  unsigned expected_failure=accepted?0:a==1?2:a==2?3:a==3?4:b==1?2:b==2?3:4;
  require_gate(gate.failure==((owned&~mask)?6:expected_failure),238);
  require_gate(release_count==(unsigned)((owned&1)!=0)+(unsigned)((owned&2)!=0),234);
  if(owned==3)require_gate(order[0]==2&&order[1]==1,235);
  if(gate.acquired){
   unsigned saved_waits=wait_count;require_gate(!mnm_cursor_gate_acquire(&gate,outer,inner,scripted_wait,scripted_release)&&gate.failure==5&&wait_count==saved_waits,236);
   release_count=0;release_mask=3;require_gate(mnm_cursor_gate_release(&gate,scripted_release)&&!gate.acquired,237);
  }
 }
}
void start(void){
 outer=CreateMutexA(0,0,0);inner=CreateMutexA(0,0,0);ready=CreateEventA(0,0,0,0);stop=CreateEventA(0,0,0,0);require_gate(outer&&inner&&ready&&stop,201);
 scripted_matrix();
 MnmCursorSourceGate gate={0};require_gate(mnm_cursor_gate_acquire(&gate,outer,inner,&gate_wait,&gate_release)&&gate.acquired==3,202);require_gate(probe_handle(outer)==0x102&&probe_handle(inner)==0x102,203);require_gate(mnm_cursor_gate_release(&gate,&gate_release)&&probe_handle(outer)==0&&probe_handle(inner)==0,204);
 // Preserve original recursive ownership: the policy releases only its extra
 // recursion level, so a competitor still times out until the caller releases.
 require_gate(WaitForSingleObject(outer,0)==0&&WaitForSingleObject(inner,0)==0,205);
 require_gate(mnm_cursor_gate_acquire(&gate,outer,inner,&gate_wait,&gate_release)&&mnm_cursor_gate_release(&gate,&gate_release),206);require_gate(probe_handle(outer)==0x102&&probe_handle(inner)==0x102,207);
 require_gate(ReleaseMutex(inner)&&ReleaseMutex(outer)&&probe_handle(outer)==0&&probe_handle(inner)==0,208);
 for(selected=0;selected<2;++selected){HANDLE thread=CreateThread(0,0,&owner_thread,0,0,0);require_gate(thread&&WaitForSingleObject(ready,10000)==0,218);
  require_gate(!mnm_cursor_gate_acquire(&gate,outer,inner,&gate_wait,&gate_release)&&gate.failure==3&&!gate.acquired,219);
  if(selected)require_gate(probe_handle(outer)==0&&probe_handle(inner)==0x102,220); // Failed inner acquisition unwound outer.
  require_gate(SetEvent(stop)&&WaitForSingleObject(thread,10000)==0&&CloseHandle(thread),221);
 }
 for(selected=0;selected<2;++selected){HANDLE abandoned=CreateThread(0,0,&abandoned_thread,0,0,0);require_gate(abandoned&&WaitForSingleObject(abandoned,10000)==0&&CloseHandle(abandoned),222);
 require_gate(!mnm_cursor_gate_acquire(&gate,outer,inner,&gate_wait,&gate_release)&&gate.failure==2&&!gate.acquired&&probe_handle(outer)==0&&probe_handle(inner)==0,223);}
 require_gate(!mnm_cursor_gate_acquire(&gate,0,inner,&gate_wait,&gate_release)&&gate.failure==1,224);
 require_gate(!mnm_cursor_gate_acquire(&gate,outer,outer,&gate_wait,&gate_release)&&gate.failure==1,225);
 require_gate(!mnm_cursor_gate_acquire(&gate,outer,inner,0,&gate_release)&&gate.failure==1,227);
 require_gate(!mnm_cursor_gate_acquire(&gate,outer,inner,&gate_wait,0)&&gate.failure==1,228);
 require_gate(!mnm_cursor_gate_acquire(&gate,(void*)0x12345678,inner,&gate_wait,&gate_release)&&gate.failure==4&&!gate.acquired,229);
 require_gate(CloseHandle(outer)&&CloseHandle(inner)&&CloseHandle(ready)&&CloseHandle(stop),226);ExitProcess(0);
}
