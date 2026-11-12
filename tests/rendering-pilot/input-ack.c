#include "../../runtime/shadow/win32_min.h"
#include "../../protocols/include/mnm/input_v1.h"
API i32 WIN RenderInputForTest(u32,i32,i32*);
API HANDLE WIN CreateFileMappingA(HANDLE,void*,u32,u32,u32,const char*);
API void* WIN MapViewOfFile(HANDLE,u32,u32,u32,u32);
static void expect(i32 result,u32 error){
    SetLastError(0x77);
    if((RenderInputForTest(0,39,0)&65535)!=result || GetLastError()!=error)ExitProcess(11);
}
void start(void){
    if(!RenderInputForTest(3,0,0))ExitProcess(12);
    char path[512];GetEnvironmentVariableA("MNM_RENDER_INPUT",path,sizeof(path));
    HANDLE file=CreateFileA(path,0xc0000000,3,0,3,0x80,0);
    HANDLE mapping=CreateFileMappingA(file,0,4,0,MNM_INPUT_V1_SIZE,0);CloseHandle(file);
    u32* words=MapViewOfFile(mapping,2,0,0,MNM_INPUT_V1_SIZE);CloseHandle(mapping);
    if(!words)ExitProcess(13);
    expect(0x8001,0x77);expect(0x8000,0x77);
    words[MNM_INPUT_V1_SEQUENCE_OFFSET/4]+=2;words[MNM_INPUT_V1_KEYS_OFFSET/4+39]=1;expect(0,0x77);
    words[MNM_INPUT_V1_SEQUENCE_OFFSET/4]+=2;words[MNM_INPUT_V1_KEYS_OFFSET/4+39]=0x80000002;expect(0x8001,0x77);
    words[MNM_INPUT_V1_ACTIVE_OFFSET/4]=0;expect(0x1234,0x88);
    ExitProcess(0);
}
