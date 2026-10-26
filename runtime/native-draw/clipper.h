/* Owned clip lists. GDI canonicalizes explicit unions and supplies current HWND
 * visibility; snapshots belong to one synchronous operation, never a surface. */
API int WIN IsWindow(void*);
API void* WIN CreateRectRgn(i32,i32,i32,i32);
API void* WIN ExtCreateRegion(const void*,u32,const void*);
API u32 WIN GetRegionData(void*,u32,void*);
API i32 WIN CombineRgn(void*,void*,void*,i32);
API int WIN DeleteObject(void*);
API int WIN EqualRgn(void*,void*);
API void* WIN GetDC(void*);
API i32 WIN ReleaseDC(void*,void*);
API i32 WIN GetRandomRgn(void*,void*,i32);
struct NcClipper {void** table;u32 refs;void* window;void* region;void* observed;u32 changed;};
static void* nc_table[9];static u32 nc_objects;
static const u8 nc_id[16]={0x85,0xdb,0x14,0x6c,0x33,0xa7,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60};
static i32 WIN nc_query(struct NcClipper* c,const u8* iid,void** out){
 struct NsLifeGuard lifetime __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!iid || !out)return ND_INVALID;*out=0;
 if(!nd_equal(iid,nd_unknown) && !nd_equal(iid,nc_id))return ND_NOINTERFACE;
 ++c->refs;*out=c;return 0;
}
static u32 WIN nc_retain(struct NcClipper* c){
 struct NsLifeGuard lifetime __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();return ++c->refs;}
static u32 WIN nc_release(struct NcClipper* c){
 struct NsLifeGuard lifetime __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();u32 error=GetLastError(),refs=--c->refs;
 if(!refs){if(c->region)DeleteObject(c->region);if(c->observed)DeleteObject(c->observed);HeapFree(GetProcessHeap(),0,c);--nc_objects;}nd_status();SetLastError(error);return refs;}
static i32 WIN nc_window(struct NcClipper* c,void** out){
 struct NsLifeGuard lifetime __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();if(!out)return ND_INVALID;*out=c->window;return 0;}
static i32 WIN nc_set_window(struct NcClipper* c,u32 flags,void* window){
 struct NsLifeGuard lifetime __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(flags || (window && !IsWindow(window)))return ND_INVALID;c->window=window;c->changed=1;if(c->observed)DeleteObject(c->observed);c->observed=0;return 0;
}
/* Caller frees the immutable RGNDATA snapshot. Null without error denotes an
 * attached clipper with no list; an allocated zero-count list is full exclusion. */
static i32 nc_snapshot(struct NcClipper* c,u32** out){
 *out=0;void* region=c->region;void* temporary=0;
 if(c->window){
  temporary=CreateRectRgn(0,0,0,0);if(!temporary)return (i32)0x8007000eu;
  void* dc=GetDC(c->window);i32 ok=dc?GetRandomRgn(dc,temporary,4):0;
  if(dc)ReleaseDC(c->window,dc);
  if(ok!=1){DeleteObject(temporary);return (i32)0x80004005u;}region=temporary;
 }
 if(!region)return (i32)0x887600cdu;
 u32 bytes=GetRegionData(region,0,0);i32 result=(i32)0x80004005u;
 if(bytes>=32 && bytes<=32+256*16){
  u32* data=HeapAlloc(GetProcessHeap(),0,bytes);
  if(!data)result=(i32)0x8007000eu;
  else if(GetRegionData(region,bytes,data)==bytes && data[0]==32 && data[1]==1 && data[2]<=256 && bytes==32+16*data[2]){
   if(temporary && (!c->observed || !EqualRgn(c->observed,temporary))){
    c->changed=1;if(c->observed)DeleteObject(c->observed);c->observed=temporary;temporary=0;}
   *out=data;result=0;}
  else HeapFree(GetProcessHeap(),0,data);
 }
 if(temporary)DeleteObject(temporary);return result;
}
static i32 WIN nc_get_list(struct NcClipper* c,const i32* rect,u32* out,u32* size){
 struct NsLifeGuard lifetime __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!size)return ND_INVALID;u32* data=0;i32 result=nc_snapshot(c,&data);if(result)return result;
 if(rect){
  void* region=ExtCreateRegion(0,32+16*data[2],data);void* cut=CreateRectRgn(rect[0],rect[1],rect[2],rect[3]);
  if(!region || !cut || !CombineRgn(region,region,cut,1)){result=(i32)0x80004005u;}
  else {u32 bytes=GetRegionData(region,0,0);if(bytes>32+256*16)result=(i32)0x80004001u;
   else {HeapFree(GetProcessHeap(),0,data);data=HeapAlloc(GetProcessHeap(),0,bytes);
    if(!data || GetRegionData(region,bytes,data)!=bytes)result=(i32)0x8007000eu;}}
  if(region)DeleteObject(region);if(cut)DeleteObject(cut);
 }
 if(!result){u32 bytes=32+16*data[2],capacity=*size;*size=bytes;
  if(out && capacity<bytes)result=(i32)0x88760236u;
  else if(out)for(u32 i=0;i<bytes/4;++i)out[i]=data[i];}
 if(data)HeapFree(GetProcessHeap(),0,data);return result;
}
static i32 WIN nc_set_list(struct NcClipper* c,const u32* data,u32 flags){
 struct NsLifeGuard lifetime __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(flags)return ND_INVALID;if(c->window)return (i32)0x88760237u;
 void* region=0;
 if(data){if(data[0]!=32 || data[1]!=1 || data[2]>256 || data[3]!=16*data[2])return (i32)0x8876006eu;
  region=ExtCreateRegion(0,32+16*data[2],data);if(!region)return (i32)0x8876006eu;
  u32 bytes=GetRegionData(region,0,0);if(bytes<32 || bytes>32+256*16){DeleteObject(region);return (i32)0x80004001u;}}
 if(c->region)DeleteObject(c->region);c->region=region;c->changed=1;return 0;
}
static i32 WIN nc_changed(struct NcClipper* c,i32* out){
 struct NsLifeGuard lifetime __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 if(!out)return ND_INVALID;if(c->window){u32* data=0;i32 result=nc_snapshot(c,&data);if(result)return result;HeapFree(GetProcessHeap(),0,data);}
 *out=c->changed!=0;c->changed=0;return 0;
}
static i32 WIN nd_clipper(struct NdObject* o,u32 flags,void** out,void* outer){
 struct NsLifeGuard lifetime __attribute__((cleanup(ns_life_guard_leave)))=ns_life_guard_enter();
 (void)o;if(!out)return ND_INVALID;*out=0;if(outer)return (i32)0x80040110u;
 if(flags)return ND_INVALID;if(nc_objects>=64)return (i32)0x8007000eu;
 struct NcClipper* c=HeapAlloc(GetProcessHeap(),8,sizeof(*c));if(!c)return (i32)0x8007000eu;
 c->table=nc_table;c->refs=1;*out=c;++nc_objects;nd_status();return 0;
}
static void nc_init(void){
 for(u32 i=0;i<9;++i)nc_table[i]=nd_refusals_3[i];
 nc_table[0]=(void*)&nc_query;nc_table[1]=(void*)&nc_retain;nc_table[2]=(void*)&nc_release;
 nc_table[4]=(void*)&nc_window;nc_table[8]=(void*)&nc_set_window;
 nc_table[3]=(void*)&nc_get_list;nc_table[6]=(void*)&nc_changed;nc_table[7]=(void*)&nc_set_list;
}
