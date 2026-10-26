/* Pinned NoCD lifetime envelope. Original cursor/helper bodies remain services.
 * The native queue captures wrapper values and provider pins, not wrapper
 * addresses. This is an owning native policy, not recovered queue semantics.
 * Admission order: a private duplicate of the original outer mutex, then the
 * provider gate, then bridge copy/tracker gates. Never wait for outer under the
 * provider gate. All wrapper construction/retirement shares the provider gate. */
API void* WIN GetProcAddress(void*,const char*);
API HANDLE WIN GetCurrentProcess(void);
API u32 WIN DuplicateHandle(HANDLE,HANDLE,HANDLE,HANDLE*,u32,u32,u32);
API u32 WIN WaitForSingleObject(HANDLE,u32);
API u32 WIN ReleaseMutex(HANDLE);
API u32 WIN GetTickCount(void);
#include "cursor_pins.h"
struct NdCursorRole {HANDLE original,duplicate;u32 generation;};
struct NdCursorJob {u32 active,inflight,cancelled,sequence,pins[2],source_origin,dest_origin,dest_primary;u32 source[12],dest[12];};
static struct NdCursorRole nd_cursor_roles[2];
static struct NdCursorJob nd_cursor_jobs[128];
struct NdCursorPrimary {u32 original,clone;struct NdCursorPrimary* previous;};
static struct NdCursorPrimary* nd_cursor_primary;
static u32 nd_cursor_installed,nd_cursor_closing,nd_cursor_disposed,nd_cursor_sequence,nd_cursor_handle_generation;
static u32 nd_cursor_counts[16];
static HANDLE nd_cursor_journal;
static u32 nd_cursor_rows,nd_cursor_active;
static HANDLE(WIN* nd_cursor_original_create)(void*,u32,const char*);
static u32(WIN* nd_cursor_original_close)(HANDLE);
static void* nd_cursor_original[18];
struct NdCursorFx {u8 fp[512]__attribute__((aligned(16)));u32 error;};
static void nd_cursor_save(struct NdCursorFx* s){s->error=GetLastError();__asm__ volatile("fxsave (%0)"::"r"(s->fp):"memory");}
static void nd_cursor_restore(struct NdCursorFx* s){__asm__ volatile("fxrstor (%0)"::"r"(s->fp):"memory");SetLastError(s->error);}
static void nd_cursor_copy(void* dst,const void* src,u32 bytes){u8* d=dst;const u8* s=src;while(bytes--)*d++=*s++;}
static void nd_cursor_fail(u32 reason,u32 address){
 u32 words[8]={0x434d4e4d,0x31464143,1,reason,address,*(u32*)0x642c58,*(u32*)0x6e1ec0,*(u32*)0x6f98d4},done=0;
 HANDLE file=CreateFileA("native-cursor-refusal.bin",0x40000000,1,0,1,0x80,0);
 if(file!=(HANDLE)-1){WriteFile(file,words,sizeof(words),&done,0);CloseHandle(file);}ExitProcess(123);
}
static void nd_cursor_emit(u32 phase,u32 a,u32 b,u32 c){
 if(!nd_cursor_journal)return;
 if(nd_cursor_rows>=32768 && phase!=9 && phase!=10 && phase!=11 && !((phase==5 || phase==7) && a<=1))return;
 u32 r[16]={++nd_cursor_rows,phase,GetCurrentThreadId(),a,b,c,nd_cursor_counts[0],nd_cursor_counts[1],nd_cursor_counts[2],nd_cursor_counts[3],nd_cursor_counts[4],nd_cursor_counts[5],nd_cursor_counts[6],nd_cursor_counts[7],nd_cursor_counts[8],nd_cursor_counts[9]},done=0;
 if(!WriteFile(nd_cursor_journal,r,sizeof(r),&done,0)||done!=sizeof(r))nd_cursor_fail(__LINE__,0);
}
static int nd_cursor_readable(const void* p,u32 bytes){
 u32 at=(u32)p,end=at+bytes;if(!at || end<at)return 0;
 while(at<end){u32 m[7];if(!VirtualQuery((void*)at,m,sizeof(m))||m[4]!=0x1000||(m[5]&0x101)||!(m[5]&0xee))return 0;
  u32 next=m[0]+m[3];if(next<=at)return 0;at=next;}
 return 1;
}
static void nd_cursor_finish_job(u32 slot,u32 cancelled){
 struct NdCursorJob* j=nd_cursor_jobs+slot;u32* q=(u32*)(0x6f6b50+slot*44);
 if(!j->active || j->inflight)nd_cursor_fail(__LINE__,0);
 if(NativeDrawUnpinV1(j->pins[1]) || NativeDrawUnpinV1(j->pins[0]))nd_cursor_fail(__LINE__,0);
 j->active=0;q[0]=0;if(!*(u32*)0x6f98d4)nd_cursor_fail(__LINE__,0);--*(u32*)0x6f98d4;
 ++nd_cursor_counts[cancelled?2:1];nd_cursor_emit(cancelled?4:3,slot,j->sequence,cancelled);
}
static void nd_cursor_cancel(u32 all){
 struct NsState* atlas=ns_lifetime_find(*(void**)0x642008),*saved=ns_lifetime_find(*(void**)0x641fd0);
 for(u32 i=0;i<128;++i){struct NdCursorJob* j=nd_cursor_jobs+i;
  if(!j->active)continue;
  struct NsState* a=ns_lifetime_find((void*)j->source[2]),*b=ns_lifetime_find((void*)j->dest[2]);
  if(all || (atlas && (a==atlas || b==atlas)) || (saved && (a==saved || b==saved))){
   if(j->inflight)j->cancelled=1;else nd_cursor_finish_job(i,1);
  }
 }
}
static int nd_cursor_wrapper(u32* copy,const void* p,u32* pin){
 if(!nd_cursor_readable(p,48))return 0;nd_cursor_copy(copy,p,48);
 struct NsState *a=ns_lifetime_find((void*)copy[1]),*b=ns_lifetime_find((void*)copy[2]);
 if(!a || a!=b)return 0;
 return NativeDrawPinV1((void*)copy[2],pin)==0;
}
static void __attribute__((thiscall)) nd_cursor_enqueue(void* source,u32 kind,void* dest,const i32* rectangle,u32 x,u32 y,u32 priority){
 struct NdCursorFx saved __attribute__((cleanup(nd_cursor_restore)));nd_cursor_save(&saved);
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(nd_cursor_closing){++nd_cursor_counts[4];return;}
 u32 slot=0;for(;slot<128;++slot){u32* q=(u32*)(0x6f6b50+slot*44);if((q[0]!=0)!=(nd_cursor_jobs[slot].active!=0))nd_cursor_fail(__LINE__,0);if(!q[0])break;}
 if(slot==128){++nd_cursor_counts[3];return;}
 if(kind>2 || priority>2 || (rectangle && !nd_cursor_readable(rectangle,16)))nd_cursor_fail(__LINE__,0);
 struct NdCursorJob* j=nd_cursor_jobs+slot;
 if(!nd_cursor_wrapper(j->source,source,j->pins))nd_cursor_fail(__LINE__,0);
 if(!nd_cursor_wrapper(j->dest,dest,j->pins+1)){NativeDrawUnpinV1(j->pins[0]);nd_cursor_fail(__LINE__,0);}
 if(nd_cursor_sequence==0xffffffffu)nd_cursor_fail(__LINE__,0);
 j->sequence=++nd_cursor_sequence;j->source_origin=(u32)source;j->dest_origin=(u32)dest;j->dest_primary=(u32)dest==(nd_cursor_primary?nd_cursor_primary->original:*(u32*)0x6f98c0);j->cancelled=j->inflight=0;
 u32* q=(u32*)(0x6f6b50+slot*44);q[1]=priority;q[2]=(u32)j->source;q[3]=(u32)j->dest;q[4]=kind;
 if(rectangle)for(u32 k=0;k<4;++k)q[5+k]=(u32)rectangle[k];
 q[9]=x;q[10]=y;j->active=q[0]=1;++*(u32*)0x6f98d4;++nd_cursor_counts[0];nd_cursor_emit(2,slot,j->sequence,kind);
}
static u32 nd_cursor_geometry(const struct NdCursorJob* j,const u32* q){
 struct NsState *s=ns_lifetime_find((void*)j->source[2]),*d=ns_lifetime_find((void*)j->dest[2]);
 if(!s || !d)nd_cursor_fail(__LINE__,0);
 if(ns_borrowed(s)||ns_borrowed(d))return 1;
 long long l=q[4]==1?0:(i32)q[5],t=q[4]==1?0:(i32)q[6],r=q[4]==1?s->width:(i32)q[7],b=q[4]==1?s->height:(i32)q[8];
 if(l<0 || t<0 || r<=l || b<=t || r>s->width || b>s->height ||
    q[9]>0x7fffffffu || q[10]>0x7fffffffu || q[9]+r-l>d->width || q[10]+b-t>d->height)return 2;
 return 0;
}
#include "cursor_copy_leases.h"
static void nd_cursor_drain(void){
 struct NdCursorFx saved __attribute__((cleanup(nd_cursor_restore)));nd_cursor_save(&saved);
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(nd_cursor_closing){nd_cursor_cancel(1);return;}
 const u32 cutoff=nd_cursor_sequence;
 for(u32 priority=0;priority<3;++priority)for(u32 slot=0;slot<128;++slot){
  struct NdCursorJob* j=nd_cursor_jobs+slot;u32* q=(u32*)(0x6f6b50+slot*44);
  if((q[0]!=0)!=(j->active!=0))nd_cursor_fail(__LINE__,0);
  if(!j->active || j->inflight || j->sequence>cutoff || q[1]!=priority)continue;
  u32 geometry=nd_cursor_geometry(j,q);
  if(geometry==1){++nd_cursor_counts[5];continue;}
  if(geometry==2){++nd_cursor_counts[6];nd_cursor_finish_job(slot,1);continue;}
  j->inflight=1;*(u32*)0x6f98d8=1;
  struct NdCursorPrimary primary={*(u32*)0x6f98c0,(u32)j->dest,nd_cursor_primary};
  if(j->dest_primary){nd_cursor_primary=&primary;*(u32*)0x6f98c0=primary.clone;}
  if(q[4]==1)((void(__attribute__((thiscall))*)(void*,void*,u32,u32))0x58c140)(j->source,j->dest,q[9],q[10]);
  else ((void(__attribute__((thiscall))*)(void*,void*,const u32*,u32,u32))(q[4]==0?0x58c8a0:0x58c4a0))(j->source,j->dest,q+5,q[9],q[10]);
  if(j->dest_primary){if(*(u32*)0x6f98c0==primary.clone)*(u32*)0x6f98c0=primary.original;nd_cursor_primary=primary.previous;}
  *(u32*)0x6f98d8=0;j->inflight=0;nd_cursor_finish_job(slot,0);
 }
}
/* No original outer wait occurs under the provider lock. Duplicate lookup and
 * Close interception share that lock, so numeric HANDLE reuse is never a lease. */
static HANDLE nd_cursor_outer(u32* generation){
 HANDLE result=0;struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(nd_cursor_roles[0].duplicate){
  if(!DuplicateHandle(GetCurrentProcess(),nd_cursor_roles[0].duplicate,GetCurrentProcess(),&result,0,0,2))nd_cursor_fail(__LINE__,0);
  *generation=nd_cursor_roles[0].generation;
 }
 return result;
}
static int nd_cursor_ready(u32 slot){
 if(slot==0 || slot==1 || slot==3)return 1;
 if(slot==2 && !*(u32*)0x642c64)return 1;
 if(nd_cursor_disposed || !nd_cursor_roles[1].duplicate || *(HANDLE*)0x642c58!=nd_cursor_roles[1].original)return 0;
 u32 target=slot==7||slot==5?0x6a4fe0:0x6a49e0;
 const u32 wrappers[3]={slot==8?0x642000:0x641fc8,slot==4||slot==5?target:0x642000,target};
 u32 count=slot==8?1:slot==4||slot==5?2:3;
 for(u32 i=0;i<count;++i){struct NsState* s=ns_lifetime_find(*(void**)(wrappers[i]+8));
  if(!s || ns_borrowed(s))return 0;}
 return 1;
}
static u32 nd_cursor_run(u32 slot,u32 argument){
 struct NdCursorFx entry,exit;nd_cursor_save(&entry);u32 generation=0;HANDLE outer=nd_cursor_outer(&generation);
 u32 acquired=0,wait=0;
 if(outer){
  u32 nested=ns_lifetime_mode && ns_lifetime_owner==GetCurrentThreadId() && ns_lifetime_depth;
  wait=WaitForSingleObject(outer,nested?0:(slot<=1?5000:10));
  if(wait==0 || wait==0x80)acquired=1;
  if(wait){if(acquired&&!ReleaseMutex(outer))nd_cursor_fail(__LINE__,0);nd_cursor_original_close(outer);nd_cursor_restore(&entry);return 0;}
 }
 ns_lifetime_enter();u32 result=0;
 if(slot==0 && !nd_cursor_roles[0].duplicate)nd_cursor_closing=0;
 if((nd_cursor_closing && slot!=1) || (outer && generation!=nd_cursor_roles[0].generation) || !nd_cursor_ready(slot)){
  ++nd_cursor_counts[7];nd_cursor_emit(6,slot,wait,generation);nd_cursor_save(&exit);
 }else{
  ++nd_cursor_counts[8];++nd_cursor_active;nd_cursor_emit(5,slot,0,generation);
  if(slot==0){nd_cursor_cancel(0);nd_cursor_disposed=0;}
  if(slot==1){nd_cursor_disposed=1;nd_cursor_cancel(0);}
  struct NdCursorScope scope;
  if(!nd_cursor_scope_enter(&scope,slot))nd_cursor_fail(__LINE__,0);
  nd_cursor_restore(&entry);
  if(slot==2)result=((u32(__attribute__((fastcall))*)(u32))nd_cursor_original[slot])(argument);
  else result=((u32(*)(void))nd_cursor_original[slot])();
  nd_cursor_save(&exit);if(scope.failed)*(u32*)0x642c54=0;nd_cursor_scope_leave(&scope);--nd_cursor_active;nd_cursor_emit(7,slot,result,generation);
 }
 ns_lifetime_leave();if(acquired&&!ReleaseMutex(outer))nd_cursor_fail(__LINE__,0);if(outer&&!nd_cursor_original_close(outer))nd_cursor_fail(__LINE__,0);
 nd_cursor_restore(&exit);return result;
}
#define ND_CURSOR_CLIENT(name,slot) static u32 name(void){return nd_cursor_run(slot,0);}
ND_CURSOR_CLIENT(nd_cursor_setup,0)
ND_CURSOR_CLIENT(nd_cursor_dispose,1)
static u32 __attribute__((fastcall)) nd_cursor_select(u32 type){return nd_cursor_run(2,type);}
ND_CURSOR_CLIENT(nd_cursor_invalidate,3)
ND_CURSOR_CLIENT(nd_cursor_restore_back,4)
ND_CURSOR_CLIENT(nd_cursor_restore_front,5)
ND_CURSOR_CLIENT(nd_cursor_foreground,6)
ND_CURSOR_CLIENT(nd_cursor_timer,7)
ND_CURSOR_CLIENT(nd_cursor_reload,8)
static u32 __attribute__((thiscall)) nd_cursor_create(void* self,u32* desc,u32 flags,u32 caps){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 return ((u32(__attribute__((thiscall))*)(void*,u32*,u32,u32))nd_cursor_original[11])(self,desc,flags,caps);
}
static u32 __attribute__((thiscall)) nd_cursor_release(void* self){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 u32 result=((u32(__attribute__((thiscall))*)(void*))nd_cursor_original[12])(self);
 for(struct NdCursorPrimary* p=nd_cursor_primary;p;p=p->previous)if(p->original==(u32)self)p->original=0;
 return result;
}
static HANDLE WIN nd_cursor_create_mutex(void* security,u32 owned,const char* name){
 u32 caller=(u32)__builtin_return_address(0);HANDLE result=nd_cursor_original_create(security,owned,name);
 struct NdCursorFx saved __attribute__((cleanup(nd_cursor_restore)));nd_cursor_save(&saved);
 if(result && !security && !owned && !name && (caller==0x4a3706 || caller==0x4e8f75)){
  struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();u32 role=caller==0x4a3706?1:0;
  struct NdCursorRole* r=nd_cursor_roles+role;if(r->duplicate)nd_cursor_fail(__LINE__,0);
  HANDLE copy=0;if(!DuplicateHandle(GetCurrentProcess(),result,GetCurrentProcess(),&copy,0,0,2))nd_cursor_fail(__LINE__,0);
  if(nd_cursor_handle_generation==0xffffffffu)nd_cursor_fail(__LINE__,0);
  *r=(struct NdCursorRole){result,copy,++nd_cursor_handle_generation};nd_cursor_emit(8,role,r->generation,1);
 }
 return result;
}
static u32 WIN nd_cursor_close(HANDLE handle){
 struct NdCursorFx entry,exit;nd_cursor_save(&entry);u32 role=2;
 ns_lifetime_enter();for(u32 i=0;i<2;++i)if(nd_cursor_roles[i].original==handle && nd_cursor_roles[i].duplicate)role=i;
 HANDLE outer=0;u32 generation=0;
 if(role==0){nd_cursor_closing=1;outer=nd_cursor_outer(&generation);}
 ns_lifetime_leave();
 if(outer){u32 wait=WaitForSingleObject(outer,5000);if(wait && wait!=0x80)nd_cursor_fail(__LINE__,0);}
 ns_lifetime_enter();if(role<2){
  if(role==0)nd_cursor_cancel(1);
  struct NdCursorRole* r=nd_cursor_roles+role;
  if(r->original!=handle || !r->duplicate)nd_cursor_fail(__LINE__,0);
  if(!nd_cursor_original_close(r->duplicate))nd_cursor_fail(__LINE__,0);r->duplicate=0;r->original=0;nd_cursor_emit(8,role,r->generation,0);
 }
 nd_cursor_restore(&entry);u32 result=nd_cursor_original_close(handle);nd_cursor_save(&exit);
 if(role==0){++nd_cursor_counts[9];nd_cursor_emit(9,*(u32*)0x6f98d4,nd_cursor_counts[0]-nd_cursor_counts[1]-nd_cursor_counts[2],0);
  }
 ns_lifetime_leave();if(outer){if(!ReleaseMutex(outer)||!nd_cursor_original_close(outer))nd_cursor_fail(__LINE__,0);}
 nd_cursor_restore(&exit);return result;
}
static void nd_cursor_patch(void* target,const void* bytes,u32 length){
 u32 old=0,ignored=0;if(!VirtualProtect(target,length,0x40,&old))nd_cursor_fail(__LINE__,0);
 nd_cursor_copy(target,bytes,length);if(!FlushInstructionCache(GetCurrentProcess(),target,length)||!VirtualProtect(target,length,old,&ignored))nd_cursor_fail(__LINE__,0);
}
__declspec(dllexport) u32 WIN NativeDrawCursorStopV1(void){
 if(ns_lifetime_mode!=2 || !nd_cursor_installed)return 1;
 struct NdCursorFx saved __attribute__((cleanup(nd_cursor_restore)));nd_cursor_save(&saved);
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 nd_cursor_closing=1;nd_cursor_cancel(1);++nd_cursor_counts[11];nd_cursor_emit(11,nd_cursor_active,*(u32*)0x6f98d4,0);
 return nd_cursor_active==0 && *(u32*)0x6f98d4==0;
}
__declspec(dllexport) u32 WIN NativeDrawCursorStatusV1(u32* words,u32 bytes){
 struct NsLifeGuard guard __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!words || bytes!=112)return 0;
 u32 active=0,inflight=0,pins=0;for(u32 i=0;i<128;++i){active+=nd_cursor_jobs[i].active;inflight+=nd_cursor_jobs[i].inflight;}
 for(u32 i=0;i<512;++i)pins+=ns_lifetime_pins[i].identity!=0;
 u32 v[12]={1,nd_cursor_installed,nd_cursor_closing,nd_cursor_disposed,nd_cursor_sequence,active,inflight,pins,nd_cursor_roles[0].duplicate!=0,nd_cursor_roles[1].duplicate!=0,nd_cursor_active,nd_cursor_rows};
 for(u32 i=0;i<12;++i)words[i]=v[i];for(u32 i=0;i<16;++i)words[12+i]=nd_cursor_counts[i];return 1;
}
static void nd_cursor_detach(void){
 if(nd_cursor_journal){nd_cursor_emit(10,nd_cursor_active,nd_cursor_counts[0]-nd_cursor_counts[1]-nd_cursor_counts[2],nd_cursor_closing);
  nd_cursor_original_close(nd_cursor_journal);nd_cursor_journal=0;}
}
static void nd_cursor_install(void){
 if(ns_lifetime_mode!=2 || nd_cursor_installed)return;
 if((u32)GetModuleHandleA(0)!=0x400000 || *(u32*)0x642c58 || *(u32*)0x6f98d4)nd_cursor_fail(__LINE__,0);
 const u32 addresses[18]={0x4a30b0,0x4a37d0,0x4a39a0,0x4a4370,0x4a4380,0x4a44b0,0x4a45b0,0x4a4910,0x4a2f90,0x58d5b0,0x58d660,0x58ad90,0x58b3e0,0x58c140,0x58c360,0x58c4a0,0x58c8a0,0x58ca90};
 const u8* pins[18]={nd_cursor_pin_setup,nd_cursor_pin_dispose,nd_cursor_pin_select,nd_cursor_pin_invalidate,nd_cursor_pin_restore_back,nd_cursor_pin_restore_front,nd_cursor_pin_foreground,nd_cursor_pin_timer,nd_cursor_pin_reload,nd_cursor_pin_enqueue,nd_cursor_pin_drain,nd_cursor_pin_create,nd_cursor_pin_release,nd_cursor_pin_copy_full,nd_cursor_pin_copy_guarded,nd_cursor_pin_copy_opaque,nd_cursor_pin_copy_keyed,nd_cursor_pin_copy_front_keyed};
 const u32 lengths[18]={sizeof(nd_cursor_pin_setup),sizeof(nd_cursor_pin_dispose),sizeof(nd_cursor_pin_select),sizeof(nd_cursor_pin_invalidate),sizeof(nd_cursor_pin_restore_back),sizeof(nd_cursor_pin_restore_front),sizeof(nd_cursor_pin_foreground),sizeof(nd_cursor_pin_timer),sizeof(nd_cursor_pin_reload),sizeof(nd_cursor_pin_enqueue),sizeof(nd_cursor_pin_drain),sizeof(nd_cursor_pin_create),sizeof(nd_cursor_pin_release),sizeof(nd_cursor_pin_copy_full),sizeof(nd_cursor_pin_copy_guarded),sizeof(nd_cursor_pin_copy_opaque),sizeof(nd_cursor_pin_copy_keyed),sizeof(nd_cursor_pin_copy_front_keyed)};
 void* replacements[18]={(void*)nd_cursor_setup,(void*)nd_cursor_dispose,(void*)nd_cursor_select,(void*)nd_cursor_invalidate,(void*)nd_cursor_restore_back,(void*)nd_cursor_restore_front,(void*)nd_cursor_foreground,(void*)nd_cursor_timer,(void*)nd_cursor_reload,(void*)nd_cursor_enqueue,(void*)nd_cursor_drain,(void*)nd_cursor_create,(void*)nd_cursor_release,(void*)nd_cursor_copy_full,(void*)nd_cursor_copy_guarded,(void*)nd_cursor_copy_opaque,(void*)nd_cursor_copy_keyed,(void*)nd_cursor_copy_front_keyed};
 void* chains[18]={0};
 typedef u32(WIN* SceneHook)(u32,const u8*,u32,u32*);
 void* scene=GetModuleHandleA("MnmScene.dll");SceneHook scene_hook=scene?(SceneHook)GetProcAddress(scene,"SceneCursorHookV1"):0;
 for(u32 i=0;i<18;++i){
  if(!nd_cursor_readable((void*)addresses[i],lengths[i]))nd_cursor_fail(0x100+i,addresses[i]);
  u32 match=1;for(u32 k=0;k<lengths[i];++k)if(((u8*)addresses[i])[k]!=pins[i][k])match=0;
  if(!match){u32 chain[2];if(!scene_hook || !scene_hook(addresses[i],pins[i],lengths[i],chain))nd_cursor_fail(0x200+i,addresses[i]);
   /* Create/release observers keep public wrapper identities. Copies already
    * enter MnmRender through retained native COM aliases; the scene wrapper-
    * address model cannot represent a retired generation's private views. */
   chains[i]=(void*)chain[i==11||i==12?0:1];
  }
 }
 void* kernel=GetModuleHandleA("kernel32.dll");nd_cursor_original_create=(HANDLE(WIN*)(void*,u32,const char*))GetProcAddress(kernel,"CreateMutexA");nd_cursor_original_close=(u32(WIN*)(HANDLE))GetProcAddress(kernel,"CloseHandle");
 if(!nd_cursor_original_create || !nd_cursor_original_close || *(void**)0x5c50b8!=(void*)nd_cursor_original_create || *(void**)0x5c50a0!=(void*)nd_cursor_original_close)nd_cursor_fail(__LINE__,0);
 typedef u32(WIN* PinModule)(u32,const char*,void**);void* pinned=0;PinModule pin=(PinModule)GetProcAddress(kernel,"GetModuleHandleExA");
 if(!pin || !pin(5,(const char*)&nd_cursor_install,&pinned))nd_cursor_fail(__LINE__,0);
 u8* trampolines=VirtualAlloc(0,18*32,0x3000,0x40);if(!trampolines)nd_cursor_fail(__LINE__,0);
 for(u32 i=0;i<18;++i){u8* at=trampolines+i*32;nd_cursor_copy(at,pins[i],lengths[i]);at[lengths[i]]=0xe9;*(u32*)(at+lengths[i]+1)=addresses[i]+lengths[i]-(u32)(at+lengths[i]+5);nd_cursor_original[i]=chains[i]?chains[i]:at;}
 if(!FlushInstructionCache(GetCurrentProcess(),trampolines,18*32))nd_cursor_fail(__LINE__,0);
 char journal[512];u32 n=GetEnvironmentVariableA("MNM_NATIVE_CURSOR_LOG",journal,sizeof(journal));
 if(n){if(n>=sizeof(journal))nd_cursor_fail(__LINE__,0);nd_cursor_journal=CreateFileA(journal,0x40000000,1,0,1,0x80,0);u32 h[16]={0x434d4e4d,0x31574f52,1,64,0x40209ca7,18,32768},done=0;
  if(nd_cursor_journal==(HANDLE)-1||!WriteFile(nd_cursor_journal,h,64,&done,0)||done!=64)nd_cursor_fail(__LINE__,0);}
 /* The original main loop publishes outer before its first graphics
  * factory call. Startup still has no inner cursor mutex or queued job. Admit
  * that actual kernel mutex by a private duplicate, never its numeric value. */
 HANDLE startup_outer=*(HANDLE*)0x6e1ec0;
 if(startup_outer){HANDLE duplicate=0;if(!DuplicateHandle(GetCurrentProcess(),startup_outer,GetCurrentProcess(),&duplicate,0,0,2))nd_cursor_fail(__LINE__,0);
  u32 wait=WaitForSingleObject(duplicate,0);if(wait!=0 || !ReleaseMutex(duplicate))nd_cursor_fail(__LINE__,0);
  nd_cursor_roles[0]=(struct NdCursorRole){startup_outer,duplicate,++nd_cursor_handle_generation};
 }
 for(u32 i=0;i<18;++i){u8 jump[5]={0xe9};*(u32*)(jump+1)=(u32)replacements[i]-addresses[i]-5;nd_cursor_patch((void*)addresses[i],jump,5);}
 void* create=(void*)nd_cursor_create_mutex,*close=(void*)nd_cursor_close;nd_cursor_patch((void*)0x5c50b8,&create,4);nd_cursor_patch((void*)0x5c50a0,&close,4);
 nd_cursor_installed=1;nd_cursor_emit(1,18,0,0);if(startup_outer)nd_cursor_emit(8,0,nd_cursor_roles[0].generation,1);
}
