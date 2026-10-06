/* Start/join only from normal application callbacks/exports, never DllMain.
 * Worker owns transport retries only, not tracker or original surface pointers. */
API HANDLE WIN CreateThread(void*,u32,u32 (WIN *)(void*),void*,u32,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
static HANDLE command_worker;
static u32 command_worker_started,command_worker_stop,command_worker_joined,command_shutdown_busy,command_shutdown_complete;
static u32 command_pressure_logged;
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
    command_channel_pump();return 0;
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
            if(mnm_ring_load(command_ring.map+6)>=2)break;
            if(GetTickCount()-start>=milliseconds){command_queue_refuse(MNM_RENDER_COMMANDS_V2_REASON_INTERRUPTED);command_channel_pump();break;}
            Sleep(1);
        }while(1);
    }
    __atomic_store_n(&command_worker_stop,1,__ATOMIC_RELEASE);
    if(command_worker){
        /* Retry work is bounded; a timed-out join retains mapping/queue. */
        if(WaitForSingleObject(command_worker,1000)!=0)return 0;
        CloseHandle(command_worker);command_worker=0;
    }
    command_worker_joined=1;
    command_shutdown_complete=!command_channel_refused && (!command_channel || (mnm_ring_load(command_channel+6)==MNM_RENDER_COMMANDS_V2_STATE_ENDED &&
        !__atomic_load_n(&command_queue_failure,__ATOMIC_ACQUIRE)));
    command_channel_close();return command_shutdown_complete;
}
/* Process teardown cannot join. Retain worker-visible storage until OS cleanup.
 * Manual unloading while installed hook callbacks remain possible is unsupported. */
static void command_scheduler_detach(void){
    __atomic_store_n(&command_worker_stop,1,__ATOMIC_RELEASE);
    if(command_worker_started && !command_worker_joined && command_queue){
        if(mnm_ring_load(command_ring.map+6)==1){
            command_queue_refuse(MNM_RENDER_COMMANDS_V2_REASON_INTERRUPTED);
            command_channel_pump();
        }
        return;
    }
    command_channel_close();
}
