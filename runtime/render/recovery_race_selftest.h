API void WIN RenderLifecyclePauseForTest(u32);
static u32 rr_kind,rr_results[4];
static u32 WIN rr_worker(void* unused){
    (void)unused;SetLastError(0x77);
    rr_results[3]=rr_kind==1?RenderStartup():rr_kind==2?RenderShutdown(3000):RenderRecover("commands-00000001.bin");
    if(GetLastError()!=0x77)ExitProcess(490);return 0;
}
static void rr_overlap(u32 phase){
    RenderLifecyclePauseForTest(rr_kind);HANDLE worker=CreateThread(0,0,rr_worker,0,0,0);if(!worker)ExitProcess(491);
    u32 entered=0;for(u32 i=0;i<5000;++i){HANDLE file=CreateFileA("transition-enter.bin",0x80000000,3,0,3,0x80,0);if(file!=(HANDLE)-1){CloseHandle(file);entered=1;break;}Sleep(1);}if(!entered)ExitProcess(501);
    SetLastError(0x77);rr_results[0]=RenderStartup();if(GetLastError()!=0x77)ExitProcess(492);
    rr_results[1]=RenderShutdown(0);if(GetLastError()!=0x77)ExitProcess(493);
    rr_results[2]=RenderRecover(rr_kind==3?"commands-00000002.bin":"commands-00000001.bin");if(GetLastError()!=0x77)ExitProcess(494);
    if(rr_results[0] || rr_results[1] || rr_results[2])ExitProcess(495);
    rc_mark("collision-",phase,rr_kind);rc_wait("collision-go-",phase);pl_file("transition-release.bin",&phase,4);
    if(WaitForSingleObject(worker,3000)!=0 || rr_results[3]!=1)ExitProcess(496);CloseHandle(worker);
}
static void test_recovery_race(const char* mode){
    rr_kind=rs_mode(mode,"startup")?1:rs_mode(mode,"shutdown")?2:3;
    SetLastError(0x77);if(!RenderStartup() || GetLastError()!=0x77)ExitProcess(497);
    cs_setup(&cs_small,4,4,16,0);cs_setup(&cs_big,512,512,32,1);
    for(u32 phase=0;phase<2;++phase){
        if(phase){if(rr_kind==3)rr_overlap(phase);else rc_try("commands-00000001.bin",1);SetLastError(0x77);if(!RenderStartup() || GetLastError()!=0x77)ExitProcess(498);}
        cs_update(&cs_small,0x1200+phase);
        for(u32 f=0;f<2;++f){cs_update(&cs_big,0xa5000000u|((phase*2+f)*0x254713u&0xffffff));Sleep(75);}
        rc_mark("drawn-",phase,phase);rc_wait("go-",phase);
        if(!phase && rr_kind!=3)rr_overlap(phase);
        SetLastError(0x77);if(!RenderShutdown(3000) || GetLastError()!=0x77)ExitProcess(499);
        rc_mark("closed-",phase,phase);rc_wait("next-",phase);
    }
    pl_file("race-results.bin",rr_results,sizeof(rr_results));u32 counts[3]={cs_locks,cs_unlocks,2};pl_file("engine-counts.bin",counts,sizeof(counts));
    SetLastError(0x77);if(!RenderShutdown(0) || RenderStartup()!=0 || GetLastError()!=0x77)ExitProcess(500);ExitProcess(0);
}
