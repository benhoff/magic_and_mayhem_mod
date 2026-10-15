/* Native minimap pixel passes; original clipping/projection/order/lifecycle. */
#include "../shadow/win32_min.h"
#include "producer_status.h"
#include "minimap_route.h"
API u32 WIN GetCurrentThreadId(void);
API void* WIN GetProcAddress(void*,const char*);
#include "history.h"
extern void mnm_map_terrain_enter(void),mnm_map_cell_enter(void),mnm_map_creature_enter(void),
 mnm_map_camera0_enter(void),mnm_map_camera1_enter(void),mnm_map_camera2_enter(void),mnm_map_camera3_enter(void);
typedef i32 (WIN *Plane)(const void*,u32*,u32);
static Plane owned_plane;
static volatile u32 owner;
static int installed;
static u32 word(u32 at){const u8* b=(const u8*)at;return b[0]|(u32)b[1]<<8|(u32)b[2]<<16|(u32)b[3]<<24;}
static int span(u32 at,u32 bytes,int write){
 if(!at || !bytes || at>0xffffffffu-bytes)return 0;const u32 end=at+bytes;
 while(at<end){u32 m[7];if(VirtualQuery((void*)at,m,28)!=28 || m[4]!=0x1000 || (m[5]&0x101) || !(m[5]&0xee) ||
  (write && !(m[5]&0xcc)) || m[0]>at || !m[3] || m[0]>0xffffffffu-m[3] || m[0]+m[3]<=at){return 0;}at=m[0]+m[3];}return 1;
}
static int plane(u32 pixels,struct MnmPixelPlane* p,int write){
 u32 d[6];if(!owned_plane || owned_plane((void*)pixels,d,sizeof(d)) || d[0]!=1 || d[5]!=word(0x6e1f88) || d[3]>2048)return 0;
 /* Original minimap writes address physical row storage, including padding. */
 *p=(struct MnmPixelPlane){(u16*)pixels,d[3],d[2],d[3],word(0x6e1f88)};
 return mnm_pixel_plane(p) && span(pixels,p->stride*p->height*2,write);
}
static void release(const struct MnmMinimapTerrain* t){
 const u32 error=GetLastError();if(t->cells)HeapFree(GetProcessHeap(),0,(void*)t->cells);SetLastError(error);
}
static int resolve(u32* r,unsigned kind,struct MnmPixelPlane* p,struct MnmPixelPlane* history,struct MnmMinimapTerrain* t,
 struct MnmMinimapView* v,struct MnmMinimapCamera* c,u32* a){
 const u32 error=GetLastError();int ok=0;u32* stack=r+9;
 if(kind==MNM_MAP_TERRAIN && span((u32)stack,0x20,0))cpu_history_require((void*)stack[0x1c/4]);
 if(kind>=MNM_MAP_SITES || !span((u32)stack,0x40,0) || word(0x6e1f88)>1)goto done;
 const u32 object=kind==MNM_MAP_TERRAIN?r[1]:kind==MNM_MAP_CELL || kind==MNM_MAP_CREATURE?r[6]:r[1];
 if(!span(object,0x6210,0) || !plane(word(object+0xdf),p,1) || word(object+0xeb)!=p->stride)goto done;
 if(object<(u32)p->pixels+p->stride*p->height*2&&(u32)p->pixels<object+0x6210)goto done;
 const u32 thread=GetCurrentThreadId();if(!owner)__sync_bool_compare_and_swap(&owner,0,thread);if(owner!=thread)goto done;
 if(kind==MNM_MAP_TERRAIN){
  const u32 w=word(object+0x6199),h=word(object+0x619d),cells=word(object+0x29),first=word(object+0xe7);
  if(!w || w>256 || (w&1) || !h || h>256 || !span(cells,w*h*16,0) || first<(u32)p->pixels || ((first-(u32)p->pixels)&1) ||
   !plane(stack[0x1c/4],history,0) || history->stride!=stack[0x18/4])goto done;
  const u32 offset=(first-(u32)p->pixels)/2;if(offset>=p->stride*p->height)goto done;
  if(cells<(u32)p->pixels+p->stride*p->height*2&&(u32)p->pixels<cells+w*h*16)goto done;
  *t=(struct MnmMinimapTerrain){w,h,word(object+0x61a1),word(object+0x61a5),word(object+0xc5),(i32)(offset%p->stride),(i32)(offset/p->stride),0};
  u8* copy=HeapAlloc(GetProcessHeap(),0,w*h*3);if(!copy)goto done;t->cells=copy;
  const int fog=word(0x5e1404)!=0;const i32 hidden=(i32)word(0x5e18f0);
  for(u32 i=0;i<w*h;++i){copy[i*3]=*(const u8*)(cells+i*16+12);copy[i*3+1]=*(const u8*)(cells+i*16+13);copy[i*3+2]=0;
   if(fog){const u32 state=word(cells+i*16+8);if(!span(state,1,0)||(state>=(u32)p->pixels&&state<(u32)p->pixels+p->stride*p->height*2))goto done;copy[i*3+2]=(i32)*(const signed char*)state==hidden;}}
 }else if(kind==MNM_MAP_CELL){
  const u32 index=r[5];if(index>=9 || (i32)r[7]<-512 || (i32)r[7]>1024 || (i32)r[0]<-2048 || (i32)r[0]>2048)goto done;
  a[0]=r[4];a[1]=(u32)(((i32)r[7]>>1)+(i32)r[0]);a[2]=*(const u16*)(object+0xcb+index*2);a[3]=r[1] && word(object+0x6141);
 }else if(kind==MNM_MAP_CREATURE){
  const i32 u=(i32)r[7],z=(i32)r[5],ox=(i32)word(object+0xf7),oy=(i32)word(object+0xfb);
  if(u<-256 || u>256 || z<-256 || z>256 || ox<-2048 || ox>2048 || oy<-2048 || oy>2048)goto done;
  a[0]=(u32)(ox+u-z);a[1]=(u32)(oy+(u+z)/2);
  const i32 row=(u+z)/2;const u32 rows=word(object+0xf3);const long long address=(long long)rows+(long long)row*4;
  const long long expected=(long long)(u32)p->pixels+((long long)(oy+row)*p->stride+ox)*2;
  if(address<=0 || address>0xffffffffll || expected<0 || expected>0xffffffffll || !span((u32)address,4,0) || word((u32)address)!=(u32)expected)goto done;
 }else{
  const u32 rect=word(0x6c482c);if(!span(rect,16,0))goto done;
  *v=(struct MnmMinimapView){(i32)word(0x6c5494),(i32)word(0x6c5498),(i32)word(object+0x61a1),(i32)word(object+0x61a5),
   (i32)word(object+0x103),(i32)word(object+0x107),(i32)word(object+0xf7),(i32)word(object+0xfb),kind-MNM_MAP_CAMERA0};
  const long long width=(long long)(i32)word(rect+8)-(i32)word(rect),height=(long long)(i32)word(rect+12)-(i32)word(rect+4);
  if(width<0 || width>8192 || height<0 || height>8192)goto done;
  c->viewport_width=(i32)width;c->viewport_height=(i32)height;c->border=!word(object+0x6207);c->border_colour=(u16)r[7];
  for(u32 i=0;i<8;++i)c->directions[i]=(i32)word(object+0x6121+i*4);
 }
 ok=1;
done:SetLastError(error);return ok;
}
static const struct Site{u32 at,length,tail;u8 bytes[8],tail_bytes[8];void(*entry)(void);} sites[MNM_MAP_SITES]={
 {0x553a6f,5,0x5547f3,{0x8b,0x46,0x29,0x85,0xc0},{0x8b,0x44,0x24,0x2c,0x8b,0x54,0x24,0x28},mnm_map_terrain_enter},
 {0x553799,8,0x553847,{0x66,0x8b,0x94,0x51,0xcb,0,0,0},{0xc2,0x10,0,0x90,0x90,0x90,0x90,0x90},mnm_map_cell_enter},
 {0x553986,6,0x553a13,{0x8b,0xb9,0xeb,0,0,0},{0x8b,0x54,0x24,0x10,0x8b,0x44,0x24,0x14},mnm_map_creature_enter},
 {0x55280b,6,0x552a9f,{0x39,0x8e,7,0x62,0,0},{0x8b,0x86,0x41,0x61,0,0,0x85,0xc0},mnm_map_camera0_enter},
 {0x552bdc,6,0x552e72,{0x39,0x8e,7,0x62,0,0},{0x8b,0x86,0x41,0x61,0,0,0x85,0xc0},mnm_map_camera1_enter},
 {0x552fac,6,0x553242,{0x39,0x8e,7,0x62,0,0},{0x8b,0x86,0x41,0x61,0,0,0x85,0xc0},mnm_map_camera2_enter},
 {0x55337c,6,0x553612,{0x39,0x8e,7,0x62,0,0},{0x8b,0x86,0x41,0x61,0,0,0x85,0xc0},mnm_map_camera3_enter}
};
static int same(const u8* a,const u8* b,u32 n){for(u32 i=0;i<n;++i)if(a[i]!=b[i])return 0;return 1;}
static void jump(u8* at,void* target,u32 n){at[0]=0xe9;*(u32*)(at+1)=(u32)target-(u32)at-5;for(u32 i=5;i<n;++i)at[i]=0x90;}
static int install(void){
 if(installed)return 1;char flag[4];const u32 n=GetEnvironmentVariableA("MNM_MINIMAP_TAKEOVER",flag,sizeof(flag));if(!n)return 1;
 if(n!=1 || flag[0]!='1' || (u32)GetModuleHandleA(0)!=0x400000)return 0;
 if(!cpu_history_install())return 0;
 if(!producer_start(3,7))return 0;
 for(u32 i=0;i<MNM_MAP_SITES;++i)if(!span(sites[i].at,sites[i].length,0) || !same((u8*)sites[i].at,sites[i].bytes,sites[i].length) ||
  !span(sites[i].tail,8,0) || !same((u8*)sites[i].tail,sites[i].tail_bytes,8))return 0;
 owned_plane=(Plane)GetProcAddress(GetModuleHandleA("MnmDraw.dll"),"NativeDrawPlaneV1");if(!owned_plane)return 0;
 u8* tramp=VirtualAlloc(0,4096,0x3000,0x40);if(!tramp)return 0;
 for(u32 i=0;i<MNM_MAP_SITES;++i){u8* t=tramp+i*32;for(u32 j=0;j<sites[i].length;++j)t[j]=sites[i].bytes[j];jump(t+sites[i].length,(void*)(sites[i].at+sites[i].length),5);mnm_map_original[i]=(uintptr_t)t;mnm_map_resume[i]=sites[i].tail;}
 u32 old;if(!VirtualProtect(tramp,4096,0x20,&old) || !FlushInstructionCache(GetCurrentProcess(),tramp,4096))return 0;
 u32 pages[7],protection[7],count=0;
 for(u32 i=0;i<MNM_MAP_SITES;++i){const u32 page=sites[i].at&~4095u;u32 j=0;while(j<count && pages[j]!=page)++j;if(j<count)continue;
  if(!VirtualProtect((void*)page,4096,0x40,protection+count)){while(count){--count;u32 ignored;if(!VirtualProtect((void*)pages[count],4096,protection[count],&ignored))ExitProcess(123);}return 0;}pages[count++]=page;}
 mnm_map_resolve=resolve;mnm_map_notice=producer_notice;mnm_map_release=release;
 for(u32 i=0;i<MNM_MAP_SITES;++i)jump((u8*)sites[i].at,(void*)sites[i].entry,sites[i].length);
 while(count){--count;u32 ignored;if(!VirtualProtect((void*)pages[count],4096,protection[count],&ignored) || !FlushInstructionCache(GetCurrentProcess(),(void*)pages[count],4096))ExitProcess(123);}
 installed=1;return 1;
}
__declspec(dllexport) void MinimapWritersAnchor(void){}
#ifdef MNM_MINIMAP_WRITER_SELFTEST
__declspec(dllexport) int WIN MinimapWritersInstallForTest(void){return install();}
#endif
int WIN DllMain(HANDLE module,u32 reason,void* reserved){(void)module;(void)reserved;if(reason!=1)return 1;
#ifdef MNM_MINIMAP_WRITER_SELFTEST
 return 1;
#else
 const u32 error=GetLastError();const int result=install();SetLastError(error);return result;
#endif
}
