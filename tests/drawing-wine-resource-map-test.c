#define MNM_DRAWING_SELFTEST
#define DllMain FixtureDllMain
#include "../runtime/drawing/session.c"
static u32 calls,fail;
static u8 expected_fp[512] __attribute__((aligned(16)));
static void original_fp(void){__asm__ volatile("fninit; fld1":::"memory");snapshot_fp(expected_fp);}
static void check_fp(void){u8 actual[512] __attribute__((aligned(16)));snapshot_fp(actual);if(!equal(actual,expected_fp,512))ExitProcess(221);}
static void* __attribute__((cdecl)) getter(void* texture){if(texture!=(void*)1)ExitProcess(211);++calls;SetLastError(0xcafe);original_fp();return (void*)2;}
static u32 __attribute__((cdecl)) mapper(void* resource,u32 sub,u32* descriptor,const u32* box,u32 flags){if(resource!=(void*)2||sub||(box&&!fail)||flags!=7)ExitProcess(212);++calls;SetLastError(0xabcd);original_fp();if(fail)return 0x8876086cu;descriptor[0]=1600;descriptor[1]=960000;descriptor[2]=3;return 0;}
static u32 __attribute__((cdecl)) unmapper(void* resource,u32 sub){if(resource!=(void*)2||sub)ExitProcess(213);++calls;SetLastError(0x1234);original_fp();return fail?0x8876086cu:0;}
void start(void){
 if(!initialize())ExitProcess(210);wine_resource_original[0]=(void*)&getter;wine_resource_original[1]=(void*)&mapper;wine_resource_original[2]=(void*)&unmapper;u32 descriptor[3]={0};
 SetLastError(0xbeef);if(wine_resource_get((void*)1)!=(void*)2||GetLastError()!=0xcafe)ExitProcess(214);check_fp();
 if(wine_resource_map((void*)2,0,descriptor,0,7)||GetLastError()!=0xabcd||descriptor[0]!=1600)ExitProcess(215);check_fp();
 fail=1;if(wine_resource_unmap((void*)2,0)!=0x8876086cu||GetLastError()!=0x1234)ExitProcess(216);check_fp();fail=0;if(wine_resource_unmap((void*)2,0)||GetLastError()!=0x1234)ExitProcess(217);check_fp();
 if(wine_resource_unmap((void*)2,0))ExitProcess(218);check_fp();fail=1;if(wine_resource_map((void*)2,0,0,0,7)!=0x8876086cu)ExitProcess(219);check_fp();if(wine_resource_map((void*)2,0,0,(void*)1,7)!=0x8876086cu)ExitProcess(222);check_fp();if(calls!=7)ExitProcess(220);wine_resource_close(1);ExitProcess(0);
}
