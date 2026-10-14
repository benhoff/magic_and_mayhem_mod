/* Process-lifetime diagnostic worker. It owns no original/tracker pointers and
 * does not share the publication worker's I/O or administrative recovery work.
 * The installed adapter pins its DLL; dynamic unloading remains unsupported. */
API HANDLE WIN CreateThread(void*,u32,u32 (WIN *)(void*),void*,u32,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
static HANDLE lock_diagnostic_worker;
/* 0 absent,1 starting,2 creation failed,3 started,4 joined. */
static u32 lock_diagnostic_started,lock_diagnostic_stop,lock_diagnostic_thread;
static volatile i32 lock_diagnostic_finish_busy;
static u32 WIN lock_diagnostic_worker_run(void* unused){
    (void)unused;
    for(;;){
        lock_diagnostic_drain();
        u32 count=__atomic_load_n(&lock_diagnostic_count,__ATOMIC_ACQUIRE);
        if(__atomic_load_n(&lock_diagnostic_stop,__ATOMIC_ACQUIRE) ||
           __atomic_load_n(&lock_diagnostic_failed,__ATOMIC_ACQUIRE) ||
           (count>=128 && lock_diagnostic_read>=count))break;
        /* Compare immediately before sleeping; producer publication plus wake
         * handles changes between this sample and the address wait. */
        if(lock_diagnostic_read==count)WaitOnAddress(&lock_diagnostic_count,&count,4,1000);
    }
    lock_diagnostic_drain();return 0;
}
static void lock_diagnostic_worker_start(void){
    if(!game_session_continuous || !lock_capture_path_length || __atomic_load_n(&lock_diagnostic_stop,__ATOMIC_ACQUIRE) ||
       !__sync_bool_compare_and_swap(&lock_diagnostic_started,0,1))return;
    u32 saved=GetLastError();
    HANDLE worker=CreateThread(0,0,lock_diagnostic_worker_run,0,0,&lock_diagnostic_thread);
    __atomic_store_n(&lock_diagnostic_worker,worker,__ATOMIC_RELEASE);
    /* Diagnostic creation failure retains the bounded records. Never fall back
     * to callback-side disk I/O or make optional diagnostics fail presentation. */
    __atomic_store_n(&lock_diagnostic_started,worker?3u:2u,__ATOMIC_RELEASE);
    SetLastError(saved);
}
static int lock_diagnostic_worker_finish(u32 milliseconds){
    u32 saved=GetLastError(),thread=GetCurrentThreadId();int joined=0;
    if(!__sync_bool_compare_and_swap(&lock_diagnostic_finish_busy,0,1))return 0;
    /* This is called only on normal application exit, outside DllMain. A
     * reentrant exit must not join while retaining either drawing ownership. */
    if(__atomic_load_n(&game_copy_owner,__ATOMIC_ACQUIRE)==thread ||
       __atomic_load_n(&game_locks_owner,__ATOMIC_ACQUIRE)==thread)goto done;
    __atomic_store_n(&lock_diagnostic_stop,1,__ATOMIC_RELEASE);
    WakeByAddressAll(&lock_diagnostic_count);
    u32 state=__atomic_load_n(&lock_diagnostic_started,__ATOMIC_ACQUIRE);
    HANDLE worker=__atomic_load_n(&lock_diagnostic_worker,__ATOMIC_ACQUIRE);
    if(!worker){joined=state==0 || state==4;goto done;}
    if(state!=3 || lock_diagnostic_thread==thread)goto done;
    if(milliseconds>50)milliseconds=50;
    if(WaitForSingleObject(worker,milliseconds)==0){
        CloseHandle(worker);__atomic_store_n(&lock_diagnostic_worker,0,__ATOMIC_RELEASE);
        __atomic_store_n(&lock_diagnostic_started,4,__ATOMIC_RELEASE);joined=1;
    }
    /* A blocked writer owns only static module storage. A refused join retains
     * its handle/storage for OS process teardown; no free or second writer. */
 done:SetLastError(saved);__sync_lock_release(&lock_diagnostic_finish_busy);return joined;
}
