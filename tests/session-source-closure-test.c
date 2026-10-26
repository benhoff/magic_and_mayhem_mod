#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef uint32_t u32;typedef void* HANDLE;
#define API
#define WIN
#define MAX_FRAMES 128
static u32 session_producers=1,producer_trace_mode=1,whole_update_capture,stopped;
static u32 failures,raster_world_open,queue=3,limit=120,sequence=12,bytes=64,oracle,journal_failed;
static HANDLE file=(HANDLE)1;
static struct Frame{u32 sp,tag,owner,primitive,caller,ecx;unsigned char* record;} frames[128];
static unsigned char done[64],blocked[4160];static u32 done_bytes,blocked_bytes,close_calls,probe,close_flag,flush_ok=1;
static u32 GetLastError(void){return 203;}static void SetLastError(u32 n){(void)n;}
static u32 GetEnvironmentVariableA(const char* n,char* p,u32 cap){(void)n;(void)cap;*p='1';return 1;}
static u32 GetCurrentThreadId(void){return 23;}static u32 GetTickCount(void){return 501;}
static void copy(void* d,const void* s,u32 n){memcpy(d,s,n);}
static int readable(u32 p,u32 n){return p&&n==96;}
static void path(char* p,const char* prefix,u32 n,const char* suffix){sprintf(p,"%s%04u%s",prefix,n,suffix);}
static u32 GetFileAttributesA(const char* p){return (strstr(p,".probe")?probe:close_flag)?0:0xffffffffu;}
static HANDLE CreateFileA(const char* p,u32 a,u32 b,void* c,u32 d,u32 e,void* f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return strstr(p,".blocked")?(HANDLE)3:(HANDLE)2;}
static int write(HANDLE h,const void* p,u32 n){if(h==(HANDLE)2){memcpy(done+done_bytes,p,n);done_bytes+=n;}else{memcpy(blocked+blocked_bytes,p,n);blocked_bytes+=n;}return 1;}
static int CloseHandle(HANDLE h){(void)h;return 1;}
static void ExitProcess(u32 n){exit(n);}static int journal_flush(void){return flush_ok;}
static void canvas_producers_close(void){++close_calls;stopped=1;file=0;}
#include "../runtime/scene/drawing_session_producers.h"
static void need(int v,const char* n){if(!v){fprintf(stderr,"refused: %s\n",n);exit(1);}}
static void reset(void){memset(frames,0,sizeof frames);done_bytes=blocked_bytes=close_calls=0;failures=raster_world_open=stopped=journal_failed=0;queue=3;limit=120;file=(HANDLE)1;probe=close_flag=0;flush_ok=1;session_close_dumped=0;session_close_diagnostics=0;}
int main(void){
 reset();need(session_producers_configure(),"configure");session_close_diagnostics=0;
 canvas_producers_session_finish();need(done_bytes==64&&!memcmp(done,"MNMPCS01",8),"quiescent marker");
 for(unsigned i=0;i<6;++i){reset();if(i==0)failures=1;if(i==1)raster_world_open=1;if(i==2)queue=121;if(i==3)frames[0].sp=1;if(i==4)frames[1].sp=0xffffffffu;if(i==5)flush_ok=0;canvas_producers_session_finish();need(!done_bytes,"unsafe marker refused");}
 reset();probe=close_flag=1;frames[3]=(struct Frame){.sp=0x200,.tag=7,.owner=23,.caller=0x46ba01,.ecx=0x1234};
 struct Frame before=frames[3];session_producers_poll();need(!done_bytes&&!blocked_bytes&&!close_calls,"diagnostic disabled");
 session_close_diagnostics=1;session_producers_poll();need(blocked_bytes==96&&!memcmp(blocked,"MNMPSC01",8),"requested bounded snapshot");need(!memcmp(&before,&frames[3],sizeof before)&&!done_bytes&&!close_calls,"snapshot never retires pending frame");
 session_producers_poll();need(blocked_bytes==96&&!done_bytes,"snapshot only once");
 memset(frames,0,sizeof frames);session_producers_poll();need(done_bytes==64&&close_calls==1,"actual return permits independent closure");
 reset();probe=1;session_close_diagnostics=1;session_producers_diagnose();need(blocked_bytes==64&&!done_bytes,"empty metadata grants no marker");
 puts("producer session closure: six refusal boundaries, opt-in bounded snapshot, frame preservation and actual return passed");return 0;
}
