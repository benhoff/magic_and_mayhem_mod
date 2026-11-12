/* Supplied offscreen requests compare actual Wine DirectDraw and native facade.
 * No original cursor caller execution or historical coordinate origin claim. */
#include "graphics-lifetime-common.h"
static u32 provider_pixels(void* surface,u32 width,u32 height,u32 seed,u32 initialize){
 u32 d[31];gl_require(!gl_lock(surface,0,d),181);
 gl_require(d[2]==height&&d[3]==width&&d[4]>=width*2&&d[21]==16,182);
 u32 matches=1;
 for(u32 y=0;y<height;++y)for(u32 x=0;x<width;++x){
  u16* p=(u16*)(d[9]+y*d[4])+x;u16 expected=(u16)(seed+x*3+y*17);
  if(initialize)*p=expected;else if(*p!=expected)matches=0;
 }
 gl_require(!gl_unlock(surface,0,d[9]),183);return matches;
}
static void* provider_surface(void* draw,u32 width,u32 height){
 u32 d[31];void* result=0;gl_desc(d,0);d[2]=height;d[3]=width;
 gl_require(!((i32(WIN*)(void*,u32*,void**,void*))gl_vt(draw)[6])(draw,d,&result,0)&&result,184);return result;
}
void start(void){
 u32 native;void* module=gl_module(&native);
 GlCreate create=(GlCreate)GetProcAddress(module,native?"NativeDrawCreate":"DirectDrawCreate");void* draw=0;
 gl_require(create&&!create(0,&draw,0)&&draw,185);
 gl_require(!((i32(WIN*)(void*,void*,u32))gl_vt(draw)[20])(draw,0,8),186);
 static const u32 cases[3][10]={
  {640,480,40,40,0,0,40,40,0,0},
  {640,480,40,40,640,512,680,552,0,0},
  {400,280,640,480,0,0,40,40,640,512}
 };
 u32 results[3][24];gl_zero(results,sizeof(results));
 for(u32 i=0;i<3;++i){
  const u32* c=cases[i];void* source=provider_surface(draw,c[0],c[1]);void* target=provider_surface(draw,c[2],c[3]);
  provider_pixels(source,c[0],c[1],0x1500,1);provider_pixels(target,c[2],c[3],0x5500,1);
  if(i==2){u32 key[2]={0,0};gl_require(!((i32(WIN*)(void*,u32,u32*))gl_vt(source)[29])(source,8,key),187);}
  const u32 flags=i==2?0x11:0x10;
  i32 status=((i32(WIN*)(void*,u32,u32,void*,const u32*,u32))gl_vt(target)[7])(target,c[8],c[9],source,c+4,flags);
  u32* r=results[i];r[0]=i;r[1]=(u32)status;r[2]=flags;
  for(u32 j=0;j<10;++j)r[3+j]=c[j];
  r[13]=provider_pixels(source,c[0],c[1],0x1500,0);
  r[14]=provider_pixels(target,c[2],c[3],i?0x5500:0x1500,0);
  r[15]=c[0]*c[1]+c[2]*c[3];r[16]=0x840;r[17]=16;r[18]=0xf800;r[19]=0x7e0;r[20]=31;r[21]=native;
  gl_drop(target);gl_drop(source);
 }
 gl_drop(draw);if(native)gl_counts(module,0,0,0,0,0);
 gl_save("invalid-copy-provider.bin",results,sizeof(results));ExitProcess(0);
}
