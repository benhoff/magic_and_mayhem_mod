/* Native indexed palette objects own input entries, independent of surface
 * indices and of each acquired GDI DC's color-table snapshot. */
struct NpPalette {void** table;u32 refs,flags;u32 entries[256];};
static void* np_table[7];static u32 np_objects;
static const u8 np_id[16]={0x84,0xdb,0x14,0x6c,0x33,0xa7,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60};
static i32 WIN np_query(struct NpPalette* p,const u8* iid,void** out){
 if(!iid || !out)return ND_INVALID;*out=0;
 if(!nd_equal(iid,nd_unknown) && !nd_equal(iid,np_id))return ND_NOINTERFACE;
 ++p->refs;*out=p;return 0;
}
static u32 WIN np_retain(struct NpPalette* p){return ++p->refs;}
static u32 WIN np_release(struct NpPalette* p){u32 refs=--p->refs;if(!refs){HeapFree(GetProcessHeap(),0,p);--np_objects;}return refs;}
static i32 WIN np_caps(struct NpPalette* p,u32* out){if(!out)return ND_INVALID;*out=p->flags;return 0;}
static i32 WIN np_get(struct NpPalette* p,u32 flags,u32 first,u32 count,u32* out){
 if(flags || !out || first>=256 || count>256-first)return ND_INVALID;
 for(u32 i=0;i<count;++i)out[i]=p->entries[first+i];return 0;
}
static i32 WIN np_set(struct NpPalette* p,u32 flags,u32 first,u32 count,const u32* in){
 if(flags || !in || first>=256 || count>256-first)return ND_INVALID;
 for(u32 i=0;i<count;++i)p->entries[first+i]=in[i];return 0;
}
static i32 WIN np_initialize(struct NpPalette* p,void* draw,u32 flags,const u32* entries){
 (void)p;(void)draw;(void)flags;(void)entries;return (i32)0x88760005u;
}
static i32 WIN nd_palette(struct NdObject* o,u32 flags,const u32* entries,void** out,void* outer){
 (void)o;if(!out)return ND_INVALID;*out=0;if(outer)return (i32)0x80040110u;
 /* Reserved-endpoint and 4-bit/primary palette policies remain unadmitted. */
 if(flags!=0x44 || !entries)return (i32)0x80004001u;
 if(np_objects>=256)return (i32)0x8007000eu;
 struct NpPalette* p=HeapAlloc(GetProcessHeap(),8,sizeof(*p));if(!p)return (i32)0x8007000eu;
 p->table=np_table;p->refs=1;p->flags=flags;
 for(u32 i=0;i<256;++i)p->entries[i]=entries[i];++np_objects;*out=p;return 0;
}
static void np_init(void){np_table[0]=(void*)&np_query;np_table[1]=(void*)&np_retain;np_table[2]=(void*)&np_release;
 np_table[3]=(void*)&np_caps;np_table[4]=(void*)&np_get;np_table[5]=(void*)&np_initialize;np_table[6]=(void*)&np_set;}
