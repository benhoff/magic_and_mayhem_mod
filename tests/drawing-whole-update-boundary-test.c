#include "../runtime/shadow/win32_min.h"
#include "../protocols/include/mnm/canvas_producers_v4.h"
API u32 WIN GetCurrentThreadId(void);
static u32 whole_update_capture,producer_trace_mode=1,stopped,queue=1,limit=1,update_point_active,update_point_completed=1,update_point_first[16],failures,journal_failed,sequence,oracle,bytes=64;
static char directory[220];
#define MAX_FRAMES 2
#define MAX_SURFACES 2
#define PRODUCER_TRACE_SCOPE
static u32 producer_trace_mutex[6];
API void WIN InitializeCriticalSection(void*);
API void WIN EnterCriticalSection(void*);
API void WIN LeaveCriticalSection(void*);
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
#include "../runtime/scene/whole_update_capture.h"
#include "../runtime/engine-cadence/update_boundary.h"
API HANDLE WIN CreateThread(void*,u32,void*,void*,u32,u32*);
static u32 WIN retire_frame(void* unused){(void)unused;Sleep(30);EnterCriticalSection(producer_trace_mutex);frames[0].sp=0;LeaveCriticalSection(producer_trace_mutex);return 0;}
void start(void){
 InitializeCriticalSection(producer_trace_mutex);
 if(!GetEnvironmentVariableA("MNM_SCENE_DIR",directory,sizeof(directory))||!whole_update_configure())ExitProcess(190);char flag[2];GetEnvironmentVariableA("MNM_FIXTURE_MODE",flag,2);mode=flag[0];update_point_first[2]=10;update_point_first[3]=20;update_point_first[4]=30;update_point_first[6]=GetCurrentThreadId();
 update_boundary_enabled=1;update_boundary_source=&SceneDrawingUpdateBoundaryV1;u32 event[24]={0};event[0]=11;event[1]=2;event[5]=20;event[6]=31;
 queue=0;update_boundary_observe(event);if(closed||emitted)ExitProcess(193);queue=1;
 if(mode=='s')event[6]=32;if(mode=='t')event[5]=21;if(mode=='r')event[0]=10;if(mode=='o')++update_point_first[6];if(mode=='p'){frames[0].sp=1;frames[0].owner=GetCurrentThreadId();}if(mode=='a')update_point_active=1;if(mode=='n')update_point_completed=0;if(mode=='e')failures=1;
 if(mode=='w'||mode=='z'){frames[0].sp=1;frames[0].owner=GetCurrentThreadId()+1;if(mode=='w'){u32 id;if(!CreateThread(0,0,(void*)&retire_frame,0,0,&id))ExitProcess(196);}}
 SetLastError(0xbeef);update_boundary_observe(event);if(GetLastError()!=0xbeef||closed!=1||emitted!=1||checkpoints!=2)ExitProcess(194);update_boundary_observe(event);if(closed!=1||emitted!=1)ExitProcess(195);ExitProcess(0);
}
