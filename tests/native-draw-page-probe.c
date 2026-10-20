/* Unchanged Surface2/3/4 PageLock/PageUnlock callers, including leases and
 * repeated/unbalanced hints. Compare every result and pixel with Wine ddraw. */
#include "../runtime/shadow/win32_min.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
typedef i32 (WIN *Create)(void*,void**,void*);
typedef i32 (WIN *Query)(void*,const u8*,void**);
typedef u32 (WIN *Ref)(void*);
typedef i32 (WIN *Page)(void*,u32);
static void** table(void* p){return *(void***)p;}
static const u8 ids[3][16]={
 {0x85,0x58,0x80,0x57,0xec,0x6e,0xcf,0x11,0x94,0x41,0xa8,0x23,0x03,0xc1,0x0e,0x27},
 {0x00,0x4e,0x04,0xda,0xb2,0x69,0xd0,0x11,0xa1,0xd5,0,0xaa,0,0xb8,0xdf,0xbb},
 {0x30,0x86,0x2b,0x0b,0x35,0xad,0xd0,0x11,0x8e,0xa6,0,0x60,0x97,0x97,0xea,0x5b}};
static u32 results[30][32+13*9];
static void desc(u32* d,u32 version){
 for(u32 i=0;i<31;++i)d[i]=0;
 d[0]=version==2?124:108;d[1]=0x1007;d[2]=9;d[3]=13;d[18]=32;d[19]=0x40;d[21]=16;
 d[22]=0xf800;d[23]=0x7e0;d[24]=31;d[26]=0x840;
}
static void pixels(void* surface,u32* row,u32 seed,u32 write){
 u32 d[31];desc(d,0);
 if(((i32 (WIN *)(void*,const i32*,u32*,u32,HANDLE))table(surface)[25])(surface,0,d,1,0))ExitProcess(151);
 for(u32 y=0;y<9;++y)for(u32 x=0;x<13;++x){u16* p=(u16*)(d[9]+y*d[4])+x;
  if(write)*p=(u16)(seed+x*337+y*971);if(row)row[y*13+x]=*p;}
 if(((i32 (WIN *)(void*,void*))table(surface)[32])(surface,(void*)d[9]))ExitProcess(152);
}
void start(void){
 char native[2];u32 is_native=GetEnvironmentVariableA("MNM_FACTORY_NATIVE",native,2)==1 && native[0]=='1';
 void* module=LoadLibraryA(is_native?"MnmDraw.dll":"ddraw.dll");if(!module)ExitProcess(150);
 if(is_native && GetModuleHandleA("ddraw.dll"))ExitProcess(153);
 Create create=(Create)GetProcAddress(module,"DirectDrawCreate");if(!create)ExitProcess(154);
 const u32 flags[5]={0,1,2,0x80000000u,0xffffffffu};
 for(u32 version=0;version<3;++version)for(u32 borrow=0;borrow<2;++borrow)for(u32 f=0;f<5;++f){
  u32 at=version*10+borrow*5+f,*r=results[at],d[31];void *draw=0,*surface=0,*alias=0;
  r[0]=(u32)create(0,&draw,0);if(r[0])ExitProcess(155);
  if(((i32 (WIN *)(void*,void*,u32))table(draw)[20])(draw,0,8))ExitProcess(162);desc(d,0);
  r[1]=(u32)((i32 (WIN *)(void*,const u32*,void**,void*))table(draw)[6])(draw,d,&surface,0);if(r[1])ExitProcess(156);
  pixels(surface,0,at*79+11,1);r[2]=(u32)((Query)table(surface)[0])(surface,ids[version],&alias);if(r[2])ExitProcess(157);
  desc(d,0);
  if(borrow){r[3]=(u32)((i32 (WIN *)(void*,const i32*,u32*,u32,HANDLE))table(surface)[25])(surface,0,d,1,0);if(r[3])ExitProcess(158);}
  r[4]=(u32)((Page)table(alias)[38])(alias,flags[f]);
  r[5]=(u32)((Page)table(alias)[37])(alias,flags[f]);
  r[6]=(u32)((Page)table(alias)[37])(alias,0);
  r[7]=(u32)((Page)table(alias)[38])(alias,0);
  r[8]=(u32)((Page)table(alias)[38])(alias,0);
  r[9]=(u32)((Page)table(alias)[38])(alias,0);
  if(borrow){r[10]=(u32)((i32 (WIN *)(void*,void*))table(surface)[32])(surface,(void*)d[9]);if(r[10])ExitProcess(159);}
  pixels(surface,r+32,0,0);
  r[11]=((Ref)table(alias)[1])(alias);r[12]=((Ref)table(alias)[2])(alias);r[13]=((Ref)table(alias)[2])(alias);
  r[14]=((Ref)table(surface)[2])(surface);r[15]=((Ref)table(draw)[2])(draw);
  if(is_native){
   if(((u32 (WIN *)(void))GetProcAddress(module,"NativeDrawObjects"))() ||
      ((u32 (WIN *)(void))GetProcAddress(module,"NativeDrawSurfaces"))())ExitProcess(160);
  }
 }
 HANDLE file=CreateFileA("page-results.bin",0x40000000,0,0,1,0x80,0);u32 done=0;
 if(file==(HANDLE)-1 || !WriteFile(file,results,sizeof(results),&done,0) || done!=sizeof(results))ExitProcess(161);
 CloseHandle(file);ExitProcess(0);
}
