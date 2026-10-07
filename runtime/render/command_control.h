/* Persistent administrative worker. Never created/joined in DllMain. It owns
 * no application pointers; exclusive callback admission precedes recovery. */
static int command_application_finish(u32);
static u32* command_control;
static u32 command_control_thread;
static u32 command_control_launch,command_control_stop,command_control_started;
static HANDLE command_control_worker;
static int command_control_identity(void){
    return command_control && same(command_control,MNM_RENDER_CONTROL_V1_MAGIC,8) &&
        command_control[2]==1 && command_control[3]==MNM_RENDER_CONTROL_V1_SIZE &&
        command_control[4]==command_control_launch && !command_control[13] && !command_control[14] && !command_control[15];
}
static void command_control_init(void){
    char path[512];u32 n=GetEnvironmentVariableA("MNM_RENDER_CONTROL",path,sizeof(path));
    if(!n)return;
    command_control_configured=1;command_control_invalid=1;command_gate_enabled=1;
    if(n>=sizeof(path) || !game_session_continuous)return;
    HANDLE f=CreateFileA(path,0xc0000000,3,0,3,0x80,0);
    if(f==(HANDLE)-1)return;
    u32 high=0,size=GetFileSize(f,&high);
    if(high || size!=MNM_RENDER_CONTROL_V1_SIZE){CloseHandle(f);return;}
    HANDLE mapping=CreateFileMappingA(f,0,4,0,size,0);CloseHandle(f);if(!mapping)return;
    command_control=MapViewOfFile(mapping,2,0,0,size);CloseHandle(mapping);
    if(!command_control)return;
    command_control_launch=command_control[4];
    if(!command_control_launch || !command_control_identity() ||
       (command_channel_session && command_control_launch!=command_channel_session))return;
    for(u32 i=5;i<13;++i)if(command_control[i])return;
    for(u32 i=64;i<size;++i)if(((u8*)command_control)[i])return;
    command_control_invalid=0;
}
static int command_observers_quiet(void){
    if(!game_tracker_acquire())return 0;
    int quiet=command_recovery_leases_quiet();game_tracker_release();return quiet;
}
static u32 WIN command_control_run(void* unused){
    (void)unused;u32 sequence=0;command_control_thread=GetCurrentThreadId();
    __atomic_store_n(command_control+12,MNM_RENDER_CONTROL_V1_ONLINE_RUNNING,__ATOMIC_RELEASE);
    while(!__atomic_load_n(&command_control_stop,__ATOMIC_ACQUIRE)){
        if(!command_control_identity() || mnm_ring_load(command_control+10))break;
        u32 request=mnm_ring_load(command_control+5);
        if(request==sequence){Sleep(5);continue;}
        if(command_control[6]==MNM_RENDER_CONTROL_V1_OPERATION_STOP){
            int valid=request==sequence+1 && sequence<=MNM_RENDER_CONTROL_V1_MAX_RECOVERIES &&
                !command_control[7] && !command_control[11];
            for(u32 i=64;valid && i<MNM_RENDER_CONTROL_V1_SIZE;++i)if(((u8*)command_control)[i])valid=0;
            __atomic_thread_fence(__ATOMIC_ACQUIRE);
            if(!command_control_identity() || mnm_ring_load(command_control+5)!=request || mnm_ring_load(command_control+10) || command_control[6]!=MNM_RENDER_CONTROL_V1_OPERATION_STOP)valid=0;
            __atomic_store_n(&command_application_stop_requested,1,__ATOMIC_RELEASE);
            int complete=valid && command_application_finish(3000);
            __atomic_thread_fence(__ATOMIC_ACQUIRE);
            if(!command_control_identity() || mnm_ring_load(command_control+5)!=request || mnm_ring_load(command_control+10) || command_control[6]!=MNM_RENDER_CONTROL_V1_OPERATION_STOP || command_control[7] || command_control[11])complete=0;
            for(u32 i=64;complete && i<MNM_RENDER_CONTROL_V1_SIZE;++i)if(((u8*)command_control)[i])complete=0;
            __atomic_store_n(command_control+9,complete?MNM_RENDER_CONTROL_V1_STATUS_READY:MNM_RENDER_CONTROL_V1_STATUS_REFUSED,__ATOMIC_RELAXED);
            __atomic_store_n(command_control+8,request,__ATOMIC_RELEASE);break;
        }
        char path[512];u32 n=command_control[11],target=command_control[7],operation=command_control[6];
        int valid=request==sequence+1 && sequence<MNM_RENDER_CONTROL_V1_MAX_RECOVERIES &&
            (operation==MNM_RENDER_CONTROL_V1_OPERATION_RECOVER || operation==MNM_RENDER_CONTROL_V1_OPERATION_CHECKPOINT) && target && n && n<sizeof(path) &&
            !((u8*)command_control)[64+n];
        zero(path,sizeof(path));if(valid){copy(path,(u8*)command_control+64,n);for(u32 i=0;i<n;++i)if(!path[i])valid=0;}
        __atomic_thread_fence(__ATOMIC_ACQUIRE);
        if(mnm_ring_load(command_control+5)!=request || !command_control_identity())valid=0;
        int ready=0,exclusive=valid && command_gate_close();
        if(exclusive){
            /* Old ring was cancelled by the host; joining retires worker storage.
             * Explicit shutdown result can be false for a deliberately failed ring. */
            RenderShutdown(0);
#ifdef MNM_RENDER_SELFTEST
            char delay[8];if(GetEnvironmentVariableA("MNM_RENDER_CONTROL_DELAY_FOR_TEST",delay,sizeof(delay)))Sleep(200);
#endif
            if(!__atomic_load_n(&command_application_stop_requested,__ATOMIC_ACQUIRE) && !mnm_ring_load(command_control+10) && !__atomic_load_n(&command_gate_collision,__ATOMIC_ACQUIRE))
                ready=command_recover_mode(path,target,operation==MNM_RENDER_CONTROL_V1_OPERATION_CHECKPOINT?COMMAND_RECOVER_CHECKPOINT:COMMAND_RECOVER_PREFER_CHECKPOINT,1);
            if(__atomic_load_n(&command_application_stop_requested,__ATOMIC_ACQUIRE) || mnm_ring_load(command_control+10) || !command_control_identity() ||
               mnm_ring_load(command_control+5)!=request || command_control[6]!=operation || command_control[7]!=target ||
               command_control[11]!=n || !same(path,(u8*)command_control+64,n+1) ||
               __atomic_load_n(&command_gate_collision,__ATOMIC_ACQUIRE))ready=0;
            if(!ready)RenderShutdown(0);
            /* Bypassed originals must finish before a usable recovery can reply.
             * If one is active, retire the candidate before reopening admission. */
            if(!__sync_bool_compare_and_swap(&command_gate,COMMAND_GATE_EXCLUSIVE,0)){
                ready=0;RenderShutdown(0);__atomic_fetch_and(&command_gate,0x7fffffffu,__ATOMIC_RELEASE);
            }
        }
        /* Collision can complete between the previous check and release CAS.
         * Host has not been told READY and must not poll the new mapping yet. */
        if(exclusive && (__atomic_load_n(&command_gate_collision,__ATOMIC_ACQUIRE) || mnm_ring_load(command_control+10))){ready=0;RenderShutdown(0);}
        sequence=request;
        __atomic_store_n(command_control+9,ready?MNM_RENDER_CONTROL_V1_STATUS_READY:MNM_RENDER_CONTROL_V1_STATUS_REFUSED,__ATOMIC_RELAXED);
        __atomic_store_n(command_control+8,sequence,__ATOMIC_RELEASE);
        if(!valid || !ready)break;
    }
    if(mnm_ring_load(command_control+10)){
        __atomic_store_n(&command_application_stop_requested,1,__ATOMIC_RELEASE);
        command_application_finish(0);
    }
#ifdef MNM_RENDER_SELFTEST
    command_stop_worker_pause_for_test(1);
#endif
    __atomic_store_n(command_control+12,MNM_RENDER_CONTROL_V1_ONLINE_STOPPED,__ATOMIC_RELEASE);return 0;
}
static int command_control_start(void){
    if(__atomic_load_n(&command_application_stop_requested,__ATOMIC_ACQUIRE))return 0;
    if(!command_control_configured)return 1;
    if(command_control_invalid)return 0;
    if(!__sync_bool_compare_and_swap(&command_control_started,0,1))return command_control_worker!=0;
    command_control_worker=CreateThread(0,0,command_control_run,0,0,0);
    if(!command_control_worker)command_queue_refuse(MNM_RENDER_COMMANDS_V2_REASON_INVALID);
    return command_control_worker!=0;
}
static void command_control_detach(void){
    /* Process teardown owns the handle/view. Dynamic DLL unloading unsupported. */
    __atomic_store_n(&command_control_stop,1,__ATOMIC_RELEASE);
}
