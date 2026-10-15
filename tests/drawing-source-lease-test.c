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
 require(producer_trace_configure(),150);u32 wrapper[8]={0},metadata[16];u16* pixels=VirtualAlloc(0,960000,0x3000,4);require(pixels!=0,151);wrapper[1]=0x1234;surfaces[0].object=(u32)wrapper;surfaces[0].id=1;surfaces[0].pixels=(u32)pixels;surfaces[0].width=800;surfaces[0].height=600;surfaces[0].stride=800;
 *producer_trace_depth(GetCurrentThreadId())=1;require(SceneDrawingSourceLeaseV1((void*)0x1234,0,metadata,64)==3,159);*producer_trace_depth(GetCurrentThreadId())=0;
 u32 first[7],last[7];require(VirtualQuery(pixels,first,28)==28,180);require(VirtualQuery((u8*)pixels+962559,last,28)==28,181);HANDLE diagnostic=CreateFileA("lease-region.bin",0x40000000,0,0,1,0x80,0);write(diagnostic,first,28);write(diagnostic,last,28);CloseHandle(diagnostic);if(first[1]!=last[1])ExitProcess(182);if(first[4]!=0x1000||first[5]!=4)ExitProcess(183);
 SetLastError(0xbeef);u32 result=SceneDrawingSourceLeaseV1((void*)0x1234,0,metadata,64);if(result!=0x80000001u)ExitProcess(170+result);require(GetLastError()==0xbeef,152);u32 memory[7];require(VirtualQuery(pixels,memory,28)==28&&memory[5]==2,153);
 char mode[16];if(GetEnvironmentVariableA("MNM_LEASE_TEST",mode,16)&&mode[0]=='f'){pixels[0]=1;ExitProcess(154);}
 require(SceneDrawingSourceLeaseV1((void*)0x1234,2,metadata,64)==2,155);require(SceneDrawingSourceLeaseV1((void*)0x9999,1,metadata,64)==3,160);require(SceneDrawingSourceLeaseV1((void*)0x1234,1,metadata,64)==0,156);require(VirtualQuery(pixels,memory,28)==28&&memory[5]==4,157);pixels[0]=0x1234;require(pixels[0]==0x1234,158);ExitProcess(0);
}
