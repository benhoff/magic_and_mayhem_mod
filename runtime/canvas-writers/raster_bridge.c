#include "../shadow/win32_min.h"
#include "../../reconstruction/rendering/raster_pixels.h"
#include "raster_workspace.h"
API u32 WIN GetCurrentThreadId(void);
API void* WIN GetProcAddress(void*,const char*);
typedef i32 (WIN *Plane)(const void*,u32*,u32);
#include "raster_sites.h"
uintptr_t mnm_raster_original[MNM_RASTER_SITES],mnm_raster_resume[MNM_RASTER_SITES];
u32 mnm_raster_native[MNM_RASTER_SITES],mnm_raster_forwarded[MNM_RASTER_SITES];
static Plane owned_plane;
static volatile u32 owner;
static u32 word(u32 at){const u8 *p=(const u8*)at;return p[0]|(u32)p[1]<<8|(u32)p[2]<<16|(u32)p[3]<<24;}
static int span(u32 at,u32 n,int write){
 if(!at||!n||at>0xffffffffu-n)return 0;const u32 end=at+n;
 while(at<end){u32 m[7];if(VirtualQuery((void*)at,m,28)!=28||m[4]!=0x1000||(m[5]&0x101)||!(m[5]&(write?0xcc:0xee))||m[0]>at||!m[3]||m[0]>0xffffffffu-m[3]||m[0]+m[3]<=at)return 0;at=m[0]+m[3];}return 1;
}
int mnm_raster_route(u32 *r,unsigned site){
 const u32 error=GetLastError();int ok=0;if(site>=MNM_RASTER_SITES)goto done;
 const struct RasterSite *s=sites+site;const u32 base=word(0x658174),format=word(0x6e1f88);u32 d[6],frame=0,*stack=r+9;
 struct MnmRasterPixels q={0};q.period=16;q.palette_step=2;
 if(!owned_plane||format>1||!span((u32)stack,0x44,0)||owned_plane((void*)base,d,24)||d[0]!=1||d[3]!=word(0x6a2dc8)||d[5]!=format)goto done;
 struct MnmPixelPlane p={(u16*)base,d[1],d[2],d[3],format};
 if(!mnm_pixel_plane(&p)||!span(base,p.stride*p.height*2,1))goto done;
 q.clip_left=(i32)word(0x6e0008);q.clip_top=(i32)word(0x6cbb6c);q.clip_right=(i32)word(0x6a49b8);q.clip_bottom=(i32)word(0x656618);
 if(s->family<=6){
  const u32 bp=r[2];if(!span(bp,28,0))goto done;frame=word(bp+8);q.left=(i32)word(bp+12);q.top=(i32)word(bp+16);
  q.palette=(const u8*)r[7];q.palette_step=s->family==2||s->family==3?4:2;
  q.paired_palette=s->family==2?1:s->family==3?2:0;
  q.mode=s->family==4?7:s->family==6?3:0;q.strict_horizontal=s->family==4||s->family==6;q.contiguous=s->family!=4;
 }else if(s->family==7){frame=r[7];q.palette=(const u8*)r[6];q.left=(i32)r[5];q.top=(i32)stack[0x28/4];}
 else if(s->family==8){frame=r[7];q.palette=(const u8*)r[6];q.left=(i32)r[5];q.top=(i32)stack[0x30/4];q.mode=1;}
 else if(s->family==9){frame=r[0];q.palette=(const u8*)r[7];q.left=(i32)r[5];q.top=(i32)stack[0x38/4];q.mode=4;}
 else if(s->family==10){frame=r[4];q.palette=(const u8*)r[7];q.left=(i32)r[5];q.top=(i32)stack[0x38/4];q.mode=2;}
 else if(s->family==11){frame=r[6];q.left=(i32)r[4];q.top=(i32)stack[0x34/4];q.mode=3;}
 else if(s->family==12){frame=r[6];q.left=(i32)r[5];q.top=(i32)stack[0x2c/4];q.mode=5;}
 else if(s->family==13){frame=stack[0x30/4];q.left=(i32)r[5];q.top=(i32)r[7];q.mode=6;}
 else {const u32 bp=r[2];if(!span(bp-4,32,0))goto done;frame=r[5];q.left=(i32)r[0];q.top=(i32)word(bp+16);q.palette=(const u8*)word(bp-4);}
 if(!span(frame,40,0))goto done;
 q.frame=(const u8*)frame;q.bytes=word(frame);q.indexed=word(frame+28)!=0xffffffffu;
 if(q.bytes<48||q.bytes>4*1024*1024||!span(frame,q.bytes,0))goto done;
 if(s->family<=4&&word(frame+4)>255)goto done;
 const i32 ox=(i32)word(frame+12),oy=(i32)word(frame+16);if(ox<-4096||ox>4096||oy<-4096||oy>4096||q.left<-4096||q.left>4096||q.top<-4096||q.top>4096)goto done;
 if(s->family==7||s->family==9||s->family==11||s->family==12)q.left-=ox;
 if(s->family>=7&&s->family!=13)q.top-=oy;
 if(s->family==5||s->family==1||s->family==0||s->family==2||s->family==3){
  if((s->family<=3&&!q.indexed)||(s->family==5&&q.indexed))goto done;
 }
 if(q.mode==6&&(q.indexed||*(const u16*)0x6e1f7c!=(format?0x7c00:0xf800)||*(const u16*)0x6e1f7e!=(format?0x3e0:0x7e0)||*(const u16*)0x6e1f80!=31))goto done;
 if(s->family>=7&&s->family!=13&&word(0x6e1f68))goto done;
 if(s->family==13&&(q.left<q.clip_left||(long long)q.left+word(frame+4)>q.clip_right))q.mode=0;
 if(q.indexed&&q.mode!=3&&q.mode!=5&&q.mode!=7&&!span((u32)q.palette,256*q.palette_step,0))goto done;
 if(q.mode==3||q.mode==5||q.mode==7)q.palette=0;
 if(s->family==5&&(r[8]&0x400))goto done;
 if(s->family==6){
  const u32 phase=word(0x6c4830)+1,skip=word(0x5f1e50);
  for(u32 y=0;y<16;++y){const u32 j=(phase+y-skip)&15;q.offsets[y]=(i32)word(0x656640+j*4);}
 }else if(s->family==11){
  u32 amplitude=stack[0x38/4];if(!amplitude)amplitude=word(0x5e15f8);if(!amplitude||amplitude>8)goto done;
  const i32 left=q.left,width=(i32)word(frame+4);const u32 phase=word(0x6c4830);
  if(left<q.clip_left||(long long)left+width>q.clip_right){q.legacy_wave_clip=1;for(u32 y=0;y<16;++y){const i32 n=8-(i32)((y+phase)&15);q.offsets[y]=n<0?-n:n;}}
  else {q.period=amplitude*2;for(u32 y=0;y<q.period;++y)q.offsets[y]=(i32)word(0x5f14d0+(amplitude*16+(y+phase)%q.period)*4);}
 }
 const u32 thread=GetCurrentThreadId();if(!owner)__sync_bool_compare_and_swap(&owner,0,thread);if(thread!=owner)goto done;
 if(!mnm_raster_pixels(&p,&q))goto done;
 if(s->family<=6)mnm_raster_workspace((u32*)0x5f1e50,&q,&p,frame,s->family==5&&site<11?1:s->family,s->clipped);
 /* Volatile ECX/EDX and upper AX bits of the fast/slow private bodies are not
  * C ABI results. Keep each original footer, nonvolatile state and WORD result. */
 ++mnm_raster_native[site];ok=1;
done:if(site<MNM_RASTER_SITES&&!ok)++mnm_raster_forwarded[site];SetLastError(error);return ok;
}
static int same(const u8 *a,const u8 *b,u32 n){for(u32 i=0;i<n;++i)if(a[i]!=b[i])return 0;return 1;}
static void jump(u8 *at,void *target,u32 n){at[0]=0xe9;*(u32*)(at+1)=(u32)target-(u32)at-5;for(u32 i=5;i<n;++i)at[i]=0x90;}
static int install(void){
 char flag[4];const u32 n=GetEnvironmentVariableA("MNM_RASTER_TAKEOVER",flag,4);if(!n)return 1;
 if(n!=1||flag[0]!='1'||(u32)GetModuleHandleA(0)!=0x400000)return 0;
 for(u32 i=0;i<MNM_RASTER_SITES;++i)if(!span(sites[i].at,sites[i].length,0)||!same((u8*)sites[i].at,sites[i].bytes,sites[i].length)||!span(sites[i].tail,8,0)||!same((u8*)sites[i].tail,sites[i].tail_bytes,8))return 0;
 owned_plane=(Plane)GetProcAddress(GetModuleHandleA("MnmDraw.dll"),"NativeDrawPlaneV1");if(!owned_plane)return 0;
 u8 *tramp=VirtualAlloc(0,4096,0x3000,0x40);if(!tramp)return 0;
 for(u32 i=0;i<MNM_RASTER_SITES;++i){for(u32 j=0;j<sites[i].length;++j)tramp[i*32+j]=sites[i].bytes[j];jump(tramp+i*32+sites[i].length,(void*)(sites[i].at+sites[i].length),5);mnm_raster_original[i]=(uintptr_t)(tramp+i*32);mnm_raster_resume[i]=sites[i].tail;}
 u32 old;if(!VirtualProtect(tramp,4096,0x20,&old)||!FlushInstructionCache(GetCurrentProcess(),tramp,4096))return 0;
 u32 pages[MNM_RASTER_SITES],protection[MNM_RASTER_SITES],count=0;
 for(u32 i=0;i<MNM_RASTER_SITES;++i){const u32 first=sites[i].at&~4095u,last=(sites[i].at+sites[i].length-1)&~4095u;
  for(u32 page=first;;page+=4096){u32 j=0;while(j<count&&pages[j]!=page)++j;if(j==count){
   if(!VirtualProtect((void*)page,4096,0x40,protection+count)){while(count){--count;u32 ignored;if(!VirtualProtect((void*)pages[count],4096,protection[count],&ignored))ExitProcess(123);}return 0;}pages[count++]=page;}
   if(page==last)break;}}
 for(u32 i=0;i<MNM_RASTER_SITES;++i)jump((u8*)sites[i].at,(void*)sites[i].entry,sites[i].length);
 while(count){--count;u32 ignored;if(!VirtualProtect((void*)pages[count],4096,protection[count],&ignored)||!FlushInstructionCache(GetCurrentProcess(),(void*)pages[count],4096))ExitProcess(123);}
 return 1;
}
__declspec(dllexport) void RasterWritersAnchor(void){}
#ifdef MNM_RASTER_SELFTEST
__declspec(dllexport) int WIN RasterWritersInstallForTest(void){return install();}
#endif
int WIN DllMain(HANDLE module,u32 reason,void *reserved){(void)module;(void)reserved;if(reason!=1)return 1;
#ifdef MNM_RASTER_SELFTEST
 return 1;
#else
 const u32 error=GetLastError();const int ok=install();SetLastError(error);return ok;
#endif
}
