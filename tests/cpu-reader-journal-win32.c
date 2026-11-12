#include "../runtime/shadow/win32_min.h"
API void* WIN GetProcAddress(void*,const char*);
typedef u32 (WIN *Control)(u32);
void start(void){
 Control call=(Control)GetProcAddress(GetModuleHandleA("MnmRender.dll"),"RenderReaderJournalControlForTest");
 if(!call)ExitProcess(21);
 char mode[2];GetEnvironmentVariableA("MNM_JOURNAL_CONTROL_EXPECT",mode,2);
 SetLastError(0x77);u32 enabled=call(0);
 if(enabled!=(mode[0]=='1') || GetLastError()!=0x77)ExitProcess(22);
 if(enabled){SetLastError(0x77);if(call(1)!=23 || GetLastError()!=0x77)ExitProcess(23);}
 ExitProcess(0);
}
