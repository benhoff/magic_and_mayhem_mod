#define MNM_DRAWING_SELFTEST
#define DllMain FixtureDllMain
#include "../runtime/drawing/session.c"
static u32 mode,alternate,resource_calls,blit_calls,life_calls;
static u8 expected_fp[512] __attribute__((aligned(16)));
static void original_fp(void){__asm__ volatile("fninit; fld1":::"memory");snapshot_fp(expected_fp);}
static void check_fp(void){u8 actual[512] __attribute__((aligned(16)));memset(actual,0,512);snapshot_fp(actual);if(!equal(actual,expected_fp,512))ExitProcess(221);}
static u32 __attribute__((cdecl)) create_texture(void* a,const void* b,u32 c,u32 d,u32 e,const void* f,void* g,const void* h,void** out){if(a||b||c||d||e||f||g||h||!out)ExitProcess(211);++life_calls;*out=(void*)1;SetLastError(0xbeef);original_fp();return 0;}
static u32 __attribute__((cdecl)) release_texture(void* texture){if(texture!=(void*)1)ExitProcess(212);++life_calls;SetLastError(0xbeef);original_fp();return 0;}
static void new_generation(void){void* texture=0;if(wine_lifetime_texture_create(0,0,0,0,0,0,0,0,&texture)||texture!=(void*)1)ExitProcess(213);check_fp();}
static void recycle_generation(void){if(wine_lifetime_texture_decref((void*)1))ExitProcess(214);check_fp();new_generation();}
static void* __attribute__((cdecl)) resource_getter(void* texture){if(texture!=(void*)1)ExitProcess(215);++resource_calls;SetLastError(0xabcd);original_fp();return alternate?(void*)4:(void*)2;}
static u32 __attribute__((cdecl)) resource_mapper(void* resource,u32 sub,u32* out,const u32* box,u32 flags){if((resource!=(void*)2&&resource!=(void*)4)||sub||box||flags!=7)ExitProcess(216);++resource_calls;SetLastError(0xabcd);original_fp();if(mode==1)return 0x8876086cu;out[0]=1600;out[1]=960000;out[2]=3;return 0;}
static u32 __attribute__((cdecl)) resource_unmapper(void* resource,u32 sub){if((resource!=(void*)2&&resource!=(void*)4)||sub)ExitProcess(217);++resource_calls;SetLastError(0xabcd);original_fp();return 0;}
static u32 __attribute__((cdecl)) blitter(void* a,void* b,u32 c,const void* d,void* e,u32 f,const void* g,u32 h,const void* i,u32 j){if(a||b!=(void*)6||c||d||e!=(void*)1||f||g||h||i||j)ExitProcess(218);++blit_calls;SetLastError(0x1234);original_fp();return 0;}
static u32 WIN point(void* memory,u32 row,u32 slice,u32* out,u32 size){
 if(memory!=(void*)3||row!=1600||slice!=960000||size!=64)ExitProcess(219);
 /* Real observer transitions occur during the outside-lock callback. */
 if(mode==2){mode=0;if(wine_resource_unmap((void*)2,0))ExitProcess(222);}
 else if(mode==3){mode=0;recycle_generation();}
 u32 m[16]={64,1,19,7,0,0,7,800,600,800,3,3,0,0,0,0};m[14]=GetCurrentThreadId();copy(out,m,64);SetLastError(0xdead);__asm__ volatile("fninit; fldpi":::"memory");return 1;
}
static void fixture_source_query(void){if(wine_submission_blt(0,(void*)6,0,0,(void*)1,0,0,0,0,0)||GetLastError()!=0x1234)ExitProcess(223);check_fp();}
static void observe_map(void* resource){u32 desc[3]={0};if(wine_resource_map(resource,0,desc,0,7)||GetLastError()!=0xabcd)ExitProcess(224);check_fp();}
static void observe_resource(void){if(wine_resource_get((void*)1)!=(alternate?(void*)4:(void*)2))ExitProcess(225);check_fp();}
void start(void){
 if(!initialize())ExitProcess(210);wine_lifetime_original[0]=(void*)&create_texture;wine_lifetime_original[1]=(void*)&release_texture;wine_resource_original[0]=(void*)&resource_getter;wine_resource_original[1]=(void*)&resource_mapper;wine_resource_original[2]=(void*)&resource_unmapper;wine_submission_original[0]=(void*)&blitter;wine_map_storage_getter=&point;
 new_generation();observe_resource();observe_map((void*)2);fixture_source_query();
 if(wine_resource_unmap((void*)2,0))ExitProcess(226);check_fp();fixture_source_query();
 mode=1;if(wine_resource_map((void*)2,0,0,0,7)!=0x8876086cu)ExitProcess(227);check_fp();mode=0;fixture_source_query();
 recycle_generation();fixture_source_query();observe_resource();observe_map((void*)2);fixture_source_query();
 mode=2;fixture_source_query();fixture_source_query();mode=3;fixture_source_query();fixture_source_query();
 observe_resource();observe_map((void*)2);alternate=1;observe_resource();observe_map((void*)4);fixture_source_query();
 if(wine_resource_unmap((void*)2,0)||wine_resource_unmap((void*)4,0))ExitProcess(228);check_fp();
 if(resource_calls!=13||blit_calls!=10||life_calls!=5)ExitProcess(229);wine_submission_close(1);ExitProcess(0);
}
