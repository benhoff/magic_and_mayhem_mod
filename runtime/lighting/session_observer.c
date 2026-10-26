/* Reuse the already recovered snapshot and byte-result helpers verbatim. The
 * finite diagnostic DLL and its entry point remain independent of this DLL. */
#define DllMain FiniteLightDllMain
#include "observer.c"
#undef DllMain
#include "session.h"
static void __attribute__((fastcall)) sessionEntry(void* receiver,void* unused){
 (void)unused;typedef void(__attribute__((thiscall))*Call)(void*);u32 error=GetLastError();
 if(busy)((Call)trampoline)(receiver);else sessionHook(receiver,(u32)__builtin_return_address(0));
 SetLastError(error);
}
int WIN DllMain(void* instance,u32 reason,void* reserved){
 (void)instance;(void)reserved;
 if(reason==0){if(session){UnmapViewOfFile(session);CloseHandle(session_mapping);CloseHandle(session_file);session=0;}return 1;}
 if(reason!=1)return 1;
 char name[1024];u32 n=GetEnvironmentVariableA("MNM_LIGHT_SESSION_FILE",name,sizeof(name));if(!n)return 1;if(n>=sizeof(name)||!sessionStart(name))return 0;
 const u32 site=0x4f27a0;u8 expected[8]={0x56,0x8b,0xf1,0xe8,0x28,0,0,0};u32 old,unused;
 if(!equal((void*)site,expected,8))return 0;
 u8* code=VirtualAlloc(0,13,0x3000,4);if(!code)return 0;copy(code,expected,8);put(code+4,0x4f27d0-(u32)code-8);code[8]=0xe9;put(code+9,site+8-(u32)code-13);
 if(!VirtualProtect(code,13,0x20,&unused))return 0;FlushInstructionCache(GetCurrentProcess(),code,13);trampoline=code;
 if(!VirtualProtect((void*)site,8,0x40,&old))return 0;
 u8 patch[8]={0xe9,0,0,0,0,0x90,0x90,0x90};put(patch+1,(u32)&sessionEntry-site-5);copy((void*)site,patch,8);FlushInstructionCache(GetCurrentProcess(),(void*)site,8);VirtualProtect((void*)site,8,old,&unused);return 1;
}
