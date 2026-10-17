#include "../../protocols/include/mnm/drawing_worker_producer_points_v1.h"
#include "../../protocols/include/mnm/drawing_source_point_v1.h"
typedef u32(WIN* WorkerProducerPoint)(void*,u32*,u32);
static WorkerProducerPoint worker_producer_getter;
static HANDLE worker_producer_file;
static u32 worker_producer_enabled,worker_producer_rows,worker_producer_entries,worker_producer_returns;
static int worker_producer_open(u32 worker_enabled){
 char flag[2];SetLastError(0);u32 n=GetEnvironmentVariableA("MNM_DRAW_WINE_WORKER_PRODUCER",flag,2);
 if(!n)return GetLastError()==203;
 if(n!=1||flag[0]!='1'||!worker_enabled||!producer_points_enabled)return 0;
 char name[600];path(name,"wine-worker-producer-points.bin");worker_producer_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);
 if(worker_producer_file==(HANDLE)-1){worker_producer_file=0;return 0;}
 u32 h[16]={0};copy(h,MNM_WORKER_PRODUCER_MAGIC,8);h[2]=1;h[3]=128;h[4]=0x40209ca7;h[5]=MNM_WORKER_PRODUCER_MAX_ROWS;h[6]=64;
 if(!write(worker_producer_file,h,64))return 0;__atomic_store_n(&worker_producer_enabled,1,__ATOMIC_RELEASE);return 1;
}
static void worker_producer_emit(u32* r){
 r[0]=++worker_producer_rows;if(worker_producer_rows>MNM_WORKER_PRODUCER_MAX_ROWS||!write(worker_producer_file,r,128))ExitProcess(99);
}
/* Getter runs outside journal_lock. The existing worker pending count keeps
 * ordinary stop from retiring this recorder while a worker body is observed. */
static void worker_producer_capture(const u32* worker,u32 phase){
 if(!worker[2]||!__atomic_load_n(&worker_producer_enabled,__ATOMIC_ACQUIRE))return;
 WorkerProducerPoint getter=__atomic_load_n(&worker_producer_getter,__ATOMIC_ACQUIRE);
 if(!getter){void* scene=GetModuleHandleA("MnmScene.dll");getter=scene?(WorkerProducerPoint)GetProcAddress(scene,"SceneDrawingSourcePointV1"):0;if(getter)__atomic_store_n(&worker_producer_getter,getter,__ATOMIC_RELEASE);}
 u32 r[32]={0};r[1]=phase;r[2]=worker[2];r[3]=worker[4];r[4]=GetTickCount();r[6]=worker[0];
 if(!getter)r[5]=1;else if(!getter(0,r+8,64)){r[5]=2;for(u32 i=8;i<24;++i)r[i]=0;}
 EnterCriticalSection(journal_lock);
 if(__atomic_load_n(&worker_producer_enabled,__ATOMIC_ACQUIRE)){
  if(worker_producer_rows>=MNM_WORKER_PRODUCER_MAX_ROWS-1)ExitProcess(99);
  if(phase==1)++worker_producer_entries;else ++worker_producer_returns;worker_producer_emit(r);
 }
 LeaveCriticalSection(journal_lock);
}
/* Caller owns journal_lock; forced teardown with an active callback remains
 * an incomplete terminal rather than a fabricated worker return. */
static void worker_producer_close(void){
 if(!worker_producer_file)return;__atomic_store_n(&worker_producer_enabled,0,__ATOMIC_RELEASE);
 u32 r[32]={0};r[1]=3;r[2]=worker_producer_entries;r[3]=GetCurrentThreadId();r[4]=GetTickCount();r[6]=worker_producer_returns;r[7]=worker_producer_entries-worker_producer_returns;
 worker_producer_emit(r);if(!CloseHandle(worker_producer_file))ExitProcess(99);worker_producer_file=0;
}
