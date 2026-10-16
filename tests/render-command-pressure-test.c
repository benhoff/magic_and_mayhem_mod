#define QUEUE_TEST_MAIN queue_regressions
#include "render-command-queue-test.c"
#undef main

static u32 game_resident_world=1,game_copy_order_enabled=1,game_copy_owner,tracker_owner;
static u32 game_lock_epoch,game_alias_reset_pending,history_invalid,game_cpu_transfer_wait,copy_trace_enabled;
static u32 sleeps,mode,consumed,logged_begin,logged_resume,misses;
static struct mnm_ring_reader pressure_reader;
static u8 received[65536];
static u32 GetCurrentThreadId(void){return 17;}
static u32* history_current(void){return &history_invalid;}
static void game_pixel_missed(void* p){(void)p;++misses;}
static void game_metadata_invalidate(void){}
static void lock_diagnostic(const char* name,void* p,u32 k,u32 a,u32 b,i32 c,u32 d,const u32* e){
    (void)p;(void)k;(void)a;(void)b;(void)c;(void)d;(void)e;
    if(!strcmp(name,"command_pressure_begin"))++logged_begin;
    if(!strcmp(name,"command_pressure_resume"))++logged_resume;
    last_error=0xab;
}
static void lock_diagnostic_values(const char* n,const u32* v){(void)n;(void)v;}
struct CopyWaitTiming {u32 dummy;};
#define copy_wait_begin(a,b) ((void)(a),(void)(b))
#define copy_wait_sample(a,b) ((void)(a),(void)(b))
#define copy_wait_timeout(a,b,c,d,e) ((void)(a),(void)(b),(void)(c),(void)(d),(void)(e))
#define copy_wait_before(a) ((void)(a))
#define copy_wait_after(a,b,c) ((void)(a),(void)(b),(void)(c))
#define copy_trace_owner_open(a,b,c,d) ((void)(a),(void)(b),(void)(c),(void)(d))
#define copy_trace_owner_close() ((void)0)
#define copy_trace_owner_phase(a) ((void)(a))
struct GameCopyLease {u32 held,observe,deferred;};
static void Sleep(u32);
static i32 WaitOnAddress(volatile void* p,void* c,u32 n,u32 ms){(void)p;(void)c;(void)n;(void)ms;clock_ms+=51;return 0;}
static void WakeByAddressAll(void* p){(void)p;last_error=0xaa;}
#include "../runtime/render/command_pressure.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#include "../runtime/render/copy_order.h"
#pragma GCC diagnostic pop
static void read_available(void){
    for(;;){int n=mnm_ring_read(&pressure_reader,received,sizeof(received));check(n>=0);
        if(!n)break;
        for(int i=0;i<n;++i)check(received[i]==(u8)((consumed+(u32)i)*13+31));
        consumed+=(u32)n;
    }
}
static void Sleep(u32 ms){
    /* The simulated independent worker/reader can run only outside both
     * admission and tracker ownership. Every byte has an independent oracle. */
    check(!game_copy_owner && !tracker_owner);++sleeps;clock_ms+=ms;last_error=0xbb;
    if(mode==1){command_channel_pump();read_available();}
    else if(mode==2){storage[8]=1;command_channel_pump();}
    else if(mode==3){storage[9]=storage[5]+1;command_channel_pump();}
    else if(mode==4)clock_ms+=5000;
}
static void pressure_reset(void){
    ring_reset();command_publication_worker_active=1;
    command_pressure_waits=command_pressure_resumes=command_pressure_max_ms=0;
    game_resident_world=game_copy_order_enabled=1;game_copy_owner=tracker_owner=0;
    sleeps=mode=consumed=logged_begin=logged_resume=misses=0;
    game_copy_pressure_admit=command_pressure_admit;game_copy_native_defer=0;
    check(mnm_ring_reader_bind(&pressure_reader,storage,mapped_size));
}
static void append_callback(u8* input,u32 count,u32 offset){
    last_error=0x77;struct GameCopyLease lease=game_copy_enter_at(0,123);
    check(lease.held&&lease.observe&&last_error==0x77);
    tracker_owner=17;for(u32 i=0;i<count;++i)input[i]=(u8)((offset+i)*13+31);
    check(command_channel_append(input,count,0,0,0,0));memset(input,0xa5,count);
    tracker_owner=0;last_error=0x88;game_copy_leave(&lease);check(last_error==0x88&&!game_copy_owner);
}
int main(void){
    queue_regressions();u32 packet=1024*1024+123;u8* input=malloc(COMMAND_QUEUE_CAPACITY);check(input!=0);
    /* Identical unpaced producer/refusing reader schedule demonstrates the
     * old overload before the repaired schedule transfers >32MiB losslessly. */
    pressure_reset();game_copy_pressure_admit=0;u32 baseline=0;
    while(command_channel_append(input,packet,0,0,0,0))++baseline;
    check(baseline<40 && command_queue_failure==MNM_RENDER_COMMANDS_V2_REASON_OVERFLOW);command_channel_close();
    pressure_reset();mode=1;
    for(u32 i=0;i<80;++i)append_callback(input,packet,i*packet);
    check(command_pressure_waits>0&&command_pressure_resumes==command_pressure_waits&&logged_begin==logged_resume);
    check(command_queue_peak<COMMAND_PRESSURE_HIGH+packet&&sleeps>0&&!command_queue_failure);
    command_channel_end();while(consumed<80*packet){command_channel_pump();read_available();}
    command_channel_pump();read_available();check(storage[6]==2&&consumed==80*packet);command_channel_close();
    /* A pressure wait must observe sticky cancellation/invalid peer reasons,
     * bounded expiry, clock wrap and existing same-thread reentry policy. */
    for(u32 variant=0;variant<4;++variant){
        pressure_reset();mode=variant==0?2:variant==1?3:4;
        if(variant==3)clock_ms=0xffffff00u;
        check(command_channel_append(input,COMMAND_PRESSURE_HIGH+1,0,0,0,0));
        last_error=0x77;struct GameCopyLease lease=game_copy_enter_at(0,123);
        check(!lease.held&&!lease.observe&&!game_copy_owner&&last_error==0x77);
        check(command_queue_failure==(variant==0?3u:variant==1?5u:4u));
        game_copy_leave(&lease);command_channel_close();
    }
    pressure_reset();check(command_channel_append(input,COMMAND_PRESSURE_HIGH,0,0,0,0));
    game_copy_owner=17;last_error=0x77;struct GameCopyLease nested=game_copy_enter_at(0,123);
    check(!nested.held&&nested.observe&&!sleeps&&last_error==0x77);
    nested=game_copy_bind_enter();check(!nested.held&&!nested.observe&&!sleeps);game_copy_owner=0;command_channel_close();
    for(u32 variant=0;variant<3;++variant){
        pressure_reset();if(variant==0)game_resident_world=0;else if(variant==1)game_copy_order_enabled=0;else command_publication_worker_active=0;
        check(command_channel_append(input,COMMAND_PRESSURE_HIGH,0,0,0,0));
        last_error=0x77;struct GameCopyLease lease=game_copy_enter_at(0,123);check(lease.observe&&!sleeps&&last_error==0x77);
        game_copy_leave(&lease);command_channel_close();
    }
    pressure_reset();check(command_pressure_admit(0,0));command_channel_end();check(!command_pressure_admit(1,0)&&!sleeps);command_channel_close();
    (void)game_copy_observation_enter_at;(void)game_copy_stage;(void)game_copy_unobserved;
    (void)game_lifetime_invalidate;(void)game_copy_shutdown_enter;
    (void)command_callback_missed;
    free(input);printf("{\"success\":true,\"pressure_cases\":12,\"baseline_packets\":%u,\"owned_bytes\":%u}\n",baseline,80*packet);return 0;
}
