/* Unchanged isolated clipper/DC caller and complete pixel outputs versus Wine.
 * Native primary/flip policies are checked separately, never driver equivalence. */
#include "../runtime/shadow/win32_min.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
API void* WIN GetDesktopWindow(void);
API u32 WIN GetPixel(void*,i32,i32);
API u32 WIN SetPixel(void*,i32,i32,u32);
API int WIN GdiFlush(void);
typedef i32 (WIN *Create)(void*,void**,void*);
typedef u32 (WIN *Ref)(void*);
typedef i32 (WIN *Surface)(void*,const u32*,void**,void*);
typedef i32 (WIN *Lock)(void*,const i32*,u32*,u32,HANDLE);
typedef i32 (WIN *Unlock)(void*,void*);
#define W 13
#define H 9
#define WORDS (32+3*W*H)
static u32 results[64][WORDS];
static void** table(void* p){return *(void***)p;}
static void desc(u32* d){for(u32 i=0;i<31;++i)d[i]=0;d[0]=108;d[1]=0x1007;d[2]=H;d[3]=W;
 d[18]=32;d[19]=0x40;d[21]=16;d[22]=0xf800;d[23]=0x7e0;d[24]=0x1f;d[26]=0x840;}
static void words(void* a,u32* out,u32 seed,u32 write){
 u32 d[31];desc(d);if(((Lock)table(a)[25])(a,0,d,1,0))ExitProcess(164);
 for(u32 y=0;y<H;++y){u16* row=(u16*)(d[9]+y*d[4]);for(u32 x=0;x<W;++x){
  if(write)row[x]=(u16)(seed+x*1723+y*4099);if(out)out[y*W+x]=row[x];}}
 if(((Unlock)table(a)[32])(a,(void*)d[9]))ExitProcess(165);
}
static void primary(Create create,void* module){
 void *draw=0,*front=0,*back=0,*window=GetDesktopWindow();u32 d[31];
 if(create(0,&draw,0) || ((i32 (WIN *)(void*,void*,u32))table(draw)[20])(draw,window,0x11))ExitProcess(180);
 if(((i32 (WIN *)(void*,u32,u32,u32))table(draw)[21])(draw,W,H,16))ExitProcess(181);
 desc(d);d[1]=0x21;d[26]=0x218;d[5]=1;
 if(((Surface)table(draw)[6])(draw,d,&front,0))ExitProcess(182);
 u32 caps[4]={4};if(((i32 (WIN *)(void*,const u32*,void**))table(front)[12])(front,caps,&back))ExitProcess(183);
 words(front,0,123,1);words(back,0,531,1);u32 a[W*H],b[W*H],af[W*H],bf[W*H];words(front,a,0,0);words(back,b,0,0);
 if(((i32 (WIN *)(void*,void*,u32))table(front)[11])(front,0,1))ExitProcess(184);
 words(front,af,0,0);words(back,bf,0,0);
 for(u32 i=0;i<W*H;++i)if(a[i]!=bf[i] || b[i]!=af[i])ExitProcess(185);
 void* denied=(void*)0x1234;
 if(((Surface)table(draw)[6])(draw,d,&denied,0)!=(i32)0x88760234u || denied)ExitProcess(186);
 ((Ref)table(front)[2])(front);words(back,bf,0,0);
 for(u32 i=0;i<W*H;++i)if(a[i]!=bf[i])ExitProcess(187);
 ((Ref)table(back)[2])(back);((Ref)table(draw)[2])(draw);
 typedef u32 (WIN *Count)(void);Count count=(Count)GetProcAddress(module,"NativeDrawSurfaces");if(count())ExitProcess(188);
 count=(Count)GetProcAddress(module,"NativeDrawObjects");if(count())ExitProcess(189);
 u32 policy[4]={2,W*H*3,1,0},n=0;HANDLE out=CreateFileA("services-policy.bin",0x40000000,0,0,2,0x80,0);
 if(out==(HANDLE)-1 || !WriteFile(out,policy,sizeof(policy),&n,0) || n!=sizeof(policy))ExitProcess(190);CloseHandle(out);
}
void start(void){
 char value[2];u32 native=GetEnvironmentVariableA("MNM_FACTORY_NATIVE",value,2)!=0;
 void* module=LoadLibraryA(native?"MnmDraw.dll":"ddraw.dll");if(!module)ExitProcess(160);
 Create create=(Create)GetProcAddress(module,native?"NativeDrawCreate":"DirectDrawCreate");if(!create)ExitProcess(161);
 for(u32 i=0;i<64;++i){u32* r=results[i];void *draw=0,*a=0,*clipper=0,*queried=0,*dc=0;u32 d[31];
  r[0]=(u32)create(0,&draw,0);if(r[0] || !draw)ExitProcess(162);
  r[1]=(u32)((i32 (WIN *)(void*,void*,u32))table(draw)[20])(draw,GetDesktopWindow(),8);
  desc(d);r[2]=(u32)((Surface)table(draw)[6])(draw,d,&a,0);if(r[1] || r[2] || !a)ExitProcess(163);
  words(a,0,i*809,1);
  r[3]=(u32)((i32 (WIN *)(void*,u32,void**,void*))table(draw)[4])(draw,0,&clipper,0);if(r[3] || !clipper)ExitProcess(166);
  r[4]=(u32)((i32 (WIN *)(void*,u32,void*))table(clipper)[8])(clipper,0,GetDesktopWindow());
  void* window=0;r[5]=(u32)((i32 (WIN *)(void*,void**))table(clipper)[4])(clipper,&window);r[6]=window==GetDesktopWindow();
  r[7]=(u32)((i32 (WIN *)(void*,void*))table(a)[28])(a,clipper);
  r[8]=(u32)((i32 (WIN *)(void*,void**))table(a)[15])(a,&queried);r[9]=queried==clipper;
  r[10]=((Ref)table(queried)[2])(queried);r[11]=(u32)((i32 (WIN *)(void*,void*))table(a)[28])(a,0);r[12]=((Ref)table(clipper)[2])(clipper);
  desc(d);r[13]=(u32)((Lock)table(a)[25])(a,0,d,1,0);
  r[14]=(u32)((i32 (WIN *)(void*,void**))table(a)[17])(a,&dc);
  if(r[13] || r[14] || !dc)ExitProcess(167);
  r[15]=(u32)((Unlock)table(a)[32])(a,(void*)d[9]);
  void* second=(void*)0xabababab;r[16]=(u32)((i32 (WIN *)(void*,void**))table(a)[17])(a,&second);r[17]=second==(void*)0xabababab;
  desc(d);r[18]=(u32)((Lock)table(a)[25])(a,0,d,1,0);
  for(u32 y=0;y<H;++y)for(u32 x=0;x<W;++x)r[32+y*W+x]=GetPixel(dc,(i32)x,(i32)y);
  for(u32 y=0;y<H;++y)for(u32 x=0;x<W;++x)if((x+y)%3==0){u32 color=((x*19+i*7)&255)|(((y*31+i*13)&255)<<8)|(((x*11+y*17+i*23)&255)<<16);SetPixel(dc,(i32)x,(i32)y,color);}
  r[19]=(u32)GdiFlush();for(u32 y=0;y<H;++y)for(u32 x=0;x<W;++x)r[32+W*H+y*W+x]=GetPixel(dc,(i32)x,(i32)y);
  r[20]=(u32)((i32 (WIN *)(void*,void*))table(a)[26])(a,dc);
  r[21]=(u32)((i32 (WIN *)(void*,void*))table(a)[26])(a,dc);
  words(a,r+32+2*W*H,0,0);r[22]=((Ref)table(a)[2])(a);r[23]=((Ref)table(draw)[2])(draw);
  if(native){typedef u32 (WIN *Count)(void);Count count=(Count)GetProcAddress(module,"NativeDrawDCs");r[24]=count();count=(Count)GetProcAddress(module,"NativeDrawClippers");r[25]=count();count=(Count)GetProcAddress(module,"NativeDrawSurfaces");r[26]=count();count=(Count)GetProcAddress(module,"NativeDrawObjects");r[27]=count();}
 }
 if(native){primary(create,module);if(GetModuleHandleA("ddraw.dll") || LoadLibraryA("ddraw.dll"))ExitProcess(168);}
 HANDLE out=CreateFileA("services-results.bin",0x40000000,0,0,2,0x80,0);u32 n=0;
 if(out==(HANDLE)-1 || !WriteFile(out,results,sizeof(results),&n,0) || n!=sizeof(results))ExitProcess(169);CloseHandle(out);ExitProcess(0);
}
