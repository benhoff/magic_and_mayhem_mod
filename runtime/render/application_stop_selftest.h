API u32 WIN RenderStop(u32);
API u32 WIN RenderStopStateForTest(u32*);
API i32 WIN FreeLibrary(HANDLE);
static void as_state(const char* name){u32 values[10];SetLastError(0x77);if(!RenderStopStateForTest(values) || GetLastError()!=0x77)ExitProcess(540);pl_file(name,values,sizeof(values));}
static u32 as_stop(u32 wait){SetLastError(0x77);u32 result=RenderStop(wait);if(GetLastError()!=0x77)ExitProcess(541);return result;}
static void test_application_stop(const char* mode){
    u32 dc=rs_mode(mode,"held-dc"),recovering=rs_mode(mode,"recovery-stop"),held=rs_mode(mode,"held") || dc,join=rs_mode(mode,"control-join") || rs_mode(mode,"publish-join"),host=rs_mode(mode,"host"),failure=recovering || rs_mode(mode,"cancel") || rs_mode(mode,"disappear") || rs_mode(mode,"timeout");
    SetLastError(0x77);if(!RenderStartup() || GetLastError()!=0x77)ExitProcess(542);
    cs_setup(&cs_small,4,4,16,0);cs_setup(&cs_big,512,512,32,1);cs_update(&cs_small,0x1234);
    if(dc){static void* table[33];table[25]=(void*)&cs_lock;table[32]=(void*)&cs_unlock;table[17]=(void*)&dr_get_dc;table[26]=(void*)&dr_release_dc;cs_small.table=table;u32 info[13]={40,4,(u32)-4,0x00100001,3,0,0,0,0,0,0xf800,0x7e0,0x1f};void* bits=0;dr_dc=CreateCompatibleDC(0);dr_bitmap=CreateDIBSection(dr_dc,info,0,&bits,0,0);dr_old=SelectObject(dr_dc,dr_bitmap);if(!dr_dc || !dr_bitmap || !bits)ExitProcess(551);RenderInstallForTest(&cs_small,14);}
    if(recovering)RenderLifecyclePauseForTest(4);
    for(u32 f=0;f<2;++f){cs_update(&cs_big,0xa5000000u|f*0x254713u);Sleep(75);}
    rc_mark("drawn-",0,0);rc_wait("go-",0);
    if(rs_mode(mode,"auto")){u32 counts[2]={cs_locks,cs_unlocks};pl_file("engine-counts.bin",counts,sizeof(counts));ExitProcess(0);}
    if(dc)dr_borrow(1);
    else if(held){u32 desc[31]={124};SetLastError(0x77);if(((i32 (WIN *)(void*,void*,u32*,u32,HANDLE))cs_small.table[25])(&cs_small,0,desc,1,0)!=13 || GetLastError()!=0x88)ExitProcess(543);}
    if(rs_mode(mode,"disappear"))for(u32 f=0;f<20;++f)cs_update(&cs_big,0xa5123456u+f);
    if(host)rc_wait("host-ended-",0);
    u32 first=as_stop(rs_mode(mode,"timeout")?20:3000);
    if(held || join || recovering){
        if(first)ExitProcess(544);as_state("refused-state.bin");rc_mark("refused-",0,0);rc_wait("retry-",0);
        if(dc){dr_return(1,1);if(as_stop(0))ExitProcess(552);dr_return(1,0);}
        else if(held){SetLastError(0x77);if(((i32 (WIN *)(void*,void*))cs_small.table[32])(&cs_small,0)!=19 || GetLastError()!=0x88)ExitProcess(545);}
        u32 retry=as_stop(3000);if(dc){SelectObject(dr_dc,dr_old);DeleteObject(dr_bitmap);DeleteDC(dr_dc);}as_state("retry-state.bin");if(retry!=(failure?0u:1u))ExitProcess(546);
    }else if(first!=(failure?0u:1u))ExitProcess(547);
    as_state("closed-state.bin");u32 counts[2]={cs_locks,cs_unlocks};pl_file("engine-counts.bin",counts,sizeof(counts));
    rc_mark("closed-",0,0);rc_wait("after-",0);
    if(as_stop(0)!=(failure?0u:1u))ExitProcess(548);
    rc_try("spare.bin",0);SetLastError(0x77);if(RenderStartup() || GetLastError()!=0x77)ExitProcess(549);
    /* Pinning makes FreeLibrary safe even while original callbacks stay installed. */
    HANDLE module=GetModuleHandleA("MnmRender.dll");if(!module || !FreeLibrary(module) || GetModuleHandleA("MnmRender.dll")!=module)ExitProcess(550);
    cs_update(&cs_big,0xa5ffffff);u32 after_counts[2]={cs_locks,cs_unlocks};pl_file("after-counts.bin",after_counts,sizeof(after_counts));as_state("after-state.bin");rc_mark("done-",0,0);rc_wait("exit-",0);ExitProcess(0);
}
