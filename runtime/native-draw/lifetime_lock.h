/* Provider-owned lifetime is opt-in. Lock order is provider -> ordered bridge
 * -> tracker. Bridge admission takes this same recursive gate before its own
 * copy owner; no provider lock is acquired while waiting for another copy owner.
 * LastError and the complete x86 FP/SIMD state survive lock bookkeeping. */
API u32 WIN GetCurrentThreadId(void);
API void WIN InitializeCriticalSection(void*);
API void WIN EnterCriticalSection(void*);
API void WIN LeaveCriticalSection(void*);
static u32 ns_lifetime_mode,ns_lifetime_states,ns_lifetime_owner,ns_lifetime_depth;
/* Mutations set this only when backing can retire. Routine drawing/getters
 * keep the same gate without walking every owned surface on guard exit. */
static u32 ns_lifetime_pending;
static u32 ns_lifetime_lock[6];
static void ns_lifetime_collect(void);
static void ns_lifetime_initialize(void){
 char value[2];u32 n=GetEnvironmentVariableA("MNM_NATIVE_CURSOR_OWNERSHIP",value,2);
 if(!n)return;if(n!=1 || (value[0]!='1' && value[0]!='2'))ExitProcess(123);
 ns_lifetime_mode=value[0]-'0';InitializeCriticalSection(ns_lifetime_lock);
}
/* PE32 TEB self/ClientId thread token, matching GetCurrentThreadId without
 * an API call. This integer-only nested path touches neither LastError nor
 * x87/SIMD state. Only the current owning thread can inspect/change depth. */
static inline u32 ns_lifetime_thread(void){
 u32 thread;__asm__ volatile("movl %%fs:0x18,%0; movl 0x24(%0),%0":"=r"(thread));return thread;
}
static void ns_lifetime_enter(void){
 if(!ns_lifetime_mode)return;
 u32 thread=ns_lifetime_thread();
 if(__atomic_load_n(&ns_lifetime_owner,__ATOMIC_ACQUIRE)==thread){
  if(!ns_lifetime_depth || ns_lifetime_depth==0xffffffffu)ExitProcess(123);
  ++ns_lifetime_depth;return;
 }
 u32 error=GetLastError();u8 fp[512]__attribute__((aligned(16)));__asm__ volatile("fxsave (%0)"::"r"(fp):"memory");
 EnterCriticalSection(ns_lifetime_lock);ns_lifetime_depth=1;__atomic_store_n(&ns_lifetime_owner,thread,__ATOMIC_RELEASE);
 __asm__ volatile("fxrstor (%0)"::"r"(fp):"memory");SetLastError(error);
}
static void ns_lifetime_leave(void){
 if(!ns_lifetime_mode)return;
 if(__atomic_load_n(&ns_lifetime_owner,__ATOMIC_ACQUIRE)!=ns_lifetime_thread() || !ns_lifetime_depth)ExitProcess(123);
 if(ns_lifetime_depth>1){--ns_lifetime_depth;return;}
 u32 error=GetLastError();u8 fp[512]__attribute__((aligned(16)));__asm__ volatile("fxsave (%0)"::"r"(fp):"memory");
 if(ns_lifetime_pending)ns_lifetime_collect();
 ns_lifetime_depth=0;__atomic_store_n(&ns_lifetime_owner,0,__ATOMIC_RELEASE);LeaveCriticalSection(ns_lifetime_lock);
 __asm__ volatile("fxrstor (%0)"::"r"(fp):"memory");SetLastError(error);
}
struct NsLifeGuard {u32 held;};
static struct NsLifeGuard ns_life_guard_enter(void){ns_lifetime_enter();return (struct NsLifeGuard){ns_lifetime_mode!=0};}
static void ns_life_guard_leave(struct NsLifeGuard* g){if(g->held){g->held=0;ns_lifetime_leave();}}
__declspec(dllexport) u32 WIN NativeDrawGuardBeginV1(void){ns_lifetime_enter();return ns_lifetime_mode!=0;}
__declspec(dllexport) void WIN NativeDrawGuardEndV1(void){ns_lifetime_leave();}
