#include "../../protocols/include/mnm/drawing_cursor_gate_v1.h"
/* Added diagnostic synchronization only. No recovered/source-alias claim. */
static u32 drawing_cursor_gate_enabled,drawing_cursor_gate_records;
static HANDLE drawing_cursor_gate_file;
typedef u32(WIN* DrawingCursorGateBegin)(u32,void*,u32);
typedef u32(WIN* DrawingCursorGateEnd)(u32,u32,void*,u32);
static DrawingCursorGateBegin drawing_cursor_gate_begin;
static DrawingCursorGateEnd drawing_cursor_gate_end;
static void drawing_cursor_gate_emit(struct Context* c,u32 phase){
 u32 row[48]={0};row[1]=phase;row[2]=c->r[29];row[3]=GetCurrentThreadId();row[4]=c->cursor_gate_token;row[5]=c->source_lease;row[6]=c->cursor_gate_reason;copy(row+16,c->cursor_gate_point,128);
 EnterCriticalSection(journal_lock);row[0]=++drawing_cursor_gate_records;
 if(drawing_cursor_gate_records>8192||!write(drawing_cursor_gate_file,row,192))ExitProcess(99);
 LeaveCriticalSection(journal_lock);
}
static int drawing_cursor_gate_open(void){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW_CURSOR_SOURCE_GATE",flag,2);if(!n)return 1;
 if(n!=1||flag[0]!='1'||!source_lease_enabled||!source_worker_lease_enabled||!source_held_enabled)return 0;
 char name[600];path(name,"api-cursor-gate.bin");drawing_cursor_gate_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(drawing_cursor_gate_file==(HANDLE)-1)return 0;
 u32 h[16]={0};copy(h,"MNMDGA01",8);h[2]=1;h[3]=192;h[4]=0x40209ca7;h[5]=8192;if(!write(drawing_cursor_gate_file,h,64))return 0;drawing_cursor_gate_enabled=1;return 1;
}
static void drawing_cursor_gate_acquire(struct Context* c,u32 method,const u32* args,u32 type){
 if(!drawing_cursor_gate_enabled||type!=1||method!=7||args[4]!=16)return;
 c->cursor_gate_requested=1;
 if(!drawing_cursor_gate_begin||!drawing_cursor_gate_end){void* module=GetModuleHandleA("MnmCursorGate.dll");
  drawing_cursor_gate_begin=module?(DrawingCursorGateBegin)GetProcAddress(module,"CursorSourceGateBeginV1"):0;
  drawing_cursor_gate_end=module?(DrawingCursorGateEnd)GetProcAddress(module,"CursorSourceGateEndV1"):0;
  if(!drawing_cursor_gate_begin||!drawing_cursor_gate_end)ExitProcess(99);
 }
 c->cursor_gate_token=drawing_cursor_gate_begin(GetCurrentThreadId(),c->cursor_gate_point,128);
 u32* point=c->cursor_gate_point;
 c->cursor_gate_reason=point[0]==1&&point[1]==128&&point[2]==0x40209ca7?point[4]:16;
 if(c->cursor_gate_token&&(point[5]!=c->cursor_gate_token||point[6]!=GetCurrentThreadId()||point[13]!=3||c->cursor_gate_reason))ExitProcess(99);
}
static void drawing_cursor_gate_release(struct Context* c){
 if(!c->cursor_gate_requested)return;
 if(c->cursor_gate_token){memset(c->cursor_gate_point,0,128);
  u32 result=drawing_cursor_gate_end(GetCurrentThreadId(),c->cursor_gate_token,c->cursor_gate_point,128);
  if(!result||c->cursor_gate_point[0]!=1||c->cursor_gate_point[1]!=128||c->cursor_gate_point[5]!=c->cursor_gate_token||c->cursor_gate_point[13]){drawing_cursor_gate_emit(c,3);ExitProcess(99);}
 }
 drawing_cursor_gate_emit(c,2);c->cursor_gate_token=0;
}
static void drawing_cursor_gate_close(u32 reason){
 if(!drawing_cursor_gate_file)return;u32 row[48]={0};row[0]=++drawing_cursor_gate_records;row[1]=4;row[2]=reason;
 if(drawing_cursor_gate_records>8192||!write(drawing_cursor_gate_file,row,192)||!CloseHandle(drawing_cursor_gate_file))ExitProcess(99);drawing_cursor_gate_file=0;
}
