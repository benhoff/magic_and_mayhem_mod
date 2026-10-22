#include "../runtime/shadow/win32_min.h"
#include "../protocols/include/mnm/canvas_producers_v4.h"
API u32 WIN GetCurrentThreadId(void);
API u32 WIN QueryPerformanceCounter(void*);
static u32 retired_qpc[2];
static u32 whole_update_capture,producer_trace_mode=1,stopped,queue=1,limit=1,update_point_active,update_point_completed=1,update_point_first[16],failures,journal_failed,sequence,oracle,bytes=64;
static char directory[220];
#define MAX_FRAMES 2
#define MAX_SURFACES 2
static u32 producer_trace_mutex[6];
API void WIN InitializeCriticalSection(void*);
API void WIN EnterCriticalSection(void*);
API void WIN LeaveCriticalSection(void*);
API u32 WIN TryEnterCriticalSection(void*);
API HANDLE WIN CreateEventA(void*,u32,u32,const char*);
API u32 WIN SetEvent(HANDLE);
API u32 WIN WaitForSingleObject(HANDLE,u32);
static void unlock_scope(u32* unused){(void)unused;LeaveCriticalSection(producer_trace_mutex);}
#define PRODUCER_TRACE_SCOPE u32 scope __attribute__((cleanup(unlock_scope)))=(EnterCriticalSection(producer_trace_mutex),0)
static struct {u32 sp,tag,owner;} frames[MAX_FRAMES];
static struct Surface {u32 id;} surfaces[MAX_SURFACES];
static u32 emitted,closed,checkpoints;static char mode;
static void copy(void* out,const void* in,u32 n){u8* d=out;const u8* s=in;while(n--)*d++=*s++;}
static void path(char* out,const char* name,u32 number,const char* suffix){(void)number;u32 i=0;while(directory[i]){out[i]=directory[i];++i;}out[i++]='\\';while(*name)out[i++]=*name++;copy(out+i,"0000",4);i+=4;while(*suffix)out[i++]=*suffix++;out[i]=0;}
static int write(HANDLE f,const void* p,u32 n){u32 done;return WriteFile(f,p,n,&done,0)&&done==n;}
static void checkpoint(struct Surface* s,u32 reason){(void)s;if(reason!=5)ExitProcess(191);++checkpoints;if(mode=='c')stopped=1;}
static void emit(u32* r,const void* payload,u32 n){(void)payload;if(n||r[2]!=27||r[14]!=1||r[15]!=30||r[16]!=20||r[17]!=31||r[19]!=11||r[22]!=3)ExitProcess(192);++emitted;++sequence;bytes+=96;}
static int journal_flush(void){return mode!='f';}
static void canvas_producers_close(void){++closed;stopped=1;}
static HANDLE probe_start,probe_done;
static u32 probe_locked,observed_boundaries;
static void primitive_lifetimes_boundary(u32 record,u32 accepted,u32 pending){
 (void)record;(void)accepted;if(pending)ExitProcess(209);++observed_boundaries;
 if(mode=='l'&&observed_boundaries==1){
  if(!SetEvent(probe_start)||WaitForSingleObject(probe_done,1000)!=0||probe_locked)ExitProcess(210);
 }
}
#ifndef MNM_WHOLE_UPDATE_HEADER
#define MNM_WHOLE_UPDATE_HEADER "../runtime/scene/whole_update_capture.h"
#endif
#include MNM_WHOLE_UPDATE_HEADER
#include "../runtime/engine-cadence/update_boundary.h"
API HANDLE WIN CreateThread(void*,u32,void*,void*,u32,u32*);
static u32 WIN retire_frame(void* unused){(void)unused;Sleep(30);EnterCriticalSection(producer_trace_mutex);if(!QueryPerformanceCounter(retired_qpc))ExitProcess(216);frames[0].sp=0;if(mode=='j')stopped=1;LeaveCriticalSection(producer_trace_mutex);return 0;}
static u32 WIN probe_lock(void* unused){
 (void)unused;if(WaitForSingleObject(probe_start,1000)!=0)ExitProcess(211);
 probe_locked=TryEnterCriticalSection(producer_trace_mutex);
 if(probe_locked)LeaveCriticalSection(producer_trace_mutex);
 if(!SetEvent(probe_done))ExitProcess(212);return 0;
}
static void observe_boundary(u32* event){
 SetLastError(0xbeef);update_boundary_prepare(event);if(GetLastError()!=0xbeef)ExitProcess(217);
 if(!QueryPerformanceCounter(event+3))ExitProcess(218);
 if((retired_qpc[0]||retired_qpc[1])&&(((unsigned long long)event[4]<<32)|event[3])<(((unsigned long long)retired_qpc[1]<<32)|retired_qpc[0]))ExitProcess(219);
 update_boundary_observe(event);
}
static u32 WIN foreign_cancel(void* value){
 if(SceneDrawingUpdateCancelV1((u32)value)||SceneDrawingUpdatePrepareV1(99,1,1,0))ExitProcess(220);return 0;
}
void start(void){
 InitializeCriticalSection(producer_trace_mutex);
 if(!whole_update_configure()||source_windows_file||!GetEnvironmentVariableA("MNM_SCENE_DIR",directory,sizeof(directory))||!source_windows_open())ExitProcess(190);char flag[2];GetEnvironmentVariableA("MNM_FIXTURE_MODE",flag,2);mode=flag[0];update_point_first[2]=10;update_point_first[3]=20;update_point_first[4]=30;update_point_first[6]=GetCurrentThreadId();
 update_boundary_enabled=1;update_boundary_source_prepare=&SceneDrawingUpdatePrepareV1;update_boundary_source_commit=&SceneDrawingUpdateCommitV1;update_boundary_source_cancel=&SceneDrawingUpdateCancelV1;u32 event[24]={0};event[0]=11;event[1]=2;event[5]=20;event[6]=31;
 if(mode=='m')boundary_lease_next=0xffffffffu;
 if(mode=='h'||mode=='i'||mode=='k'){
  u32 token=SceneDrawingUpdatePrepareV1(8,29,18,0);if(!token)ExitProcess(221);
  if(SceneDrawingUpdatePrepareV1(8,29,18,0)||SceneDrawingUpdateCancelV1(token+1)||SceneDrawingUpdateCommitV1(token,8,30,18,0))ExitProcess(222);
  if(mode=='i'){u32 id;HANDLE thread=CreateThread(0,0,(void*)&foreign_cancel,(void*)token,0,&id);if(!thread||WaitForSingleObject(thread,1000)!=0)ExitProcess(223);CloseHandle(thread);}
  if(!SceneDrawingUpdateCancelV1(token)||SceneDrawingUpdateCancelV1(token))ExitProcess(224);
  if(mode=='k'){
   update_boundary_prepare(event);update_boundary_cancel();
   if(update_boundary_token)ExitProcess(225);
  }
 }
 if(mode=='l'){
  probe_start=CreateEventA(0,1,0,0);probe_done=CreateEventA(0,1,0,0);u32 id;
  if(!probe_start||!probe_done||!CreateThread(0,0,(void*)&probe_lock,0,0,&id))ExitProcess(213);
 }
 if(mode=='d'||mode=='q'||mode=='b'||mode=='j'||mode=='u'){
  frames[0].sp=1;frames[0].owner=GetCurrentThreadId()+(mode=='q'?0:1);
  if(mode=='u'){frames[1].sp=2;frames[1].owner=GetCurrentThreadId();frames[1].tag=99;}
  if(mode=='d'||mode=='j'){u32 id;if(!CreateThread(0,0,(void*)&retire_frame,0,0,&id))ExitProcess(214);}
 }
 if(mode=='v'||mode=='x'){
  SetLastError(0xbeef);u32 bad_record=mode=='v'?0:9,bad_mode=mode=='x'?256:0;
  if(SceneDrawingUpdateBoundaryV1(bad_record,30,19,bad_mode)||GetLastError()!=0xbeef||source_windows_records||observed_boundaries)ExitProcess(226);
 }
 queue=0;event[0]=9;event[5]=19;event[6]=30;observe_boundary(event);if(closed||emitted)ExitProcess(193);if(mode=='j'){if(!stopped||observed_boundaries)ExitProcess(215);ExitProcess(0);}queue=1;event[0]=11;event[5]=20;event[6]=31;
 if(mode=='s')event[6]=32;if(mode=='t')event[5]=21;if(mode=='r')event[0]=10;if(mode=='o')++update_point_first[6];if(mode=='p'){frames[0].sp=1;frames[0].owner=GetCurrentThreadId();}if(mode=='a')update_point_active=1;if(mode=='n')update_point_completed=0;if(mode=='e')failures=1;
 if(mode=='w'||mode=='z'){frames[0].sp=1;frames[0].owner=GetCurrentThreadId()+1;if(mode=='w'){u32 id;if(!CreateThread(0,0,(void*)&retire_frame,0,0,&id))ExitProcess(196);}}
 SetLastError(0xbeef);observe_boundary(event);if(GetLastError()!=0xbeef||closed!=1||emitted!=1||checkpoints!=2)ExitProcess(194);observe_boundary(event);if(closed!=1||emitted!=1)ExitProcess(195);ExitProcess(0);
}
