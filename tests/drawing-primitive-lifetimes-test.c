/* Actual PE32 two-thread fixture for the additive recorder, with owned inputs. */
#include "../runtime/shadow/win32_min.h"
API u32 WIN GetCurrentThreadId(void);
API HANDLE WIN CreateThread(void*,u32,u32(WIN*)(void*),void*,u32,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
API void WIN Sleep(u32);
#define MAX_FRAMES 3
static u32 depth,whole_update_capture=1,sequence,queue;
static char directory[220];
static struct Frame {u32 sp,tag,caller,ecx,edx,args[6],primitive,owner,lifetime_id,lifetime_parent;u8* record;} frames[MAX_FRAMES];
static struct Surface {u32 object,id,pixels,width,height,stride,dirty,sampled;} surface;
static struct Surface* object(u32 address){return address==surface.object&&address?&surface:0;}
static void copy(void* out,const void* in,u32 n){u8* d=out;const u8* s=in;while(n--)*d++=*s++;}
static int write(HANDLE file,const void* data,u32 bytes){u32 done=0;return WriteFile(file,data,bytes,&done,0)&&done==bytes;}
static HANDLE producer_heap(void){return GetProcessHeap();}
static void path(char* out,const char* name,u32 number,const char* suffix){(void)number;u32 i=0;while(directory[i]){out[i]=directory[i];++i;}out[i++]='\\';while(*name)out[i++]=*name++;copy(out+i,"0000",4);i+=4;while(*suffix)out[i++]=*suffix++;out[i]=0;}
#include "../runtime/scene/producer_trace_guard.h"
#include "../runtime/scene/drawing_primitive_lifetimes.h"
static void require(int okay,u32 code){if(!okay)ExitProcess(code);}
static u32 ready,release_child;
static u32 WIN child(void* unused){
 (void)unused;
 {PRODUCER_TRACE_SCOPE;struct Frame f={0};f.tag=32;f.caller=0x401234;f.owner=GetCurrentThreadId();primitive_lifetime_enter(&f);primitive_lifetime_record(&f,2,0,0,0);f.sp=1;frames[0]=f;}
 __atomic_store_n(&ready,1,__ATOMIC_RELEASE);
 while(!__atomic_load_n(&release_child,__ATOMIC_ACQUIRE))Sleep(1);
 {PRODUCER_TRACE_SCOPE;struct Frame f=frames[0];frames[0].sp=0;primitive_lifetime_record(&f,3,7,0,0);primitive_lifetime_retire(&f,7);}
 return 0;
}
void start(void){
 char mode[16]={0};GetEnvironmentVariableA("MNM_FIXTURE_MODE",mode,sizeof mode);
 require(producer_trace_configure()&&GetEnvironmentVariableA("MNM_SCENE_DIR",directory,sizeof directory),41);
 source_windows_file=(HANDLE)1;if(mode[0]=='p')source_windows_file=0;
 if(mode[0]=='i'||mode[0]=='p'){require(!primitive_lifetimes_open(),42);ExitProcess(0);}
 require(primitive_lifetimes_open()&&primitive_lifetimes_file,43);
 if(mode[0]=='b'){primitive_lifetime_rows=MNM_PRIMITIVE_LIFETIME_MAX_ROWS-1;primitive_lifetime_record(0,5,0,10,1);ExitProcess(44);}
 if(mode[0]=='w'){CloseHandle(primitive_lifetimes_file);primitive_lifetime_record(0,5,0,10,1);primitive_lifetimes_flush();ExitProcess(45);}
 u32 owner=GetCurrentThreadId();
 {PRODUCER_TRACE_SCOPE;struct Frame outer={0};outer.owner=owner;outer.tag=100;outer.caller=0x401234;primitive_lifetime_enter(&outer);primitive_lifetime_record(&outer,2,0,0,0);outer.sp=1;frames[0]=outer;
  struct Frame inner={0};inner.owner=owner;inner.tag=10;inner.caller=0x401234;primitive_lifetime_enter(&inner);primitive_lifetime_record(&inner,2,0,0,0);inner.sp=2;frames[1]=inner;
  SetLastError(0xbeef);primitive_lifetimes_boundary(10,1,2);require(GetLastError()==0xbeef,46);
  frames[1].sp=0;primitive_lifetime_record(&inner,3,7,0,0);primitive_lifetime_retire(&inner,7);
  frames[0].sp=0;primitive_lifetime_record(&outer,3,9,0,0);primitive_lifetime_retire(&outer,9);
 }
 HANDLE thread=CreateThread(0,0,child,0,0,0);require(thread!=0,47);
 for(u32 i=0;i<5000&&!__atomic_load_n(&ready,__ATOMIC_ACQUIRE);++i)Sleep(1);
 require(__atomic_load_n(&ready,__ATOMIC_ACQUIRE),48);
 {PRODUCER_TRACE_SCOPE;primitive_lifetimes_boundary(20,2,1);}
 __atomic_store_n(&release_child,1,__ATOMIC_RELEASE);require(WaitForSingleObject(thread,5000)==0&&CloseHandle(thread),49);
 {PRODUCER_TRACE_SCOPE;SetLastError(0xbeef);primitive_lifetimes_close();require(GetLastError()==0xbeef&&!primitive_lifetime_active&&!primitive_lifetimes_file,50);}
 ExitProcess(0);
}
