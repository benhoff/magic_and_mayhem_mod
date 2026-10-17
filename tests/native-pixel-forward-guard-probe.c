/* Owned callback stand-in. Reaching fallback writes a marker after the callback. */
#include "../runtime/shadow/win32_min.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
typedef void(WIN* Count)(u32,u32,u32);
static void marker(void){
 u32 done=0;HANDLE file=CreateFileA("original-fallback-reached.bin",0x40000000,0,0,1,0x80,0);
 if(file==(HANDLE)-1)ExitProcess(41);u32 value=1;
 if(!WriteFile(file,&value,4,&done,0)||done!=4||!CloseHandle(file))ExitProcess(42);
}
void start(void){
 char mode[32]={0};GetEnvironmentVariableA("MNM_GUARD_CASE",mode,sizeof(mode));
 void* module=LoadLibraryA("MnmDraw.dll");if(!module)ExitProcess(43);
 Count count=(Count)GetProcAddress(module,"NativeDrawProducerCountV1");if(!count)ExitProcess(44);
 const u32 routes[6]={3,8,1,7,1,23};
 if(mode[0]=='u'){count(0,0,1);ExitProcess(45);}
 for(u32 f=0;f<6;++f)for(u32 r=0;r<routes[f];++r){if(mode[0]=='m'&&f==5&&r==22)continue;count(f,r,0);}
 if(mode[0]=='d')count(0,0,0);
 if(mode[0]=='r')count(0,3,1);
 if(mode[0]=='n'||mode[0]=='m'){SetLastError(0xbeef);count(0,0,1);if(GetLastError()!=0xbeef)ExitProcess(46);ExitProcess(0);}
 count(0,0,2);marker();ExitProcess(0);
}
