#include "../runtime/shadow/win32_min.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
API void* WIN GetDesktopWindow(void);
API i32 WIN BitBlt(void*,i32,i32,i32,i32,void*,i32,i32,u32);
API void* WIN CreateRectRgn(i32,i32,i32,i32);
API i32 WIN SelectClipRgn(void*,void*);
API i32 WIN DeleteObject(void*);
typedef i32 (WIN *Copy)(void*,i32,i32,i32,i32,void*,i32,i32,u32);
typedef i32 (WIN *Plane)(const void*,u32*,u32);
typedef i32 (WIN *Create)(void*,void**,void*);
typedef u32 (WIN *Count)(void);
typedef u32 (WIN *Ref)(void*);
static u32 results[512][16];
static void** table(void* p){return *(void***)p;}
static void desc(u32* d){for(u32 i=0;i<31;++i)d[i]=0;d[0]=108;d[1]=0x1007;d[2]=9;d[3]=13;d[18]=32;d[19]=0x40;d[21]=16;d[22]=0xf800;d[23]=0x7e0;d[24]=0x1f;d[26]=0x840;}
static void* words(void* surface,u16* pixels,u32 write,Plane plane,u32* results){
 u32 d[31],fields[6];desc(d);if(((i32 (WIN *)(void*,void*,u32*,u32,HANDLE))table(surface)[25])(surface,0,d,1,0))ExitProcess(172);
 if(plane((void*)d[9],fields,24)||fields[0]!=1||fields[1]!=13||fields[2]!=9||fields[3]!=d[4]/2||fields[4]!=1||fields[5])++results[13];
 for(u32 y=0;y<9;++y)for(u32 x=0;x<13;++x){u16* p=(u16*)(d[9]+y*d[4])+x;if(write)*p=pixels[y*13+x];else pixels[y*13+x]=*p;}
 if(((i32 (WIN *)(void*,void*))table(surface)[32])(surface,(void*)d[9]))ExitProcess(173);
 if(plane((void*)d[9],fields,24)||fields[4])++results[13];return (void*)d[9];
}
void start(void){
 void* module=LoadLibraryA("MnmDraw.dll"),*route=LoadLibraryA("MnmDcCopy.dll");if(!module||!route)ExitProcess(170);
 Create create=(Create)GetProcAddress(module,"NativeDrawCreate");Copy paint=(Copy)GetProcAddress(module,"NativeDrawBitBltV1"),copy=(Copy)GetProcAddress(route,"DcCopyForTest");
 Plane plane=(Plane)GetProcAddress(module,"NativeDrawPlaneV1");Count dcs=(Count)GetProcAddress(module,"NativeDrawDCs"),surfaces=(Count)GetProcAddress(module,"NativeDrawSurfaces"),native=(Count)GetProcAddress(route,"NativeDCCopiesV1"),original=(Count)GetProcAddress(route,"OriginalDCCopiesV1");
 if(!create||!paint||!copy||!plane||!dcs||!surfaces||!native||!original)ExitProcess(171);
 for(u32 cell=0;cell<512;++cell){u32* r=results[cell];const i32 x=(i32)(cell%8)-4,y=(i32)(cell/8%4)-2,sx=(i32)(cell/32%2)*3,sy=(i32)(cell/64%2)*2;const int self=cell/128%2;
  void *draw=0,*a=0,*b=0,*s=0,*ad=0,*bd=0,*sd=0;u32 d[31],fields[6];u16 seed[117],source[117],expected[117],actual[117];
  for(u32 i=0;i<117;++i){seed[i]=(u16)(i*7919+cell*127);source[i]=(u16)(i*5927+cell*109);}
  if(create(0,&draw,0)||((i32 (WIN *)(void*,void*,u32))table(draw)[20])(draw,GetDesktopWindow(),8))ExitProcess(174);
  desc(d);typedef i32 (WIN *Surface)(void*,const u32*,void**,void*);
  if(((Surface)table(draw)[6])(draw,d,&a,0)||((Surface)table(draw)[6])(draw,d,&b,0)||((Surface)table(draw)[6])(draw,d,&s,0))ExitProcess(175);
  words(a,seed,1,plane,r);void* retired=words(b,seed,1,plane,r);words(s,source,1,plane,r);
  if(((i32 (WIN *)(void*,void**))table(a)[17])(a,&ad)||((i32 (WIN *)(void*,void**))table(b)[17])(b,&bd)||((i32 (WIN *)(void*,void**))table(s)[17])(s,&sd))ExitProcess(176);
  r[14]=(u32)plane(retired,fields,24);
  r[0]=(u32)BitBlt(ad,x,y,7,5,self?ad:sd,sx,sy,0xcc0020);
  r[1]=(u32)copy(bd,x,y,7,5,self?bd:sd,sx,sy,0xcc0020);
  r[3]=(u32)paint((void*)0x1234,x,y,7,5,sd,sx,sy,0xcc0020);
  r[4]=(u32)paint(bd,x,y,7,5,sd,sx,sy,0x1234);
  r[5]=(u32)paint(bd,x,y,7,5,(void*)0x1234,sx,sy,0xcc0020);
  void* region=CreateRectRgn(1,1,7,5);if(!region||SelectClipRgn(bd,region)==0)ExitProcess(179);
  r[6]=(u32)paint(bd,x,y,7,5,sd,sx,sy,0xcc0020);SelectClipRgn(bd,0);DeleteObject(region);
  if(((i32 (WIN *)(void*,void*))table(a)[26])(a,ad)||((i32 (WIN *)(void*,void*))table(b)[26])(b,bd)||((i32 (WIN *)(void*,void*))table(s)[26])(s,sd))ExitProcess(177);
  r[7]=(u32)paint(bd,x,y,7,5,sd,sx,sy,0xcc0020);
  words(a,expected,0,plane,r);words(b,actual,0,plane,r);for(u32 i=0;i<117;++i)if(expected[i]!=actual[i])++r[2];
  ((Ref)table(a)[2])(a);((Ref)table(b)[2])(b);((Ref)table(s)[2])(s);((Ref)table(draw)[2])(draw);
  r[8]=dcs();r[9]=surfaces();r[10]=native();r[11]=original();r[12]=(u32)plane(retired,fields,24);r[15]=self;
 }
 HANDLE f=CreateFileA("dc-copy-results.bin",0x40000000,0,0,1,0x80,0);u32 done=0;
 if(f==(HANDLE)-1||!WriteFile(f,results,sizeof(results),&done,0)||done!=sizeof(results))ExitProcess(178);CloseHandle(f);ExitProcess(0);
}
