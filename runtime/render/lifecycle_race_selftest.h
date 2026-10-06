API void WIN RenderLifecycleGuardForTest(u32);
static u32 lr_guard,lr_timeout,lr_counts[3];
static u32 WIN lr_worker(void* unused){
    (void)unused;u32 start=GetTickCount();
    if(!lr_guard)while(!__atomic_load_n(&rs_gate_ready,__ATOMIC_ACQUIRE)){if(GetTickCount()-start>2000)ExitProcess(470);Sleep(0);}
    __atomic_store_n(&rs_gate_arrived,1,__ATOMIC_RELEASE);SetLastError(0x77);lr_counts[0]=RenderStartup();if(GetLastError()!=0x77)ExitProcess(471);
    lr_counts[1]=RenderShutdown(1000);if(GetLastError()!=0x77)ExitProcess(472);
    lr_counts[2]=RenderStartup();if(GetLastError()!=0x77)ExitProcess(473);
    __atomic_store_n(&rs_gate_original,1,__ATOMIC_RELEASE);__atomic_store_n(&rs_gate_done,1,__ATOMIC_RELEASE);return 0;
}
static void test_lifecycle_race(const char* mode){
    for(u32 i=0;i<2;++i){ds_tables[i][6]=(void*)&ds_surface;ds_draws[i]=ds_tables[i];}
    ds_call(23,0);ds_make_surface();ds_update(0xa5123456);
    lr_guard=rs_mode(mode,"guard") || rs_mode(mode,"guard-off");lr_timeout=rs_mode(mode,"timeout");
    if(rs_mode(mode,"reentrant") || rs_mode(mode,"completed")){ds_reentrant_shutdown=rs_mode(mode,"reentrant");ds_update(0xa5654321);if(ds_reentrant_shutdown)ExitProcess(474);}
    else{
        if(lr_guard)RenderLifecycleGuardForTest(1);else{rs_gate_kind=5;rs_gate_timeout=lr_timeout;}
        HANDLE worker=CreateThread(0,0,lr_worker,0,0,0);if(!worker)ExitProcess(475);
        if(!lr_guard)ds_update(0xa5654321);
        if(WaitForSingleObject(worker,3000)!=0)ExitProcess(476);CloseHandle(worker);rs_gate_kind=0;
        if(lr_guard){RenderLifecycleGuardForTest(0);ds_update(0xa5654321);}
        if(lr_counts[0]!=(lr_guard?0u:1u) || lr_counts[1]!=(lr_guard || lr_timeout?0u:1u) || lr_counts[2]!=(lr_timeout?1u:0u))ExitProcess(477);
    }
    if(rs_mode(mode,"completed")){SetLastError(0x77);if(!RenderShutdown(3000) || GetLastError()!=0x77)ExitProcess(479);ds_reentrant_expected=ds_reentrant_shutdown=1;ds_update(0xa5fedcba);if(ds_reentrant_shutdown)ExitProcess(480);}
    rs_drop(&rs_primary,0);u32 counts[6]={cs_locks,cs_unlocks,rs_queries,rs_releases,rs_dc_calls,rs_frames};pl_file("resource-counts.bin",counts,sizeof(counts));
    pl_file("race-counts.bin",lr_counts,sizeof(lr_counts));SetLastError(0x77);
    if(!RenderShutdown(3000) || GetLastError()!=0x77 || !RenderShutdown(0) || RenderStartup()!=0 || GetLastError()!=0x77)ExitProcess(478);ExitProcess(0);
}
