/* Opt-in original-active recorder synchronization. No scope survives return to
 * an original body. This protects recorder state, never original surface bytes. */
API void WIN InitializeCriticalSection(void*);
API void WIN EnterCriticalSection(void*);
API void WIN LeaveCriticalSection(void*);
static u32 producer_trace_mode,producer_trace_mutex[6];
static struct ProducerTraceThread {u32 id,depth;} producer_trace_threads[32];
struct ProducerTraceGuard {u32 locked;};
static struct ProducerTraceGuard producer_trace_acquire(void){
 struct ProducerTraceGuard g={producer_trace_mode};
 if(g.locked){u32 error=GetLastError();EnterCriticalSection(producer_trace_mutex);SetLastError(error);}return g;
}
static void producer_trace_release(struct ProducerTraceGuard* g){if(g->locked){u32 error=GetLastError();LeaveCriticalSection(producer_trace_mutex);SetLastError(error);}}
#define PRODUCER_TRACE_SCOPE struct ProducerTraceGuard trace_scope __attribute__((cleanup(producer_trace_release)))=producer_trace_acquire()
static int producer_trace_configure(void){
 char flag[2];SetLastError(0);u32 n=GetEnvironmentVariableA("MNM_DRAW02_PRODUCER_BIND",flag,2);
 if(!n)return !producer_trace_mode&&GetLastError()==203;
 if(n!=1||flag[0]!='1')return 0;
 if(!producer_trace_mode){InitializeCriticalSection(producer_trace_mutex);producer_trace_mode=1;}return 1;
}
/* Caller owns the trace scope. Thread IDs are local diagnostic identities; no
 * stable ID or address across launches is assumed. Exhaustion refuses. */
static u32* producer_trace_depth(u32 owner){
 if(!producer_trace_mode)return &depth;
 if(!owner){ExitProcess(98);return 0;}
 for(u32 i=0;i<32;++i)if(producer_trace_threads[i].id==owner)return &producer_trace_threads[i].depth;
 for(u32 i=0;i<32;++i)if(!producer_trace_threads[i].id){producer_trace_threads[i].id=owner;return &producer_trace_threads[i].depth;}
 ExitProcess(98);return 0;
}
