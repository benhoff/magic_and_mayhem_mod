#define MNM_DRAWING_SELFTEST
#define DllMain FixtureDllMain
#include "../runtime/drawing/session.c"
static u32 calls,mode;
static u8 expected_fp[512] __attribute__((aligned(16)));
static u32 __attribute__((cdecl)) copier(void* context,void* dst,u32 dstsub,u32 x,u32 y,u32 z,void* src,u32 srcsub,const u32* box,u32 flags){
 if(context!=(void*)9||dst!=(void*)1||dstsub!=2||x!=3||y!=4||z!=5||src!=(void*)2||srcsub!=6||flags!=7)ExitProcess(211);
 if(mode==0&&(!box||box[2]!=800||box[3]!=600))ExitProcess(212);
 if(mode==1&&box)ExitProcess(213);if(mode==2&&box!=(void*)1)ExitProcess(214);
 ++calls;SetLastError(0xabcd);__asm__ volatile("fninit; fld1":::"memory");snapshot_fp(expected_fp);return mode==2?0x8876086c:0;
}
static void check(u32 result){
 if(result!=(mode==2?0x8876086c:0)||GetLastError()!=0xabcd)ExitProcess(215);
 u8 actual[512] __attribute__((aligned(16)));memset(actual,0,512);snapshot_fp(actual);if(!equal(actual,expected_fp,512))ExitProcess(216);
}
void start(void){
 if(!initialize())ExitProcess(210);wine_copy_original=(void*)&copier;
 u32 box[6]={0,0,800,600,0,1};
 check(wine_copy_region((void*)9,(void*)1,2,3,4,5,(void*)2,6,box,7));
 mode=1;check(wine_copy_region((void*)9,(void*)1,2,3,4,5,(void*)2,6,0,7));
 mode=2;check(wine_copy_region((void*)9,(void*)1,2,3,4,5,(void*)2,6,(void*)1,7));
 if(calls!=3)ExitProcess(217);wine_submission_close(1);ExitProcess(0);
}
