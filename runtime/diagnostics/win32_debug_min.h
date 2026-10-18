#pragma once
/* PE32 layouts reviewed against Wine's winnt.h, minwinbase.h and
 * processthreadsapi.h. No dependency on an installed Windows C runtime/SDK. */
#include "../shadow/win32_min.h"
typedef u32 DWORD;
typedef u32 SIZE_T;
typedef struct {
    DWORD ContextFlags, Dr0, Dr1, Dr2, Dr3, Dr6, Dr7;
    u8 FloatSave[112];
    DWORD SegGs, SegFs, SegEs, SegDs;
    DWORD Edi, Esi, Ebx, Edx, Ecx, Eax, Ebp, Eip, SegCs, EFlags, Esp, SegSs;
    u8 ExtendedRegisters[512];
} CONTEXT;
typedef struct { DWORD cb; u8 unused[64]; } STARTUPINFOA;
typedef struct { HANDLE hProcess, hThread; DWORD dwProcessId, dwThreadId; } PROCESS_INFORMATION;
typedef struct {
    DWORD dwDebugEventCode, dwProcessId, dwThreadId;
    union {
        struct {
            struct { DWORD ExceptionCode, ExceptionFlags; void* ExceptionRecord; void* ExceptionAddress;
                     DWORD NumberParameters, ExceptionInformation[15]; } ExceptionRecord;
            DWORD dwFirstChance;
        } Exception;
        struct { HANDLE hThread; void* unused[2]; } CreateThread;
        struct { HANDLE hFile, hProcess, hThread; void* lpBaseOfImage; u8 unused[24]; } CreateProcessInfo;
        struct { DWORD dwExitCode; } ExitProcess;
        struct { HANDLE hFile; } LoadDll;
    } u;
} DEBUG_EVENT;
_Static_assert(sizeof(CONTEXT) == 716 && __builtin_offsetof(CONTEXT, Eip) == 0xb8, "PE32 CONTEXT layout");
_Static_assert(sizeof(STARTUPINFOA) == 68 && sizeof(DEBUG_EVENT) == 96, "PE32 debug event layout");
#define CONTEXT_CONTROL 0x10001
#define CONTEXT_INTEGER 0x10002
#define CONTEXT_DEBUG_REGISTERS 0x10010
#define GENERIC_WRITE 0x40000000u
#define FILE_SHARE_READ 1
#define CREATE_NEW 1
#define FILE_ATTRIBUTE_NORMAL 0x80
#define FILE_BEGIN 0
#define INVALID_HANDLE_VALUE ((HANDLE)-1)
#define FALSE 0
#define DEBUG_ONLY_THIS_PROCESS 2
#define DBG_CONTINUE 0x10002
#define DBG_EXCEPTION_NOT_HANDLED 0x80010001u
#define EXCEPTION_DEBUG_EVENT 1
#define CREATE_THREAD_DEBUG_EVENT 2
#define CREATE_PROCESS_DEBUG_EVENT 3
#define EXIT_PROCESS_DEBUG_EVENT 5
#define LOAD_DLL_DEBUG_EVENT 6
#define EXCEPTION_BREAKPOINT 0x80000003u
#define EXCEPTION_SINGLE_STEP 0x80000004u
#define MEM_COMMIT 0x1000
#define MEM_RESERVE 0x2000
#define PAGE_EXECUTE_READWRITE 0x40
API DWORD WIN SetFilePointer(HANDLE,i32,i32*,DWORD);
API int WIN FlushFileBuffers(HANDLE);
API char* WIN GetCommandLineA(void);
API int WIN CreateProcessA(const char*,char*,void*,void*,int,DWORD,void*,const char*,STARTUPINFOA*,PROCESS_INFORMATION*);
API int WIN WaitForDebugEvent(DEBUG_EVENT*,DWORD);
API int WIN ContinueDebugEvent(DWORD,DWORD,DWORD);
API int WIN GetThreadContext(HANDLE,CONTEXT*);
API int WIN SetThreadContext(HANDLE,const CONTEXT*);
API int WIN ReadProcessMemory(HANDLE,const void*,void*,SIZE_T,SIZE_T*);
void* memset(void* p, int value, unsigned bytes) {
    volatile u8* out = p;
    for (unsigned i = 0; i < bytes; ++i) out[i] = (u8)value;
    return p;
}
void* memcpy(void* p, const void* q, unsigned bytes) {
    volatile u8* out = p; const volatile u8* in = q;
    for (unsigned i = 0; i < bytes; ++i) out[i] = in[i];
    return p;
}
