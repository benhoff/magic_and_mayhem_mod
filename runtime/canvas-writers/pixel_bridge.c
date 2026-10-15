/* Independent post-preparation pixel splices. No capture I/O or call budget. */
#include "../shadow/win32_min.h"
#include "producer_status.h"
#include "pixel_route.h"
#include "fade_route.h"
API u32 WIN GetCurrentThreadId(void);
API void* WIN GetProcAddress(void*,const char*);
#define MNM_HISTORY_FAMILY 2
#include "history.h"
extern void mnm_pcx_surface_enter(void),mnm_rgb_add_enter(void),mnm_pcx_indices_enter(void),
 mnm_pcx_raw_enter(void),mnm_brighten_enter(void),mnm_row_copy_enter(void),mnm_fade_enter(void);
static volatile u32 owner;
static int installed;
static int span(u32 at,u32 bytes,int writable) {
 if(!at || !bytes || at>0xffffffffu-bytes)return 0;
 const u32 end=at+bytes;
 while(at<end){u32 m[7];
  if(VirtualQuery((void*)at,m,28)!=28 || m[4]!=0x1000 || (m[5]&0x101) || !(m[5]&0xee) ||
   (writable && !(m[5]&0xcc)) || m[0]>at || !m[3] || m[0]>0xffffffffu-m[3] || m[0]+m[3]<=at)return 0;
  at=m[0]+m[3];}
 return 1;
}
static int owns_thread(void) {
 const u32 thread=GetCurrentThreadId();if(!owner)__sync_bool_compare_and_swap(&owner,0,thread);return owner==thread;
}
static int pcx(u32 object,struct MnmPcxInput* s,int palette) {
 if(!span(object,20,0))return 0;
 const u32* o=(const u32*)object;
 *s=(struct MnmPcxInput){(const u8*)o[0],palette?(const u8*)o[2]:0,o[1],o[3],o[4]};
 return s->bytes<=16u*1024u*1024u && span(o[0],s->bytes,0) && (!palette || span(o[2],768,0)) && mnm_pcx_admit(s);
}
static int plane(u32 object,u32 pixels,struct MnmPixelPlane* p,int original_word_pitch) {
 if(!span(object,28,0))return 0;
 const u32* o=(const u32*)object;
 if(!original_word_pitch && (o[6]&1))return 0;
 *p=(struct MnmPixelPlane){(u16*)pixels,o[4],o[5],original_word_pitch?(o[6]&65535):o[6]/2,*(const u32*)0x6e1f88};
 return mnm_pixel_plane(p) && span(pixels,p->stride*p->height*2,1);
}
static int resolve(u32* r,unsigned kind,struct MnmPixelPlane* p,struct MnmPcxInput* s,u32* a) {
 const u32 error=GetLastError();int ok=0;u32* stack=r+9;
 if(kind==MNM_RGB_ADD)cpu_history_require_route(*(void**)0x658174,kind);
 else if(kind==MNM_BRIGHTEN)cpu_history_require_route((void*)r[7],kind);
 else if(kind==MNM_ROW_COPY)cpu_history_require_route((void*)r[1],kind);
 if(kind>=MNM_PIXEL_SITES || (r[8]&0x400) || !span((u32)stack,kind==MNM_ROW_COPY?0x424:0x38,0) || !owns_thread())goto done;
 if(kind==MNM_PCX_SURFACE){
  if(!pcx(r[0],s,1) || !plane(r[4],r[6],p,0))goto done;
 }else if(kind==MNM_PCX_INDICES || kind==MNM_PCX_RAW_WORDS){
  if(!pcx(kind==MNM_PCX_INDICES?r[0]:r[2],s,kind!=MNM_PCX_INDICES))goto done;
  const u32 stride=stack[(kind==MNM_PCX_INDICES?0x20:0x24)/4];
  const u32 pixels=kind==MNM_PCX_INDICES?r[2]:stack[0x20/4];
  if(stride<s->width || stride>4096 || !s->height || s->height>2048)goto done;
  a[0]=stride*s->height*(kind==MNM_PCX_INDICES?1:2);
  if(!span(pixels,a[0],1))goto done;
  *p=(struct MnmPixelPlane){(u16*)pixels,s->width,s->height,stride,*(const u32*)0x6e1f88};
 }else if(kind==MNM_RGB_ADD){
  const u32 object=stack[0x10/4];if(!span(object,20,0))goto done;
  const u32* o=(const u32*)object;const i32 x=(i32)stack[0x30/4],skip=(i32)stack[0x28/4];
  if(r[7]>2048 || r[4]>2048 || r[7]<=r[4])goto done;
  const i32 rows=(i32)(r[7]-r[4]);
  const u32 stride=*(const u32*)0x6a2dc8,height=*(const u32*)0x656618,width=*(const u32*)0x6a49b8;
  *p=(struct MnmPixelPlane){*(u16**)0x658174,width,height,stride,*(const u32*)0x6e1f88};
  if(!mnm_pixel_plane(p) || !span((u32)p->pixels,stride*height*2,1) ||
   skip<0 || x<=skip || x>2048 || rows<=0 || rows>2048 || r[0]<(u32)p->pixels || ((r[0]-(u32)p->pixels)&1))goto done;
  const u32 start=(r[0]-(u32)p->pixels)/2,left=start%stride,top=start/stride;
  if(left>=width || (u32)(x-skip)>width-left || top>=height || (u32)rows>height-top)goto done;
  a[0]=left;a[1]=top;a[2]=left+x-skip;a[3]=top+rows;
  a[4]=o[2]&65535;a[5]=o[3]&65535;a[6]=o[4]&65535;a[7]=(u32)x;a[8]=(u32)skip;
 }else if(kind==MNM_BRIGHTEN){if(!plane(r[0],r[7],p,0))goto done;
 }else{
  struct MnmPixelPlane source;
  if(!plane(stack[0x10/4],r[1],&source,1) || !plane(r[0],r[4],p,1) || !span(r[2],16,0))goto done;
  const u32* rect=(const u32*)r[2];a[0]=(u32)source.pixels;a[1]=source.width;a[2]=source.height;a[3]=source.stride;a[4]=rect[2];a[5]=rect[3];
 }
 ok=1;
done:SetLastError(error);return ok;
}
static int fade_resolve(const u32* r,struct MnmFadePlane* f) {
 const u32 error=GetLastError();struct MnmPixelPlane p;int ok=0;
 cpu_history_require((void*)r[7]);
 if(!(r[8]&0x400) && span((u32)(r+9),16,0) && r[12]==r[7] && owns_thread() && plane(r[0],r[7],&p,0)){
  *f=(struct MnmFadePlane){p.pixels,p.width,p.height,p.stride,p.format};ok=1;}
 SetLastError(error);return ok;
}
typedef i32 (WIN *DibCall)(void*,i32,i32,i32,i32,i32,i32,i32,i32,const u8*,const u8*,u32,u32);
static DibCall native_dib;
static i32 WIN dib(void* dc,i32 x,i32 y,i32 w,i32 h,i32 sx,i32 sy,i32 sw,i32 sh,const u8* bits,const u8* info,u32 usage,u32 rop) {
 const u32 error=GetLastError();i32 result=(i32)0x80000000u;
 if(native_dib && owns_thread() && w>0 && w<=2048 && h>0 && h<=2048 && span((u32)info,40,0)){
  const u32 depth=(u32)info[14]|(u32)info[15]<<8;
  if((depth==1 || depth==4 || depth==8 || depth==24) &&
   span((u32)info,40+(depth==24?0:(1u<<depth)*4),0) && span((u32)bits,((w*depth+31)/32)*4*(u32)h,0))
    result=native_dib(dc,x,y,w,h,sx,sy,sw,sh,bits,info,usage,rop);
 }
 SetLastError(error);
 if(result!=(i32)0x80000000u){producer_notice(7,1);return result;}
 producer_notice(7,0);
 return (*(DibCall*)0x5c5034)(dc,x,y,w,h,sx,sy,sw,sh,bits,info,usage,rop);
}
static const struct Site {u32 at,length,tail;u8 bytes[8],tail_bytes[8];void(*entry)(void);} sites[MNM_PIXEL_SITES]={
 {0x54b31d,8,0x54b4f1,{0xc7,0x44,0x24,0x14,0,0,0,0},{0x8b,0x43,4,0x51,0x50,0x8b,0x10,0xff},mnm_pcx_surface_enter},
 {0x545ce9,6,0x545dae,{0x2b,0xc3,0x89,0x44,0x24,0x14},{0x5f,0x5e,0x5d,0x66,0x33,0xc0,0x5b,0x83},mnm_rgb_add_enter},
 {0x54b53d,8,0x54b5e5,{0xc7,0x44,0x24,0x1c,0,0,0,0},{0x5f,0x5d,0x83,0xc4,0x10,0xc2,8,0},mnm_pcx_indices_enter},
 {0x54b618,8,0x54b7f3,{0xc7,0x44,0x24,0x0c,0,0,0,0},{0x5e,0x5d,0x83,0xc4,0x14,0xc2,8,0},mnm_pcx_raw_enter},
 {0x58d7e7,8,0x58d8a1,{0xc7,0x44,0x24,0x14,0,0,0,0},{0x8b,0x7f,4,0x50,0x57,0x8b,0x0f,0xff},mnm_brighten_enter},
 {0x58d430,5,0x58d47c,{0x8b,0x45,0x0c,0x33,0xc9},{0x8b,0x44,0x24,0x10,0x56,0x8b,0x40,4},mnm_row_copy_enter}
};
static int same(const u8* a,const u8* b,u32 n){for(u32 i=0;i<n;++i)if(a[i]!=b[i])return 0;return 1;}
static void branch(u8* at,void* target,u32 n,u8 opcode){at[0]=opcode;*(u32*)(at+1)=(u32)target-(u32)at-5;for(u32 i=5;i<n;++i)at[i]=0x90;}
static int install(void) {
 if(installed)return 1;char value[4];const u32 n=GetEnvironmentVariableA("MNM_PIXEL_TAKEOVER",value,sizeof(value));
 if(!n)return 1;if(n!=1 || value[0]!='1' || (u32)GetModuleHandleA(0)!=0x400000)return 0;
 if(!cpu_history_install())return 0;
 if(!producer_start(1,8))return 0;
 for(u32 i=0;i<MNM_PIXEL_SITES;++i)if(!span(sites[i].at,sites[i].length,0) || !same((u8*)sites[i].at,sites[i].bytes,sites[i].length) ||
  !span(sites[i].tail,8,0) || !same((u8*)sites[i].tail,sites[i].tail_bytes,8))return 0;
 const u8 fade[]={0x53,0x8b,0x47,0x10,0x33,0xdb,0x99,0x2b,0xc2,0xd1,0xf8,0x85,0xc0,0x7e,0x19,
  0x8b,0x01,0x83,0xc1,0x04,0xd1,0xe8,0x23,0xc6,0x43,0x89,0x41,0xfc,0x8b,0x47,0x10,0x99,
  0x2b,0xc2,0xd1,0xf8,0x3b,0xd8,0x7c,0xe7,0x8b,0x47,0x14,0x45,0x3b,0xe8,0x7c,0xd1,
  0x8b,0x44,0x24,0x10,0x5b};
 const u8 dib_call[]={0xff,0x15,0x34,0x50,0x5c,0};
 if(!same((u8*)0x58edb0,fade,sizeof(fade)) || !same((u8*)0x486aca,dib_call,6))return 0;
 native_dib=(DibCall)GetProcAddress(GetModuleHandleA("MnmDraw.dll"),"NativeDrawDibWordsV1");
 if(!native_dib)return 0;
 u8* tramp=VirtualAlloc(0,4096,0x3000,0x40);if(!tramp)return 0;
 for(u32 i=0;i<MNM_PIXEL_SITES;++i){u8* t=tramp+i*32;for(u32 j=0;j<sites[i].length;++j)t[j]=sites[i].bytes[j];
  branch(t+sites[i].length,(void*)(sites[i].at+sites[i].length),5,0xe9);mnm_pixel_original[i]=(uintptr_t)t;mnm_pixel_resume[i]=sites[i].tail;}
 /* Both original back-edges land inside the overwritten prefix. Freeze the
  * complete loop, including its balanced PUSH/POP, for original-once refusal. */
 for(u32 i=0;i<sizeof(fade);++i)tramp[256+i]=fade[i];branch(tramp+256+sizeof(fade),(void*)0x58ede5,5,0xe9);
 mnm_fade_original=(uintptr_t)(tramp+256);mnm_fade_resume=0x58ede5;mnm_fade_resolve=fade_resolve;
 u32 old;if(!VirtualProtect(tramp,4096,0x20,&old) || !FlushInstructionCache(GetCurrentProcess(),tramp,4096))return 0;
 /* Obtain all pages before any site mutation; roll back protections on refusal. */
 u32 pages[9],protection[9],count=0;
 for(u32 i=0;i<MNM_PIXEL_SITES+2;++i){const u32 at=i<MNM_PIXEL_SITES?sites[i].at:i==MNM_PIXEL_SITES?0x58edb0:0x486aca,page=at&~4095u;
  u32 j=0;while(j<count && pages[j]!=page)++j;if(j<count)continue;
  if(!VirtualProtect((void*)page,4096,0x40,protection+count)){
   while(count){--count;u32 ignored;if(!VirtualProtect((void*)pages[count],4096,protection[count],&ignored))ExitProcess(123);}return 0;}
  pages[count++]=page;
 }
 mnm_pixel_resolve=resolve;mnm_pixel_notice=producer_notice;mnm_fade_notice=producer_notice;
 for(u32 i=0;i<MNM_PIXEL_SITES;++i)branch((u8*)sites[i].at,(void*)sites[i].entry,sites[i].length,0xe9);
 branch((u8*)0x58edb0,(void*)mnm_fade_enter,6,0xe9);branch((u8*)0x486aca,(void*)dib,6,0xe8);
 while(count){--count;u32 ignored;if(!VirtualProtect((void*)pages[count],4096,protection[count],&ignored) || !FlushInstructionCache(GetCurrentProcess(),(void*)pages[count],4096))ExitProcess(123);}
 installed=1;return 1;
}
__declspec(dllexport) void PixelWritersAnchor(void) {}
#ifdef MNM_PIXEL_WRITER_SELFTEST
__declspec(dllexport) int WIN PixelWritersInstallForTest(void) {return install();}
#endif
int WIN DllMain(HANDLE module,u32 reason,void* reserved) {
 (void)module;(void)reserved;if(reason!=1)return 1;
#ifdef MNM_PIXEL_WRITER_SELFTEST
 return 1;
#else
 const u32 error=GetLastError();const int result=install();SetLastError(error);return result;
#endif
}
