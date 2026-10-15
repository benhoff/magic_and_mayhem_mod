#include "../runtime/shadow/win32_min.h"
API u32 WIN GetCurrentThreadId(void);
static u32 depth,enabled=1,stopped,sequence=7,queue,failures;
#include "../runtime/scene/producer_trace_guard.h"
#define MAX_SURFACES 2
#define MAX_FRAMES 128
API void* WIN AddVectoredExceptionHandler(u32,void*);
static void path(char* out,const char* name,u32 number,const char* suffix){(void)number;u32 n=0;while(*name)out[n++]=*name++;while(*suffix)out[n++]=*suffix++;out[n]=0;}
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
 require(producer_trace_configure(),130);u32 wrapper[8]={0},before[16],after[16];u16 pixels[12]={1,2,3,99,4,5,6,99,7,8,9,99};u8 output[18];wrapper[1]=0x1234;surfaces[0].object=(u32)wrapper;surfaces[0].id=1;surfaces[0].pixels=(u32)pixels;surfaces[0].width=3;surfaces[0].height=3;surfaces[0].stride=4;
 SetLastError(0xbeef);require(SceneDrawingSourceBytesV1((void*)0x1234,before,after,output,18)==0&&GetLastError()==0xbeef,131);for(u32 i=0;i<9;++i)require(((u16*)output)[i]==i+1,132);
 require(SceneDrawingSourceBytesV1((void*)0x1234,before,after,output,17)==3,133);require(SceneDrawingSourceBytesV1((void*)0x9999,before,after,output,18)==2,134);
 frames[0].sp=1;require(SceneDrawingSourceBytesV1((void*)0x1234,before,after,output,18)==2,135);frames[0].tag=7;frames[0].owner=GetCurrentThreadId();frames[0].ecx=surfaces[0].object;require(SceneDrawingSourceBytesV1((void*)0x1234,before,after,output,18)==0,139);frames[0].sp=0;
 *producer_trace_depth(GetCurrentThreadId())=1;require(SceneDrawingSourceBytesV1((void*)0x1234,before,after,output,18)==2,136);*producer_trace_depth(GetCurrentThreadId())=0;
 require(SceneDrawingSourceBytesV1((void*)0x1234,0,after,output,18)==1,137);stopped=1;require(SceneDrawingSourceBytesV1((void*)0x1234,before,after,output,18)==2,138);ExitProcess(0);
}
