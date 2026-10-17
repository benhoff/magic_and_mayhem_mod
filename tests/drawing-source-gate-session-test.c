#include "../runtime/shadow/win32_min.h"
API unsigned WIN SetEnvironmentVariableA(const char*,const char*);
#define MNM_DRAWING_SELFTEST 1
#define DllMain unused_drawing_main
#include "../runtime/drawing/session.c"
#undef DllMain
void* memcpy(void* out,const void* in,unsigned bytes){copy(out,in,bytes);return out;}
static unsigned starts,ends,refuse;
static unsigned WIN gate_start(unsigned owner,void* output,unsigned bytes){
 unsigned* p=output;if(bytes!=128||owner!=GetCurrentThreadId())ExitProcess(201);
 p[0]=1;p[1]=128;p[2]=0x40209ca7;p[3]=1;p[4]=refuse?3:0;p[5]=refuse?8:7;p[6]=owner;p[7]=1;p[8]=2;p[13]=refuse?0:3;++starts;return refuse?0:7;
}
static unsigned WIN gate_stop(unsigned owner,unsigned token,void* output,unsigned bytes){
 unsigned* p=output;if(bytes!=128||owner!=GetCurrentThreadId()||token!=7)ExitProcess(202);
 p[0]=1;p[1]=128;p[2]=0x40209ca7;p[3]=2;p[5]=7;p[6]=owner;p[7]=1;p[8]=2;p[11]=p[12]=1;++ends;return 1;
}
static void check(int ok,unsigned code){if(!ok)ExitProcess(code);}
void start(void){
 check(SetEnvironmentVariableA("MNM_DRAW_CURSOR_SOURCE_GATE",0)&&initialize(),203);
 check(SetEnvironmentVariableA("MNM_DRAW_CURSOR_SOURCE_GATE","1"),209);source_lease_enabled=source_worker_lease_enabled=source_held_enabled=1;
 check(drawing_cursor_gate_open(),204);source_lease_enabled=source_worker_lease_enabled=source_held_enabled=0;drawing_cursor_gate_begin=gate_start;drawing_cursor_gate_end=gate_stop;
 unsigned args[6]={0,0,0,0,16,0};SetLastError(0xabcdef01);
 struct Context c=begin(0,7,args,5,0x4a1000,1);
 check(c.observed&&c.cursor_gate_token==7&&starts==1&&GetLastError()==0xabcdef01,205);
 // Source/worker callbacks remain disabled in this bounded full begin/end test.
 source_lease_enabled=0;readbacks_enabled=0;SetLastError(0xdef01234);
 check(end(&c,0,0x80004005)==0x80004005&&GetLastError()==0xdef01234&&ends==1&&!c.cursor_gate_token,206);
 refuse=1;SetLastError(0xabcdef01);c=begin(0,7,args,5,0x4a1000,1);
 check(c.observed&&!c.cursor_gate_token&&c.cursor_gate_reason==3&&starts==2,207);
 check(end(&c,0,0x80004005)==0x80004005&&ends==1,208);
 source_worker_lease_enabled=source_held_enabled=0;finish(1);ExitProcess(0);
}
