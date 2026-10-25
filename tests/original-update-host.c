/* Disposable official-update host. Only the command-line-owned directory is
 * supplied to the original synchronous patch API. No game is launched. */
#include "../runtime/shadow/win32_min.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
typedef u32 (WIN *Apply)(char*,void*,u32);
void mainCRTStartup(void) {
    static char command[4096];
    u32 length=GetEnvironmentVariableA("MNM_OFFICIAL_UPDATE_COMMAND",command,sizeof(command));
    if(!length||length>=sizeof(command))ExitProcess(120);
    void* library=LoadLibraryA("PATCHW32.dll");
    if(!library)ExitProcess(121);
    Apply apply=(Apply)GetProcAddress(library,"RTPatchApply32@12");
    if(!apply)ExitProcess(122);
    ExitProcess(apply(command,0,1));
}
