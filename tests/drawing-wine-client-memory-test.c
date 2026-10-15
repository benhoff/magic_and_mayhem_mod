#define MNM_DRAWING_SELFTEST
#define DllMain FixtureDllMain
#include "../runtime/drawing/session.c"
static u32 mode,calls,queries,fpchecks,life_calls;
static u8 expected_fp[512] __attribute__((aligned(16)));
static void original_fp(void){__asm__ volatile("fninit; fld1":::"memory");snapshot_fp(expected_fp);}
static void check_fp(void){u8 actual[512] __attribute__((aligned(16)));memset(actual,0,512);snapshot_fp(actual);++fpchecks;if(!equal(actual,expected_fp,512)){u32 diagnostic[4]={fpchecks,mode,calls,512};for(u32 i=0;i<512;++i)if(actual[i]!=expected_fp[i]){diagnostic[3]=i;break;}char name[600];path(name,"fixture-fp-difference.bin");HANDLE f=CreateFileA(name,0x40000000,0,0,1,0x80,0);if(f!=(HANDLE)-1){write(f,diagnostic,16);write(f,expected_fp,512);write(f,actual,512);CloseHandle(f);}ExitProcess(221);}}
static u32 __attribute__((cdecl)) setter(void* texture,u32 sub,void* memory,u32 pitch){
 if(texture!=(void*)1||sub||(mode==1?(memory!=(void*)5||pitch!=1700):mode==2?(memory||pitch):(memory!=(void*)3||pitch!=1600)))ExitProcess(211);
 ++calls;SetLastError(0xabcd);original_fp();return mode==1?0x8876086cu:0;
}
static u32 __attribute__((cdecl)) blitter(void* context,void* dst,u32 dstsub,const void* dstrect,void* src,u32 srcsub,const void* srcrect,u32 flags,const void* fx,u32 filter){
 if(context||dst!=(void*)4||dstsub||dstrect||src!=(void*)1||srcsub||srcrect||flags||fx||filter)ExitProcess(212);
 ++calls;SetLastError(0x1234);original_fp();return 0;
}
static u32 __attribute__((cdecl)) create_texture(void* device,const void* desc,u32 layers,u32 levels,u32 flags,const void* data,void* parent,const void* ops,void** output){
 if(device||desc||layers||levels||flags||data||parent||ops||!output)ExitProcess(222);++life_calls;*output=(void*)1;SetLastError(0xbeef);original_fp();return 0;
}
static u32 __attribute__((cdecl)) release_texture(void* texture){if(texture!=(void*)1)ExitProcess(223);++life_calls;SetLastError(0xbeef);original_fp();return 0;}
static void create_generation(void){void* created=0;if(wine_lifetime_texture_create(0,0,0,0,0,0,0,0,&created)||created!=(void*)1||GetLastError()!=0xbeef)ExitProcess(224);check_fp();}
static u32 WIN point(void* memory,u32 pitch,u32* out,u32 size){
 if(size!=64)ExitProcess(213);u32 r[16]={64,1,3,7,0,0,0,0,0,0,0,0,0,0,0,0};r[14]=GetCurrentThreadId();
 if(memory==(void*)3&&pitch==1600){r[2]=83;r[6]=7;r[7]=800;r[8]=600;r[9]=800;r[10]=7;r[11]=7;}
 copy(out,r,64);SetLastError(0xdead);__asm__ volatile("fninit; fldpi":::"memory");return 1;
}
static void fixture_source_query(void){if(wine_submission_blt(0,(void*)4,0,0,(void*)1,0,0,0,0,0)||GetLastError()!=0x1234)ExitProcess(214);check_fp();++queries;}
void start(void){
 if(!initialize())ExitProcess(210);wine_client_original=(void*)&setter;wine_client_getter=&point;wine_submission_original[0]=(void*)&blitter;wine_lifetime_original[0]=(void*)&create_texture;wine_lifetime_original[1]=(void*)&release_texture;create_generation();
 if(wine_client_update((void*)1,0,(void*)3,1600)||GetLastError()!=0xabcd)ExitProcess(215);check_fp();fixture_source_query();
 mode=1;if(wine_client_update((void*)1,0,(void*)5,1700)!=0x8876086cu||GetLastError()!=0xabcd)ExitProcess(216);check_fp();fixture_source_query();
 mode=2;if(wine_client_update((void*)1,0,0,0)||GetLastError()!=0xabcd)ExitProcess(217);check_fp();fixture_source_query();
 /* Same local address with a new texture generation cannot reuse old binding. */
 if(wine_lifetime_texture_decref((void*)1)||GetLastError()!=0xbeef)ExitProcess(225);check_fp();create_generation();fixture_source_query();mode=0;
 if(wine_client_update((void*)1,0,(void*)3,1600)||GetLastError()!=0xabcd)ExitProcess(218);check_fp();fixture_source_query();
 if(calls!=9||queries!=5||wine_client_versions!=3||life_calls!=3)ExitProcess(219);wine_submission_close(1);ExitProcess(0);
}
