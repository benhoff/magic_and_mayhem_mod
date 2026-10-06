API void* WIN GetProcAddress(void*,const char*);
API u32 WIN RenderStartup(void);
API u32 WIN RenderExitInstallForTest(void*);
static void test_orchestration(const char* mode){
    SetLastError(0x77);u32 ready=RenderStartup();
    if(GetLastError()!=0x77 || ready!=(mode[0]=='i'?0u:1u))ExitProcess(390);
    if(mode[0]=='i'){
        if(RenderShutdown(0) || RenderStartup() || GetLastError()!=0x77)ExitProcess(405);
        ExitProcess(0);
    }
    if(mode[0]=='e')ExitProcess(0);
    if(!RenderStartup() || GetLastError()!=0x77)ExitProcess(391);
    cs_setup(&cs_small,4,4,16,0);cs_setup(&cs_big,512,512,32,1);
    cs_update(&cs_small,0x1234);
    u32 frames=mode[0]=='n'?0:2;
    for(u32 i=0;i<frames;++i){cs_update(&cs_big,0xa5000000u|i*0x254713u);Sleep(75);}
    if(mode[0]=='h'){
        u32 d[31]={124};SetLastError(0x77);
        if(((i32 (WIN *)(void*,void*,void*,u32,HANDLE))cs_big.table[25])(&cs_big,0,d,1,0)!=13 || GetLastError()!=0x88)ExitProcess(392);
    }
    u32 result[3]={cs_locks,cs_unlocks,frames};pl_file("engine-counts.bin",result,12);
    if(mode[0]=='x'){
        SetLastError(0x77);if(!RenderShutdown(3000) || GetLastError()!=0x77 || RenderStartup()!=0 || GetLastError()!=0x77)ExitProcess(393);
        /* Original calls remain installed but cannot reopen the completed stream. */
        cs_update(&cs_big,0xa5ffffff);
        if(RenderShutdown(0)!=1 || GetLastError()!=0x88)ExitProcess(394);
    }
    SetLastError(0x77);ExitProcess(0);
}
/* Installer guards on controlled PE32 bytes; install on this real executable
 * last, proving the actual imported ExitProcess dispatch, not a direct wrapper. */
static void test_exit_guards(void){
    u8* image=HeapAlloc(GetProcessHeap(),8,4096);if(!image)ExitProcess(395);
    SetLastError(0x77);if(RenderExitInstallForTest(image) || GetLastError()!=0x77)ExitProcess(396);
    *(u16*)image=0x5a4d;*(u32*)(image+60)=64;*(u32*)(image+64)=0x4550;
    *(u16*)(image+68)=0x8664;
    if(RenderExitInstallForTest(image))ExitProcess(397);
    *(u16*)(image+68)=0x14c;*(u16*)(image+84)=224;*(u16*)(image+88)=0x10b;
    *(u32*)(image+144)=4096;*(u32*)(image+180)=2;*(u32*)(image+192)=512;*(u32*)(image+196)=40;
    /* Empty imports, outside-image directory and unterminated table. */
    if(RenderExitInstallForTest(image))ExitProcess(398);
    *(u32*)(image+192)=4090;if(RenderExitInstallForTest(image))ExitProcess(399);
    *(u32*)(image+192)=512;*(u32*)(image+528)=4094;
    if(RenderExitInstallForTest(image))ExitProcess(400);
    *(u32*)(image+528)=1024;
    void* expected=GetProcAddress(GetModuleHandleA("kernel32.dll"),"ExitProcess");
    ((void**)(image+1024))[0]=expected;((void**)(image+1024))[1]=expected;
    if(RenderExitInstallForTest(image) || ((void**)(image+1024))[0]!=expected ||
       ((void**)(image+1024))[1]!=expected)ExitProcess(402);
    ((void**)(image+1024))[1]=0;*(u32*)(image+196)=20;
    if(RenderExitInstallForTest(image) || ((void**)(image+1024))[0]!=expected)ExitProcess(403);
    *(u32*)(image+196)=40;((void**)(image+1024))[0]=(void*)0x1234;
    if(RenderExitInstallForTest(image))ExitProcess(404);
    HeapFree(GetProcessHeap(),0,image);
    SetLastError(0x77);if(!RenderExitInstallForTest(GetModuleHandleA(0)) || GetLastError()!=0x77 ||
       !RenderExitInstallForTest(GetModuleHandleA(0)))ExitProcess(401);
    test_orchestration("auto");
}
