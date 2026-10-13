#include "../runtime/shadow/win32_min.h"
API u32 WIN GetCurrentThreadId(void);
API void* WIN AddVectoredExceptionHandler(u32,void*);
API HANDLE WIN CreateThread(void*,u32,u32(WIN*)(void*),void*,u32,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
API HANDLE WIN CreateFileMappingA(HANDLE,void*,u32,u32,u32,const char*);
API void* WIN MapViewOfFile(HANDLE,u32,u32,u32,u32);
static u32 depth,enabled=1,stopped,sequence=7,queue=2,failures,mode;
#include "../runtime/scene/producer_trace_guard.h"
#define MAX_SURFACES 6
#define MAX_FRAMES 128
static void path(char* out,const char* stem,u32 number,const char* suffix){
 u32 n=0;while(*stem)out[n++]=*stem++;
 for(u32 d=1000;d;d/=10)out[n++]=(char)('0'+number/d%10);
 while(*suffix)out[n++]=*suffix++;out[n]=0;
}
static int write(HANDLE file,const void* p,u32 n){u32 done;return WriteFile(file,p,n,&done,0)&&done==n;}
struct BatchExceptionRecord{u32 code,flags,record,address,count,information[15];};
struct BatchExceptionPointers{struct BatchExceptionRecord* record;void* context;};
static u32 get(const void* p){return *(const u32*)p;}
static struct Surface{u32 object,id,pixels,width,height,stride,dirty,sampled;} surfaces[MAX_SURFACES];
static struct{u32 sp,tag,owner,ecx,primitive,caller;u8* record;} frames[MAX_FRAMES];
static void copy(void* out,const void* in,u32 n){u8* a=out;const u8* b=in;while(n--)*a++=*b++;}
static int readable(u32 p,u32 n){u32 m[7];return p&&p+n>=p&&VirtualQuery((void*)p,m,28)==28&&m[4]==0x1000&&!(m[5]&0x101)&&p+n<=m[0]+m[3];}
static int bypass_writable(u32 p,u32 n){u32 m[7];return readable(p,n)&&VirtualQuery((void*)p,m,28)==28&&(m[5]&0xcc);}
static u16* pixels;
static int WIN fixture_protect(void*,u32,u32,u32*);
#define VirtualProtect fixture_protect
#include "../runtime/scene/drawing_source_point.h"
#undef VirtualProtect
static void require(int value,u32 code){if(!value)ExitProcess(code);}
/* This store intentionally bypasses every observed engine/Wine write API. */
static u32 WIN __attribute__((noinline)) direct_writer(void* target){*(volatile u16*)target=0xabcd;return 0;}
static void foreign_write(void* target){
 HANDLE thread=CreateThread(0,0,&direct_writer,target,0,0);require(thread!=0,210);
 require(WaitForSingleObject(thread,5000)==0,211);require(CloseHandle(thread),212);
}
static int WIN fixture_protect(void* p,u32 bytes,u32 protection,u32* old){
 int result=VirtualProtect(p,bytes,protection,old);
 /* Real write fault after native protection, before the lease API returns. */
 if(result&&protection==2&&mode==5)foreign_write(pixels);
 return result;
}
void start(void){
 char value[2];if(GetEnvironmentVariableA("MNM_WRITE_TEST",value,2)==1)mode=value[0]-'0';
 require(producer_trace_configure(),150);u8* base=0;u8* second=0;
 if(mode==7){
  HANDLE mapping=CreateFileMappingA((HANDLE)-1,0,4,0,1048576,0);require(mapping!=0,151);
  base=MapViewOfFile(mapping,0xf001f,0,0,1048576);second=MapViewOfFile(mapping,0xf001f,0,0,1048576);
  require(base&&second&&base!=second,152);
 }else base=VirtualAlloc(0,1048576,0x3000,4);
 require(base!=0,153);pixels=(u16*)(base+128);pixels[0]=0x1111;
 u32 wrappers[MAX_SURFACES][8],metadata[16];
 for(u32 i=0;i<MAX_SURFACES;++i){
  for(u32 j=0;j<8;++j)wrappers[i][j]=0;wrappers[i][1]=0x1234+i;surfaces[i].object=(u32)wrappers[i];surfaces[i].id=i+1;
  surfaces[i].width=surfaces[i].height=surfaces[i].stride=1;
 }
 surfaces[0].pixels=(u32)pixels;surfaces[0].width=800;surfaces[0].height=600;surfaces[0].stride=800;
 surfaces[1]=surfaces[0];surfaces[1].object=(u32)wrappers[1];surfaces[1].id=2;
 if(second)surfaces[1].pixels=(u32)second+128;
 surfaces[2].pixels=(u32)base+1728;surfaces[2].width=surfaces[2].stride=16;
 surfaces[3].pixels=(u32)base+960128;surfaces[4].pixels=(u32)base+966656;
 *producer_trace_depth(GetCurrentThreadId())=1;require(SceneDrawingSourceLeaseV1((void*)0x1234,0,metadata,64)==3,165);*producer_trace_depth(GetCurrentThreadId())=0;
 SetLastError(0xbeef);u32 result=SceneDrawingSourceLeaseV1((void*)0x1234,0,metadata,64);
 if(mode==9){require(result==4,154);pixels[0]=0x2222;require(!source_lease_active,155);ExitProcess(0);}
 require(result==0x80000001u&&GetLastError()==0xbeef,156);
 u32 memory[7];require(VirtualQuery(pixels,memory,28)==28&&memory[5]==2,157);
 if(mode==1)foreign_write(pixels);
 if(mode==2)foreign_write((void*)surfaces[1].pixels);
 if(mode==3)foreign_write((void*)surfaces[2].pixels);
 if(mode==4)foreign_write((void*)surfaces[3].pixels);
 if(mode==8)foreign_write(base+64);
 if(mode==6)foreign_write((void*)surfaces[4].pixels);
 if(mode==7){
  u16 pair[2]={pixels[0],0};foreign_write((void*)surfaces[1].pixels);pair[1]=pixels[0];require(pair[1]==0xabcd,158);
  HANDLE evidence=CreateFileA("fixture-shared-read.bin",0x40000000,0,0,1,0x80,0);
  require(evidence!=(HANDLE)-1&&write(evidence,pair,4)&&CloseHandle(evidence),164);
 }
 if(mode==0||mode==6)require(pixels[0]==0x1111,159);
 require(SceneDrawingSourceLeaseV1((void*)0x1234,2,metadata,64)==2,160);
 SetLastError(0xbeef);require(SceneDrawingSourceLeaseV1((void*)0x1234,1,metadata,64)==0&&GetLastError()==0xbeef,161);
 require(VirtualQuery(pixels,memory,28)==28&&memory[5]==4,162);pixels[0]=0x2222;
 require(pixels[0]==0x2222,163);ExitProcess(0);
}
