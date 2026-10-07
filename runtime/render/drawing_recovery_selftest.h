/* Real callback and borrowing boundaries, independent original fixture pixels. */
static u32 dr_results[4],dr_reentrant,dr_dc_held,dr_dc_calls,dr_release_fail;
static HANDLE dr_dc,dr_bitmap,dr_old;
static i32 WIN dr_unlock(void* object,void* argument){
    if(dr_reentrant){dr_reentrant=0;rc_try("commands-00000001.bin",0);++dr_results[1];}
    return cs_unlock(object,argument);
}
static i32 WIN dr_get_dc(void* object,void** out){
    (void)object;if(GetLastError()!=0x77 || dr_dc_held)ExitProcess(510);
    ++dr_dc_calls;dr_dc_held=1;*out=dr_dc;SetLastError(0x88);return 23;
}
static i32 WIN dr_release_dc(void* object,void* handle){
    (void)object;if(GetLastError()!=0x77 || !dr_dc_held || handle!=dr_dc)ExitProcess(511);
    ++dr_dc_calls;SetLastError(0x88);if(dr_release_fail){dr_release_fail=0;return -1;}dr_dc_held=0;return 23;
}
static void dr_borrow(u32 dc){
    SetLastError(0x77);if(dc){void* out=0;if(((i32 (WIN *)(void*,void**))cs_small.table[17])(&cs_small,&out)!=23 || out!=dr_dc)ExitProcess(512);}
    else{u32 desc[31]={124};if(((i32 (WIN *)(void*,void*,u32*,u32,HANDLE))cs_small.table[25])(&cs_small,0,desc,1,0)!=13)ExitProcess(513);}
    if(GetLastError()!=0x88)ExitProcess(514);
}

static void dr_return(u32 dc,u32 failed){
    SetLastError(0x77);if(dc){dr_release_fail=failed;if(((i32 (WIN *)(void*,void*))cs_small.table[26])(&cs_small,dr_dc)!=(failed?-1:23))ExitProcess(515);}
    else if(((i32 (WIN *)(void*,void*))cs_small.table[32])(&cs_small,0)!=19)ExitProcess(516);
    if(GetLastError()!=0x88)ExitProcess(517);
}
static void dr_wait_callback(void){rc_wait("callback-go-",0);}
static u32 WIN dr_draw(void* unused){(void)unused;cs_update(&cs_small,0x4567);return 0;}
static u32 WIN dr_recover(void* unused){(void)unused;SetLastError(0x77);dr_results[0]=RenderRecover("commands-00000001.bin");if(GetLastError()!=0x77)ExitProcess(518);return 0;}
static void test_drawing_recovery(const char* mode){
    u32 dc=rs_mode(mode,"dc") || rs_mode(mode,"late-dc"),late=rs_mode(mode,"late-lock") || rs_mode(mode,"late-lock-off") || rs_mode(mode,"late-dc"),claimed=rs_mode(mode,"claimed");
    SetLastError(0x77);if(!RenderStartup() || GetLastError()!=0x77)ExitProcess(519);
    /* Supply the original methods before installing the common fixture table. */
    static void* table[33];table[25]=(void*)&cs_lock;table[32]=(void*)&dr_unlock;table[17]=(void*)&dr_get_dc;table[26]=(void*)&dr_release_dc;
    cs_setup(&cs_small,4,4,16,0);cs_setup(&cs_big,512,512,32,1);cs_small.table=table;RenderInstallForTest(&cs_small,14);
    if(dc){u32 info[10]={40,4,(u32)-4,0x00100001,0};void* bits=0;dr_dc=CreateCompatibleDC(0);dr_bitmap=CreateDIBSection(dr_dc,info,0,&bits,0,0);dr_old=SelectObject(dr_dc,dr_bitmap);if(!dr_dc || !dr_bitmap || !bits)ExitProcess(520);}
    for(u32 phase=0;phase<2;++phase){
        cs_update(&cs_small,0x1200+phase);for(u32 f=0;f<2;++f){cs_update(&cs_big,0xa5000000u|((phase*2+f)*0x254713u&0xffffff));Sleep(75);}
        rc_mark("drawn-",phase,phase);rc_wait("go-",phase);SetLastError(0x77);if(!RenderShutdown(3000) || GetLastError()!=0x77)ExitProcess(521);
        rc_mark("closed-",phase,phase);rc_wait("next-",phase);
        if(phase)break;
        HANDLE worker=0;
        if(late || claimed){
            RenderLifecyclePauseForTest(claimed?5:4);worker=CreateThread(0,0,dr_recover,0,0,0);if(!worker)ExitProcess(522);
            rc_wait("transition-enter-",0); /* Host translates the transition rendezvous. */
            if(claimed)cs_update(&cs_small,0x4567);else dr_borrow(dc);
            pl_file("transition-release.bin",&phase,4);
            if(WaitForSingleObject(worker,3000)!=0 || dr_results[0])ExitProcess(523);CloseHandle(worker);
        }else if(rs_mode(mode,"callback") || rs_mode(mode,"callback-off") || rs_mode(mode,"callback-short")){
            cs_delay=rs_mode(mode,"callback-short")?25:1;cs_wait_release=rs_mode(mode,"callback-short")?0:dr_wait_callback;worker=CreateThread(0,0,dr_draw,0,0,0);if(!worker)ExitProcess(524);rc_wait("callback-entered-",0);rc_try("commands-00000001.bin",rs_mode(mode,"callback-short"));
        }else if(rs_mode(mode,"reentrant")){dr_reentrant=1;cs_update(&cs_small,0x4567);if(dr_reentrant || dr_results[1]!=1)ExitProcess(525);}
        else{dr_borrow(dc);rc_try("commands-00000001.bin",0);}
        rc_mark("collision-",0,claimed);rc_wait("collision-go-",0);
        if(worker && !(late || claimed)){rc_mark("callback-go-",0,0);if(WaitForSingleObject(worker,3000)!=0)ExitProcess(526);CloseHandle(worker);}
        if(late || rs_mode(mode,"held") || rs_mode(mode,"dc")){
            /* Bypassed borrows stay pinned after callback completion. Failed DC
             * return does not resolve ownership; successful original return does. */
            rc_try("commands-00000001.bin",0);++dr_results[2];
            if(dc){dr_return(1,1);rc_try("commands-00000001.bin",0);++dr_results[2];}
            dr_return(dc,0);
        }
        if(claimed){SetLastError(0x77);if(RenderShutdown(3000)!=0 || GetLastError()!=0x77)ExitProcess(527);}
        if(!rs_mode(mode,"callback-short"))rc_try(claimed?"commands-00000002.bin":"commands-00000001.bin",1);dr_results[3]=1;
        SetLastError(0x77);if(!RenderStartup() || GetLastError()!=0x77)ExitProcess(528);
    }
    if(dc){SelectObject(dr_dc,dr_old);DeleteObject(dr_bitmap);DeleteDC(dr_dc);}
    pl_file("race-results.bin",dr_results,sizeof(dr_results));u32 counts[4]={cs_locks,cs_unlocks,2,dr_dc_calls};pl_file("engine-counts.bin",counts,sizeof(counts));ExitProcess(0);
}
