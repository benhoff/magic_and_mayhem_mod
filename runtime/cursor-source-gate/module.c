/* Separate opt-in diagnostic module; isolated fixtures, no live integration. */
#include "../shadow/win32_min.h"
API void* WIN GetProcAddress(void*,const char*);
API u32 WIN GetCurrentThreadId(void);
API u32 WIN GetTickCount(void);
API u32 WIN WaitForSingleObject(HANDLE,u32);
API u32 WIN ReleaseMutex(HANDLE);
API u32 WIN DuplicateHandle(HANDLE,HANDLE,HANDLE,HANDLE*,u32,u32,u32);
API void WIN InitializeCriticalSection(void*);
API void WIN EnterCriticalSection(void*);
API void WIN LeaveCriticalSection(void*);
#include "../cursor-mutex/cursor_kernel_adapter.h"
#include "../../protocols/include/mnm/drawing_cursor_gate_v1.h"
#include "pins.h"
#ifndef MNM_CURSOR_GATE_BASE
#define MNM_CURSOR_GATE_BASE 0x400000u
#define MNM_CURSOR_GATE_CREATE_INNER 0x4a3700u
#define MNM_CURSOR_GATE_CREATE_OUTER 0x4e8f6fu
#define MNM_CURSOR_GATE_CLOSE_INNER 0x4a37dau
#define MNM_CURSOR_GATE_CLOSE_OUTER 0x4e9e00u
#define MNM_CURSOR_GATE_GLOBAL_INNER 0x642c58u
#define MNM_CURSOR_GATE_GLOBAL_OUTER 0x6e1ec0u
#define MNM_CURSOR_GATE_IAT 0x5c50a0u
#endif
void* memset(void* output,int value,unsigned bytes){unsigned char* p=output;while(bytes--)*p++=(unsigned char)value;return output;}
static MnmCursorKernelAdapter cursor_gate_adapter;
static unsigned cursor_gate_shutdown,cursor_gate_wrappers,cursor_gate_calls;
static unsigned cursor_gate_lock[6],cursor_gate_enabled,cursor_gate_next_identity,cursor_gate_publishing[2];
static int cursor_gate_readable(const void* pointer,unsigned bytes){
 unsigned at=(unsigned)pointer,end=at+bytes;if(!at||end<at)return 0;
 while(at<end){unsigned m[7];if(VirtualQuery((void*)at,m,28)!=28||m[4]!=0x1000||(m[5]&0x101))return 0;unsigned next=m[0]+m[3];if(next<=at)return 0;at=next;}return 1;
}
static int cursor_gate_equal(const void* pointer,const unsigned char* expected,unsigned bytes){
 if(!cursor_gate_readable(pointer,bytes))return 0;const unsigned char* p=pointer;while(bytes--)if(*p++!=*expected++)return 0;return 1;
}
static int cursor_gate_pins(void){
 return (unsigned)GetModuleHandleA(0)==MNM_CURSOR_GATE_BASE&&
  cursor_gate_equal((void*)MNM_CURSOR_GATE_CREATE_INNER,cursor_gate_create_inner_pin,sizeof(cursor_gate_create_inner_pin))&&
  cursor_gate_equal((void*)MNM_CURSOR_GATE_CREATE_OUTER,cursor_gate_create_outer_pin,sizeof(cursor_gate_create_outer_pin))&&
  cursor_gate_equal((void*)MNM_CURSOR_GATE_CLOSE_INNER,cursor_gate_close_inner_pin,sizeof(cursor_gate_close_inner_pin))&&
  cursor_gate_equal((void*)MNM_CURSOR_GATE_CLOSE_OUTER,cursor_gate_close_outer_pin,sizeof(cursor_gate_close_outer_pin));
}
static int cursor_gate_writable(void* pointer,unsigned bytes){
 unsigned at=(unsigned)pointer,end=at+bytes;if(!at||end<at)return 0;
 while(at<end){unsigned m[7];if(VirtualQuery((void*)at,m,28)!=28||m[4]!=0x1000||(m[5]&0x101))return 0;
  unsigned protection=m[5]&255;if(protection!=4&&protection!=8&&protection!=0x40&&protection!=0x80)return 0;
  unsigned next=m[0]+m[3];if(next<=at)return 0;at=next;
 }return 1;
}
static void cursor_gate_fx_save(void* out){__asm__ volatile("fxsave (%0)"::"r"(out):"memory");}
static void cursor_gate_fx_restore(void* in){__asm__ volatile("fxrstor (%0)"::"r"(in):"memory");}
static void cursor_gate_copy(void* output,const void* input,unsigned bytes){
 unsigned char* a=output;const unsigned char* b=input;while(bytes--)*a++=*b++;
}
#include "journal.h"
typedef unsigned(WIN* CursorWait)(HANDLE,unsigned);
typedef unsigned(WIN* CursorHandleCall)(HANDLE);
typedef HANDLE(WIN* CursorCreate)(void*,unsigned,const char*);
static CursorWait cursor_gate_original_wait;
static CursorHandleCall cursor_gate_original_release,cursor_gate_original_close;
static CursorCreate cursor_gate_original_create;
static HANDLE cursor_gate_orphans[64];static unsigned cursor_gate_orphan_count;
static struct {unsigned owner;unsigned* point;unsigned waits;} cursor_gate_raw_cells[64];
static int cursor_gate_raw_begin(unsigned* point){
 unsigned owner=GetCurrentThreadId();int ok=0;EnterCriticalSection(cursor_gate_lock);
 for(unsigned i=0;i<64;++i)if(cursor_gate_raw_cells[i].owner==owner){LeaveCriticalSection(cursor_gate_lock);return 0;}
 for(unsigned i=0;i<64;++i)if(!cursor_gate_raw_cells[i].owner){cursor_gate_raw_cells[i].owner=owner;cursor_gate_raw_cells[i].point=point;cursor_gate_raw_cells[i].waits=0;ok=1;break;}
 LeaveCriticalSection(cursor_gate_lock);return ok;
}
static void cursor_gate_raw_end(void){unsigned owner=GetCurrentThreadId();EnterCriticalSection(cursor_gate_lock);for(unsigned i=0;i<64;++i)if(cursor_gate_raw_cells[i].owner==owner){cursor_gate_raw_cells[i].owner=0;cursor_gate_raw_cells[i].point=0;}LeaveCriticalSection(cursor_gate_lock);}
static unsigned cursor_gate_wait(void* handle,unsigned timeout){
 unsigned result=cursor_gate_original_wait(handle,timeout),owner=GetCurrentThreadId();EnterCriticalSection(cursor_gate_lock);
 for(unsigned i=0;i<64;++i)if(cursor_gate_raw_cells[i].owner==owner&&cursor_gate_raw_cells[i].waits<2)cursor_gate_raw_cells[i].point[9+cursor_gate_raw_cells[i].waits++]=result;
 LeaveCriticalSection(cursor_gate_lock);return result;
}
static unsigned cursor_gate_release(void* handle){
 unsigned result=cursor_gate_original_release(handle),owner=GetCurrentThreadId();EnterCriticalSection(cursor_gate_lock);
 for(unsigned i=0;i<64;++i)if(cursor_gate_raw_cells[i].owner==owner){unsigned role=handle==cursor_gate_adapter.outer.duplicate?0:1;cursor_gate_raw_cells[i].point[11+role]=result;}
 LeaveCriticalSection(cursor_gate_lock);return result;
}
/* Private close claim/result phases own lifetime_lock; actual Close outside. */
static int cursor_gate_close_retired(unsigned role){
 EnterCriticalSection(cursor_gate_lock);
 MnmCursorLifetime* cell=role?&cursor_gate_adapter.inner:&cursor_gate_adapter.outer;
 unsigned identity=cell->identity;void* duplicate=mnm_cursor_kernel_close_begin(&cursor_gate_adapter,role);
 LeaveCriticalSection(cursor_gate_lock);if(!duplicate)return 1;
 unsigned result=cursor_gate_original_close(duplicate);
 EnterCriticalSection(cursor_gate_lock);
 int accepted=mnm_cursor_kernel_close_end(&cursor_gate_adapter,role,duplicate,identity,result);
 LeaveCriticalSection(cursor_gate_lock);cursor_gate_journal_emit(5,accepted?0:10,0,result,GetLastError(),1u<<role,0,identity,0,0);return accepted&&result;
}
static HANDLE cursor_gate_create_at(void* security,unsigned initial_owner,const char* name,unsigned caller){
 unsigned entry_error=GetLastError();unsigned char entry_fx[512]__attribute__((aligned(16)));cursor_gate_fx_save(entry_fx);
 EnterCriticalSection(cursor_gate_lock);unsigned call=++cursor_gate_calls;++cursor_gate_wrappers;LeaveCriticalSection(cursor_gate_lock);
 cursor_gate_fx_restore(entry_fx);SetLastError(entry_error);
 HANDLE result=cursor_gate_original_create(security,initial_owner,name);
 unsigned error=GetLastError();unsigned char fx[512]__attribute__((aligned(16)));cursor_gate_fx_save(fx);
 cursor_gate_journal_emit(1,0,result!=0,0,error,0,call,0,0,0);
 if(cursor_gate_enabled&&result&&!security&&!initial_owner&&!name&&(caller==(MNM_CURSOR_GATE_CREATE_INNER+6)||caller==(MNM_CURSOR_GATE_CREATE_OUTER+6))){
  unsigned role=caller==(MNM_CURSOR_GATE_CREATE_INNER+6)?1:0;HANDLE duplicate=0;
  EnterCriticalSection(cursor_gate_lock);
  MnmCursorLifetime* cell=role?&cursor_gate_adapter.inner:&cursor_gate_adapter.outer;
  unsigned allowed=!cursor_gate_adapter.fault&&!cursor_gate_publishing[role]&&!cell->duplicate&&!cell->leases&&!cell->closing;
  if(allowed)cursor_gate_publishing[role]=1;
  LeaveCriticalSection(cursor_gate_lock);
  unsigned copied=allowed?DuplicateHandle(GetCurrentProcess(),result,GetCurrentProcess(),&duplicate,0,0,2):0;
  if(copied&&duplicate){
   EnterCriticalSection(cursor_gate_lock);
   unsigned identity=cursor_gate_next_identity==~0u?0:++cursor_gate_next_identity;
   int published=mnm_cursor_kernel_publish(&cursor_gate_adapter,role,result,duplicate,identity);cursor_gate_publishing[role]=0;
   LeaveCriticalSection(cursor_gate_lock);
   cursor_gate_journal_emit(2,published?0:11,0,published,error,1u<<role,call,identity,0,0);
   if(!published&&!cursor_gate_original_close(duplicate)){
    EnterCriticalSection(cursor_gate_lock);cursor_gate_adapter.fault=3;if(cursor_gate_orphan_count<64)cursor_gate_orphans[cursor_gate_orphan_count++]=duplicate;LeaveCriticalSection(cursor_gate_lock);
   }
  }else if(allowed){EnterCriticalSection(cursor_gate_lock);cursor_gate_publishing[role]=0;LeaveCriticalSection(cursor_gate_lock);cursor_gate_journal_emit(2,12,0,0,error,1u<<role,call,0,0,0);}
 }
 EnterCriticalSection(cursor_gate_lock);--cursor_gate_wrappers;unsigned pending=cursor_gate_wrappers;LeaveCriticalSection(cursor_gate_lock);
 (void)pending;
 cursor_gate_fx_restore(fx);SetLastError(error);return result;
}
static HANDLE WIN cursor_gate_create(void* security,unsigned initial_owner,const char* name){return cursor_gate_create_at(security,initial_owner,name,(unsigned)__builtin_return_address(0));}
static unsigned WIN cursor_gate_close(HANDLE handle){
 unsigned error=GetLastError();unsigned char fx[512]__attribute__((aligned(16)));cursor_gate_fx_save(fx);
 unsigned retired=0,ids[2]={0};
 EnterCriticalSection(cursor_gate_lock);unsigned call=++cursor_gate_calls;++cursor_gate_wrappers;LeaveCriticalSection(cursor_gate_lock);
 if(cursor_gate_enabled){
  EnterCriticalSection(cursor_gate_lock);
  for(unsigned role=0;role<2;++role){MnmCursorLifetime* cell=role?&cursor_gate_adapter.inner:&cursor_gate_adapter.outer;
   if(cell->original==handle&&cell->identity&&mnm_cursor_kernel_retire(&cursor_gate_adapter,role,handle,cell->identity)){retired|=1u<<role;ids[role]=cell->identity;}
  }
  LeaveCriticalSection(cursor_gate_lock);
 }
 for(unsigned role=0;role<2;++role)if(ids[role])cursor_gate_journal_emit(3,0,0,0,error,1u<<role,call,ids[role],0,0);
 cursor_gate_fx_restore(fx);SetLastError(error);
 unsigned result=cursor_gate_original_close(handle);error=GetLastError();cursor_gate_fx_save(fx);
 cursor_gate_journal_emit(4,0,result,0,error,retired,call,0,0,0);
 if(retired&1)cursor_gate_close_retired(0);if(retired&2)cursor_gate_close_retired(1);
 EnterCriticalSection(cursor_gate_lock);--cursor_gate_wrappers;LeaveCriticalSection(cursor_gate_lock);
 cursor_gate_fx_restore(fx);SetLastError(error);return result;
}
static void cursor_gate_point(unsigned* point,unsigned phase,unsigned reason,
 MnmCursorKernelLease* lease){
 for(unsigned i=0;i<32;++i)point[i]=0;
 point[0]=1;point[1]=128;point[2]=0x40209ca7;point[3]=phase;point[4]=reason;point[6]=GetCurrentThreadId();
 if(lease&&lease->token){point[5]=lease->token->token;point[7]=lease->token->outer_identity;point[8]=lease->token->inner_identity;point[13]=lease->gate.acquired;}
}
__declspec(dllexport) unsigned WIN CursorSourceGateBeginV1(unsigned owner,void* output,unsigned bytes){
 unsigned error=GetLastError(),result=0;unsigned char fx[512]__attribute__((aligned(16)));cursor_gate_fx_save(fx);
 if(cursor_gate_enabled&&!cursor_gate_shutdown&&owner==GetCurrentThreadId()&&bytes==128&&cursor_gate_writable(output,bytes)){
  unsigned point[32];MnmCursorKernelLease* lease=0;
  EnterCriticalSection(cursor_gate_lock);
  if(cursor_gate_readable((void*)MNM_CURSOR_GATE_GLOBAL_OUTER,4)&&cursor_gate_readable((void*)MNM_CURSOR_GATE_GLOBAL_INNER,4))
   lease=mnm_cursor_kernel_reserve(&cursor_gate_adapter,owner,*(void**)MNM_CURSOR_GATE_GLOBAL_OUTER,*(void**)MNM_CURSOR_GATE_GLOBAL_INNER);
  LeaveCriticalSection(cursor_gate_lock);
  if(lease){
   unsigned token=lease->token->token;
   cursor_gate_point(point,1,0,lease);
   int raw=cursor_gate_raw_begin(point);
   int acquired=raw?mnm_cursor_gate_acquire(&lease->gate,lease->gate.outer,lease->gate.inner,cursor_gate_wait,cursor_gate_release):0;
   if(raw)cursor_gate_raw_end();else lease->gate.failure=9;
   point[4]=lease->gate.failure;point[13]=lease->gate.acquired;
   if(acquired)result=token;
   else if(!lease->gate.acquired){
    EnterCriticalSection(cursor_gate_lock);mnm_cursor_kernel_return(&cursor_gate_adapter,owner,token);LeaveCriticalSection(cursor_gate_lock);
    cursor_gate_close_retired(1);cursor_gate_close_retired(0);
   }
  }else cursor_gate_point(point,1,7,0);
  cursor_gate_copy(output,point,128);cursor_gate_journal_emit(point[3]==1?6:7,point[4],0,result,error,0,0,0,0,point);
 }else cursor_gate_journal_emit(6,13,0,0,error,0,0,0,0,0);
 cursor_gate_fx_restore(fx);SetLastError(error);return result;
}
__declspec(dllexport) unsigned WIN CursorSourceGateEndV1(unsigned owner,unsigned token,void* output,unsigned bytes){
 unsigned error=GetLastError(),result=0;unsigned char fx[512]__attribute__((aligned(16)));cursor_gate_fx_save(fx);
 if(cursor_gate_enabled&&owner==GetCurrentThreadId()&&bytes==128&&cursor_gate_writable(output,bytes)){
  unsigned point[32];MnmCursorKernelLease* lease=0;
  EnterCriticalSection(cursor_gate_lock);MnmCursorGateToken* identity=mnm_cursor_token_find(&cursor_gate_adapter.tokens,owner,token);
  if(identity)lease=cursor_gate_adapter.leases+(identity-cursor_gate_adapter.tokens.cells);
  LeaveCriticalSection(cursor_gate_lock);
  if(lease&&lease->token==identity){
   cursor_gate_point(point,2,0,lease);
   int raw=cursor_gate_raw_begin(point);
   int released=raw?mnm_cursor_gate_release(&lease->gate,cursor_gate_release):0;
   if(raw)cursor_gate_raw_end();else lease->gate.failure=9;
   point[4]=lease->gate.failure;point[13]=lease->gate.acquired;
   if(released){
    EnterCriticalSection(cursor_gate_lock);result=mnm_cursor_kernel_return(&cursor_gate_adapter,owner,token);LeaveCriticalSection(cursor_gate_lock);
    cursor_gate_journal_emit(10,result?0:15,0,result,error,0,0,0,0,point);
    cursor_gate_close_retired(1);cursor_gate_close_retired(0);
   }
  }else cursor_gate_point(point,2,8,0);
  cursor_gate_copy(output,point,128);cursor_gate_journal_emit(point[3]==1?6:7,point[4],0,result,error,0,0,0,0,point);
 }else cursor_gate_journal_emit(7,13,0,0,error,0,0,0,0,0);
 cursor_gate_fx_restore(fx);SetLastError(error);return result;
}
__declspec(dllexport) unsigned WIN CursorSourceGateShutdownV1(unsigned owner,void* output,unsigned bytes){
 unsigned error=GetLastError(),result=0;unsigned char fx[512]__attribute__((aligned(16)));cursor_gate_fx_save(fx);
 if(cursor_gate_enabled&&owner==GetCurrentThreadId()&&bytes==128&&cursor_gate_writable(output,bytes)){
  unsigned point[32],active=0,fault=0,orphans=0;
  EnterCriticalSection(cursor_gate_lock);cursor_gate_shutdown=1;
  active=cursor_gate_wrappers+cursor_gate_publishing[0]+cursor_gate_publishing[1];
  for(unsigned i=0;i<64;++i)if(cursor_gate_adapter.tokens.cells[i].token)++active;
  fault=cursor_gate_adapter.fault;orphans=cursor_gate_orphan_count;
  if(!active&&!fault&&!orphans){
   if(cursor_gate_adapter.outer.identity)mnm_cursor_kernel_retire(&cursor_gate_adapter,0,cursor_gate_adapter.outer.original,cursor_gate_adapter.outer.identity);
   if(cursor_gate_adapter.inner.identity)mnm_cursor_kernel_retire(&cursor_gate_adapter,1,cursor_gate_adapter.inner.original,cursor_gate_adapter.inner.identity);
  }
  LeaveCriticalSection(cursor_gate_lock);
  if(!active&&!fault&&!orphans){cursor_gate_close_retired(1);cursor_gate_close_retired(0);}
  EnterCriticalSection(cursor_gate_lock);
  unsigned live=(cursor_gate_adapter.outer.duplicate!=0)+(cursor_gate_adapter.inner.duplicate!=0);
  LeaveCriticalSection(cursor_gate_lock);
  unsigned reason=!active&&!fault&&!orphans&&!live?3:2; /*3 diagnostic snapshot; quiescent normal terminal pending*/
  cursor_gate_point(point,3,reason==3?0:14,0);point[13]=active;point[14]=live;point[15]=fault;
  cursor_gate_copy(output,point,128);cursor_gate_journal_emit(8,reason,0,0,error,0,0,0,active,point);
  cursor_gate_journal_emit(9,reason,0,0,error,0,0,0,active,point);
  if(!cursor_gate_wrappers&&!cursor_gate_publishing[0]&&!cursor_gate_publishing[1])result=cursor_gate_journal_finish(reason,active,fault,orphans)&&reason==3;
 }
 cursor_gate_fx_restore(fx);SetLastError(error);return result;
}
static int cursor_gate_install(void){
 char flag[2];SetLastError(0);unsigned n=GetEnvironmentVariableA("MNM_DRAW_CURSOR_SOURCE_GATE",flag,2);
 if(!n)return GetLastError()==203;if(n!=1||flag[0]!='1'||!cursor_gate_pins())return 0;
 void* kernel=GetModuleHandleA("kernel32.dll");
 cursor_gate_original_create=(CursorCreate)GetProcAddress(kernel,"CreateMutexA");
 cursor_gate_original_close=(CursorHandleCall)GetProcAddress(kernel,"CloseHandle");
 cursor_gate_original_wait=(CursorWait)GetProcAddress(kernel,"WaitForSingleObject");
 cursor_gate_original_release=(CursorHandleCall)GetProcAddress(kernel,"ReleaseMutex");
 if(!cursor_gate_original_create||!cursor_gate_original_close||!cursor_gate_original_wait||!cursor_gate_original_release||
  !cursor_gate_readable((void*)MNM_CURSOR_GATE_IAT,0xa4)||*(void**)(MNM_CURSOR_GATE_IAT+0x18)!=(void*)cursor_gate_original_create||*(void**)MNM_CURSOR_GATE_IAT!=(void*)cursor_gate_original_close||
  *(void**)(MNM_CURSOR_GATE_IAT+0xa0)!=(void*)cursor_gate_original_wait||*(void**)(MNM_CURSOR_GATE_IAT+0x9c)!=(void*)cursor_gate_original_release)return 0;
 InitializeCriticalSection(cursor_gate_lock);
 if(!cursor_gate_journal_open())return 0;
 unsigned old,restored;if(!VirtualProtect((void*)MNM_CURSOR_GATE_IAT,0x1c,4,&old))return 0;
 *(void**)(MNM_CURSOR_GATE_IAT+0x18)=(void*)&cursor_gate_create;*(void**)MNM_CURSOR_GATE_IAT=(void*)&cursor_gate_close;
 if(!VirtualProtect((void*)MNM_CURSOR_GATE_IAT,0x1c,old,&restored)){
  *(void**)(MNM_CURSOR_GATE_IAT+0x18)=(void*)cursor_gate_original_create;*(void**)MNM_CURSOR_GATE_IAT=(void*)cursor_gate_original_close;
  VirtualProtect((void*)MNM_CURSOR_GATE_IAT,0x1c,old,&restored);return 0;
 }
 cursor_gate_enabled=1;return 1;
}
__declspec(dllexport) void CursorSourceGateAnchor(void){}
int WIN DllMain(void* module,unsigned reason,void* reserved){
 (void)module;(void)reserved;unsigned error=GetLastError();unsigned char fx[512]__attribute__((aligned(16)));cursor_gate_fx_save(fx);int result=1;
 if(reason==1)result=cursor_gate_install();
 /* No blocking worker/lease cleanup inside loader-lock detach. Process-owned
  * private handles remain resident; bounded explicit shutdown still pending. */
 cursor_gate_fx_restore(fx);SetLastError(error);return result;
}
