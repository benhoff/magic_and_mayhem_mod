/* Fixture owns fake original CPU pixels. Never calls RenderRecover; Qt drives
 * the actual asynchronous PE32 control worker while application drawing runs. */
API HANDLE WIN CreateThread(void*,u32,u32 (WIN *)(void*),void*,u32,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
static u32 WIN hc_blocked(void* unused){(void)unused;cs_delay=500;cs_update(&cs_big,0xa5ffffff);return 0;}
static void test_host_recovery(const char* mode){
    SetLastError(0x77);if(!RenderStartup() || GetLastError()!=0x77)ExitProcess(430);
    if(mode[0]=='s' && mode[1]=='t')RenderShutdown(0);
    cs_setup(&cs_big,512,512,32,1);
    for(u32 i=0;i<2;++i){cs_update(&cs_big,0xa5000000u|(i*0x254713u&0xffffff));Sleep(mode[0]=='e'?200:mode[0]=='r'?150:75);}
    rc_mark("drawn-",0,0);rc_wait("go-",0);
    HANDLE thread=0;int held=mode[0]=='h' && mode[2]=='l';
    if(held){u32 d[31]={124};SetLastError(0x77);if(((i32 (WIN *)(void*,void*,void*,u32,HANDLE))cs_big.table[25])(&cs_big,0,d,1,0)!=13 || GetLastError()!=0x88)ExitProcess(431);}
    if(mode[0]=='b'){thread=CreateThread(0,0,hc_blocked,0,0,0);if(!thread)ExitProcess(432);rc_wait("request-",0);if(WaitForSingleObject(thread,2000)!=0)ExitProcess(433);CloseHandle(thread);thread=0;}
    for(u32 i=2;i<40;++i){if(!held && mode[0]!='f')cs_update(&cs_big,0xa5000000u|(i*0x254713u&0xffffff));Sleep(mode[0]=='e'?200:mode[0]=='r'?150:75);}
    if(thread){if(WaitForSingleObject(thread,2000)!=0)ExitProcess(433);CloseHandle(thread);}
    if(held){SetLastError(0x77);if(((i32 (WIN *)(void*,void*))cs_big.table[32])(&cs_big,0)!=19 || GetLastError()!=0x88)ExitProcess(434);}
    u32 counts[2]={cs_locks,cs_unlocks};pl_file("engine-counts.bin",counts,8);
    rc_mark("done-",0,0);rc_wait("exit-",0);ExitProcess(0);
}
