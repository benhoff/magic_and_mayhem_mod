static void wine_gpu_complete(const void*,u32);
static void wine_worker_context_publish(const void*,u32,u32);
static void wine_blit_packet_worker(const void*,u32,u32);
/* Selected opcode42 worker metadata; no pixel payload or inferred queue pairing. */
static HANDLE wine_blit_worker_file;
static u32 wine_blit_worker_enabled,wine_blit_worker_records,wine_blit_worker_calls,wine_blit_worker_pending;
static void* wine_blit_worker_original;
#include "worker_producer_points.h"
static void wine_blit_worker_emit(u32* r){r[0]=++wine_blit_worker_records;if(wine_blit_worker_records>131072||!write(wine_blit_worker_file,r,192))ExitProcess(99);}
static void __attribute__((cdecl)) wine_blit_worker(void* context,const u32* packet){
 u32 error=GetLastError(),observed=0,r[48]={0};u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);
 wine_source_ownership_require();
 EnterCriticalSection(journal_lock);if(wine_submission_enabled){observed=1;if(!readable(packet,100)||packet[0]!=42)ExitProcess(99);r[1]=1;++wine_blit_worker_pending;r[2]=++wine_blit_worker_calls;r[3]=42;r[4]=GetCurrentThreadId();r[5]=GetTickCount();r[7]=wine_lifetime_records;
  r[8]=wine_submission_identity((void*)packet[1],1);r[9]=wine_lifetime_find((void*)packet[1],1);r[10]=wine_submission_identity((void*)packet[9],1);r[11]=wine_lifetime_find((void*)packet[9],1);
  r[12]=packet[2];r[13]=packet[10];copy(r+14,packet+3,24);copy(r+20,packet+11,24);r[26]=packet[17];copy(r+27,packet+18,24);r[33]=packet[24];wine_blit_packet_worker(packet,r[2],2);wine_worker_context_publish(packet,r[2],2);wine_blit_worker_emit(r);}LeaveCriticalSection(journal_lock);
 if(observed)worker_producer_capture(r,1);restore_fp(fp);SetLastError(error);((void(__attribute__((cdecl))*)(void*,const void*))wine_blit_worker_original)(context,packet);
 error=GetLastError();snapshot_fp(fp);if(observed){worker_producer_capture(r,2);wine_gpu_complete(packet,r[2]);EnterCriticalSection(journal_lock);r[1]=2;r[5]=GetTickCount();r[6]=error;wine_worker_context_publish(packet,r[2],3);wine_blit_packet_worker(packet,r[2],3);wine_blit_worker_emit(r);--wine_blit_worker_pending;LeaveCriticalSection(journal_lock);}restore_fp(fp);SetLastError(error);
}
static int wine_blit_worker_open(void){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW_WINE_BLIT_WORKER",flag,2);if(!n)return worker_producer_open(0);if(n!=1||flag[0]!='1')return 0;
 char name[600];path(name,"wine-blit-worker.bin");wine_blit_worker_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(wine_blit_worker_file==(HANDLE)-1)return 0;u32 h[16]={0};copy(h,"MNMWBK01",8);h[2]=1;h[3]=192;h[4]=0x40209ca7;h[5]=131072;h[6]=42;if(!write(wine_blit_worker_file,h,64))return 0;wine_blit_worker_enabled=1;return worker_producer_open(1);
}
static int wine_blit_worker_install(void* module){
 if(!wine_blit_worker_enabled)return 1;void** slot=(void**)((u32)module+0x24d420+42*4);u8* expected=(u8*)module+0x1ce280;const u8 prologue[6]={0x55,0x89,0xe5,0x83,0xec,0x68};
 if(!readable(slot,4)||*slot!=expected||!readable(expected,6)||!equal(expected,prologue,6))return 0;u32 old,unused;if(!VirtualProtect(slot,4,4,&old))return 0;wine_blit_worker_original=*slot;*slot=(void*)&wine_blit_worker;if(!VirtualProtect(slot,4,old,&unused))return 0;return 1;
}
static void wine_blit_worker_close(u32 reason){if(!wine_blit_worker_file)return;worker_producer_close();u32 r[48]={0};r[1]=4;r[2]=reason;r[3]=wine_blit_worker_calls;r[4]=GetCurrentThreadId();r[5]=GetTickCount();wine_blit_worker_emit(r);if(!CloseHandle(wine_blit_worker_file))ExitProcess(99);wine_blit_worker_file=0;}
