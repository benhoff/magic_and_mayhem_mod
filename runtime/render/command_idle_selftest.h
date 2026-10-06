/* Actual PE32 thread/queue/loader path; no original artifacts or borrowed pixels. */
API u32 WIN RenderQueueForTest(const void*,u32,u32);
API u32 WIN RenderShutdown(u32);
static void test_command_idle(const char* mode){
    const u32 length=2*1024*1024+123;u8* bytes=HeapAlloc(GetProcessHeap(),0,length);
    if(!bytes)ExitProcess(281);
    for(u32 i=0;i<length;++i)bytes[i]=(u8)(i*13+31);
    SetLastError(0x77);if(!RenderQueueForTest(bytes,length,1) || GetLastError()!=0x77)ExitProcess(282);
    for(u32 i=0;i<length;++i)bytes[i]=0xa5;
    HeapFree(GetProcessHeap(),0,bytes);
    /* No further tracker/drawing callbacks: idle worker must make progress. */
    if(mode[0]=='t'){if(RenderShutdown(0)!=0 || GetLastError()!=0x77)ExitProcess(283);ExitProcess(0);}
    Sleep(mode[0]=='s'?100:1000);
    if(mode[0]=='d')ExitProcess(0);
    SetLastError(0x77);u32 complete=RenderShutdown(3000);
    if(complete!=(mode[0]=='c'?0u:1u) || GetLastError()!=0x77)ExitProcess(284);
    /* Explicit shutdown is idempotent, even after resources are released. */
    if(RenderShutdown(0)!=complete || GetLastError()!=0x77)ExitProcess(285);
    ExitProcess(0);
}
