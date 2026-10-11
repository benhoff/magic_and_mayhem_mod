/* Unchanged caller/pixel corpus runs against Wine DirectDraw and native storage.
 * No original game execution or GPU World-history read is asserted. */
#include "../runtime/shadow/win32_min.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
API void* WIN GetDesktopWindow(void);
typedef i32 (WIN *Create)(void*,void**,void*);
typedef i32 (WIN *Query)(void*,const u8*,void**);
typedef u32 (WIN *Ref)(void*);
typedef i32 (WIN *Lock)(void*,const i32*,u32*,u32,HANDLE);
typedef i32 (WIN *Unlock)(void*,void*);
typedef i32 (WIN *Blt)(void*,const i32*,void*,const i32*,u32,const u32*);
typedef i32 (WIN *Fast)(void*,u32,u32,void*,const i32*,u32);
#define WIDTH 13
#define HEIGHT 9
#define WORDS (32+5*WIDTH*HEIGHT)
static u32 results[64][WORDS];
static const u8 dd4[16]={0x9a,0x50,0x59,0x9c,0xbd,0x39,0xd1,0x11,0x8c,0x4a,0,0xc0,0x4f,0xd9,0x30,0xc5};
static const u8 surface2[16]={0x85,0x58,0x80,0x57,0xec,0x6e,0xcf,0x11,0x94,0x41,0xa8,0x23,0x03,0xc1,0x0e,0x27};
static const u8 unknown[16]={0,0,0,0,0,0,0,0,0xc0,0,0,0,0,0,0,0x46};
static void** table(void* p){return *(void***)p;}
static u32 describe(u32* d,u32 newer){
 for(u32 j=0;j<31;++j)d[j]=0;d[0]=newer?124:108;d[1]=0x1007;d[2]=HEIGHT;d[3]=WIDTH;
 d[18]=32;d[19]=0x40;d[21]=16;d[22]=0xf800;d[23]=0x7e0;d[24]=0x1f;d[26]=0x840;return d[0];
}
static void pixels(void* object,u32 newer,u32* output,u32 seed,u32 write){
 u32 d[31]={0};d[0]=newer?124:108;
 if(((Lock)table(object)[25])(object,0,d,1,0))ExitProcess(122);
 if(d[2]!=HEIGHT || d[3]!=WIDTH || d[21]!=16 || d[22]!=0xf800 || d[23]!=0x7e0 || d[24]!=0x1f)ExitProcess(123);
 for(u32 y=0;y<HEIGHT;++y){u16* row=(u16*)(d[9]+y*d[4]);
  for(u32 x=0;x<WIDTH;++x){if(write)row[x]=(u16)(seed==0xffff?0x2bab:((x+y)%4?seed+x*811+y*241:0));
   if(output)output[y*WIDTH+x]=row[x];}}
 if(((Unlock)table(object)[32])(object,newer?0:(void*)d[9]))ExitProcess(124);
}
/* The public tooltip requests a 510x180 offscreen surface with no memory
 * placement or pixel format. Native policy selects owned system-memory RGB565.
 * This checks that policy, separately from the unchanged Wine corpus below. */
static void default_offscreen(Create create){
 for(u32 i=0;i<4;++i){
  void *draw=0,*fourth=0,*object=0;u32 d[31],newer=i&1;
  if(create(0,&draw,0) || ((Query)table(draw)[0])(draw,dd4,&fourth))ExitProcess(157);
  void* device=newer?fourth:draw;describe(d,newer);d[2]=180;d[3]=510;d[26]=0x800;
  if(!(i&2)){d[1]=7;for(u32 j=18;j<26;++j)d[j]=0;}
  if(((i32 (WIN *)(void*,const u32*,void**,void*))table(device)[6])(device,d,&object,0) || !object)ExitProcess(158);
  describe(d,newer);if(((i32 (WIN *)(void*,u32*))table(object)[22])(object,d) || d[2]!=180 || d[3]!=510 || d[26]!=0x840)ExitProcess(159);
  for(u32 pass=0;pass<2;++pass){
   describe(d,newer);if(((Lock)table(object)[25])(object,0,d,1,0) || d[4]!=1024 || d[21]!=16 || d[22]!=0xf800 || d[23]!=0x7e0 || d[24]!=0x1f)ExitProcess(160);
   for(u32 y=0;y<180;++y){u16* row=(u16*)(d[9]+y*d[4]);
    for(u32 x=0;x<510;++x){u16 expected=(u16)(i*911+x*811+y*241);if(!pass)row[x]=expected;else if(row[x]!=expected)ExitProcess(161);}}
   if(((Unlock)table(object)[32])(object,newer?0:(void*)d[9]))ExitProcess(162);
  }
  ((Ref)table(object)[2])(object);((Ref)table(fourth)[2])(fourth);((Ref)table(draw)[2])(draw);
 }
}
static void policies(Create create,void* module){
 void *draw=0,*a=0,*b=0,*held[512];u32 d[31],before[WIDTH*HEIGHT],after[WIDTH*HEIGHT];
 if(create(0,&draw,0))ExitProcess(140);describe(d,0);
 typedef i32 (WIN *Surface)(void*,const u32*,void**,void*);
 Surface surface=(Surface)table(draw)[6];
 if(surface(draw,d,&a,0) || surface(draw,d,&b,0))ExitProcess(141);
 pixels(a,0,0,17,1);pixels(b,0,before,0xffff,1);
 i32 source[4]={0,0,6,4},target[4]={0,0,7,4},invalid[4]={-1,0,6,4};
 if(((Blt)table(b)[5])(b,target,a,source,0x1000000,0)!=(i32)0x80004001u)ExitProcess(142);
 if(((Blt)table(b)[5])(b,invalid,a,source,0x1000000,0)!=(i32)0x88760096u)ExitProcess(143);
 if(((Fast)table(b)[7])(b,0xffffffffu,0,a,source,0x10)!=(i32)0x88760096u)ExitProcess(144);
 if(((Blt)table(b)[5])(b,0,a,0,0x1008000,0)!=(i32)0x887600d7u)ExitProcess(145);
 if(((Blt)table(b)[5])(b,0,a,0,0x200,0)!=(i32)0x80004001u)ExitProcess(146);
 describe(d,0);d[0]=107;if(((Lock)table(b)[25])(b,0,d,1,0)!=(i32)0x80070057u)ExitProcess(147);
 describe(d,0);if(((Lock)table(b)[25])(b,0,d,1,0))ExitProcess(148);
 u32 again[31];describe(again,0);if(((Lock)table(b)[25])(b,0,again,1,0)!=(i32)0x887601aeu)ExitProcess(149);
 if(((Unlock)table(b)[32])(b,(void*)d[9]) || ((Unlock)table(b)[32])(b,0)!=(i32)0x88760248u)ExitProcess(150);
 pixels(b,0,after,0,0);for(u32 i=0;i<WIDTH*HEIGHT;++i)if(before[i]!=after[i])ExitProcess(151);
 ((Ref)table(a)[2])(a);((Ref)table(b)[2])(b);
 describe(d,0);d[26]=0x1800;void* rejected=(void*)0x1234;
 if(surface(draw,d,&rejected,0)!=(i32)0x80004001u || rejected)ExitProcess(163);
 describe(d,0);d[26]=0x800;d[22]=0x7c00;rejected=(void*)0x1234;
 if(surface(draw,d,&rejected,0)!=(i32)0x80004001u || rejected)ExitProcess(164);
 default_offscreen(create);
 describe(d,0);d[2]=d[3]=1;
 for(u32 i=0;i<512;++i)if(surface(draw,d,&held[i],0))ExitProcess(152);
 void* denied=(void*)0x1234;if(surface(draw,d,&denied,0)!=(i32)0x8007000eu || denied)ExitProcess(153);
 for(u32 i=0;i<512;++i)((Ref)table(held[i])[2])(held[i]);((Ref)table(draw)[2])(draw);
 typedef u32 (WIN *Count)(void);Count count=(Count)GetProcAddress(module,"NativeDrawSurfaces");if(count())ExitProcess(154);
 count=(Count)GetProcAddress(module,"NativeDrawObjects");if(count())ExitProcess(155);
 u32 policy[6]={10,WIDTH*HEIGHT,512,0,4,4*510*180},n=0;HANDLE out=CreateFileA("surface-policy.bin",0x40000000,0,0,2,0x80,0);
 if(out==(HANDLE)-1 || !WriteFile(out,policy,sizeof(policy),&n,0) || n!=sizeof(policy))ExitProcess(156);CloseHandle(out);
}
void start(void){
 char value[2];u32 native=GetEnvironmentVariableA("MNM_FACTORY_NATIVE",value,2)!=0;
 void* module=LoadLibraryA(native?"MnmDraw.dll":"ddraw.dll");if(!module)ExitProcess(120);
 Create create=(Create)GetProcAddress(module,native?"NativeDrawCreate":"DirectDrawCreate");if(!create)ExitProcess(121);
 for(u32 i=0;i<64;++i){u32* r=results[i];u32 newer=i&1;void *draw=0,*fourth=0,*a=0,*b=0;u32 d[31];
  r[0]=(u32)create(0,&draw,0);if(r[0] || !draw)ExitProcess(125);
  r[1]=(u32)((Query)table(draw)[0])(draw,dd4,&fourth);if(r[1] || !fourth)ExitProcess(126);
  void* device=newer?fourth:draw;
  r[2]=(u32)((i32 (WIN *)(void*,void*,u32))table(device)[20])(device,GetDesktopWindow(),8);
  describe(d,newer);r[3]=(u32)((i32 (WIN *)(void*,const u32*,void**,void*))table(device)[6])(device,d,&a,0);
  describe(d,newer);r[4]=(u32)((i32 (WIN *)(void*,const u32*,void**,void*))table(device)[6])(device,d,&b,0);
  if(r[2] || r[3] || r[4] || !a || !b)ExitProcess(127);
  describe(d,newer);r[5]=(u32)((i32 (WIN *)(void*,u32*))table(a)[22])(a,d);
  r[6]=d[2];r[7]=d[3];r[8]=d[4];r[9]=d[26];
  pixels(a,newer,0,i*127+31,1);pixels(b,newer,r+32,0xffff,1);
  i32 source[4]={1,1,10,7},target[4]={2,2,11,8};u32 key[2]={0,0};
  r[10]=(u32)((i32 (WIN *)(void*,u32,const u32*))table(a)[29])(a,8,key);
  r[11]=(u32)((Blt)table(b)[5])(b,target,a,source,0x1008000,0);
  pixels(b,newer,r+32+WIDTH*HEIGHT,0,0);
  source[0]=0;source[1]=0;source[2]=6;source[3]=4;
  r[12]=(u32)((Fast)table(b)[7])(b,1,0,a,source,0x10);
  pixels(b,newer,r+32+2*WIDTH*HEIGHT,0,0);
  target[0]=2;target[1]=1;target[2]=8;target[3]=5;
  r[13]=(u32)((Blt)table(b)[5])(b,target,b,source,0x1000000,0);
  pixels(b,newer,r+32+3*WIDTH*HEIGHT,0,0);
  u32 fx[25]={100};fx[20]=0x7e0;target[0]=0;target[1]=6;target[2]=WIDTH;target[3]=HEIGHT;
  r[14]=(u32)((Blt)table(b)[5])(b,target,0,0,0x1000400,fx);
  pixels(b,newer,r+32+4*WIDTH*HEIGHT,0,0);
  key[0]=key[1]=0xabab;r[15]=(u32)((i32 (WIN *)(void*,u32,u32*))table(a)[16])(a,8,key);r[16]=key[0];r[17]=key[1];
  void *alias=0,*id1=0,*id2=0;
  r[18]=(u32)((Query)table(a)[0])(a,surface2,&alias);if(r[18] || !alias)ExitProcess(128);
  r[19]=(u32)((Query)table(a)[0])(a,unknown,&id1);r[20]=(u32)((Query)table(alias)[0])(alias,unknown,&id2);r[21]=id1==id2;
  r[22]=((Ref)table(a)[1])(a);r[23]=((Ref)table(id1)[2])(id1);r[24]=((Ref)table(id2)[2])(id2);
  r[25]=((Ref)table(alias)[2])(alias);r[26]=((Ref)table(a)[2])(a);r[27]=((Ref)table(a)[2])(a);r[28]=((Ref)table(b)[2])(b);
  ((Ref)table(fourth)[2])(fourth);((Ref)table(draw)[2])(draw);
  if(native){typedef u32 (WIN *Count)(void);Count count=(Count)GetProcAddress(module,"NativeDrawSurfaces");if(!count)ExitProcess(129);r[29]=count();
   count=(Count)GetProcAddress(module,"NativeDrawObjects");r[30]=count();}
 }
 if(native){policies(create,module);if(GetModuleHandleA("ddraw.dll") || LoadLibraryA("ddraw.dll"))ExitProcess(130);}
 HANDLE out=CreateFileA("surface-results.bin",0x40000000,0,0,2,0x80,0);u32 n=0;
 if(out==(HANDLE)-1 || !WriteFile(out,results,sizeof(results),&n,0) || n!=sizeof(results))ExitProcess(131);
 CloseHandle(out);ExitProcess(0);
}
