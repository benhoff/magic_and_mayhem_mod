#include "../shadow/win32_min.h"
#include "producer_status.h"
#include "../../reconstruction/rendering/colour_rect.h"
API u32 WIN GetCurrentThreadId(void);
API void* WIN GetProcAddress(void*,const char*);
#define MNM_HISTORY_FAMILY 5
#include "history.h"
typedef i32 (WIN *Plane)(const void*,u32*,u32);
extern void mnm_colour_rect_enter(void);
uintptr_t mnm_colour_rect_original,mnm_colour_rect_resume;
u32 mnm_colour_rect_native,mnm_colour_rect_forwarded;
static Plane owned_plane;
static volatile u32 owner;
static u32 word(u32 at){const u8 *p=(const u8*)at;return p[0]|(u32)p[1]<<8|(u32)p[2]<<16|(u32)p[3]<<24;}
static int span(u32 at,u32 n,int write){
 if(!at||!n||at>0xffffffffu-n)return 0;const u32 end=at+n;
 while(at<end){u32 m[7];if(VirtualQuery((void*)at,m,28)!=28||m[4]!=0x1000||(m[5]&0x101)||!(m[5]&(write?0xcc:0xee))||m[0]>at||!m[3]||m[0]>0xffffffffu-m[3]||m[0]+m[3]<=at)return 0;at=m[0]+m[3];}return 1;
}
#ifndef MNM_COLOUR_RECT_SELFTEST
static u32 refusal_count;
static void refusal(u32 reason,const u32* r,const u32* d,const struct MnmColourRect* c){
 char enabled[2];if(refusal_count>=8||GetEnvironmentVariableA("MNM_COLOUR_RECT_DIAGNOSTICS",enabled,2)!=1||enabled[0]!='1')return;
 u32 data[64],done=0;for(u32 i=0;i<64;++i)data[i]=0;
 data[0]=0x4652434d;data[1]=1;data[2]=reason;
 for(u32 i=0;i<14;++i)data[4+i]=r[i];
 for(u32 i=0;i<6;++i)data[18+i]=d[i];
 data[24]=word(0x658174);data[25]=word(0x6e1f88);
 data[26]=c->width;data[27]=c->height;data[28]=c->mode;data[29]=c->source_x;data[30]=c->source_y;
 data[31]=c->visible_width;data[32]=c->visible_height;data[33]=c->x;data[34]=c->y;
 data[35]=c->red;data[36]=c->green;data[37]=c->blue;data[38]=(u32)c->triples;
 if(span((u32)(r+9),0x30,0))data[39]=r[9+0x2c/4];
 if(span(r[0],22,0))for(u32 i=0;i<22;++i)((u8*)(data+40))[i]=((const u8*)r[0])[i];
 if(c->triples&&span((u32)c->triples,36,0))for(u32 i=0;i<36;++i)((u8*)(data+46))[i]=c->triples[i];
 char name[]="colour-rect-refusal-0.bin";name[20]=(char)('0'+refusal_count++);
 HANDLE f=CreateFileA(name,0x40000000,0,0,1,0x80,0);
 if(f!=(HANDLE)-1){WriteFile(f,data,sizeof(data),&done,0);CloseHandle(f);}
}
#else
static void refusal(u32 reason,const u32* r,const u32* d,const struct MnmColourRect* c){(void)reason;(void)r;(void)d;(void)c;}
#endif
#define RECT_REFUSE do{reason=__LINE__;goto done;}while(0)
int mnm_colour_rect_route(u32 *r){
 const u32 error=GetLastError();int ok=0;u32 reason=0;u32 *stack=r+9;struct MnmColourRect c={0};u32 d[6]={0},blue=0;
 const u32 shape=r[0],base=word(0x658174),format=word(0x6e1f88);
 if(!owned_plane||!span(shape,22,0)||!span((u32)stack,0x40,0)||format>1||owned_plane((void*)base,d,24)||d[0]!=1||d[3]!=word(0x6a2dc8)||d[5]!=format)RECT_REFUSE;
 struct MnmPixelPlane p={(u16*)base,d[1],d[2],d[3],format};
 if(!mnm_pixel_plane(&p)||!span(base,p.stride*p.height*2,1)||r[5]<base||((r[5]-base)&1))RECT_REFUSE;
 if(shape<base+p.stride*p.height*2&&base<shape+22)RECT_REFUSE;
 const u32 offset=(r[5]-base)/2;if(offset>p.stride*p.height+p.width)RECT_REFUSE;
 const u32 thread=GetCurrentThreadId();if(!owner)__sync_bool_compare_and_swap(&owner,0,thread);if(thread!=owner)RECT_REFUSE;
 c.width=word(shape+4);c.height=word(shape+8);c.mode=word(shape);c.source_x=r[1];c.source_y=stack[0x3c/4];
 if((i32)r[4]<(i32)r[1]||(i32)r[6]<(i32)c.source_y)RECT_REFUSE;
 c.visible_width=r[4]-r[1];c.visible_height=r[6]-c.source_y;c.x=offset%p.stride;c.y=offset/p.stride;
 c.red=(u16)word(shape+12);c.green=(u16)word(shape+14);c.blue=(u16)word(shape+16);c.triples=(const u8*)word(shape+18);
 if(c.width>2048||c.height>2048||(c.triples&&!span((u32)c.triples,c.width*c.height*6,0)))RECT_REFUSE;
 if(!c.visible_width||!c.visible_height){
  /* The original enters this splice for edge-touching rectangles. Its row
   * bookkeeping still changes volatile registers, but no pixel is read or
   * written. Preserve that state without forwarding or reading stale history. */
  if(!c.width||!c.height||c.mode>3||c.source_x>c.width||c.source_y>c.height||
   c.visible_width>c.width-c.source_x||c.visible_height>c.height-c.source_y||
   (u32)c.x>p.width||(u32)c.y>p.height||c.visible_height>p.height-(u32)c.y)RECT_REFUSE;
  const u32 packed=((u32)c.red<<(format?10:11)&65535u)|((u32)c.green<<5&65535u)|c.blue;
  if(!c.visible_height){
   r[7]=c.triples?(u32)c.triples+(c.source_y*c.width+c.source_x)*6:c.mode?c.source_y:packed;
  }else{
   r[5]+=c.visible_height*p.stride*2;
   r[7]=c.triples?(u32)c.triples+((c.source_y+c.visible_height)*c.width+c.source_x)*6:c.mode?0:packed;
   r[6]=c.triples||!c.mode?0:c.visible_height;
  }
  ++mnm_colour_rect_native;ok=1;goto done;
 }
 if(c.mode)cpu_history_require((void*)base);
 if(!c.triples&&!c.mode&&(r[8]&0x400))RECT_REFUSE;
 if(!mnm_colour_rect(&p,&c,&blue))RECT_REFUSE;
 r[5]+=c.visible_height*p.stride*2;
 if(c.triples){r[7]=(u32)c.triples+((c.source_y+c.visible_height)*c.width+c.source_x)*6;r[6]=0;}
 else {r[7]=c.mode?0:((u32)c.red<<(format?10:11)|(u32)c.green<<5|c.blue);
       r[6]=c.mode==0?0:c.mode==3?blue>>2:c.mode==1?(stack[0x2c/4]&0xffff0000u)|(c.blue>>1):c.blue>>1;}
 ++mnm_colour_rect_native;ok=1;
done:if(!ok&&(!span(shape,4,0)||word(shape)))cpu_history_require((void*)base);
 producer_notice(0,ok);if(!ok){refusal(reason,r,d,&c);++mnm_colour_rect_forwarded;}SetLastError(error);return ok;
}
static int install(void){
 char flag[4];const u32 n=GetEnvironmentVariableA("MNM_COLOUR_RECT_TAKEOVER",flag,4);if(!n)return 1;
 const u8 expected[5]={0x8b,0x47,0x12,0x85,0xc0},tail[8]={0x5f,0x5e,0x5d,0x66,0x33,0xc0,0x5b,0x83};
 if(n!=1||flag[0]!='1'||(u32)GetModuleHandleA(0)!=0x400000)return 0;
 if(!cpu_history_install())return 0;
 if(!producer_start(4,1))return 0;
 for(u32 i=0;i<5;++i)if(*(const u8*)(0x547e53+i)!=expected[i])return 0;
 for(u32 i=0;i<8;++i)if(*(const u8*)(0x54852f+i)!=tail[i])return 0;
 owned_plane=(Plane)GetProcAddress(GetModuleHandleA("MnmDraw.dll"),"NativeDrawPlaneV1");if(!owned_plane)return 0;
 u8 *tramp=VirtualAlloc(0,4096,0x3000,0x40);if(!tramp)return 0;
 for(u32 i=0;i<5;++i)tramp[i]=expected[i];tramp[5]=0xe9;*(u32*)(tramp+6)=0x547e58-(u32)tramp-10;
 u32 old;if(!VirtualProtect(tramp,4096,0x20,&old)||!FlushInstructionCache(GetCurrentProcess(),tramp,10))return 0;
 mnm_colour_rect_original=(uintptr_t)tramp;mnm_colour_rect_resume=0x54852f;
 if(!VirtualProtect((void*)0x547e53,5,0x40,&old))return 0;
 *(u8*)0x547e53=0xe9;*(u32*)0x547e54=(u32)mnm_colour_rect_enter-0x547e58;
 u32 ignored;if(!VirtualProtect((void*)0x547e53,5,old,&ignored)||!FlushInstructionCache(GetCurrentProcess(),(void*)0x547e53,5))ExitProcess(123);
 return 1;
}
__declspec(dllexport) void ColourRectAnchor(void){}
#ifdef MNM_COLOUR_RECT_SELFTEST
__declspec(dllexport) int WIN ColourRectInstallForTest(void){return install();}
#endif
int WIN DllMain(HANDLE module,u32 reason,void *reserved){(void)module;(void)reserved;if(reason!=1)return 1;
#ifdef MNM_COLOUR_RECT_SELFTEST
 return 1;
#else
 const u32 error=GetLastError();const int ok=install();SetLastError(error);return ok;
#endif
}
