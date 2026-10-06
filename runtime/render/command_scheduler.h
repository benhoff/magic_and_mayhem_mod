/* Start/join only from normal application callbacks/exports, never DllMain.
 * Worker owns transport retries only, not tracker or original surface pointers. */
API HANDLE WIN CreateThread(void*,u32,u32 (WIN *)(void*),void*,u32,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
static HANDLE command_worker;
static u32 command_worker_started,command_worker_stop,command_worker_joined,command_shutdown_busy,command_shutdown_complete;
static u32 WIN command_worker_run(void* unused){
    (void)unused;
    while(!__atomic_load_n(&command_worker_stop,__ATOMIC_ACQUIRE)){
        command_channel_pump();
        if(mnm_ring_load(command_ring.map+6)>=2)break;
        Sleep(10);
    }
    command_channel_pump();return 0;
}
static void command_scheduler_start(void){
    if(!__sync_bool_compare_and_swap(&command_shutdown_busy,0,1))return;
    /* Serialize startup with explicit shutdown, including handle publication. */
    if(command_worker_joined || command_worker_started || !command_queue){__sync_lock_release(&command_shutdown_busy);return;}
    command_worker_started=1;command_worker=CreateThread(0,0,command_worker_run,0,0,0);
    if(!command_worker){command_queue_refuse(MNM_RENDER_COMMANDS_V2_REASON_INVALID);command_channel_pump();}
    __sync_lock_release(&command_shutdown_busy);
}
/* Producers must already be stopped under the tracker. ACK is owned-copy
 * completion, not GPU completion. Deadline does not wait under the tracker. */
static int command_scheduler_shutdown(u32 milliseconds){
    if(!__sync_bool_compare_and_swap(&command_shutdown_busy,0,1))return 0;
    if(command_worker_joined){__sync_lock_release(&command_shutdown_busy);return command_shutdown_complete;}
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
        if(WaitForSingleObject(command_worker,1000)!=0){__sync_lock_release(&command_shutdown_busy);return 0;}
        CloseHandle(command_worker);command_worker=0;
    }
    command_worker_joined=1;
    command_shutdown_complete=!command_channel || mnm_ring_load(command_channel+6)==MNM_RENDER_COMMANDS_V2_STATE_ENDED;
    command_channel_close();__sync_lock_release(&command_shutdown_busy);return command_shutdown_complete;
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
