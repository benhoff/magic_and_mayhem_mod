#pragma once
/* PE32 process-private exception ABI, never a shared wire contract.
 * Reviewed against Wine and mingw-w64 winnt.h CONTEXT i386 definitions.
 * https://github.com/mirror/mingw-w64/blob/master/mingw-w64-headers/include/winnt.h
 * https://learn.microsoft.com/en-us/windows/win32/debug/using-a-vectored-exception-handler
 */
struct ApiContextI386 {
 u32 flags,debug[6];u8 float_save[112];u32 segments[4];
 u32 edi,esi,ebx,edx,ecx,eax,ebp,eip,cs,eflags,esp,ss;
 u8 extended[512];
};
struct ApiExceptionRecord {u32 code,flags;void* nested;void* address;u32 count,information[15];};
struct ApiExceptionPointers {struct ApiExceptionRecord* record;struct ApiContextI386* context;};
_Static_assert(__builtin_offsetof(struct ApiContextI386,eip)==184,"PE32 EIP offset");
_Static_assert(__builtin_offsetof(struct ApiContextI386,eflags)==192,"PE32 flags offset");
_Static_assert(__builtin_offsetof(struct ApiContextI386,esp)==196,"PE32 ESP offset");
_Static_assert(sizeof(struct ApiContextI386)==716,"PE32 CONTEXT extent");
_Static_assert(__builtin_offsetof(struct ApiExceptionRecord,address)==12,"PE32 exception address offset");
API void* WIN AddVectoredExceptionHandler(u32,i32(WIN*)(struct ApiExceptionPointers*));
