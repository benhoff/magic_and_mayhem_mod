/* Provider-owned lifetime is opt-in. Lock order is provider -> ordered bridge
 * -> tracker. Bridge admission takes this same recursive gate before its own
 * copy owner; no provider lock is acquired while waiting for another copy owner.
 * LastError and the complete x86 FP/SIMD state survive lock bookkeeping. */
API u32 WIN GetCurrentThreadId(void);
API void WIN InitializeCriticalSection(void*);
API void WIN EnterCriticalSection(void*);
API void WIN LeaveCriticalSection(void*);
static u32 ns_lifetime_mode,ns_lifetime_states,ns_lifetime_owner,ns_lifetime_depth;
static u32 ns_lifetime_lock[6];
static void ns_lifetime_collect(void);
static void ns_lifetime_initialize(void){
 char value[2];u32 n=GetEnvironmentVariableA("MNM_NATIVE_CURSOR_OWNERSHIP",value,2);
 if(!n)return;if(n!=1 || (value[0]!='1' && value[0]!='2'))ExitProcess(123);
 ns_lifetime_mode=value[0]-'0';InitializeCriticalSection(ns_lifetime_lock);
}
static void ns_lifetime_enter(void){
 if(!ns_lifetime_mode)return;
 u32 error=GetLastError();u8 fp[512]__attribute__((aligned(16)));__asm__ volatile("fxsave (%0)"::"r"(fp):"memory");
 EnterCriticalSection(ns_lifetime_lock);ns_lifetime_owner=GetCurrentThreadId();++ns_lifetime_depth;
 __asm__ volatile("fxrstor (%0)"::"r"(fp):"memory");SetLastError(error);
}
static void ns_lifetime_leave(void){
 if(!ns_lifetime_mode)return;
 u32 error=GetLastError();u8 fp[512]__attribute__((aligned(16)));__asm__ volatile("fxsave (%0)"::"r"(fp):"memory");
 if(!ns_lifetime_depth || ns_lifetime_owner!=GetCurrentThreadId())ExitProcess(123);
 if(ns_lifetime_depth==1)ns_lifetime_collect();
 if(!--ns_lifetime_depth)ns_lifetime_owner=0;LeaveCriticalSection(ns_lifetime_lock);
 __asm__ volatile("fxrstor (%0)"::"r"(fp):"memory");SetLastError(error);
}
struct NsLifeGuard {u32 held;};
static struct NsLifeGuard ns_life_guard_enter(void){ns_lifetime_enter();return (struct NsLifeGuard){ns_lifetime_mode!=0};}
static void ns_life_guard_leave(struct NsLifeGuard* g){if(g->held){g->held=0;ns_lifetime_leave();}}
__declspec(dllexport) u32 WIN NativeDrawGuardBeginV1(void){ns_lifetime_enter();return ns_lifetime_mode!=0;}
__declspec(dllexport) void WIN NativeDrawGuardEndV1(void){ns_lifetime_leave();}
