#pragma once
/* PE32-only declarations, no CRT or external Windows SDK required. */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int i32;
typedef void* HANDLE;
#define WIN __attribute__((stdcall))
#define API __declspec(dllimport)
API void* WIN GetModuleHandleA(const char*);
API void* WIN VirtualAlloc(void*,u32,u32,u32);
API int WIN VirtualProtect(void*,u32,u32,u32*);
API u32 WIN VirtualQuery(const void*,void*,u32);
API int WIN FlushInstructionCache(HANDLE,const void*,u32);
API HANDLE WIN GetCurrentProcess(void);
API HANDLE WIN GetProcessHeap(void);
API void* WIN HeapAlloc(HANDLE,u32,u32);
API int WIN HeapFree(HANDLE,u32,void*);
API HANDLE WIN CreateFileA(const char*,u32,u32,void*,u32,u32,HANDLE);
API int WIN WriteFile(HANDLE,const void*,u32,u32*,void*);
API int WIN CloseHandle(HANDLE);
API u32 WIN GetLastError(void);
API void WIN SetLastError(u32);
API u32 WIN GetEnvironmentVariableA(const char*,char*,u32);
API void WIN ExitProcess(u32);
