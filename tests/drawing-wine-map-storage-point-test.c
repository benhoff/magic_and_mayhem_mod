#define MNM_DRAWING_SELFTEST
#define DllMain FixtureDllMain
#include "../runtime/drawing/session.c"
static u32 mode,calls,exports;
static u8 expected_fp[512] __attribute__((aligned(16)));
static void original_fp(void){__asm__ volatile("fninit; fld1":::"memory");snapshot_fp(expected_fp);}
static void check_fp(void){u8 actual[512] __attribute__((aligned(16)));snapshot_fp(actual);if(!equal(actual,expected_fp,512))ExitProcess(221);}
static void* __attribute__((cdecl)) getter(void* texture){if(texture!=(void*)1)ExitProcess(211);++calls;SetLastError(0xcafe);original_fp();return (void*)2;}
static u32 __attribute__((cdecl)) mapper(void* resource,u32 sub,u32* out,const u32* box,u32 flags){
 if(resource!=(void*)2||sub||box||flags!=7)ExitProcess(212);++calls;SetLastError(0xabcd);original_fp();if(mode==5)return 0x8876086cu;out[0]=1600;out[1]=960000;out[2]=3;return 0;
}
static u32 __attribute__((cdecl)) unmapper(void* resource,u32 sub){if(resource!=(void*)2||sub)ExitProcess(213);++calls;SetLastError(0x1234);original_fp();return 0;}
static u32 WIN point(void* data,u32 row,u32 slice,u32* out,u32 size){
 if(data!=(void*)3||row!=1600||slice!=960000||size!=64)ExitProcess(222);++exports;
 if(wine_map_storage_points!=mode)ExitProcess(223);u32 r[16]={64,1,3,7,0,0,0,0,0,0,0,0,0,0,0,0};r[14]=GetCurrentThreadId();
 if(!mode){r[2]=19;r[6]=13;r[7]=800;r[8]=600;r[9]=800;r[10]=7;r[11]=7;}
 else r[2]=mode==1?3:mode==2?11:mode==3?35:7;
 copy(out,r,64);SetLastError(0xdead);__asm__ volatile("fninit; fldpi":::"memory");return 1;
}
void start(void){
 if(!initialize())ExitProcess(210);wine_resource_original[0]=(void*)&getter;wine_resource_original[1]=(void*)&mapper;wine_resource_original[2]=(void*)&unmapper;wine_map_storage_getter=&point;
 if(wine_resource_get((void*)1)!=(void*)2||GetLastError()!=0xcafe)ExitProcess(214);check_fp();
 for(mode=0;mode<5;++mode){u32 descriptor[3]={0};if(wine_resource_map((void*)2,0,descriptor,0,7)||GetLastError()!=0xabcd)ExitProcess(215);check_fp();if(wine_resource_unmap((void*)2,0)||GetLastError()!=0x1234)ExitProcess(216);check_fp();}
 if(wine_resource_map((void*)2,0,0,0,7)!=0x8876086cu||GetLastError()!=0xabcd||exports!=5||calls!=12)ExitProcess(217);check_fp();wine_resource_close(1);ExitProcess(0);
}
