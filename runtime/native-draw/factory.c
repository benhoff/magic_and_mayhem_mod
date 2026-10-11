/* Isolated PE32 native compatibility factory, no DirectDraw imports or calls.
 * Selected identity, device and owned RGB565 services are admitted. Unknown
 * method invocation terminates rather than silently forwarding to a driver. */
#include "../shadow/win32_min.h"
#define ND_INVALID ((i32)0x80070057u)
#define ND_NOINTERFACE ((i32)0x80004002u)
struct NdState;
struct NdObject {void** table;struct NdState* state;u32 kind;};
struct NdState {struct NdObject first,second,unknown,fourth;u32 refs[4],interfaces;void* window;u32 coop,width,height,depth;};
static void* nd_tables[4][28];
static u32 nd_objects,nd_surfaces;
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
  if(!refs && !--self->state->interfaces){HeapFree(GetProcessHeap(),0,self->state);--nd_objects;}
  SetLastError(error);return refs;
}
static i32 WIN nd_unadmitted(struct NdObject* self) {
  (void)self;ExitProcess(97);return ND_NOINTERFACE;
}
#include "surfaces.h"
__declspec(dllexport) i32 WIN NativeDrawCreate(void* guid,void** out,void* outer) {
  u32 error=GetLastError();
  if(!out)return ND_INVALID;
  *out=0;
  if(outer)return (i32)0x80040110u;
  if(guid)return (i32)0x88760231u;
  if(nd_objects>=64)return (i32)0x8007000eu;
  struct NdState* s=HeapAlloc(GetProcessHeap(),8,sizeof(*s));
  if(!s){SetLastError(error);return (i32)0x8007000eu;}
  s->first=(struct NdObject){nd_tables[1],s,1};s->second=(struct NdObject){nd_tables[2],s,2};
  s->unknown=(struct NdObject){nd_tables[0],s,0};s->fourth=(struct NdObject){nd_tables[3],s,3};
  s->width=800;s->height=600;s->depth=16;s->refs[1]=1;s->interfaces=1;
  ++nd_objects;*out=&s->first;SetLastError(error);return 0;
}
typedef i32 (WIN *NdEnumCallback)(void*,const char*,const char*,void*);
__declspec(dllexport) i32 WIN NativeDrawEnumerateA(NdEnumCallback callback,void* context) {
  if(!callback)return ND_INVALID;
  u32 error=GetLastError();callback(0,"Native owned RGB565","native",context);SetLastError(error);return 0;
}
__declspec(dllexport) u32 WIN NativeDrawSurfaces(void) {return nd_surfaces;}
__declspec(dllexport) void WIN NativeDrawAnchor(void) {}
__declspec(dllexport) u32 WIN NativeDrawObjects(void) {return nd_objects;}
i32 WIN DllMain(HANDLE instance,u32 reason,void* reserved) {(void)instance;(void)reserved;if(reason==1)nd_init();return 1;}
