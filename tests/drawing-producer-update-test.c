#include "../runtime/shadow/win32_min.h"
API u32 WIN GetCurrentThreadId(void);
static u32 producer_trace_mode=1,limit=1,queue,sequence,failures;
static char directory[220];
struct Surface {u32 id;};static struct Surface surface={7};static struct Surface* bound(void){return &surface;}
static void copy(void* out,const void* in,u32 n){u8* d=out;const u8* s=in;while(n--)*d++=*s++;}
static int write(HANDLE f,const void* p,u32 n){u32 done;return WriteFile(f,p,n,&done,0)&&done==n;}
static void path(char* out,const char* name,u32 number,const char* suffix){(void)number;u32 i=0;while(directory[i]){out[i]=directory[i];++i;}out[i++]='\\';while(*name)out[i++]=*name++;copy(out+i,"0000",4);i+=4;while(*suffix)out[i++]=*suffix++;out[i]=0;}
#include "../runtime/scene/drawing_update_points.h"
static u32 calls;static char mode;
static u32 WIN getter(void* object,u32 caller,u32* out,u32 size){
 if(object!=(void*)1||caller!=0x4fd4b0||size!=64)ExitProcess(181);++calls;u32 p[16]={64,1,10,20,30,0,0,1,40,0,1000,0,8,1,0,0};p[6]=GetCurrentThreadId();if(mode=='m'&&calls==2)p[4]=31;copy(out,p,64);return mode=='r'?0:1;
}
void start(void){
 if(!GetEnvironmentVariableA("MNM_SCENE_DIR",directory,sizeof(directory))||!drawing_update_points_open())ExitProcess(180);char flag[2];GetEnvironmentVariableA("MNM_FIXTURE_MODE",flag,2);mode=flag[0];update_point_getter=&getter;queue=1;sequence=10;drawing_update_point(1,1,0x4fd4b0);
 if(mode=='d')drawing_update_point(1,1,0x4fd4b0);if(mode=='q')queue=2;sequence=20;drawing_update_point(2,1,0x4fd4b0);if(calls!=2||update_point_active||update_point_completed!=1)ExitProcess(182);drawing_update_points_close();ExitProcess(0);
}
