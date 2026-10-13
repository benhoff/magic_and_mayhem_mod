#define MNM_DRAWING_SELFTEST
#define DllMain FixtureDllMain
#include "../runtime/drawing/session.c"
static u32 mode,blits,maps,unmaps,active,getters,descriptors;
static void* expected_source;static u8* backing;static HANDLE forwards;
static u8 entry_fp[512] __attribute__((aligned(16))),return_fp[512] __attribute__((aligned(16)));
static void check_fp(const u8* expected){u8 actual[512] __attribute__((aligned(16)))={0};snapshot_fp(actual);if(!equal(actual,expected,512))ExitProcess(231);}
static void run(void*);
static void* __attribute__((cdecl)) resource(void* texture){if(texture!=(void*)2)ExitProcess(232);++getters;SetLastError(0xcafe);__asm__ volatile("fninit; fldpi":::"memory");return mode==13?0:(void*)3;}
static u32 __attribute__((cdecl)) descriptor(void* object,u32 sub,u32* out){
 if(object!=(void*)3||sub)ExitProcess(233);++descriptors;for(u32 i=0;i<10;++i)out[i]=0;
 out[0]=mode==1?117:116;out[5]=mode==2?14:13;out[6]=mode==11?400:mode==12?50:40;out[7]=mode==11?280:40;out[8]=1;out[9]=out[6]*out[7]*2;
 SetLastError(0xabcd);__asm__ volatile("fninit; fldpi":::"memory");return mode==14?0x8876086cu:0;
}
static u32 __attribute__((cdecl)) fixture_map(void* object,u32 sub,u32* out,const u32* box,u32 flags){
 if(object!=(void*)3||sub||box||flags!=0x80000000u||active)ExitProcess(234);++maps;
 SetLastError(0xface);__asm__ volatile("fninit; fldpi":::"memory");if(mode==4)return 0x8876086cu;
 active=1;out[0]=mode==11?804:mode==6?79:mode==7?32769:84;out[1]=mode==10?0:out[0]*(mode==11?280:40);out[2]=mode==5?0:mode==8?1:(u32)backing;
 if(mode==9){for(u32 i=0;i<2048;++i)if(wine_lifetime_objects[i].pointer==(void*)2)wine_lifetime_objects[i].pointer=0;++wine_lifetime_records;}
 if(mode==16)run((void*)2);
 return 0;
}
static u32 __attribute__((cdecl)) unmap(void* object,u32 sub){
 if(object!=(void*)3||sub||!active)ExitProcess(235);++unmaps;SetLastError(0xf00d);__asm__ volatile("fninit; fldpi":::"memory");if(mode==15)return 0x8876086cu;
 active=0;for(u32 i=0;i<225120;++i)backing[i]=0x77;return 0;
}
static u32 __attribute__((cdecl)) blt(void* context,void* dst,u32 dstsub,const void* dstrect,void* src,u32 srcsub,const void* srcrect,u32 flags,const void* fx,u32 filter){
 if(context!=(void*)6||dst!=(void*)1||dstsub||dstrect||src!=expected_source||srcsub||srcrect||flags!=7||fx||filter!=1||active||GetLastError()!=0xbeef)ExitProcess(236);
 check_fp(entry_fp);++blits;if(!write(forwards,&blits,4))ExitProcess(237);SetLastError(0x5678);__asm__ volatile("fninit; fld1":::"memory");snapshot_fp(return_fp);return 0x87654321u;
}
static void run(void* source){
 u32 width=mode==11?400:40,height=mode==11?280:40,pitch=width*2+4;
 for(u32 y=0;y<height;++y){for(u32 x=0;x<width*2;++x)backing[y*pitch+x]=(u8)(x*3+y*5+7);for(u32 x=width*2;x<pitch;++x)backing[y*pitch+x]=0xee;}
 expected_source=source;SetLastError(0xbeef);__asm__ volatile("fninit; fldz":::"memory");snapshot_fp(entry_fp);
 if(wine_submission_blt((void*)6,(void*)1,0,0,source,0,0,7,0,1)!=0x87654321u||GetLastError()!=0x5678)ExitProcess(238);check_fp(return_fp);
}
void start(void){
 if(!initialize()||!wine_source_map_enabled)ExitProcess(230);
 backing=VirtualAlloc(0,225120,0x3000,4);if(!backing)ExitProcess(239);
 char name[600];path(name,"fixture-forwards.bin");forwards=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(forwards==(HANDLE)-1)ExitProcess(240);
 wine_resource_original[0]=(void*)&resource;wine_resource_original[1]=(void*)&fixture_map;wine_resource_original[2]=(void*)&unmap;wine_source_descriptor_get=&descriptor;wine_submission_original[0]=(void*)&blt;wine_lifetime_create((void*)2,1);
 char fatal[2];if(GetEnvironmentVariableA("MNM_TEST_SOURCE_MAP_FATAL",fatal,2)==1){mode=fatal[0]=='1'?15:16;run((void*)2);ExitProcess(241);}
 wine_source_map_enabled=wine_source_descriptor_enabled=0;run((void*)2);if(maps||unmaps||wine_source_map_samples||getters||descriptors)ExitProcess(242);wine_source_map_enabled=wine_source_descriptor_enabled=1;
 for(mode=0;mode<=12;++mode){
  if(mode==3){wine_resource_active[0].resource=(void*)3;wine_resource_active[0].sub=0;}
  run((void*)2);wine_resource_active[0].resource=0;
  if(mode==9)wine_lifetime_create((void*)2,1);
 }
 mode=0;for(u32 i=0;i<8;++i)run((void*)2);run((void*)2);run((void*)4);run(0);mode=13;run((void*)2);mode=14;run((void*)2);
 if(blits!=27||maps!=17||unmaps!=16||wine_source_map_samples!=26||wine_source_map_payloads!=11||wine_source_map_attempts[0]!=16||wine_source_map_attempts[1]!=1||active)ExitProcess(243);
 if(!CloseHandle(forwards))ExitProcess(244);wine_submission_close(1);ExitProcess(0);
}
