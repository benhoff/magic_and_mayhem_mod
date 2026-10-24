#include "graphics-lifetime-common.h"
static u32 results[128][16+5*117];
void start(void){
 u32 native;void* module=gl_module(&native);GlCreate create=(GlCreate)GetProcAddress(module,"DirectDrawCreate");gl_require(create!=0,173);u32 id=0;
 for(u32 a=0;a<4;++a)for(u32 b=0;b<4;++b)for(u32 dc_mode=0;dc_mode<2;++dc_mode)
  for(u32 early=0;early<2;++early)for(u32 reverse=0;reverse<2;++reverse){
  u32* r=results[id];r[0]=id++;r[1]=a;r[2]=b;r[3]=dc_mode;r[4]=early;r[5]=reverse;
  void *draw=0,*dc=0;gl_require(!create(0,&draw,0)&&draw,174);
  gl_require(!((i32(WIN*)(void*,void*,u32))gl_vt(draw)[20])(draw,0,8),175);
  void* surface=gl_surface(draw);void* left=gl_query(surface,gl_iids[a]);void* right=gl_query(left,gl_iids[b]);
  void* one=gl_query(left,gl_unknown);void* two=gl_query(right,gl_unknown);r[6]=one==two;gl_require(r[6],176);gl_drop(one);gl_drop(two);
  gl_pixels(left,a,r+16,0x1234,1);
  u32 d[31],other[31];r[7]=dc_mode?gl_dc(left,&dc):gl_lock(left,a,d);gl_require(!r[7],177);
  if(early){gl_drop(surface);surface=0;}
  r[8]=gl_lock(right,b,other);if(!r[8])gl_require(!gl_unlock(right,b,other[9]),178);
  void* another=0;r[9]=gl_dc(right,&another);
  if(!r[9])gl_require(!gl_undc(right,another),179);
  if(dc_mode){
   for(u32 y=0;y<3;++y)for(u32 x=0;x<5;++x)gl_require(SetPixel(dc,x,y,0x2468ac)!=0xffffffffu,180);
   gl_require(GdiFlush(),181);r[10]=gl_undc(right,dc);r[11]=gl_undc(left,dc);
  }else{
   for(u32 y=0;y<3;++y)for(u32 x=0;x<5;++x)*((u16*)(d[9]+y*d[4])+x)=(u16)(0xabc0+x+y*5);
   r[10]=gl_unlock(right,b,d[9]);r[11]=gl_unlock(left,a,d[9]);
  }
  gl_require(!r[10],182);gl_pixels(right,b,r+16+117,0,0);gl_pixels(left,a,r+16+234,0,0);
  void* fresh=gl_surface(draw);gl_pixels(fresh,0,r+16+351,0x4567,1);gl_pixels(right,b,r+16+468,0,0);gl_drop(fresh);
  if(surface)gl_drop(surface);if(reverse){gl_drop(right);gl_drop(left);}else{gl_drop(left);gl_drop(right);}
  gl_drop(draw);if(native)gl_counts(module,0,0,0,0,0);
 }
 gl_require(id==128,183);gl_save("alias-leases.bin",results,sizeof(results));ExitProcess(0);
}
