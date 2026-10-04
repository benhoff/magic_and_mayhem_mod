#include "../../protocols/include/mnm/input_v1.h"
API i32 WIN RenderInputForTest(u32,i32,i32*);
static i32 input_coop_result;
static i32 WIN test_input_coop(void* object,void* window,u32 flags){
    if(!object || !window || flags!=8 || GetLastError()!=0x77)ExitProcess(298);SetLastError(0x88);return input_coop_result;
}
static void input_expect(u32 action,i32 code,i32 expected,u32 error){
    SetLastError(0x77);if((RenderInputForTest(action,code,0)&0xffff)!=(expected&0xffff) || GetLastError()!=error)ExitProcess(293);
}
static void test_input(void){
    if(!RenderInputForTest(3,0,0))ExitProcess(294);
    if(!RenderInputForTest(6,0,0))ExitProcess(299);
    static void* table[21];table[20]=(void*)&test_input_coop;void** draw=table;RenderInstallForTest(&draw,1);
    input_coop_result=-1;SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*,u32))table[20])(&draw,(void*)0x5678,8)!=-1 || GetLastError()!=0x88)ExitProcess(300);
    i32 initial[2];SetLastError(0x77);if(RenderInputForTest(2,0,initial)!=7 || GetLastError()!=0x88)ExitProcess(301);
    input_coop_result=17;SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*,u32))table[20])(&draw,(void*)0x1234,8)!=17 || GetLastError()!=0x88)ExitProcess(302);
    char path[512];GetEnvironmentVariableA("MNM_RENDER_INPUT",path,sizeof(path));
    HANDLE file=CreateFileA(path,0xc0000000,3,0,3,0x80,0),mapping=CreateFileMappingA(file,0,4,0,MNM_INPUT_V1_SIZE,0);CloseHandle(file);
    u32* words=MapViewOfFile(mapping,2,0,0,MNM_INPUT_V1_SIZE);CloseHandle(mapping);if(!words)ExitProcess(295);
    input_expect(0,65,0x8001,0x77);input_expect(0,65,0x8000,0x77);input_expect(1,65,0x8000,0x77);
    input_expect(0,66,0,0x77);input_expect(1,20,0x1234,0x88);
    input_expect(0,91,0x1234,0x88);input_expect(1,160,0x1234,0x88);
    i32 point[2];SetLastError(0x77);if(RenderInputForTest(2,0,point)!=1 || point[0]!=500 || point[1]!=500 || GetLastError()!=0x77)ExitProcess(296);
    words[MNM_INPUT_V1_SEQUENCE_OFFSET/4]+=2;words[MNM_INPUT_V1_KEYS_OFFSET/4+65]=1;input_expect(0,65,0,0x77);
    words[MNM_INPUT_V1_SEQUENCE_OFFSET/4]+=2;words[MNM_INPUT_V1_KEYS_OFFSET/4+65]=2;input_expect(0,65,1,0x77);input_expect(0,65,0,0x77);
    words[MNM_INPUT_V1_SEQUENCE_OFFSET/4]|=1;input_expect(0,65,0x1234,0x88);words[MNM_INPUT_V1_SEQUENCE_OFFSET/4]+=1;
    words[MNM_INPUT_V1_ACTIVE_OFFSET/4]=0;input_expect(0,65,0x1234,0x88);words[MNM_INPUT_V1_ACTIVE_OFFSET/4]=1;
    words[MNM_INPUT_V1_CURSOR_X_OFFSET/4]=900;input_expect(1,65,0x1234,0x88);words[MNM_INPUT_V1_CURSOR_X_OFFSET/4]=400;
    RenderInputForTest(4,0,0);input_expect(0,65,0x1234,0x88);RenderInputForTest(5,0,0);
    input_expect(0,65,0,0x77);
    SetLastError(0x77);words[MNM_INPUT_V1_ACTIVE_OFFSET/4]=0;if(RenderInputForTest(2,0,point)!=7 || point[0]!=10 || point[1]!=20 || GetLastError()!=0x88)ExitProcess(297);
    ExitProcess(0);
}
