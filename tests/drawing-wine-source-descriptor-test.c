#define MNM_DRAWING_SELFTEST
#define DllMain FixtureDllMain
#include "../runtime/drawing/session.c"
static u32 mode,blits,getters,descriptors;
static void* expected_source;
static u8 entry_fp[512] __attribute__((aligned(16))),return_fp[512] __attribute__((aligned(16)));
static void check_fp(const u8* expected){u8 actual[512] __attribute__((aligned(16)))={0};snapshot_fp(actual);if(!equal(actual,expected,512))ExitProcess(231);}
static void* __attribute__((cdecl)) get_resource(void* texture){
 if(texture!=(void*)2)ExitProcess(232);++getters;SetLastError(0xcafe);__asm__ volatile("fninit; fldpi":::"memory");return mode==2?0:(void*)3;
}
static u32 __attribute__((cdecl)) get_descriptor(void* resource,u32 sub,u32* out){
 if(resource!=(void*)3||sub)ExitProcess(233);++descriptors;
 for(u32 i=0;i<10;++i)out[i]=i+100;out[0]=116;out[6]=40;out[7]=40;out[8]=1;out[9]=3200;
 if(mode==4){for(u32 i=0;i<2048;++i)if(wine_lifetime_objects[i].pointer==(void*)2)wine_lifetime_objects[i].pointer=0;++wine_lifetime_records;}
 SetLastError(0xabcd);__asm__ volatile("fninit; fldpi":::"memory");return mode==3?0x8876086cu:0;
}
static u32 __attribute__((cdecl)) blt(void* context,void* dst,u32 dstsub,const void* dstrect,void* src,u32 srcsub,const void* srcrect,u32 flags,const void* fx,u32 filter){
 if(context!=(void*)6||dst!=(void*)1||dstsub||dstrect||src!=expected_source||srcsub||srcrect||flags!=7||fx||filter!=1||GetLastError()!=0xbeef)ExitProcess(234);
 check_fp(entry_fp);++blits;SetLastError(0x5678);__asm__ volatile("fninit; fld1":::"memory");snapshot_fp(return_fp);return 0x87654321u;
}
static void run(void* source){
 expected_source=source;SetLastError(0xbeef);__asm__ volatile("fninit; fldz":::"memory");snapshot_fp(entry_fp);
 if(wine_submission_blt((void*)6,(void*)1,0,0,source,0,0,7,0,1)!=0x87654321u||GetLastError()!=0x5678)ExitProcess(235);check_fp(return_fp);
}
void start(void){
 if(!initialize()||!wine_source_descriptor_enabled)ExitProcess(230);
 wine_resource_original[0]=(void*)&get_resource;wine_source_descriptor_get=&get_descriptor;wine_submission_original[0]=(void*)&blt;
 wine_lifetime_create((void*)2,1);
 wine_source_descriptor_enabled=0;run((void*)2);if(getters||descriptors)ExitProcess(236);wine_source_descriptor_enabled=1;
 mode=0;run((void*)2);run((void*)4);mode=2;run((void*)2);mode=3;run((void*)2);mode=4;run((void*)2);run(0);
 if(blits!=7||getters!=4||descriptors!=3||wine_source_descriptor_samples!=6)ExitProcess(237);
 wine_submission_close(1);ExitProcess(0);
}
