/* Isolated i386 host substitutes for Win32 APIs. Include actual route code;
 * installation and real OS permissions/files/reentry are validated separately.
 * Valid images, frames and canvases are supplied by the guarded reference. */
#define __declspec(x)
#define word_original _word_original
#define word_enter_scalar host_unused_scalar
#define word_enter_forward host_unused_forward
#include "../runtime/scene/word_route.c"
#include <stdlib.h>
static u32 host_error,host_fail,host_frame,host_length,host_canvas,host_bytes;
u32 mnm_host_original_calls;
extern void mnm_host_original_scalar(void),mnm_host_original_forward(void);
void host_unused_scalar(void){}
void host_unused_forward(void){}
void mnm_host_configure(u32 frame,u32 length,u32 canvas,u32 bytes,u32 failure){
    const u32 initial[16]={0x574d4e4d,0x31304452,1,64};
    const u32 initial_clip[20]={0x574d4e4d,0x31304c43,1,80};
    for(u32 i=0;i<16;++i)stats[i]=initial[i];
    for(u32 i=0;i<20;++i)clip_stats[i]=initial_clip[i];
    image_base=0x400000;mode=3;installed=1;stopped=failure==5;busy=failure==4;
    scalar_trampoline=(u32)&mnm_host_original_scalar;forward_trampoline=(u32)&mnm_host_original_forward;
    host_error=0x7a21;host_fail=failure;host_frame=frame;host_length=length;
    host_canvas=canvas;host_bytes=bytes;mnm_host_original_calls=0;
    log_file=(HANDLE)1;clip_log=(HANDLE)2;directory[0]='h';directory[1]=0;
}
u32 mnm_host_last_error(void){return host_error;}
u32 mnm_host_compared(void){return clip_stats[6];}
u32 mnm_host_forwarded(void){return clip_stats[11];}
u32 mnm_host_clipped(void){return clip_stats[7];}
u32 mnm_host_mismatches(void){return clip_stats[12];}
int _word_route(u32* registers){return word_route(registers);}
u32 WIN GetLastError(void){return host_error;}
void WIN SetLastError(u32 n){host_error=n;}
u32 WIN VirtualQuery(const void* p,void* result,u32 size){
    u32 at=(u32)p;host_error=0xde01;
    if(size!=28||(host_fail==2&&at>=host_frame&&at<host_frame+host_length))return 0;
    u32* m=result;for(u32 i=0;i<7;++i)m[i]=0;
    m[0]=at&~4095u;m[3]=4096;m[4]=0x1000;m[5]=4;
    if(host_fail==3&&at>=host_canvas&&at<host_canvas+host_bytes)m[5]=2;
    return 28;
}
HANDLE WIN GetProcessHeap(void){host_error=0xde02;return (HANDLE)1;}
void* WIN HeapAlloc(HANDLE h,u32 flags,u32 bytes){(void)h;(void)flags;host_error=0xde03;return host_fail==1?0:malloc(bytes);}
int WIN HeapFree(HANDLE h,u32 flags,void* p){(void)h;(void)flags;free(p);host_error=0xde04;return 1;}
HANDLE WIN CreateFileA(const char* path,u32 access,u32 share,void* security,u32 mode,u32 flags,HANDLE template){
    (void)path;(void)access;(void)share;(void)security;(void)mode;(void)flags;(void)template;host_error=0xde05;return (HANDLE)3;
}
int WIN WriteFile(HANDLE f,const void* p,u32 bytes,u32* wrote,void* overlap){(void)f;(void)p;(void)overlap;*wrote=bytes;host_error=0xde06;return 1;}
int WIN CloseHandle(HANDLE f){(void)f;host_error=0xde07;return 1;}
void* WIN GetModuleHandleA(const char* n){(void)n;return (void*)0x400000;}
void* WIN VirtualAlloc(void* p,u32 n,u32 type,u32 protect){(void)p;(void)n;(void)type;(void)protect;return 0;}
int WIN VirtualProtect(void* p,u32 n,u32 flags,u32* old){(void)p;(void)n;(void)flags;(void)old;return 0;}
int WIN FlushInstructionCache(HANDLE h,const void* p,u32 n){(void)h;(void)p;(void)n;return 1;}
HANDLE WIN GetCurrentProcess(void){return (HANDLE)1;}
u32 WIN GetEnvironmentVariableA(const char* n,char* out,u32 cap){(void)n;(void)out;(void)cap;return 0;}
void WIN ExitProcess(u32 n){exit(n);}
