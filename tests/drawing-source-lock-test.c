/* Execute the production lease/snapshot callbacks against an allocating scene
 * producer. Reintroducing Drawing -> Scene lock order must trip the watchdog. */
#define MNM_DRAWING_SELFTEST
#define DllMain SourceLockFixtureDllMain
#include "../runtime/drawing/session.c"
API HANDLE WIN CreateThread(void*,u32,u32(WIN*)(void*),void*,u32,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
static u32 scene_lock[6],ready,requested,finished,rounds;
static void check(int value){if(!value)ExitProcess(210);}
static u32 WIN watchdog(void* unused){(void)unused;Sleep(8000);if(!__atomic_load_n(&finished,__ATOMIC_ACQUIRE))ExitProcess(211);return 0;}
static u32 WIN scene_writer(void* unused){
 (void)unused;EnterCriticalSection(scene_lock);__atomic_store_n(&ready,1,__ATOMIC_RELEASE);
 while(!__atomic_load_n(&requested,__ATOMIC_ACQUIRE))Sleep(1);
 /* This is the journal critical section entered by observed scene heap calls. */
 EnterCriticalSection(journal_lock);++rounds;LeaveCriticalSection(journal_lock);
 LeaveCriticalSection(scene_lock);return 0;
}
static void scene_roundtrip(void){
 __atomic_store_n(&ready,0,__ATOMIC_RELEASE);__atomic_store_n(&requested,0,__ATOMIC_RELEASE);
 HANDLE worker=CreateThread(0,0,scene_writer,0,0,0);check(worker!=0);
 while(!__atomic_load_n(&ready,__ATOMIC_ACQUIRE))Sleep(1);
 __atomic_store_n(&requested,1,__ATOMIC_RELEASE);EnterCriticalSection(scene_lock);LeaveCriticalSection(scene_lock);
 check(WaitForSingleObject(worker,2000)==0);check(CloseHandle(worker));
}
static u32 WIN lease(void* com,u32 token,u32* point,u32 bytes){
 check(com==(void*)7&&bytes==64);scene_roundtrip();memset(point,0,64);return token?0:0x8000007bu;
}
static u32 WIN snapshot(void* com,u32* before,u32* after,u8* pixels,u32 capacity){
 check(com==(void*)7&&capacity==960000);scene_roundtrip();memset(before,0,64);before[0]=64;before[1]=1;before[2]=19;before[6]=9;before[7]=before[9]=800;before[8]=600;copy(after,before,64);memset(pixels,3,capacity);return 0;
}
void start(void){
 InitializeCriticalSection(journal_lock);InitializeCriticalSection(scene_lock);
 check(GetEnvironmentVariableA("MNM_DRAW_SESSION_DIRECTORY",directory,sizeof(directory))>0);
 char name[600];path(name,"fixture-lease.bin");source_lease_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);check(source_lease_file!=(HANDLE)-1);
 path(name,"fixture-held.bin");source_held_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);check(source_held_file!=(HANDLE)-1);
 source_lease_enabled=source_held_enabled=1;source_lease_getter=lease;source_bytes_getter=snapshot;
 struct Context c={0};u32 rect[4]={0,0,800,600};c.observed=1;c.r[1]=1;c.r[2]=7;c.r[29]=1;c.args[2]=7;c.args[3]=(u32)rect;c.args[4]=16;
 HANDLE timer=CreateThread(0,0,watchdog,0,0,0);check(timer!=0);
 char inverse[2];u32 negative=GetEnvironmentVariableA("MNM_TEST_INVERSE_LOCK",inverse,2);
 if(negative)EnterCriticalSection(journal_lock);
 source_lease_capture(&c,1);check(c.source_lease==123);source_worker_lease.count=source_worker_lease.completed=1;
 source_lease_capture(&c,2);check(rounds==4&&!source_held_first&&source_held_planes==2&&source_lease_records==2);
 check(CloseHandle(source_lease_file)&&CloseHandle(source_held_file));__atomic_store_n(&finished,1,__ATOMIC_RELEASE);CloseHandle(timer);ExitProcess(0);
}
