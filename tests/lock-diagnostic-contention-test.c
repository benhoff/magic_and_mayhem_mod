#define _GNU_SOURCE
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sched.h>
typedef unsigned char u8;typedef uint32_t u32;typedef int32_t i32;typedef void* HANDLE;
#define API
#define WIN
static _Thread_local u32 thread_id=1,last_error=0x77;
static u32 game_session_continuous=1,game_copy_order_enabled=1,game_lock_epoch,game_alias_reset_pending,game_locks_owner;
static u32 history_invalid,misses,io_calls,inside_owner_io,opens,closes,creation_failed,file_failed;
static char lock_capture_path[512]="capture";static u32 lock_capture_path_length=7;
static pthread_mutex_t event_mutex=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t event_condition=PTHREAD_COND_INITIALIZER;
static u32 blocked_io,release_io,stall_io;
static char output[32768];static u32 output_length;
struct TestThread {pthread_t thread;u32 (WIN *fn)(void*);void* argument;};static struct TestThread diagnostic_thread;
static void require(int ok,const char* why){if(!ok){fprintf(stderr,"Diagnostic contention: %s\n",why);exit(1);}}
static u32 GetLastError(void){return last_error;}static void SetLastError(u32 n){last_error=n;}
static u32 GetCurrentThreadId(void){return thread_id;}
static u32 GetTickCount(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return (u32)((uint64_t)t.tv_sec*1000+t.tv_nsec/1000000);}
static void copy(void* a,const void* b,u32 n){memcpy(a,b,n);}
static int same(const void* a,const void* b,u32 n){return !memcmp(a,b,n);}
static u32 failure_hex(char* p,u32 n){static const char h[]="0123456789abcdef";for(u32 i=0;i<8;++i)p[i]=h[(n>>(28-4*i))&15];return 8;}
static void deadline(struct timespec* t,u32 ms){clock_gettime(CLOCK_REALTIME,t);t->tv_sec+=ms/1000;t->tv_nsec+=(ms%1000)*1000000;if(t->tv_nsec>=1000000000){++t->tv_sec;t->tv_nsec-=1000000000;}}
static i32 WaitOnAddress(volatile void* p,void* compare,u32 size,u32 ms){
    require(size==4,"address size");u32 expected;copy(&expected,compare,4);struct timespec until;deadline(&until,ms);
    pthread_mutex_lock(&event_mutex);int status=0;
    while(__atomic_load_n((u32*)p,__ATOMIC_ACQUIRE)==expected && status!=ETIMEDOUT)status=pthread_cond_timedwait(&event_condition,&event_mutex,&until);
    pthread_mutex_unlock(&event_mutex);last_error=0xbb;return status!=ETIMEDOUT;
}
static void WakeByAddressAll(void* p){(void)p;pthread_mutex_lock(&event_mutex);pthread_cond_broadcast(&event_condition);pthread_mutex_unlock(&event_mutex);last_error=0xaa;}
static void Sleep(u32 ms){if(!ms){sched_yield();return;}struct timespec t={ms/1000,(ms%1000)*1000000};nanosleep(&t,0);}
static HANDLE CreateFileA(const char*,u32,u32,void*,u32,u32,HANDLE);
static int write_all(HANDLE,const void*,u32);
static int CloseHandle(HANDLE p){if(p==&diagnostic_thread)++closes;last_error=0xcc;return 1;}
static void* diagnostic_run(void* p){struct TestThread* t=p;thread_id=33;t->fn(t->argument);return 0;}
static HANDLE CreateThread(void* a,u32 b,u32 (WIN *fn)(void*),void* arg,u32 c,u32* id){
    (void)a;(void)b;(void)c;last_error=0xcc;if(creation_failed)return 0;
    diagnostic_thread.fn=fn;diagnostic_thread.argument=arg;if(id)*id=33;
    require(!pthread_create(&diagnostic_thread.thread,0,diagnostic_run,&diagnostic_thread),"create diagnostic thread");return &diagnostic_thread;
}
static u32 WaitForSingleObject(HANDLE p,u32 ms){require(p==&diagnostic_thread,"worker identity");struct timespec until;deadline(&until,ms);int n=pthread_timedjoin_np(diagnostic_thread.thread,0,&until);require(!n || n==ETIMEDOUT,"join result");last_error=0xcc;return n?258:0;}
#ifndef LOCK_DIAGNOSTICS_HEADER
#define LOCK_DIAGNOSTICS_HEADER "../runtime/render/lock_diagnostics.h"
#endif
#include LOCK_DIAGNOSTICS_HEADER
struct GameCopyLease {u32 held,observe;};
static u32* history_current(void){return &history_invalid;}
static void game_pixel_missed(void* p){(void)p;__atomic_add_fetch(&misses,1,__ATOMIC_RELAXED);}
static void game_metadata_invalidate(void){}
static u32 command_channel_session;
static void zero(void* p,u32 n){memset(p,0,n);}
static u32 GetEnvironmentVariableA(const char* name,char* value,u32 size){(void)name;(void)value;(void)size;return 0;}
int WIN QueryPerformanceCounter(unsigned long long* value){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);*value=(unsigned long long)t.tv_sec*1000000000+t.tv_nsec;return 1;}
int WIN QueryPerformanceFrequency(unsigned long long* value){*value=1000000000;return 1;}
#include "../runtime/render/copy_timing.h"
#include "../runtime/render/copy_order.h"
#ifdef MNM_LOCK_DIAGNOSTICS_DEFERRED
#include "../runtime/render/lock_diagnostic_worker.h"
#else
static void lock_diagnostic_drain(void){}
static void lock_diagnostic_worker_start(void){}
static int lock_diagnostic_worker_finish(u32 ms){(void)ms;return 1;}
#endif
static void observe_io(void){__atomic_add_fetch(&io_calls,1,__ATOMIC_RELAXED);if(game_copy_owner==thread_id || game_locks_owner==thread_id)__atomic_add_fetch(&inside_owner_io,1,__ATOMIC_RELAXED);}
static HANDLE CreateFileA(const char* name,u32 a,u32 b,void* c,u32 d,u32 e,HANDLE f){
    (void)a;(void)b;(void)c;(void)d;(void)e;(void)f;require(!strcmp(name,"capture\\lifecycle.log"),"same diagnostic path");observe_io();++opens;last_error=0xdd;return (HANDLE)1;
}
static int write_all(HANDLE p,const void* bytes,u32 n){
    require(p==(HANDLE)1,"file identity");observe_io();last_error=0xee;
    pthread_mutex_lock(&event_mutex);
    if(stall_io && !blocked_io){blocked_io=1;pthread_cond_broadcast(&event_condition);while(!release_io)pthread_cond_wait(&event_condition,&event_mutex);}
    pthread_mutex_unlock(&event_mutex);
    if(file_failed)return 0;
    require(n<=sizeof(output)-output_length,"bounded output");
    copy(output+output_length,bytes,n);output_length+=n;return 1;
}
static u32 original_pixels[300*128],native_pixels[300*128],original_calls[2],callback_errors[2],callback_results[2];
static void* drawing_callback(void* argument){
    u32 which=(u32)(uintptr_t)argument;thread_id=11+which;last_error=0x77;
    struct GameCopyLease lease=game_copy_enter_at((void*)0x1000,242+which);
    if(lease.observe)lock_diagnostic(which?"probe_bltfast_inside":"probe_blt_inside",(void*)0x1000,2,which,0,0,0,0);
    ++original_calls[which];for(u32 i=0;i<300*128;++i)original_pixels[i]=which?0x2222:0x1111;
    callback_results[which]=0x1234+which;last_error=0x88;
    if(lease.observe)copy(native_pixels,original_pixels,sizeof(native_pixels));else game_copy_unobserved((void*)0x1000);
    game_copy_leave(&lease);callback_errors[which]=last_error;return 0;
}
static void stall_case(int baseline){
    stall_io=1;pthread_t first,second;require(!pthread_create(&first,0,drawing_callback,0),"first callback");
#ifdef MNM_LOCK_DIAGNOSTICS_DEFERRED
    require(!pthread_join(first,0),"first callback independent of logger");lock_diagnostic_worker_start();
#endif
    struct timespec until;deadline(&until,5000);pthread_mutex_lock(&event_mutex);
    while(!blocked_io){
        require(pthread_cond_timedwait(&event_condition,&event_mutex,&until)!=ETIMEDOUT,"writer became blocked");
    }
    pthread_mutex_unlock(&event_mutex);
    require(!pthread_create(&second,0,drawing_callback,(void*)1),"second callback");require(!pthread_join(second,0),"second callback completed with stalled logger");
    u32 unsafe=history_invalid!=0;
#ifdef MNM_LOCK_DIAGNOSTICS_DEFERRED
    require(lock_diagnostic_worker_finish(1)==0 && !closes,"blocked join retains worker");
#endif
    pthread_mutex_lock(&event_mutex);release_io=1;pthread_cond_broadcast(&event_condition);pthread_mutex_unlock(&event_mutex);
#ifdef MNM_LOCK_DIAGNOSTICS_DEFERRED
    require(lock_diagnostic_worker_finish(50)==1 && closes==1,"released worker joins");require(lock_diagnostic_worker_finish(0)==1 && closes==1,"join idempotence");
#else
    require(!pthread_join(first,0),"released original logger");
#endif
    require(original_calls[0]==1 && original_calls[1]==1,"original calls exactly once");require(callback_results[0]==0x1234 && callback_results[1]==0x1235,"original result retained");require(callback_errors[0]==0x88 && callback_errors[1]==0x88,"callback LastError retained");
    if(baseline)require(unsafe && misses==2 && inside_owner_io>0,"base slow diagnostic causes drawing-history loss");
    else require(!unsafe && !misses && !inside_owner_io && same(native_pixels,original_pixels,sizeof(native_pixels)),"drawing stays tracked while logging is blocked");
    printf("{\"success\":true,\"unsafe\":%u,\"misses\":%u,\"inside_owner_io\":%u,\"original_once\":true,\"pixel_words\":38400,\"live_validated\":false}\n",unsafe,misses,inside_owner_io);
}
#ifdef MNM_LOCK_DIAGNOSTICS_DEFERRED
static void guards_case(void){
    u32 v[19]={1};char name[41]="owned_name";lock_diagnostic_values(name,v);strcpy(name,"changed");require(!io_calls && last_error==0x77,"producer has no file I/O and preserves error");
    game_copy_owner=1;struct GameCopyLease lease=game_copy_enter_at(0,1);require(!lease.held && lease.observe,"original same-thread reentry");lease=game_copy_bind_enter();require(!lease.held && !lease.observe,"native bind reentry refusal");
    require(!io_calls,"reentry diagnostics remain deferred");require(!lock_diagnostic_worker_finish(0) && !lock_diagnostic_stop,"finish refuses current copy owner");game_copy_owner=0;
    game_locks_owner=1;require(!lock_diagnostic_worker_finish(0) && !lock_diagnostic_stop,"finish refuses current tracker owner");game_locks_owner=0;
    lock_diagnostic_worker_start();require(lock_diagnostic_worker_finish(50)==1,"guard logger joined");require(output_length && strstr(output,"owned_name ") && !strstr(output,"changed "),"producer owns copied reason");require(!inside_owner_io && last_error==0x77,"guard I/O outside locks and error retained");
    puts("{\"success\":true,\"guards\":6,\"live_validated\":false}");
}
static void limits_case(void){
    char names[32][41];u32 v[19]={};
    for(u32 i=0;i<32;++i){snprintf(names[i],sizeof(names[i]),"reason_%u",i);for(u32 j=0;j<5;++j){v[0]=j;lock_diagnostic_values(names[i],v);}}
    require(lock_diagnostic_count==128 && !io_calls,"four records per reason and128-slot cap");lock_diagnostic_worker_start();require(lock_diagnostic_worker_finish(50)==1,"bounded prefix drain");
    u32 lines=0;for(u32 i=0;i<output_length;++i)if(output[i]=='\n')++lines;require(lines==128 && opens==1 && !inside_owner_io,"same bounded text delivery");
    puts("{\"success\":true,\"records\":128,\"live_validated\":false}");
}
static void failure_case(int thread_failure){
    creation_failed=thread_failure;file_failed=!thread_failure;u32 v[19]={1};lock_diagnostic_values("failure",v);lock_diagnostic_worker_start();
    if(thread_failure){require(lock_diagnostic_worker_finish(0)==0 && !io_calls && lock_diagnostic_started==2,"creation failure has no callback fallback");}
    else require(lock_diagnostic_worker_finish(50)==1 && lock_diagnostic_failed && !inside_owner_io,"write failure contained in logger");
    puts("{\"success\":true,\"failure_guard\":true,\"live_validated\":false}");
}
#endif
static void* finite_writer(void* ignored){
    (void)ignored;thread_id=11;u32 v[19]={1};lock_diagnostic_values("finite_first",v);return 0;
}
static void finite_concurrent_case(void){
    game_session_continuous=0;stall_io=1;pthread_t first;
    require(!pthread_create(&first,0,finite_writer,0),"finite writer starts");
    struct timespec until;deadline(&until,5000);pthread_mutex_lock(&event_mutex);
    while(!blocked_io){require(pthread_cond_timedwait(&event_condition,&event_mutex,&until)!=ETIMEDOUT,"finite writer blocked");}
    pthread_mutex_unlock(&event_mutex);
    u32 before=lock_diagnostic_count,v[19]={2};lock_diagnostic_values("finite_second",v);
    require(lock_diagnostic_count==before && last_error==0x77,"finite concurrent record refused without deferred worker");
    pthread_mutex_lock(&event_mutex);release_io=1;pthread_cond_broadcast(&event_condition);pthread_mutex_unlock(&event_mutex);
    require(!pthread_join(first,0),"finite writer joined");
    require(output_length && strstr(output,"finite_first ") && !strstr(output,"finite_second "),"finite delivery matches previous nonwaiting policy");
    puts("{\"success\":true,\"finite_concurrent\":true,\"live_validated\":false}");
}
static void finite_case(void){game_session_continuous=0;u32 v[19]={1};lock_diagnostic_values("finite",v);require(io_calls==2 && opens==1 && output_length && last_error==0x77,"finite synchronous delivery retained");puts("{\"success\":true,\"finite\":true,\"live_validated\":false}");}
int main(int argc,char** argv){require(argc==2,"case required");
    if(!strcmp(argv[1],"stall"))stall_case(0);else if(!strcmp(argv[1],"baseline"))stall_case(1);
#ifdef MNM_LOCK_DIAGNOSTICS_DEFERRED
    else if(!strcmp(argv[1],"guards"))guards_case();else if(!strcmp(argv[1],"limits"))limits_case();else if(!strcmp(argv[1],"thread-failure"))failure_case(1);else if(!strcmp(argv[1],"file-failure"))failure_case(0);
#endif
    else if(!strcmp(argv[1],"finite"))finite_case();else if(!strcmp(argv[1],"finite-concurrent"))finite_concurrent_case();else require(0,"unknown case");
    return 0;
}
