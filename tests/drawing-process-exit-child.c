#include "../runtime/shadow/win32_min.h"
API u32 WIN GetCurrentThreadId(void);
API u32 WIN GetCurrentProcessId(void);
API i32 WIN FlushFileBuffers(HANDLE);
API void WIN Sleep(u32);
void start(void){
 char path[1024],release[1024],value[16];u32 n=GetEnvironmentVariableA("MNM_CHILD_READY_FILE",path,sizeof(path));if(!n||n>=sizeof(path))ExitProcess(211);
 n=GetEnvironmentVariableA("MNM_CHILD_RELEASE_FILE",release,sizeof(release));if(!n||n>=sizeof(release))ExitProcess(211);
 n=GetEnvironmentVariableA("MNM_CHILD_EXIT_CODE",value,sizeof(value));if(!n||n>=sizeof(value))ExitProcess(211);
 u32 code=0;for(u32 i=0;i<n;++i){if(value[i]<'0'||value[i]>'9'||code>259)ExitProcess(211);code=code*10+(u32)(value[i]-'0');}if(code>259)ExitProcess(211);
 HANDLE file=CreateFileA(path,0x40000000,3,0,1,0x80,0);u32 ready[2]={GetCurrentThreadId(),GetCurrentProcessId()},written=0;
 if(file==(HANDLE)-1||!WriteFile(file,ready,8,&written,0)||written!=8||!FlushFileBuffers(file)||!CloseHandle(file))ExitProcess(212);
 for(u32 i=0;i<750;++i){file=CreateFileA(release,0x80000000,3,0,3,0x80,0);if(file!=(HANDLE)-1){CloseHandle(file);ExitProcess(code);}Sleep(20);}
 ExitProcess(213);
}
