/* Isolated PE32 native compatibility factory, no DirectDraw imports or calls.
 * Selected identity, device and owned RGB565 services are admitted. Unknown
 * method invocation terminates rather than silently forwarding to a driver. */
#include "../shadow/win32_min.h"
#include "../../reconstruction/rendering/pixel_kernel.h"
#define ND_INVALID ((i32)0x80070057u)
#define ND_NOINTERFACE ((i32)0x80004002u)
struct NdState;
struct NsState;
struct NdObject {void** table;struct NdState* state;u32 kind;};
struct NdState {struct NdObject first,second,unknown,fourth;u32 refs[4],interfaces;void* window;u32 coop,width,height,depth;struct NsState* primary;u32 creator;struct NdState *previous_owned,*next_owned;};
static struct NdState* nd_owned;
static void* nd_tables[4][28];
static u32 nd_objects,nd_surfaces,nd_creates,nd_attempts,nd_enumerations,nd_last_create;
static void nd_status(void);
static volatile u32 nd_status_phase;
static const u8 nd_default_guid[16]={0};
static const u8 nd_unknown[16]={0,0,0,0,0,0,0,0,0xc0,0,0,0,0,0,0,0x46};
static const u8 nd_draw1[16]={0x80,0xdb,0x14,0x6c,0x33,0xa7,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60};
static const u8 nd_draw2[16]={0xe0,0xf3,0xa6,0xb3,0x43,0x2b,0xcf,0x11,0xa2,0xde,0,0xaa,0,0xb9,0x33,0x56};
static const u8 nd_draw4[16]={0x9a,0x50,0x59,0x9c,0xbd,0x39,0xd1,0x11,0x8c,0x4a,0,0xc0,0x4f,0xd9,0x30,0xc5};
static int nd_equal(const u8* a,const u8* b) {for(u32 i=0;i<16;++i)if(a[i]!=b[i])return 0;return 1;}
static i32 WIN nd_query(struct NdObject* self,const u8* iid,void** out) {
  if(!iid || !out)return ND_INVALID;
  *out=0;
  if(nd_equal(iid,nd_unknown))*out=&self->state->unknown;
  else if(nd_equal(iid,nd_draw1))*out=&self->state->first;
  else if(nd_equal(iid,nd_draw2))*out=&self->state->second;
  else if(nd_equal(iid,nd_draw4))*out=&self->state->fourth;
  else return ND_NOINTERFACE;
  struct NdObject* selected=*out;
  if(!self->state->refs[selected->kind]++)++self->state->interfaces;return 0;
}
static u32 WIN nd_retain(struct NdObject* self) {
  if(!self->state->refs[self->kind]++)++self->state->interfaces;
  return self->state->refs[self->kind];
}
static u32 WIN nd_release(struct NdObject* self) {
  u32 error=GetLastError(),refs=--self->state->refs[self->kind];
  if(!refs && !--self->state->interfaces){struct NdState* s=self->state;
   if(s->previous_owned)s->previous_owned->next_owned=s->next_owned;else nd_owned=s->next_owned;
   if(s->next_owned)s->next_owned->previous_owned=s->previous_owned;
   HeapFree(GetProcessHeap(),0,s);--nd_objects;}
  nd_status();SetLastError(error);return refs;
}
#include "refusal.h"
#include "palette.h"
#include "clipper.h"
#include "surfaces.h"
#include "history.h"
#include "cpu_copy_boundary.h"
#include "status.h"
#include "producer_status.h"
#include "retirement.h"
#include "startup_retirement.h"
static i32 nd_create(void* guid,void** out,void* outer,u32 caller) {
  u32 error=GetLastError();
  if(!out)return ND_INVALID;
  *out=0;
  if(outer)return (i32)0x80040110u;
  if(guid && ((u32)guid<65536 || !nd_equal(guid,nd_default_guid)))return (i32)0x88760231u;
  if(nd_objects>=64)return (i32)0x8007000eu;
  struct NdState* s=HeapAlloc(GetProcessHeap(),8,sizeof(*s));
  if(!s){SetLastError(error);return (i32)0x8007000eu;}
  s->first=(struct NdObject){nd_tables[1],s,1};s->second=(struct NdObject){nd_tables[2],s,2};
  s->unknown=(struct NdObject){nd_tables[0],s,0};s->fourth=(struct NdObject){nd_tables[3],s,3};
  s->width=800;s->height=600;s->depth=16;s->refs[1]=1;s->interfaces=1;
  s->creator=caller;s->next_owned=nd_owned;if(nd_owned)nd_owned->previous_owned=s;nd_owned=s;
  ++nd_objects;++nd_creates;*out=&s->first;nd_status();SetLastError(error);return 0;
}
__declspec(dllexport) i32 WIN NativeDrawCreate(void* guid,void** out,void* outer) {
 u32 error=GetLastError();ns_guard_initialize();ns_image_copy_install();++nd_attempts;i32 result=nd_create(guid,out,outer,(u32)__builtin_return_address(0));nd_last_create=(u32)result;nd_status();SetLastError(error);return result;
}
typedef i32 (WIN *NdEnumCallback)(void*,const char*,const char*,void*);
__declspec(dllexport) i32 WIN NativeDrawEnumerateA(NdEnumCallback callback,void* context) {
  ++nd_enumerations;nd_status();if(!callback)return ND_INVALID;
  u32 error=GetLastError();callback(0,"Native owned RGB565","native",context);SetLastError(error);return 0;
}
__declspec(dllexport) u32 WIN NativeDrawClippers(void) {return nc_objects;}
__declspec(dllexport) u32 WIN NativeDrawDCs(void) {return ns_dcs;}
/* Process-local versioned call boundary. A lease cannot escape its owning
 * surface or survive ReleaseDC. INT_MIN means refusal before any mutation. */
__declspec(dllexport) i32 WIN NativeDrawDibWordsV1(void* dc,i32 x,i32 y,i32 w,i32 h,
 i32 sx,i32 sy,i32 sw,i32 sh,const u8* bits,const u8* info,u32 usage,u32 rop) {
 struct NsState* s=ns_dc_owner(dc);
 if(!s || s->bits!=16 || !info || !bits || w<1 || w>2048 || h<1 || h>2048 || sx || sy || sw!=w || sh!=h || rop!=0xcc0020)return (i32)0x80000000u;
 /* Producer adapter proves these source spans readable before this call. */
 u32 depth=(u32)info[14]|(u32)info[15]<<8;
 if(depth!=1 && depth!=4 && depth!=8 && depth!=24)return (i32)0x80000000u;
 struct MnmDibInput in={info,bits,40+(depth==24?0:(1u<<depth)*4),((w*depth+31)/32)*4*(u32)h,usage};
 struct MnmPixelPlane p={(u16*)s->dc_bits,s->width,s->height,((s->width*2+3)&~3u)/2,s->masks[0]==0x7c00};
 if(!ns_dc_plain(dc)||!GdiFlush()||!mnm_dib_words(&p,&in,x,y))return (i32)0x80000000u;
 return h;
}
/* SRCCOPY between current owned top-down RGB565 DC leases. Negative source
 * extents, custom transforms/clip/ROP and unknown handles refuse atomically. */
__declspec(dllexport) i32 WIN NativeDrawBitBltV1(void* dc,i32 x,i32 y,i32 w,i32 h,void* source,i32 sx,i32 sy,u32 rop){
 const u32 error=GetLastError();i32 result=(i32)0x80000000u;
 struct NsState *d=ns_dc_owner(dc),*s=ns_dc_owner(source);
 if(!d||!s||d->bits!=16||s->bits!=16||d->masks[0]!=s->masks[0]||w<0||w>2048||h<0||h>2048||rop!=0xcc0020||!ns_dc_plain(dc)||!ns_dc_plain(source))goto done;
 long long left=x,top=y,right=(long long)x+w,bottom=(long long)y+h;
 if(left<0)left=0;if(top<0)top=0;if(right>d->width)right=d->width;if(bottom>d->height)bottom=d->height;
 if(right<=left||bottom<=top){result=1;goto done;}
 const long long a=(long long)sx+left-x,b=(long long)sy+top-y;
 if(a<0||b<0||a+right-left>s->width||b+bottom-top>s->height||!GdiFlush())goto done;
 const u32 dp=(d->width+1)&~1u,sp=(s->width+1)&~1u,cols=(u32)(right-left),rows=(u32)(bottom-top);
 const int backwards=d==s&&top>b;
 for(u32 i=0;i<rows;++i){const u32 row=backwards?rows-1-i:i;
  u16* out=(u16*)d->dc_bits+((u32)top+row)*dp+(u32)left;const u16* in=(u16*)s->dc_bits+((u32)b+row)*sp+(u32)a;
  if(d==s&&top==b&&left>a){for(u32 j=cols;j;--j)out[j-1]=in[j-1];}
  else {for(u32 j=0;j<cols;++j)out[j]=in[j];}}
 result=1;
done:SetLastError(error);return result;
}
__declspec(dllexport) u32 WIN NativeDrawSurfaces(void) {return nd_surfaces;}
/* Process-local synchronous storage lookup. No pointer survives this call's
 * owner-thread operation; retired surfaces and active DC leases refuse. */
__declspec(dllexport) i32 WIN NativeDrawPlaneV1(const void* pixels,u32* fields,u32 bytes){
 if(!pixels || !fields || bytes!=24)return ND_INVALID;
 for(struct NsState* s=ns_owned;s;s=s->next_owned)if(s->pixels==pixels){
  if(s->dc || s->storage_dc)return NS_BUSY;if(s->bits!=16)return NS_UNSUPPORTED;
  fields[0]=1;fields[1]=s->width;fields[2]=s->height;fields[3]=s->pitch/2;fields[4]=s->locked;fields[5]=s->masks[0]==0x7c00;return 0;
 }
 return ND_INVALID;
}
__declspec(dllexport) void WIN NativeDrawAnchor(void) {}
/* Read-only launch-local diagnostic; never takes loader or drawing locks. */
__declspec(dllexport) u32 WIN NativeDrawStatusStage(void) {return __atomic_load_n(&nd_status_phase,__ATOMIC_RELAXED);}
__declspec(dllexport) void WIN NativeDrawStatusDeferV1(u32 begin) {nd_status_defer(begin);}
__declspec(dllexport) u32 WIN NativeDrawObjects(void) {return nd_objects;}
__declspec(dllexport) u32 WIN NativeDrawPalettes(void) {return np_objects;}
i32 WIN DllMain(HANDLE instance,u32 reason,void* reserved) {(void)instance;if(reason==1){nd_init();nd_status_start();np_status_start();}else if(reason==0){nd_retirement_report(reserved!=0);ns_guard_finish();np_status_finish();nd_status_finish();}return 1;}
