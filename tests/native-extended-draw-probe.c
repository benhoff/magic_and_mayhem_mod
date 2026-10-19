/* Actual PE32 extended drawing and whole image-copy replacement controls. */
#define MNM_TRANSFORM_HELPERS_ONLY
#include "native-copy-transform-probe.c"
#include "../runtime/native-draw/image_copy_original.h"
__declspec(allocate(".own")) static u8 image_address_space[0x200000] __attribute__((used));
typedef i32(WIN* Blt)(void*,const i32*,void*,const i32*,u32,const u32*);
typedef i32(__attribute__((thiscall))* ImageCopy)(const u32*,u8*,i32,u32,i32,i32,const i32*);
static i32 WIN mode_callback(u32* description,void* context){
 u32* calls=context;require(description[0]==108 && (description[21]==8 || description[21]==16 || description[21]==24 || description[21]==32),98);++*calls;return 1;
}
static u32 pixel(const u32* d,u32 x,u32 y){
 const u8* p=(const u8*)d[9]+y*d[4]+x*(d[21]/8);u32 v=0;
 for(u32 i=0;i<d[21]/8;++i)v|=(u32)p[i]<<(i*8);return v;
}
static void effect_cases(void* draw){
 void* s=make_surface(draw,2);void* d=make_surface(draw,2);void* p=make_surface(draw,2);
 paint(s,2,1);paint(p,2,1);i32 rect[4]={0,0,13,9};
 u32 patternLock[27]={108};require(!((Lock)table(p)[25])(p,0,patternLock,1,0),79);
 for(u32 y=0;y<9;++y)for(u32 x=0;x<13;++x)*(u16*)((u8*)patternLock[9]+y*patternLock[4]+x*2)=(u16)colours[2][(x*3+y*7+1)%5];
 require(!((i32(WIN*)(void*,void*))table(p)[32])(p,(void*)patternLock[9]),79);
 for(u32 rop=0;rop<256;++rop){
  paint(d,2,0);u32 fx[25]={100};fx[2]=rop<<16;fx[20]=(u32)p;
  require(!((Blt)table(d)[5])(d,rect,s,rect,0x20000,fx),80);
  u32 lock[27]={108};require(!((Lock)table(d)[25])(d,0,lock,0x10,0),81);
  for(u32 y=0;y<9;++y)for(u32 x=0;x<13;++x){u32 v=colours[2][(x+y*13)%5],pattern=colours[2][(x*3+y*7+1)%5],expected=0;
   /* Independent per-bit truth-table oracle, unlike native word minterms. */
   for(u32 bit=0;bit<16;++bit){u32 input=(((pattern>>bit)&1)<<2)|(((v>>bit)&1)<<1)|((0x1fu>>bit)&1);
    expected|=((rop>>input)&1)<<bit;}
   require(pixel(lock,x,y)==expected,82);}
  require(!((i32(WIN*)(void*,void*))table(d)[32])(d,(void*)lock[9]),83);
 }
 for(u32 rotate=0;rotate<4;++rotate)for(u32 mirror=0;mirror<4;++mirror){
  u32 fx[25]={100};fx[1]=(mirror<<1)|(rotate==1?0x40:rotate==2?0x10:rotate==3?0x20:0);
  require(!((Blt)table(d)[5])(d,rect,s,rect,0x800,fx),84);
  u32 lock[27]={108};require(!((Lock)table(d)[25])(d,0,lock,0x10,0),85);
  for(u32 y=0;y<9;++y)for(u32 x=0;x<13;++x){u32 sx=x,sy=y;
   if(rotate==1){sx=y*13/9;sy=8-x*9/13;}else if(rotate==2){sx=12-x;sy=8-y;}
   else if(rotate==3){sx=12-y*13/9;sy=x*9/13;}
   if(mirror&1)sx=12-sx;if(mirror&2)sy=8-sy;
   require(pixel(lock,x,y)==colours[2][(sx+sy*13)%5],86);}
  require(!((i32(WIN*)(void*,void*))table(d)[32])(d,(void*)lock[9]),87);
 }
 paint(d,2,1);u32 key[2]={0x7e0,0x7e0};
 require(!((i32(WIN*)(void*,u32,const u32*))table(d)[29])(d,2,key),88);
 require(!((Blt)table(d)[5])(d,rect,s,rect,0x2000,0),89);
 require(!((i32(WIN*)(void*,u32,u32,void*,const i32*,u32))table(d)[7])(d,0,0,s,rect,0x12),89);
 /* Override source key + destination key is tested with deliberately different pixels. */
 paint(s,2,0);u32 fx[25]={100};fx[21]=fx[22]=0x7e0;fx[23]=fx[24]=0xffff;
 require(!((Blt)table(d)[5])(d,rect,s,rect,0x14000,fx),90);
 u32 lock[27]={108};require(!((Lock)table(d)[25])(d,0,lock,0x10,0),91);
 for(u32 y=0;y<9;++y)for(u32 x=0;x<13;++x){u32 value=colours[2][(x+y*13)%5];require(pixel(lock,x,y)==(value==0x7e0?0x1f:value),92);}
 require(!((i32(WIN*)(void*,void*))table(d)[32])(d,(void*)lock[9]),93);
 require(!((i32(WIN*)(void*,u32,const u32*))table(d)[29])(d,2,0),94);
 require(!release(s)&&!release(d)&&!release(p),95);
}
static void palette_and_modes(void* draw){
 u32 calls=0;require(!((i32(WIN*)(void*,u32,const u32*,void*,void*))table(draw)[8])(draw,0,0,&calls,(void*)mode_callback) && calls==8,99);
 u32 filter[27]={108,0x1000};filter[18]=32;filter[21]=8;calls=0;
 require(!((i32(WIN*)(void*,u32,const u32*,void*,void*))table(draw)[8])(draw,0,filter,&calls,(void*)mode_callback) && calls==2,99);
 u32 entries[256];for(u32 i=0;i<256;++i)entries[i]=i*0x010101;
 const u32 flags[]={4,0x14,0x44,1,0x11,0x100,0x200};
 for(u32 i=0;i<sizeof(flags)/sizeof(flags[0]);++i){void* p=0;u32 count=(flags[i]&4)?256:(flags[i]&1)?16:(flags[i]&0x100)?2:4;
  require(!((i32(WIN*)(void*,u32,const u32*,void**,void*))table(draw)[5])(draw,flags[i],entries,&p,0),100);
  u32 got[256];require(!((i32(WIN*)(void*,u32,u32,u32,u32*))table(p)[4])(p,0,0,count,got),101);
  for(u32 n=0;n<count;++n)require(got[n]==entries[n],102);
  u32 newColor=0x123456;require(!((i32(WIN*)(void*,u32,u32,u32,const u32*))table(p)[6])(p,0,0,1,&newColor),103);
  require(!((i32(WIN*)(void*,u32,u32,u32,u32*))table(p)[4])(p,0,0,1,got),104);
  require(got[0]==(count==256 && !(flags[i]&0x40)?0:newColor),105);require(!release(p),106);
 }
 require(!((i32(WIN*)(void*,void*,u32))table(draw)[20])(draw,0,8),107);
 /* Use a real window for exclusive-mode admission. */
 API void* WIN GetDesktopWindow(void);
 require(!((i32(WIN*)(void*,void*,u32))table(draw)[20])(draw,GetDesktopWindow(),0x11),108);
 const u32 depths[4]={8,16,24,32};
 for(u32 i=0;i<4;++i){require(!((i32(WIN*)(void*,u32,u32,u32))table(draw)[21])(draw,64,48,depths[i]),109);
  u32 desc[27]={108,1};desc[26]=0x200;void* primary=0;
  require(!((Surface)table(draw)[6])(draw,desc,&primary,0),110);
  u32 lock[27]={108};require(!((Lock)table(primary)[25])(primary,0,lock,0,0),111);
  require(lock[21]==depths[i] && lock[3]==64 && lock[2]==48 && lock[4]>=64*(depths[i]/8),112);
  require(!((i32(WIN*)(void*,void*))table(primary)[32])(primary,(void*)lock[9]),113);require(!release(primary),114);
 }
}
static void image_cases(void){
 ImageCopy original=(ImageCopy)0x533d00,native=(ImageCopy)0x533a00;
 static u8 source[4096],expected[4096],actual[4096];for(u32 i=0;i<4096;++i)source[i]=(u8)(i*17+3);
 for(u32 format=0;format<2;++format)for(i32 x=-4;x<7;++x)for(i32 y=-3;y<5;++y)for(u32 clipMode=0;clipMode<3;++clipMode){
  for(u32 i=0;i<4096;++i)expected[i]=actual[i]=0xa5;
  u32 image[5]={(u32)(source+1024),0,13,9,format};
  i32 clip[4]={clipMode==1?3:0,clipMode==2?2:0,clipMode==1?11:26,clipMode==2?7:9};
  original(image,expected+512,64,0,x,y,clip);SetLastError(0x1234);
  native(image,actual+512,64,0,x,y,clip);require(GetLastError()==0x1234,120);
  for(u32 i=0;i<4096;++i)require(actual[i]==expected[i],121);
 }
}
void start(void){
 char path[1024];require(GetEnvironmentVariableA("MNM_TEST_LIBRARY",path,sizeof(path))>0,130);
 void* module=LoadLibraryA(path);require(module!=0,131);
 u8* entry=(u8*)0x533a00;u32 old;require(VirtualProtect(entry,4096,0x40,&old),132);
 for(u32 i=0;i<sizeof(ns_image_copy_original);++i){entry[i]=ns_image_copy_original[i];((u8*)0x533d00)[i]=ns_image_copy_original[i];}
 require(VirtualProtect((void*)0x5c5000,4096,4,&old),133);
 void* user=LoadLibraryA("user32.dll");require(user!=0,134);*(void**)0x5c5294=GetProcAddress(user,"IntersectRect");require(*(void**)0x5c5294!=0,135);
 Create create=GetProcAddress(module,"NativeDrawCreate");void* draw=0;require(create&&!create(0,&draw,0),136);
 for(u32 i=8;i<sizeof(ns_image_copy_original);++i)entry[i]=0xcc;
 image_cases();effect_cases(draw);palette_and_modes(draw);
 require(!release(draw),137);ExitProcess(0);
}
