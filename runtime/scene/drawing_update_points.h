#include "../../protocols/include/mnm/engine_cadence_queue_point_v1.h"
API void* WIN GetProcAddress(void*,const char*);
typedef u32(WIN* CadenceQueuePoint)(void*,u32,u32*,u32);
static CadenceQueuePoint update_point_getter;
static HANDLE update_point_file;
static u32 update_point_enabled,update_point_records,update_point_active,update_point_completed,update_point_first[16];
static int drawing_update_points_open(void){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW_UPDATE_BINDING",flag,2);if(!n)return 1;if(n!=1||flag[0]!='1'||!producer_trace_mode||!limit||limit>120)return 0;
 char name[260];path(name,"producer-update-points-",0,".bin");update_point_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(update_point_file==(HANDLE)-1)return 0;
 u32 h[16]={0};copy(h,"MNMDUP01",8);h[2]=1;h[3]=96;h[4]=0x40209ca7;h[5]=256;h[6]=limit;if(!write(update_point_file,h,64))return 0;update_point_enabled=1;return 1;
}
static void drawing_update_point(u32 phase,u32 object,u32 caller){
 if(!update_point_enabled)return;if(!update_point_getter){void* module=GetModuleHandleA("MnmEngineCadence.dll");update_point_getter=module?(CadenceQueuePoint)GetProcAddress(module,"EngineCadenceQueuePointV1"):0;if(!update_point_getter)ExitProcess(99);}
 if(phase==1){if(update_point_active)ExitProcess(99);update_point_active=queue;}else if(phase!=2||update_point_active!=queue)ExitProcess(99);
 u32 r[24]={0};r[0]=++update_point_records;r[1]=phase;r[2]=queue;r[3]=GetCurrentThreadId();r[4]=sequence;struct Surface* s=bound();r[5]=s?s->id:0;r[6]=caller==0x4fd4b0?1:caller==0x4fd559?2:0;
 r[7]=update_point_getter((void*)object,caller,r+8,64);if(update_point_records>256||!write(update_point_file,r,96)||r[7]!=1)ExitProcess(99);
 if(r[8]!=64||r[9]!=1||!r[10]||r[13]>255||r[14]!=r[3]||!r[6]||r[15]!=r[6]||(!r[18]&&!r[19])||r[20]!=8||r[21]>1||r[22]||r[23])ExitProcess(99);
 if(phase==1)copy(update_point_first,r+8,64);else{for(u32 i=0;i<16;++i)if(update_point_first[i]!=r[8+i])ExitProcess(99);update_point_active=0;++update_point_completed;}
}
static void drawing_update_points_close(void){if(!update_point_file)return;u32 r[24]={0};r[0]=++update_point_records;r[1]=4;r[2]=1;r[3]=update_point_active!=0;r[4]=update_point_completed;r[5]=sequence;r[6]=failures;if(!write(update_point_file,r,96)||!CloseHandle(update_point_file))ExitProcess(99);update_point_file=0;update_point_enabled=0;}
