#include "../runtime/shadow/win32_min.h"
API u32 WIN GetCurrentThreadId(void);
static u32 depth,enabled=1,stopped,sequence=7,queue,failures;
#include "../runtime/scene/producer_trace_guard.h"
#define MAX_SURFACES 2
#define MAX_FRAMES 128
API void* WIN AddVectoredExceptionHandler(u32,void*);
static char directory[512];
static void path(char* out,const char* name,u32 number,const char* suffix){u32 i=0;while(directory[i]){out[i]=directory[i];++i;}out[i++]='\\';while(*name)out[i++]=*name++;for(u32 d=1000;d;d/=10)out[i++]=(char)('0'+number/d%10);while(*suffix)out[i++]=*suffix++;out[i]=0;}
static int write(HANDLE file,const void* p,u32 n){u32 done;return WriteFile(file,p,n,&done,0)&&done==n;}
struct BatchExceptionRecord{u32 code,flags,record,address,count,information[15];};
struct BatchExceptionPointers{struct BatchExceptionRecord* record;void* context;};
static u32 get(const void* p){return *(const u32*)p;}
static struct Surface{u32 object,id,pixels,width,height,stride,dirty,sampled;} surfaces[2];
struct Thread{u32 unused;};
static struct{u32 sp,tag,owner,ecx,primitive,caller;u8* record;} frames[128];
static void copy(void* out,const void* in,u32 n){u8* a=out;const u8* b=in;while(n--)*a++=*b++;}
static int readable(u32 p,u32 n){u32 m[7];return p&&p+n>=p&&VirtualQuery((void*)p,m,28)==28&&m[4]==0x1000&&!(m[5]&0x101)&&p+n<=m[0]+m[3];}
static int bypass_writable(u32 p,u32 n){u32 m[7];return readable(p,n)&&VirtualQuery((void*)p,m,28)==28&&(m[5]&0xcc);}
#include "../runtime/scene/drawing_source_point.h"
static void require(int value,u32 code){if(!value)ExitProcess(code);}
void start(void){
 require(GetEnvironmentVariableA("MNM_FIXTURE_DIRECTORY",directory,512)>0,130);require(producer_trace_configure(),131);require(!*producer_trace_depth(GetCurrentThreadId()),142);
 u32 wrapper[8]={0},other[8]={0},before[16],after[16];u8* pixels=VirtualAlloc(0,960000,0x3000,4);u8* output=VirtualAlloc(0,960000,0x3000,4);require(pixels&&output,132);output[0]=0xaa;
 wrapper[1]=0x1234;surfaces[0]=(struct Surface){(u32)wrapper,13,(u32)pixels,800,600,800,1,1};surfaces[1]=(struct Surface){(u32)other,2,0,1,1,1,0,0};
 u32 record[24]={96,1,4,2,13};frames[0].sp=1;frames[0].tag=7;frames[0].owner=GetCurrentThreadId();frames[0].ecx=(u32)wrapper;frames[0].caller=0x46ba01;frames[0].record=(u8*)record;
 frames[1].sp=2;frames[1].tag=5;frames[1].owner=999;frames[1].ecx=(u32)other;frames[1].caller=0x58bc10;
 /* Unleased pending refusals are never recorded. */
 require(SceneDrawingSourceBytesV1((void*)0x1234,before,after,output,960000)==2&&!source_pending_recorded_lease,133);frames[1].sp=0;
 require(SceneDrawingSourceLeaseV1((void*)0x1234,0,before,64)==0x80000001u,134);frames[1].sp=2;
 SetLastError(0xcafe);require(SceneDrawingSourceBytesV1((void*)0x1234,before,after,output,960000)==2&&GetLastError()==0xcafe&&output[0]==0xaa&&source_pending_recorded_lease==1,135);
 require(SceneDrawingSourceBytesV1((void*)0x1234,before,after,output,960000)==2&&source_pending_recorded_lease==1,136);frames[1].sp=0;require(!SceneDrawingSourceLeaseV1((void*)0x1234,1,before,64),137);
 require(SceneDrawingSourceLeaseV1((void*)0x1234,0,before,64)==0x80000002u,138);frames[1]=frames[0];frames[1].sp=2;
 require(SceneDrawingSourceBytesV1((void*)0x1234,before,after,output,960000)==2&&source_pending_recorded_lease==2,139);frames[1].sp=0;require(!SceneDrawingSourceLeaseV1((void*)0x1234,2,before,64),140);
 require(!source_lease_active&&output[0]==0xaa,141);ExitProcess(0);
}
