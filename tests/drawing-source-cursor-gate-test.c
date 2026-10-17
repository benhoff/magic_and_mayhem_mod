#include "../runtime/shadow/win32_min.h"
API u32 WIN GetCurrentThreadId(void);
API void* WIN GetProcAddress(void*,const char*);
API void WIN InitializeCriticalSection(void*);
API void WIN EnterCriticalSection(void*);
API void WIN LeaveCriticalSection(void*);
struct Context{u32 r[32],args[6],observed,interval_related,source_lease,cursor_gate_requested,cursor_gate_token,cursor_gate_reason,cursor_gate_point[32];};
static u32 journal_lock[6],source_lease_enabled=1,source_worker_lease_enabled=1,source_held_enabled=1;
static void copy(void* out,const void* in,u32 n){u8* a=out;const u8* b=in;while(n--)*a++=*b++;}
void* memset(void* out,int v,u32 n){u8* a=out;while(n--)*a++=(u8)v;return out;}
static int write(HANDLE h,const void* p,u32 n){u32 wrote=0;return WriteFile(h,p,n,&wrote,0)&&wrote==n;}
static void path(char* out,const char* suffix){(void)suffix;u32 n=GetEnvironmentVariableA("MNM_GATE_BINDING_TEST_FILE",out,600);if(!n||n>=600)ExitProcess(201);}
#include "../runtime/drawing/source_cursor_gate.h"
static unsigned begins,ends,refuse;
static void check(int ok,unsigned n){if(!ok)ExitProcess(n);}
static unsigned WIN gate_begin(unsigned owner,void* out,unsigned bytes){check(owner==GetCurrentThreadId()&&bytes==128,210);unsigned* p=out;p[0]=1;p[1]=128;p[2]=0x40209ca7;p[3]=1;p[4]=refuse?3:0;p[5]=refuse?8:7;p[6]=owner;p[7]=1;p[8]=2;p[13]=refuse?0:3;++begins;return refuse?0:7;}
static unsigned WIN gate_end(unsigned owner,unsigned token,void* out,unsigned bytes){check(owner==GetCurrentThreadId()&&token==7&&bytes==128,211);unsigned* p=out;p[0]=1;p[1]=128;p[2]=0x40209ca7;p[3]=2;p[5]=7;p[6]=owner;p[7]=1;p[8]=2;p[11]=p[12]=1;++ends;return 1;}
void start(void){
 InitializeCriticalSection(journal_lock);check(drawing_cursor_gate_open(),202);drawing_cursor_gate_begin=gate_begin;drawing_cursor_gate_end=gate_end;
 unsigned args[6]={0,0,1,0,16,0};struct Context c={0};drawing_cursor_gate_acquire(&c,7,args,1);check(c.cursor_gate_token==7&&c.cursor_gate_requested&&begins==1,203);
 c.r[29]=42;drawing_cursor_gate_emit(&c,1);c.source_lease=9;drawing_cursor_gate_release(&c);check(!c.cursor_gate_token&&ends==1,204);
 refuse=1;struct Context rejected={0};drawing_cursor_gate_acquire(&rejected,7,args,1);check(rejected.cursor_gate_requested&&!rejected.cursor_gate_token&&rejected.cursor_gate_reason==3,205);rejected.r[29]=43;drawing_cursor_gate_emit(&rejected,1);drawing_cursor_gate_release(&rejected);check(ends==1,206);
 struct Context ignored={0};drawing_cursor_gate_acquire(&ignored,5,args,1);check(!ignored.cursor_gate_requested&&begins==2,207);
 drawing_cursor_gate_close(1);check(drawing_cursor_gate_records==5,208);ExitProcess(0);
}
