/* Owned PE32 caller stubs at the pinned call-return RVAs. These are synthetic
 * guards/ownership fixtures, not execution of the original DirectX checker. */
#include "../runtime/shadow/win32_min.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
typedef i32 (WIN *Create)(void*,void**,void*);
typedef i32 (WIN *Surface)(void*,const u32*,void**,void*);
typedef i32 (WIN *Query)(void*,const u8*,void**);
typedef u32 (WIN *Ref)(void*);
typedef u32 (WIN *Finish)(void*);
static Finish finish;
static void** table(void* p){return *(void***)p;}
static const u8 ids[2][16]={
 {0x00,0x4e,0x04,0xda,0xb2,0x69,0xd0,0x11,0xa1,0xd5,0,0xaa,0,0xb8,0xdf,0xbb},
 {0x30,0x86,0x2b,0x0b,0x35,0xad,0xd0,0x11,0x8e,0xa6,0,0x60,0x97,0x97,0xea,0x5b}};
static void word(u8* p,u32 v){for(u32 i=0;i<4;++i)p[i]=(u8)(v>>(i*8));}
static void* forward(u32 at,u32 argc,void* function){
 u8* p=(u8*)at;
 for(u32 i=0;i<argc;++i){*p++=0xff;*p++=0x74;*p++=0x24;*p++=(u8)(argc*4);}
 *p++=0xb8;word(p,(u32)function);p+=4;*p++=0xff;*p++=0xd0;
 *p++=0xc2;*p++=(u8)(argc*4);*p=0;
 if(!FlushInstructionCache((HANDLE)-1,(void*)at,(u32)(p-(u8*)at)+1))ExitProcess(153);return (void*)at;
}
static void* cleanup(u32 at){
 u8* p=(u8*)at;*p++=0xff;*p++=0x74;*p++=0x24;*p++=0x10;
 *p++=0xff;*p++=0x15;word(p,(u32)&finish);p+=4;*p++=0xc2;*p++=16;*p=0;
 if(!FlushInstructionCache((HANDLE)-1,(void*)at,(u32)(p-(u8*)at)+1))ExitProcess(153);return (void*)at;
}
void start(void){
 char mode[2];if(GetEnvironmentVariableA("MNM_PROBE_CASE",mode,2)!=1)ExitProcess(140);
 void* module=LoadLibraryA("MnmDraw.dll");if(!module || GetModuleHandleA("ddraw.dll") || (u32)GetModuleHandleA(0)!=0x400000)ExitProcess(141);
 Create original=(Create)GetProcAddress(module,"NativeDrawCreate");
 finish=(Finish)GetProcAddress(module,"NativeDrawReleaseStartupProbeV1");if(!original || !finish)ExitProcess(142);
 u32 old_protect=0;
 if(!VirtualProtect((void*)0x4bd000,4096,0x40,&old_protect))ExitProcess(143);
 Create create=(Create)forward(0x4bd95a,3,(void*)original);
 void *draw=0,*surface=0,*three=0,*four=0;u32 desc[31]={108};
 if((mode[0]=='C'?original:create)(0,&draw,0))ExitProcess(144);
 if(((i32 (WIN *)(void*,void*,u32))table(draw)[20])(draw,0,8))ExitProcess(145);
 Surface surface_call=(Surface)forward(0x4bdac5,4,table(draw)[6]);
 desc[1]=1;desc[26]=0x200;if(surface_call(draw,desc,&surface,0))ExitProcess(146);
 if(mode[0]!='1' && ((Query)table(surface)[0])(surface,ids[0],&three))ExitProcess(147);
 if(mode[0]!='1' && mode[0]!='2'){
  if(((Query)table(surface)[0])(surface,ids[1],&four))ExitProcess(148);
  ((Ref)table(surface)[2])(surface);
 }
 if(mode[0]=='R')((Ref)table(four)[1])(four);
 if(mode[0]=='B'){
  u32 locked[31]={124};
  if(((i32 (WIN *)(void*,const i32*,u32*,u32,HANDLE))table(four)[25])(four,0,locked,1,0))ExitProcess(149);
 }
 if(mode[0]=='E'){
  u32 other[31]={108};void* child=0;other[1]=7;other[2]=2;other[3]=2;other[26]=0x840;
  if(((Surface)table(draw)[6])(draw,other,&child,0))ExitProcess(150);
 }
 u32 result;SetLastError(0x51);
 if(mode[0]=='W')result=finish(draw);
 else {
  void* code=cleanup(mode[0]=='1'?0x4bdb20:0x4bdb65);
  result=((u32 (WIN *)(u32,u32,u32,void*))code)(0,0,0,draw);
 }
 if(result!=1 || GetLastError()!=0x51 || ((u32 (WIN *)(void))GetProcAddress(module,"NativeDrawObjects"))() ||
    ((u32 (WIN *)(void))GetProcAddress(module,"NativeDrawSurfaces"))())ExitProcess(151);
 u32 record[4]={1,(u32)mode[0],0x51,0},done=0;
 HANDLE file=CreateFileA("startup-policy.bin",0x40000000,0,0,1,0x80,0);
 if(file==(HANDLE)-1 || !WriteFile(file,record,sizeof(record),&done,0) || done!=sizeof(record))ExitProcess(152);
 CloseHandle(file);ExitProcess(0);
}
