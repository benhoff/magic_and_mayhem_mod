/* Bounded HWND binding. Region/occlusion APIs remain unadmitted; attaching a
 * clipper does not authorize clipped Blt until that contract is validated. */
API int WIN IsWindow(void*);
struct NcClipper {void** table;u32 refs;void* window;};
static void* nc_table[9];static u32 nc_objects;
static const u8 nc_id[16]={0x85,0xdb,0x14,0x6c,0x33,0xa7,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60};
static i32 WIN nc_query(struct NcClipper* c,const u8* iid,void** out){
 if(!iid || !out)return ND_INVALID;*out=0;
 if(!nd_equal(iid,nd_unknown) && !nd_equal(iid,nc_id))return ND_NOINTERFACE;
 ++c->refs;*out=c;return 0;
}
static u32 WIN nc_retain(struct NcClipper* c){return ++c->refs;}
static u32 WIN nc_release(struct NcClipper* c){u32 error=GetLastError(),refs=--c->refs;
 if(!refs){HeapFree(GetProcessHeap(),0,c);--nc_objects;}SetLastError(error);return refs;}
static i32 WIN nc_window(struct NcClipper* c,void** out){if(!out)return ND_INVALID;*out=c->window;return 0;}
static i32 WIN nc_set_window(struct NcClipper* c,u32 flags,void* window){
 if(flags || (window && !IsWindow(window)))return ND_INVALID;c->window=window;return 0;
}
static i32 WIN nd_clipper(struct NdObject* o,u32 flags,void** out,void* outer){
 (void)o;if(!out)return ND_INVALID;*out=0;if(outer)return (i32)0x80040110u;
 if(flags)return ND_INVALID;if(nc_objects>=64)return (i32)0x8007000eu;
 struct NcClipper* c=HeapAlloc(GetProcessHeap(),8,sizeof(*c));if(!c)return (i32)0x8007000eu;
 c->table=nc_table;c->refs=1;*out=c;++nc_objects;return 0;
}
static i32 WIN nc_unadmitted(struct NcClipper* c){(void)c;ExitProcess(99);return ND_NOINTERFACE;}
static void nc_init(void){
 for(u32 i=0;i<9;++i)nc_table[i]=(void*)&nc_unadmitted;
 nc_table[0]=(void*)&nc_query;nc_table[1]=(void*)&nc_retain;nc_table[2]=(void*)&nc_release;
 nc_table[4]=(void*)&nc_window;nc_table[8]=(void*)&nc_set_window;
}
