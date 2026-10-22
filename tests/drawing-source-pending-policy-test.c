#include "../runtime/shadow/win32_min.h"
API u32 WIN GetCurrentThreadId(void);
API HANDLE WIN CreateThread(void*,u32,u32(WIN*)(void*),void*,u32,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
static u32 depth,enabled=1,stopped,sequence=7,queue,failures;
#include "../runtime/scene/producer_trace_guard.h"
#define MAX_SURFACES 3
#define MAX_FRAMES 128
API void* WIN AddVectoredExceptionHandler(u32,void*);
static char directory[512];
static void path(char* out,const char* name,u32 number,const char* suffix){u32 i=0;while(directory[i]){out[i]=directory[i];++i;}out[i++]='\\';while(*name)out[i++]=*name++;for(u32 d=1000;d;d/=10)out[i++]=(char)('0'+number/d%10);while(*suffix)out[i++]=*suffix++;out[i]=0;}
static int write(HANDLE file,const void* p,u32 n){u32 done;return WriteFile(file,p,n,&done,0)&&done==n;}
struct BatchExceptionRecord{u32 code,flags,record,address,count,information[15];};
struct BatchExceptionPointers{struct BatchExceptionRecord* record;void* context;};
static u32 get(const void* p){return *(const u32*)p;}
static struct Surface{u32 object,id,pixels,width,height,stride,dirty,sampled;} surfaces[3];
static struct Frame{u32 sp,tag,owner,ecx,primitive,caller;u8* record;} frames[128];
static void copy(void* out,const void* in,u32 n){u8* a=out;const u8* b=in;while(n--)*a++=*b++;}
static int readable(u32 p,u32 n){u32 m[7];return p&&p+n>=p&&VirtualQuery((void*)p,m,28)==28&&m[4]==0x1000&&!(m[5]&0x101)&&p+n<=m[0]+m[3];}
static int bypass_writable(u32 p,u32 n){u32 m[7];return readable(p,n)&&VirtualQuery((void*)p,m,28)==28&&(m[5]&0xcc);}
#include "../runtime/scene/drawing_source_point.h"
static void require(int value,u32 code){if(!value)ExitProcess(code);}
static u32 WIN alias_writer(void* address){*(volatile u8*)address=0x77;return 0;}
void start(void){
 require(GetEnvironmentVariableA("MNM_FIXTURE_DIRECTORY",directory,512)>0&&producer_trace_configure(),130);require(!*producer_trace_depth(GetCurrentThreadId()),167);
 u32 wrapper[8]={0},other[8]={0},dest[8]={0},before[16],after[16];u8* pixels=VirtualAlloc(0,960000,0x3000,4);u8* output=VirtualAlloc(0,960000,0x3000,4);require(pixels&&output,131);pixels[0]=0x5a;output[0]=0xaa;
 wrapper[1]=0x1234;surfaces[0]=(struct Surface){(u32)wrapper,13,(u32)pixels,800,600,800,1,1};surfaces[1]=(struct Surface){(u32)other,2,0,1,1,1,0,0};surfaces[2]=(struct Surface){(u32)dest,3,0,1,1,1,0,0};
 u32 selected[24]={96,1,6,3,13},record[24]={96,2,6,3,2};frames[0]=(struct Frame){1,7,GetCurrentThreadId(),(u32)wrapper,0,0x46ba01,(u8*)selected};frames[1]=(struct Frame){2,32,999,(u32)other,0,0x4a4b01,(u8*)record};
 require(SceneDrawingSourceBytesV1((void*)0x1234,before,after,output,960000)==2&&output[0]==0xaa,132);
 if(!source_pending_copy_enabled()){require(SceneDrawingSourceLeaseV1((void*)0x1234,0,before,64)==3&&!source_lease_active,133);ExitProcess(0);}
 SetLastError(0xcafe);require(SceneDrawingSourceLeaseV1((void*)0x1234,0,before,64)==0x80000001u&&GetLastError()==0xcafe,134);
 require(SceneDrawingSourceBytesV1((void*)0x1234,before,after,output,960000)==0&&GetLastError()==0xcafe&&output[0]==0x5a&&source_pending_admissions==1,135);
 require(SceneDrawingSourceAdmissionV1(before,64)==1&&GetLastError()==0xcafe,168);++before[3];require(!SceneDrawingSourceAdmissionV1(before,64),169);--before[3];require(!SceneDrawingSourceAdmissionV1(before,63),170);
 char mode[16];if(GetEnvironmentVariableA("MNM_PENDING_POLICY_TEST",mode,16)&&mode[0]=='f'){
  /* Different numeric surfaces can share the protected source allocation. */
  surfaces[2].pixels=(u32)pixels;HANDLE thread=CreateThread(0,0,alias_writer,pixels,0,0);require(thread!=0,136);WaitForSingleObject(thread,5000);ExitProcess(137);
 }
 SetLastError(0xcafe);struct Frame foreign=frames[1];
 u32 saved_base=source_lease_base,saved_extent=source_lease_extent;
 source_lease_extent=0;require(!source_lease_protected(surfaces),171);
 source_lease_base=0xfffff000u;source_lease_extent=0x2000;require(!source_lease_protected(surfaces),172);
 source_lease_base=0;source_lease_extent=saved_extent;require(!source_lease_protected(surfaces),173);
 source_lease_base=saved_base;require(source_lease_protected(surfaces),174);
 for(u32 test=0;test<16;++test){
  frames[1]=foreign;record[0]=96;record[2]=6;record[3]=3;record[4]=2;output[0]=0xaa;
  switch(test){case 0:frames[1].sp=0xffffffffu;break;case 1:frames[1].primitive=1;break;case 2:frames[1].tag=1;break;case 3:frames[1].owner=GetCurrentThreadId();break;case 4:frames[1].owner=0;break;case 5:frames[1].record=0;break;case 6:record[0]=95;break;case 7:record[2]=4;break;case 8:record[3]=13;break;case 9:record[4]=13;break;case 10:record[3]=0;break;case 11:record[4]=99;break;case 12:frames[1].ecx=(u32)wrapper;break;case 13:frames[1].ecx=(u32)dest;break;case 14:frames[1]=frames[0];frames[1].sp=2;break;case 15:frames[1].tag=33;break;}
  require(SceneDrawingSourceBytesV1((void*)0x1234,before,after,output,960000)==2&&GetLastError()==0xcafe&&output[0]==0xaa,140+test);
 }
 frames[1]=foreign;record[0]=96;record[2]=6;record[3]=3;record[4]=2;
 u32 old;require(VirtualProtect(pixels,source_lease_extent,4,&old)&&old==2,160);output[0]=0xaa;require(SceneDrawingSourceBytesV1((void*)0x1234,before,after,output,960000)==2&&output[0]==0xaa,161);require(VirtualProtect(pixels,source_lease_extent,2,&old)&&old==4,162);
 frames[1].tag=39;require(SceneDrawingSourceBytesV1((void*)0x1234,before,after,output,960000)==0&&source_pending_admissions==2&&output[0]==0x5a,163);
 ++surfaces[0].object;require(SceneDrawingSourceLeaseV1((void*)0x1234,1,before,64)==3,164);--surfaces[0].object;
 require(!SceneDrawingSourceLeaseV1((void*)0x1234,1,before,64)&&GetLastError()==0xcafe&&!source_lease_active,165);require(SceneDrawingSourceBytesV1((void*)0x1234,before,after,output,960000)==2&&!SceneDrawingSourceAdmissionV1(before,64),166);ExitProcess(0);
}
