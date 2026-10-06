/* Actual idle scheduler/ring. Opaque test bytes are poisoned immediately after
 * admission; the independent reader must retain every byte through FULL. */
static void test_backpressure(const char* mode){
    u32 small=rs_mode(mode,"small-stall") || rs_mode(mode,"idle") || rs_mode(mode,"cancel") || rs_mode(mode,"invalid-ack");
    u32 overflow=rs_mode(mode,"overflow"),valid=rs_mode(mode,"pause") || rs_mode(mode,"progress") || rs_mode(mode,"idle");
    u32 length=overflow?32*1024*1024:small?64:2*1024*1024+123;
    u8* bytes=HeapAlloc(GetProcessHeap(),0,length);if(!bytes)ExitProcess(380);
    for(u32 i=0;i<length;++i)bytes[i]=(u8)(i*13+31);
    SetLastError(0x77);if(!RenderQueueForTest(bytes,length,0) || GetLastError()!=0x77)ExitProcess(381);
    for(u32 i=0;i<length;++i)bytes[i]=0xa5;
    if(overflow){SetLastError(0x77);if(RenderQueueForTest(bytes,2*1024*1024,0) || GetLastError()!=0x77)ExitProcess(382);}
    HeapFree(GetProcessHeap(),0,bytes);
    /* No drawing callbacks drive retry or timeout while the reader is idle. */
    Sleep(rs_mode(mode,"progress")?2500:valid?900:600);
    if(valid){SetLastError(0x77);if(!RenderQueueForTest(0,0,1) || GetLastError()!=0x77)ExitProcess(383);}
    SetLastError(0x77);u32 complete=RenderShutdown(valid?3000:0);
    if(complete!=valid || GetLastError()!=0x77 || RenderShutdown(0)!=complete || GetLastError()!=0x77)ExitProcess(384);
    ExitProcess(0);
}
