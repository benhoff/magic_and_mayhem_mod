#include "../runtime/shadow/win32_min.h"
API void WIN RenderInstallForTest(void*,u32);
API u32 WIN RenderResidentBindHoldForTest(void);
API u32 WIN RenderCallbackPublicationProbeForTest(u32);
API u32 WIN RenderResidentWorldV1(u32,u32,u32,u32,u32,u32,u32,u32);
API u32 WIN RenderShutdown(u32);
API void WIN Sleep(u32);
API u32 WIN GetTickCount(void);
API HANDLE WIN CreateThread(void*,u32,u32(WIN*)(void*),void*,u32,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
API int WIN QueryPerformanceCounter(unsigned long long*);
API int WIN QueryPerformanceFrequency(unsigned long long*);
static u16 pixels[800*600];
static u32 locks,unlocks,holder;
static void output(const char* name,const void* data,u32 size){
    u32 written;HANDLE f=CreateFileA(name,0x40000000,1,0,1,0x80,0);
    if(f==(HANDLE)-1 || !WriteFile(f,data,size,&written,0) || written!=size)ExitProcess(11);
    CloseHandle(f);
}
static void await_file(const char* name){
    u32 start=GetTickCount();
    for(;;){HANDLE f=CreateFileA(name,0x80000000,3,0,3,0x80,0);
        if(f!=(HANDLE)-1){CloseHandle(f);return;}
        if(GetTickCount()-start>=2000)ExitProcess(12);Sleep(1);
    }
}
static i32 WIN lock(void* object,void* rect,u32* d,u32 flags,HANDLE event){
    if(!object || rect || event || flags!=1 || d[0]!=124 || GetLastError()!=0x77)ExitProcess(13);
    ++locks;d[1]=1;d[2]=600;d[3]=800;d[4]=1600;d[9]=(u32)pixels;
    d[19]=0x40;d[21]=16;d[22]=0xf800;d[23]=0x7e0;d[24]=31;d[26]=0x200;
    SetLastError(0x88);return 13;
}
static i32 WIN unlock(void* object,void* rect){
    if(!object || rect || GetLastError()!=0x77)ExitProcess(14);
    ++unlocks;for(u32 i=0;i<800*600;++i)pixels[i]=0xcccc;
    SetLastError(0x88);return 19;
}
static u32 WIN hold(void* unused){
    (void)unused;SetLastError(0x77);holder=RenderResidentBindHoldForTest();
    if(GetLastError()!=0x77)ExitProcess(15);return 0;
}
static u32 WIN release(void* unused){
    (void)unused;await_file("bind-attempt.bin");Sleep(20);u32 value=1;
    output("bind-release.bin",&value,4);return 0;
}
void start(void){
    static void* table[33];table[25]=(void*)lock;table[32]=(void*)unlock;void** object=table;
    RenderInstallForTest(&object,14);u32 d[31]={124};SetLastError(0x77);
    if(((i32(WIN*)(void*,void*,u32*,u32,HANDLE))table[25])(&object,0,d,1,0)!=13 || GetLastError()!=0x88)ExitProcess(16);
    HANDLE owner=CreateThread(0,0,hold,0,0,0);if(!owner)ExitProcess(17);await_file("bind-held.bin");
    HANDLE releaser=CreateThread(0,0,release,0,0,0);if(!releaser)ExitProcess(18);
    u32 attempted=1;output("bind-attempt.bin",&attempted,4);
    unsigned long long begin,end,frequency;QueryPerformanceFrequency(&frequency);QueryPerformanceCounter(&begin);
    SetLastError(0x77);u32 bound=RenderResidentWorldV1((u32)pixels,800,600,800,1,0,17,34);
    u32 error=GetLastError();QueryPerformanceCounter(&end);
    if(WaitForSingleObject(owner,2000) || WaitForSingleObject(releaser,2000))ExitProcess(19);
    CloseHandle(owner);CloseHandle(releaser);
    char probe[8];u32 publication_probe=GetEnvironmentVariableA("MNM_BIND_PUBLICATION_PROBE",probe,sizeof(probe))==1 && probe[0]=='1';
    SetLastError(0x77);if(publication_probe){RenderCallbackPublicationProbeForTest(1);if(GetLastError()!=0x77)ExitProcess(22);}
    unsigned long long unlock_begin,unlock_end;QueryPerformanceCounter(&unlock_begin);
    SetLastError(0x77);if(((i32(WIN*)(void*,void*))table[32])(&object,0)!=19 || GetLastError()!=0x88)ExitProcess(20);
    QueryPerformanceCounter(&unlock_end);SetLastError(0x77);
    u32 pump_calls=publication_probe?RenderCallbackPublicationProbeForTest(0):0;
    if(GetLastError()!=0x77)ExitProcess(23);
    SetLastError(0x77);u32 complete=RenderShutdown(3000);if(GetLastError()!=0x77)ExitProcess(21);
    unsigned long long elapsed=end-begin;
    unsigned long long unlock_elapsed=unlock_end-unlock_begin;
    u32 result[13]={bound,error,locks,unlocks,holder,complete,(u32)elapsed,(u32)(elapsed>>32),(u32)frequency,(u32)(frequency>>32),pump_calls,(u32)unlock_elapsed,(u32)(unlock_elapsed>>32)};
    output("bind-result.bin",result,sizeof(result));ExitProcess(0);
}
