/* Terminal application ownership, separate from recoverable session shutdown. */
static u32 command_application_busy,command_application_complete,command_application_finished;
static int command_application_finish(u32 milliseconds){
    u32 error=GetLastError();
    struct CommandLifecycleLease lifecycle __attribute__((cleanup(command_lifecycle_leave)))=command_lifecycle_enter();
    if(!command_gate_enabled || !lifecycle.held){SetLastError(error);return 0;}
    if(command_application_finished){SetLastError(error);return command_application_complete;}
    if(!__atomic_load_n(&command_application_closed,__ATOMIC_ACQUIRE)){
        if(!command_gate_close()){lock_diagnostic("command_stop_borrowed_or_busy",0,0,0,0,0,0,0);SetLastError(error);return 0;}
        if(!game_tracker_acquire()){__atomic_fetch_and(&command_gate,0x7fffffffu,__ATOMIC_RELEASE);SetLastError(error);return 0;}
        session_finish_owned();
        if(game_session_continuous && !session_started && !__atomic_load_n(&command_queue_end,__ATOMIC_ACQUIRE))command_channel_fail(MNM_RENDER_COMMANDS_V2_REASON_GAP);
        game_session_enabled=0;
        /* Already admitted callbacks have left; future hooks forward originals
         * without observation. Keep the gate count until late bypasses leave. */
        __atomic_store_n(&command_application_closed,1,__ATOMIC_RELEASE);game_tracker_release();
        if(!__sync_bool_compare_and_swap(&command_gate,COMMAND_GATE_EXCLUSIVE,0)){
            command_queue_refuse(MNM_RENDER_COMMANDS_V2_REASON_INVALID);__atomic_fetch_and(&command_gate,0x7fffffffu,__ATOMIC_RELEASE);
        }
        if(__atomic_load_n(&command_gate_collision,__ATOMIC_ACQUIRE))command_queue_refuse(MNM_RENDER_COMMANDS_V2_REASON_INVALID);
    }
    int complete=command_scheduler_shutdown_locked(milliseconds);
    /* A failed join retains all worker-visible storage. A bypass still in flight
     * can own a borrowed interval; do not free its bookkeeping or owned pixels. */
    if(!command_worker_joined || __atomic_load_n(&command_gate,__ATOMIC_ACQUIRE) || !command_borrows_quiet() || !game_tracker_acquire()){SetLastError(error);return 0;}
    for(u32 i=0;i<32;++i)game_lock_clear(game_locks+i);
    for(u32 i=0;i<GAME_SURFACE_COUNT;++i){game_surface_drop(game_surfaces+i);zero(game_surfaces+i,sizeof(game_surfaces[i]));}
    zero(game_palettes,sizeof(game_palettes));zero(game_aliases,sizeof(game_aliases));session_archive_close();
    history_finish();
    command_application_complete=complete;command_application_finished=1;game_tracker_release();
    lock_diagnostic("command_application_stopped",0,0,complete,game_surface_bytes,lock_capture_reserved,0,0);
    SetLastError(error);return complete;
}
__declspec(dllexport) u32 WIN RenderStop(u32 milliseconds){
    u32 error=GetLastError();if(milliseconds>5000)milliseconds=5000;
    __atomic_store_n(&command_application_stop_requested,1,__ATOMIC_RELEASE);
    __atomic_store_n(&command_control_stop,1,__ATOMIC_RELEASE);
    if(!__sync_bool_compare_and_swap(&command_application_busy,0,1)){SetLastError(error);return 0;}
    int complete=0;
    /* Never hold lifecycle/tracker/gate while joining the administrative worker:
     * it may be finishing a recovery or its own terminal publication cleanup. */
    if(command_control_worker){
        u32 wait=milliseconds<1000?milliseconds:1000;
        u32 joined=command_control_thread==GetCurrentThreadId()?0xffffffffu:WaitForSingleObject(command_control_worker,wait);
        if(joined!=0){lock_diagnostic("command_control_join_refused",0,0,joined,GetLastError(),0,wait,0);goto done;}
        CloseHandle(command_control_worker);command_control_worker=0;
    }
    complete=command_application_finish(milliseconds);
    if(command_application_finished && command_control){UnmapViewOfFile(command_control);command_control=0;}
 done:__sync_lock_release(&command_application_busy);SetLastError(error);return complete;
}
#ifdef MNM_RENDER_SELFTEST
__declspec(dllexport) u32 WIN RenderStopStateForTest(u32* out){
    u32 error=GetLastError();u32 state[10]={command_application_stop_requested,command_application_closed,command_application_finished,
        command_control_worker!=0,command_control!=0,command_worker!=0,command_queue!=0,command_channel!=0,game_surface_bytes,lock_capture_reserved};
    copy(out,state,sizeof(state));SetLastError(error);return 1;
}
#endif
