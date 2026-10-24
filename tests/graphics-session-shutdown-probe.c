/* Complete unchanged original graphics shutdown with real COM providers.
 * Extra caller credits are retired explicitly, never by a process-exit sweep. */
#include "graphics-lifetime-common.h"
#include "original-pe32-image.h"
static u32 wrappers[3][16];
static u32 results[192][24+3*117];
static const u8 draw2id[16]={0xe0,0xf3,0xa6,0xb3,0x43,0x2b,0xcf,0x11,0xa2,0xde,0,0xaa,0,0xb9,0x33,0x56};
static const u8 draw4id[16]={0x9a,0x50,0x59,0x9c,0xbd,0x39,0xd1,0x11,0x8c,0x4a,0,0xc0,0x4f,0xd9,0x30,0xc5};
void start(void){
 original_map();u8 shutdown[154],retire[216];original_copy(shutdown,(void*)0x58acf0,sizeof(shutdown));original_copy(retire,(void*)0x58b3e0,sizeof(retire));
 u32 native;void* module=gl_module(&native);GlCreate create=(GlCreate)GetProcAddress(module,"DirectDrawCreate");gl_require(create!=0,173);u32 id=0;
 for(u32 count=0;count<4;++count)for(u32 page=0;page<2;++page)for(u32 full=0;full<2;++full)
  for(u32 clip=0;clip<2;++clip)for(u32 cause=0;cause<2;++cause)for(u32 held=0;held<3;++held){
  u32* r=results[id];r[0]=id++;r[1]=count;r[2]=page;r[3]=full;r[4]=clip;r[5]=cause;r[6]=held;
  void* draw=0;gl_require(!create(0,&draw,0)&&draw,174);gl_require(!((i32(WIN*)(void*,void*,u32))gl_vt(draw)[20])(draw,0,8),175);
  void* second=gl_query(draw,draw2id);void* fourth=gl_query(draw,draw4id);void* clipper=0;
  if(clip)gl_require(!((i32(WIN*)(void*,u32,void**,void*))gl_vt(draw)[4])(draw,0,&clipper,0)&&clipper,184);
  *(void**)0x6f98c4=draw;*(void**)0x6f98c8=second;*(void**)0x6f98cc=fourth;*(void**)0x6f98d0=clipper;
  *(u32*)0x6f68d8=full;*(u32*)0x6e1f1c=page;*(u32*)0x6f8154=0;*(u32*)0x6f98c0=0;
  for(u32 i=0;i<128;++i)*(u32*)(0x6f6950+i*4)=0;
  gl_zero(wrappers,sizeof(wrappers));void* retained[3]={0};
  if(held)((GlRef)gl_vt(draw)[1])(draw); /* Supervisor device credit. */
  for(u32 i=0;i<count;++i){
   u32 d[31];gl_desc(d,0);typedef void(__attribute__((thiscall))*Wrap)(void*,u32*,u32,u32);
   ((Wrap)0x58ad90)(wrappers[i],d,0,0);
   gl_require(wrappers[i][1]&&wrappers[i][2],185);
   gl_pixels((void*)wrappers[i][2],1,0,0x1200+i*0x100,1);
   if(held)retained[i]=gl_query((void*)wrappers[i][1],gl_iids[held+1]);
  }
  if(count)*(u32*)0x6f98c0=(u32)wrappers[0];
  if(cause){u32 invalid[31];void* denied=0;gl_desc(invalid,0);invalid[3]=0;
   r[7]=((i32(WIN*)(void*,u32*,void**,void*))gl_vt(draw)[6])(draw,invalid,&denied,0);
   gl_require(r[7]&&!denied,186);r[23]=r[7];r[7]=1; /* Retain exact HRESULT separately from the common failed-create branch. */
  }
  ((void(*)(void))0x58acf0)();
  r[8]=*(u32*)0x6f8154;r[9]=*(u32*)0x6f98c0;r[10]=*(u32*)0x6f98c4|*(u32*)0x6f98c8|*(u32*)0x6f98cc;r[11]=*(u32*)0x6f68d8;r[12]=*(u32*)0x6f98d0!=0;
  gl_require(!r[8]&&!r[9]&&!r[10]&&!r[11]&&r[12]==(full&&clip),187);
  r[13]=held!=0;r[14]=held?count:0;r[15]=full&&clip;
  if(native)gl_counts(module,r[13],r[14],0,r[15],0);
  for(u32 i=0;i<count;++i){u32 dirty=0;for(u32 j=0;j<16;++j)if(j==1||j==2||j==4||j==5||j==6||j==10)dirty|=wrappers[i][j];gl_require(!dirty,188);
   if(held){gl_pixels(retained[i],held+1,r+24+i*117,0,0);gl_drop(retained[i]);}}
  if(held)gl_drop(draw);
  if(full&&clip){gl_drop(clipper);*(u32*)0x6f98d0=0;}
  if(native)gl_counts(module,0,0,0,0,0);
  ((void(*)(void))0x58acf0)(); /* Idempotent draw2-null exit. */
 }
 gl_require(id==192,189);
 for(u32 i=0;i<sizeof(shutdown);++i)gl_require(shutdown[i]==((u8*)0x58acf0)[i],190);
 for(u32 i=0;i<sizeof(retire);++i)gl_require(retire[i]==((u8*)0x58b3e0)[i],191);
 gl_save("graphics-shutdown.bin",results,sizeof(results));ExitProcess(0);
}
