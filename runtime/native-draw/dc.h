/* Owned top-down RGB565 GDI lease used by original bitmap-loading callers.
 * The DIB is separate from surface storage; both directions copy rows explicitly.
 * No DirectDraw or native GPU history readback occurs. */
API void* WIN CreateCompatibleDC(void*);
API void* WIN CreateDIBSection(void*,const void*,u32,void**,HANDLE,u32);
API void* WIN SelectObject(void*,void*);
API int WIN DeleteObject(void*);
API int WIN DeleteDC(void*);
API int WIN GdiFlush(void);
static u32 ns_dcs;
static void ns_dc_drop(struct NsState* s){
 if(s->dc){SelectObject(s->dc,s->old_bitmap);DeleteObject(s->bitmap);DeleteDC(s->dc);--ns_dcs;}
 s->dc=s->bitmap=s->old_bitmap=s->dc_bits=0;
}
static i32 WIN ns_get_dc(struct NsObject* o,void** out){
 if(!out)return ND_INVALID;struct NsState* s=o->state;
 if(s->dc)return (i32)0x8876026cu;
 if(ns_dcs>=128)return (i32)0x88760249u;
 void* dc=CreateCompatibleDC(0);if(!dc)return (i32)0x88760249u;
 u32 info[13]={40,0,0,0x00100001,3,0,0,0,0,0,0xf800,0x7e0,0x1f};
 info[1]=s->width;info[2]=0u-s->height;void* bits=0;
 void* bitmap=CreateDIBSection(dc,info,0,&bits,0,0);
 if(!bitmap || !bits){if(bitmap)DeleteObject(bitmap);DeleteDC(dc);return (i32)0x88760249u;}
 void* old=SelectObject(dc,bitmap);
 if(!old || old==(void*)-1){DeleteObject(bitmap);DeleteDC(dc);return (i32)0x88760249u;}
 u32 pitch=(s->width*2+3)&~3u;
 for(u32 y=0;y<s->height;++y){u16* target=(u16*)((u8*)bits+y*pitch);const u16* source=(const u16*)((const u8*)s->pixels+y*s->pitch);
  for(u32 x=0;x<s->width;++x)target[x]=source[x];}
 s->dc=dc;s->bitmap=bitmap;s->old_bitmap=old;s->dc_bits=bits;++ns_dcs;*out=dc;return 0;
}
static i32 WIN ns_release_dc(struct NsObject* o,void* dc){
 struct NsState* s=o->state;if(!s->dc || s->dc!=dc)return (i32)0x8876024au;
 if(!GdiFlush())return (i32)0x80004005u;
 u32 pitch=(s->width*2+3)&~3u;
 for(u32 y=0;y<s->height;++y){u16* target=(u16*)((u8*)s->pixels+y*s->pitch);const u16* source=(const u16*)((const u8*)s->dc_bits+y*pitch);
  for(u32 x=0;x<s->width;++x)target[x]=source[x];}
 ns_dc_drop(s);return 0;
}
