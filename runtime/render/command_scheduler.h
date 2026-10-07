/* Start/join only from normal application callbacks/exports, never DllMain.
 * Worker owns transport retries only, not tracker or original surface pointers. */
API HANDLE WIN CreateThread(void*,u32,u32 (WIN *)(void*),void*,u32,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
static HANDLE command_worker;
static u32 command_worker_started,command_worker_stop,command_worker_joined,command_shutdown_busy,command_shutdown_complete;
static u32 command_pressure_logged;
#ifdef MNM_RENDER_SELFTEST
static void command_stop_worker_pause_for_test(u32 kind){
    char value[8];u32 n=GetEnvironmentVariableA("MNM_RENDER_STOP_JOIN_FOR_TEST",value,sizeof(value));
    if(n!=1 || value[0]!=(kind==1?'c':'p'))return;
    char entered[32],release[32];zero(entered,sizeof(entered));zero(release,sizeof(release));
    copy(entered,kind==1?"control-join-enter.bin":"publish-join-enter.bin",22);
    copy(release,kind==1?"control-join-go.bin":"publish-join-go.bin",19);
    u32 written,start=GetTickCount();HANDLE file=CreateFileA(entered,0x40000000,1,0,1,0x80,0);
    if(file!=(HANDLE)-1){WriteFile(file,&kind,4,&written,0);CloseHandle(file);}
    while(GetTickCount()-start<10000){file=CreateFileA(release,0x80000000,3,0,3,0x80,0);if(file!=(HANDLE)-1){CloseHandle(file);break;}Sleep(1);}
}
#endif
static u32 WIN command_worker_run(void* unused){
    (void)unused;
    while(!__atomic_load_n(&command_worker_stop,__ATOMIC_ACQUIRE)){
        command_channel_pump();
        u32 failure=__atomic_load_n(&command_queue_failure,__ATOMIC_ACQUIRE);
        if(failure && !command_pressure_logged){
            command_pressure_logged=1;
            lock_diagnostic(__atomic_load_n(&command_queue_timed_out,__ATOMIC_ACQUIRE)?"command_queue_stalled":"command_queue_refused",0,0,failure,
                __atomic_load_n(&command_queue_peak,__ATOMIC_RELAXED),0,__atomic_load_n(&command_queue_full,__ATOMIC_RELAXED),0);
        }
        if(mnm_ring_load(command_ring.map+6)>=2 || failure)break;
        Sleep(__atomic_load_n(&command_queue_read,__ATOMIC_ACQUIRE)!=__atomic_load_n(&command_queue_written,__ATOMIC_ACQUIRE) &&
              !__atomic_load_n(&command_queue_blocked,__ATOMIC_ACQUIRE)?1:10);
    }
    command_channel_pump();
#ifdef MNM_RENDER_SELFTEST
    command_stop_worker_pause_for_test(2);
#endif
    return 0;
}
/* Caller owns lifecycle serialization; no tracker/original pointers here. */
static int command_scheduler_launch(void){
    command_worker_started=1;
#ifdef MNM_RENDER_SELFTEST
    char fault[16];u32 n=GetEnvironmentVariableA("MNM_RENDER_RECOVERY_FAULT_FOR_TEST",fault,sizeof(fault));
    if(command_file_count>1 && n==6 && same(fault,"worker",6)){command_queue_refuse(MNM_RENDER_COMMANDS_V2_REASON_INVALID);command_channel_pump();return 0;}
#endif
    command_worker=CreateThread(0,0,command_worker_run,0,0,0);
    if(!command_worker){command_queue_refuse(MNM_RENDER_COMMANDS_V2_REASON_INVALID);command_channel_pump();}
    int ready=command_worker && !__atomic_load_n(&command_queue_failure,__ATOMIC_ACQUIRE);
    return ready;
}
struct CommandLifecycleLease {u32 held;};
static struct CommandLifecycleLease command_lifecycle_enter(void){
    struct CommandLifecycleLease lease={(u32)__sync_bool_compare_and_swap(&command_shutdown_busy,0,1)};return lease;
}
static void command_lifecycle_leave(struct CommandLifecycleLease* lease){
    if(lease->held)__sync_lock_release(&command_shutdown_busy);
}
#ifdef MNM_RENDER_SELFTEST
/* One-shot fixture barrier inside actual transition ownership, before mutation.
 * No production export, pixel state or additional production waits. */
static u32 command_lifecycle_pause;
static int command_lifecycle_pause_for_test(u32 phase){
    if(!__sync_bool_compare_and_swap(&command_lifecycle_pause,phase,0))return 1;
    u32 error=GetLastError(),written,start=GetTickCount();
    HANDLE file=CreateFileA("transition-enter.bin",0x40000000,1,0,1,0x80,0);
    int ready=file!=(HANDLE)-1 && WriteFile(file,&phase,4,&written,0) && written==4;
    if(file!=(HANDLE)-1)CloseHandle(file);
    while(ready && GetTickCount()-start<5000){
        file=CreateFileA("transition-release.bin",0x80000000,3,0,3,0x80,0);
        if(file!=(HANDLE)-1){CloseHandle(file);SetLastError(error);return 1;}Sleep(1);
    }
    SetLastError(error);return 0;
}
#define COMMAND_LIFECYCLE_PAUSE(phase) do{if(!command_lifecycle_pause_for_test(phase)){SetLastError(error);return 0;}}while(0)
#else
#define COMMAND_LIFECYCLE_PAUSE(phase) do{}while(0)
#endif
/* Startup, shutdown and recovery serialize their entire state transition. */
static int command_scheduler_start_locked(void){
    if(command_worker_joined)return 0;
    if(command_worker_started || !command_queue)return !command_queue || (command_worker && !__atomic_load_n(&command_queue_failure,__ATOMIC_ACQUIRE));
    return command_scheduler_launch();
}
#ifdef MNM_RENDER_SELFTEST
static int command_scheduler_start(void){
    struct CommandLifecycleLease lease __attribute__((cleanup(command_lifecycle_leave)))=command_lifecycle_enter();
    return lease.held?command_scheduler_start_locked():0;
}
#endif
/* Producers must already be stopped under the tracker. ACK is owned-copy
 * completion, not GPU completion. Deadline does not wait under the tracker. */
static int command_scheduler_shutdown_locked(u32 milliseconds){
    if(command_worker_joined)return command_shutdown_complete;
    if(milliseconds>5000)milliseconds=5000;
    u32 start=GetTickCount();
    if(command_queue){
        do {
            command_channel_pump();
            if(__atomic_load_n(&command_application_stop_requested,__ATOMIC_ACQUIRE) &&
               (!mnm_ring_identity(command_ring.map,command_ring.session) || mnm_ring_load(command_ring.map+8))){command_queue_refuse(MNM_RENDER_COMMANDS_V2_REASON_INTERRUPTED);break;}
            u32 state=mnm_ring_load(command_ring.map+6);
            if(state>=2){
                if(state!=MNM_RENDER_COMMANDS_V2_STATE_ENDED || !__atomic_load_n(&command_application_stop_requested,__ATOMIC_ACQUIRE))break;
                u32 ack=mnm_ring_load(command_ring.map+9);
                if(ack==command_ring.published)break;
                if(ack>command_ring.published || ack<command_ring.acknowledged){command_queue_refuse(MNM_RENDER_COMMANDS_V2_REASON_INVALID);break;}
            }
            if(GetTickCount()-start>=milliseconds){command_queue_refuse(MNM_RENDER_COMMANDS_V2_REASON_INTERRUPTED);command_channel_pump();break;}
            Sleep(1);
        }while(1);
    }
    __atomic_store_n(&command_worker_stop,1,__ATOMIC_RELEASE);
    if(command_worker){
        /* Retry work is bounded; a timed-out join retains mapping/queue. */
        u32 joined=WaitForSingleObject(command_worker,1000);
        if(joined!=0){lock_diagnostic("command_publication_join_refused",0,0,joined,GetLastError(),0,1000,0);return 0;}
        CloseHandle(command_worker);command_worker=0;
    }
    command_worker_joined=1;
    command_shutdown_complete=!command_channel_refused && (!command_channel || (mnm_ring_load(command_channel+6)==MNM_RENDER_COMMANDS_V2_STATE_ENDED &&
        !__atomic_load_n(&command_queue_failure,__ATOMIC_ACQUIRE)));
    if(command_channel && command_queue && __atomic_load_n(&command_application_stop_requested,__ATOMIC_ACQUIRE) &&
       (!mnm_ring_identity(command_channel,command_ring.session) || mnm_ring_load(command_channel+8) ||
        mnm_ring_load(command_channel+5)!=command_ring.published || mnm_ring_load(command_channel+9)!=command_ring.published))command_shutdown_complete=0;
    command_channel_close();return command_shutdown_complete;
}
/* Process teardown cannot join. Retain worker-visible storage until OS cleanup.
 * Manual unloading while installed hook callbacks remain possible is unsupported. */
static void command_scheduler_detach(void){
    __atomic_store_n(&command_worker_stop,1,__ATOMIC_RELEASE);
    if(command_worker_started && !command_worker_joined && command_queue){
        if(mnm_ring_load(command_ring.map+6)==1){
            command_queue_refuse(MNM_RENDER_COMMANDS_V2_REASON_INTERRUPTED);
        }
        return;
    }
    /* Process teardown owns views/heap and handles. No pump, wait or free
     * under loader lock, including when a worker did not join. */
}
