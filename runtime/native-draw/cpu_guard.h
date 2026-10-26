#include "row_words.h"
/* Stale allocation protection. Opt-in mode2 single-steps discovery stores;
 * resident mode3 emulates checked stores through a private mapped view.
 * No handler ever requests a readback. */
API void* WIN AddVectoredExceptionHandler(u32,void*);
API u32 WIN GetCurrentThreadId(void);
API u32 WIN RemoveVectoredExceptionHandler(void*);
API HANDLE WIN CreateFileMappingA(HANDLE,void*,u32,u32,u32,const char*);
API void* WIN MapViewOfFile(HANDLE,u32,u32,u32,u32);
API i32 WIN UnmapViewOfFile(const void*);
#include "cpu_store.h"
#include "cpu_store_profile.h"
#include "ownership_admission.h"
static void* ns_guard_handler;
static u32 ns_guard_initialized,ns_guard_enabled,ns_guard_faulted;
static u32 ns_guard_main_module,ns_guard_draw_module;
static struct NsState* ns_guard_step;
static u32 ns_guard_step_thread;
/* The handler only redirects execution. The ordinary thread stack performs the
 * ordered restore after Windows has left exception dispatch, then retries the
 * original instruction with its complete register/flags/FX state. */
static u32 ns_cpu_reader_available;
static void ns_cpu_require_at(struct NsState*,u32,u32,u32);
struct NsCpuResume {volatile u32 owner;struct NsState* state;u32 instruction;};
static struct NsCpuResume ns_cpu_resumes[32];
__attribute__((used,noinline)) static u32 ns_cpu_resume(void){
 const u32 error=GetLastError(),thread=GetCurrentThreadId();
 for(u32 i=0;i<32;++i)if(ns_cpu_resumes[i].owner==thread){
  struct NsCpuResume* resume=ns_cpu_resumes+i;struct NsState* s=resume->state;
  if(s->locked!=1 || s->lock_owner!=thread || s->dc || s->storage_dc || s->cpu_guard_world)ExitProcess(124);
  const u32 instruction=resume->instruction;
  ns_cpu_require_at(s,0,0,instruction);
  __atomic_store_n(&resume->owner,0,__ATOMIC_RELEASE);SetLastError(error);return instruction;
 }
 ExitProcess(124);return 0;
}
__attribute__((naked)) static void ns_cpu_resume_entry(void){
 __asm__ volatile(
  "pushl $0\n\tpushfl\n\tpushal\n\tcld\n\tmovl %esp,%eax\n\tsubl $528,%esp\n\tandl $-16,%esp\n\tfxsave (%esp)\n\t"
  "pushl %eax\n\tcalll _ns_cpu_resume\n\tpopl %edx\n\tfxrstor (%esp)\n\tmovl %eax,36(%edx)\n\t"
  "movl %edx,%esp\n\tpopal\n\tpopfl\n\tretl\n\t");
}
/* PE32 CONTEXT Eip/EFlags DWORD offsets for the discovery and protected modes. */
static u32 ns_guard_store(const u8* pc){
 u32 bytes=4;if(*pc==0x66){bytes=2;++pc;}
 if(*pc==0x88 || *pc==0x89)return (pc[1]&0xc0)!=0xc0?(*pc==0x88?1:bytes):0;
 if(*pc==0xc6 || *pc==0xc7)return (pc[1]&0xc0)!=0xc0 && !(pc[1]&0x38)?(*pc==0xc6?1:bytes):0;
 if(*pc==0xa2 || *pc==0xaa)return 1;
 return *pc==0xa3 || *pc==0xab?bytes:0;
}
struct NsGuardExceptionRecord {u32 code,flags,record,address,count,information[15];};
struct NsGuardExceptionPointers {struct NsGuardExceptionRecord* record;void* context;};
static u32 ns_guard_bytes(const struct NsState* s){return (s->pitch*s->height+2+4095u)&~4095u;}
static void ns_guard_dirty_clear(struct NsState* s){
 u8* dirty=s->cpu_dirty;const u32 length=(s->width*s->height*2+7)/8;
 if(dirty)for(u32 i=0;i<length;++i)dirty[i]=0;
 s->cpu_dirty_count=s->cpu_dirty_partial=0;
}
#include "dirty_bytes.h"
static void ns_guard_dirty(struct NsState* s,u32 offset,u32 bytes){
 if(!s->cpu_dirty || !s->history_stale || s->locked<=0)return;
 ns_dirty_bytes(s->cpu_dirty,s->width,s->height,s->pitch,offset,bytes,&s->cpu_dirty_count,&s->cpu_dirty_partial);
}
static void ns_guard_set(struct NsState*,u32);
static void ns_guard_refresh(struct NsState*);
static i32 WIN ns_guard_fault(struct NsGuardExceptionPointers* pointers){
 struct NsGuardExceptionRecord* e=pointers?pointers->record:0;
 if(!e)return 0;
 u32* context=pointers->context;
 if(ns_guard_step){
  struct NsState* s=ns_guard_step;
  if(e->code==0x80000004u && context && GetCurrentThreadId()==ns_guard_step_thread){
   ns_guard_step=0;context[48]&=~0x100u;ns_guard_refresh(s);return -1;
  }
  /* No nested fault, foreign trap or cross-thread continuation is admitted. */
  ExitProcess(124);
 }
 if(e->code!=0xc0000005u || e->count<2)return 0;
 for(struct NsState* s=ns_lifetime_mode?__atomic_load_n(&ns_lifetime_all,__ATOMIC_ACQUIRE):ns_owned;
     s;s=ns_lifetime_mode?s->lifetime_next_all:s->next_owned){
  if(__atomic_load_n(&s->lifetime_retired,__ATOMIC_ACQUIRE))continue;
  const u32 at=e->information[1],base=(u32)s->pixels;
  if(!s->cpu_guard || at<base || at-base>=ns_guard_bytes(s))continue;
  /* The mandatory World guard owns its interval, including diagnostics. The
   * protected alias keeps pages closed while that existing handler runs. */
  if(s->pixels_alias && s->cpu_guard_world && !s->cpu_guard_world_ready)return 0;
  if(ns_guard_enabled==3 && e->information[0]==1 && s->pixels_alias &&
     s->locked>0 && s->lock_owner==GetCurrentThreadId() && !s->dc && !s->storage_dc &&
     !s->cpu_guard_world && context && context[46]==e->address && !(context[48]&0x100u) &&
     context[38]==context[50]){
   struct NsCpuStore store;
   if(ns_cpu_decode_store((const u8*)e->address,context,&store) &&
      (!store.stos || context[37]==context[38]) && store.address>=base &&
      store.address-base<=ns_guard_bytes(s)-store.width &&
      at>=store.address && at-store.address<store.width){
    volatile u8* target=(volatile u8*)s->pixels_alias+store.address-base;
    for(u32 i=0;i<store.width;++i)target[i]=(u8)(store.value>>(i*8));
    ns_guard_dirty(s,store.address-base,store.width);
    ns_store_profile_record(e->address,store.width,store.length);
    if(store.stos)context[39]+=(context[48]&0x400u)?0u-store.width:store.width;
    context[46]+=store.length;return -1;
   }
  }
  if(ns_guard_enabled==3 && ns_cpu_reader_available && s->pixels_alias && s->history_stale &&
     s->locked==1 && s->lock_owner==GetCurrentThreadId() && !s->dc && !s->storage_dc &&
     !s->cpu_guard_world && context && context[46]==e->address && e->information[0]<=1 &&
     !(context[48]&0x100u) && at-base<s->pitch*s->height){
   const u32 thread=GetCurrentThreadId();
   for(u32 i=0;i<32;++i)if(ns_cpu_resumes[i].owner==thread)ExitProcess(124);
   for(u32 i=0;i<32;++i)if(__sync_bool_compare_and_swap(&ns_cpu_resumes[i].owner,0,thread)){
    ns_cpu_resumes[i].state=s;ns_cpu_resumes[i].instruction=e->address;
    context[46]=(u32)ns_cpu_resume_entry;return -1;
   }
   ExitProcess(124);
  }
  /* The diagnostic mode2 whitelist excludes RMW, string copies, REP, LOCK, SIMD and
   * unknown encodings. A read or unsupported write is terminal. The temporary
   * open interval executes one pure store only, then the owned TF trap rearms.
   * This is a single-thread discovery aid, not concurrent-reader exclusion. */
  u32 store_bytes=0;
  if(ns_guard_enabled==2 && e->information[0]==1 && s->locked>0 && !s->storage_dc &&
     context && context[46]==e->address && !(context[48]&0x100u) &&
     (store_bytes=ns_guard_store((const u8*)e->address)) && at-base<=ns_guard_bytes(s)-store_bytes){
   ns_guard_step=s;ns_guard_step_thread=GetCurrentThreadId();
   ns_guard_set(s,0);context[48]|=0x100u;return -1;
  }
  if(__atomic_exchange_n(&ns_guard_faulted,1,__ATOMIC_ACQ_REL))ExitProcess(124);
  /* All module identities were cached outside the fault handler. No drawing,
   * GL, loader lookup, heap allocation, retry or renderer wait occurs here. */
  u32 r[24]={0x434d4e4d,0x31465550,1,96,e->address,at,e->information[0],GetCurrentThreadId(),
   base,s->pitch*s->height,ns_guard_bytes(s),s->width,s->height,s->pitch,
   s->history_generation[0],s->history_generation[1],s->history_nonce[0],s->history_nonce[1],
   ns_guard_main_module,ns_guard_draw_module,(u32)s->locked,s->storage_dc!=0,1,0},done=0;
  HANDLE file=CreateFileA("native-cpu-guard-fault.bin",0x40000000,0,0,1,0x80,0);
  if(file!=(HANDLE)-1){WriteFile(file,r,sizeof(r),&done,0);CloseHandle(file);}
  ExitProcess(124);
 }
 return 0; /* Unrelated faults belong to the existing exception chain. */
}
static void ns_guard_initialize(void){
 if(ns_guard_initialized)return;
 u32 error=GetLastError();char mode[2];u32 n=GetEnvironmentVariableA("MNM_NATIVE_CPU_GUARD",mode,2);
 if(!n){char resident[2];if(GetEnvironmentVariableA("MNM_PUBLIC_RESIDENT_WORLD",resident,2)==1 && resident[0]=='1'){mode[0]='3';n=1;}}
 if(n && (n!=1 || (mode[0]!='0' && mode[0]!='1' && mode[0]!='2' && mode[0]!='3')))ExitProcess(124);
 if(n && mode[0]=='0')n=0;
 if(n){
  ns_guard_main_module=(u32)GetModuleHandleA(0);ns_guard_draw_module=(u32)GetModuleHandleA("MnmDraw.dll");
  ns_guard_handler=AddVectoredExceptionHandler(1,(void*)ns_guard_fault);if(!ns_guard_handler)ExitProcess(124);
  ns_guard_enabled=(u32)(mode[0]-'0');
  if(ns_guard_enabled>=2){
   const char* names[10]={0,"MnmDraw.dll","MnmRender.dll","MnmScene.dll","MnmPanelWriters.dll","MnmPixelWriters.dll","MnmMinimapWriters.dll","MnmColourRect.dll","MnmRasterWriters.dll","MnmJpegPixels.dll"};
   u32 modules[14]={0x434d4e4d,ns_guard_enabled==3?0x33304d47:0x32304d47,ns_guard_enabled,56},done=0;
   for(u32 i=0;i<10;++i)modules[4+i]=(u32)GetModuleHandleA(names[i]);
   HANDLE file=CreateFileA("native-cpu-guard-modules.bin",0x40000000,0,0,1,0x80,0);
   if(file!=(HANDLE)-1){WriteFile(file,modules,sizeof(modules),&done,0);CloseHandle(file);}
  }
 }
 ns_store_profile_initialize(ns_guard_enabled);
 ns_ownership_admit(ns_guard_enabled);ns_native_only_initialize();
 ns_guard_initialized=1;SetLastError(error);
}
static void ns_guard_set(struct NsState* s,u32 guarded){
 if(!ns_guard_enabled || s->cpu_guard==guarded)return;
 u32 error=GetLastError(),old=0,expected=guarded?4u:1u;
 /* Owned allocations/views have their own pages, including the trailing WORD.
  * A permission mismatch is terminal rather than silently erasing another guard. */
 if(!VirtualProtect(s->pixels,ns_guard_bytes(s),guarded?1:4,&old) || old!=expected){
  u32 r[6]={0x474d4e4d,1,(u32)s->pixels,guarded,old,expected},done=0;
  HANDLE file=CreateFileA("native-cpu-guard-refusal.bin",0x40000000,0,0,1,0x80,0);
  if(file!=(HANDLE)-1){WriteFile(file,r,sizeof(r),&done,0);CloseHandle(file);}ExitProcess(124);
 }
 s->cpu_guard=guarded;SetLastError(error);
}
static void ns_guard_refresh(struct NsState* s){
 ns_guard_set(s,s->history_stale && (ns_guard_enabled>=2 || s->locked<=0) && !s->dc && !s->storage_dc);
}
/* Private compatibility bytes only, under the existing ordered facade lease.
 * Opening this interval is not reader admission or a claim of CPU authority. */
static void ns_guard_baseline(struct NsState* s,u16* baseline){
 if(!s->pixels_alias)ns_guard_set(s,0);
 const u16* bytes=s->pixels_alias?s->pixels_alias:s->pixels;
 ns_copy_word_rows(baseline,s->width,bytes,s->pitch/2,s->width,s->height);
 if(!s->pixels_alias)ns_guard_refresh(s);
}
/* A process-local exception callback must not survive explicit DLL unload. */
static void ns_guard_finish(void){
 if(ns_guard_handler){void* handler=ns_guard_handler;ns_guard_handler=0;
  if(!RemoveVectoredExceptionHandler(handler))ExitProcess(124);}
 ns_store_profile_finish();
}
