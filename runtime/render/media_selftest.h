API i32 WIN RenderMediaForTest(u32,const char*,u32);
static void media_check(u32 operation,const char* path,u32 flags,i32 expected,u32 error){
    SetLastError(0x77);
    i32 result=RenderMediaForTest(operation,path,flags);u32 actual=GetLastError();
    if(result!=expected)ExitProcess(320+operation);
    if(actual!=error)ExitProcess(330+operation);
}
static void test_media(void){
    media_check(4,0,0,1,0x77);
    media_check(1,"FMV/Test.avi",0,1,0x77);
    media_check(1,"FMV/Skip.avi",0,0,0x77);
    media_check(2,"Sounds/Test.wav",0x20009,1,0x77);
    media_check(2,"in-memory",4,23,0x88);
    media_check(2,"Sounds/Test.wav",0x20009,1,0x77);
    media_check(2,"Sounds/Test.wav",0x20011,0,0x77);
    media_check(3,0,0,1,0x77);
    media_check(1,"FMV/Missing.avi",0,17,0x88);
    media_check(1,"../FMV/Test.avi",0,17,0x88);
    media_check(2,"in-memory",4,23,0x88);
    /* Host exits after the eighth result: stale lease must fall back. */
    media_check(1,"FMV/Test.avi",0,17,0x88);
    ExitProcess(0);
}
