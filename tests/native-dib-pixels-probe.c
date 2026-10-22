#include "../runtime/shadow/win32_min.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
API void* WIN GetDesktopWindow(void);
API i32 WIN StretchDIBits(void*,i32,i32,i32,i32,i32,i32,i32,i32,const void*,const void*,u32,u32);
API void* WIN CreateCompatibleBitmap(void*,i32,i32);
API void* WIN SelectObject(void*,void*);
API i32 WIN DeleteObject(void*);
typedef i32 (WIN *Dib)(void*,i32,i32,i32,i32,i32,i32,i32,i32,const u8*,const u8*,u32,u32);
typedef i32 (WIN *Create)(void*,void**,void*);
typedef u32 (WIN *Count)(void);
typedef u32 (WIN *Ref)(void*);
static u32 results[256][12];
static void** table(void* p){return *(void***)p;}
static u32 selected_format;
static void desc(u32* d){for(u32 i=0;i<31;++i)d[i]=0;d[0]=108;d[1]=0x1007;d[2]=9;d[3]=13;
 d[18]=32;d[19]=0x40;d[21]=16;d[22]=selected_format?0x7c00:0xf800;d[23]=selected_format?0x3e0:0x7e0;d[24]=0x1f;d[26]=0x840;}
static void words(void* surface,u16* pixels,u32 write){
 u32 d[31];desc(d);if(((i32 (WIN *)(void*,void*,u32*,u32,HANDLE))table(surface)[25])(surface,0,d,1,0))ExitProcess(172);
 for(u32 y=0;y<9;++y)for(u32 x=0;x<13;++x){u16* p=(u16*)(d[9]+y*d[4])+x;
  if(write)*p=pixels[y*13+x];else pixels[y*13+x]=*p;}
 if(((i32 (WIN *)(void*,void*))table(surface)[32])(surface,(void*)d[9]))ExitProcess(173);
}
void start(void){
 void* module=LoadLibraryA("MnmDraw.dll");if(!module)ExitProcess(170);
 Create create=(Create)GetProcAddress(module,"NativeDrawCreate");Dib paint=(Dib)GetProcAddress(module,"NativeDrawDibWordsV1");
 Count dcs=(Count)GetProcAddress(module,"NativeDrawDCs"),surfaces=(Count)GetProcAddress(module,"NativeDrawSurfaces");if(!create || !paint || !dcs || !surfaces)ExitProcess(171);
 for(u32 cell=0;cell<256;++cell){selected_format=cell/128;u32* r=results[cell];const u32 bits=cell%4==0?1:cell%4==1?4:cell%4==2?8:24,w=7,h=5,stride=((w*bits+31)/32)*4,usage=bits==24?1:0;
  u32 info[266];u8 source[160];for(u32 i=0;i<10;++i)((volatile u32*)info)[i]=0;
  info[0]=40;info[1]=w;info[2]=h;info[3]=1|(bits<<16);
  for(u32 i=0;i<256;++i){u8* c=(u8*)(info+10+i);c[0]=(u8)(i*71+cell*29);c[1]=(u8)(i*137+cell*43);c[2]=(u8)(i*97+cell*13);c[3]=0;}
  for(u32 i=0;i<stride*h;++i)source[i]=(u8)(i*113+cell*17);
  const i32 x=(i32)(cell/4%8)-4,y=(i32)(cell/32%4)-2;
  void* draw=0,*a=0,*b=0,*ad=0,*bd=0;u32 d[31];u16 seed[117],expected[117],actual[117];
  for(u32 i=0;i<117;++i)seed[i]=(u16)(i*7919+cell*127);
  if(create(0,&draw,0) || ((i32 (WIN *)(void*,void*,u32))table(draw)[20])(draw,GetDesktopWindow(),8))ExitProcess(174);
  desc(d);typedef i32 (WIN *Surface)(void*,const u32*,void**,void*);
  if(((Surface)table(draw)[6])(draw,d,&a,0) || ((Surface)table(draw)[6])(draw,d,&b,0))ExitProcess(175);
  words(a,seed,1);words(b,seed,1);
  if(((i32 (WIN *)(void*,void**))table(a)[17])(a,&ad) || ((i32 (WIN *)(void*,void**))table(b)[17])(b,&bd))ExitProcess(176);
  r[0]=(u32)StretchDIBits(ad,x,y,w,h,0,0,w,h,source,info,usage,0xcc0020);
  r[1]=(u32)paint(bd,x,y,w,h,0,0,w,h,source,(const u8*)info,usage,0xcc0020);
  /* A caller-selected replacement bitmap cannot acquire our owned lease. */
  void* foreign=CreateCompatibleBitmap(bd,13,9);if(!foreign)ExitProcess(179);
  void* selected=SelectObject(bd,foreign);if(!selected)ExitProcess(180);
  if((u32)paint(bd,x,y,w,h,0,0,w,h,source,(const u8*)info,usage,0xcc0020)!=0x80000000u)ExitProcess(181);
  if(SelectObject(bd,selected)!=foreign||!DeleteObject(foreign))ExitProcess(182);
  r[3]=(u32)paint((void*)0x1234,x,y,w,h,0,0,w,h,source,(const u8*)info,usage,0xcc0020);
  r[4]=(u32)paint(bd,x,y,w,h,0,0,w,h,source,(const u8*)info,usage,0x1234);
  r[5]=(u32)paint(bd,x,y,w,h,1,0,w,h,source,(const u8*)info,usage,0xcc0020);
  if(((i32 (WIN *)(void*,void*))table(a)[26])(a,ad) || ((i32 (WIN *)(void*,void*))table(b)[26])(b,bd))ExitProcess(177);
  r[6]=(u32)paint(bd,x,y,w,h,0,0,w,h,source,(const u8*)info,usage,0xcc0020);
  words(a,expected,0);words(b,actual,0);for(u32 i=0;i<117;++i)if(expected[i]!=actual[i])++r[2];
  ((Ref)table(a)[2])(a);((Ref)table(b)[2])(b);((Ref)table(draw)[2])(draw);r[7]=dcs();r[8]=surfaces();r[9]=bits;r[10]=(u32)x;r[11]=(u32)y;
 }
 HANDLE f=CreateFileA("dib-pixels-results.bin",0x40000000,0,0,1,0x80,0);u32 done=0;
 if(f==(HANDLE)-1 || !WriteFile(f,results,sizeof(results),&done,0) || done!=sizeof(results))ExitProcess(178);
 CloseHandle(f);ExitProcess(0);
}
