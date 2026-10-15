/* Owned top-down canonical-format GDI lease used by original bitmap-loading callers.
 * The DIB is separate from surface storage; both directions copy rows explicitly.
 * No DirectDraw or native GPU history readback occurs. */
API void* WIN CreateCompatibleDC(void*);
API void* WIN CreateDIBSection(void*,const void*,u32,void**,HANDLE,u32);
API void* WIN SelectObject(void*,void*);
API int WIN DeleteObject(void*);
API int WIN DeleteDC(void*);
API int WIN GdiFlush(void);
API i32 WIN GetMapMode(void*);
API i32 WIN GetGraphicsMode(void*);
API i32 WIN GetViewportOrgEx(void*,i32*);
API i32 WIN GetWindowOrgEx(void*,i32*);
API void* WIN CreateRectRgn(i32,i32,i32,i32);
API i32 WIN GetClipRgn(void*,void*);
API void* WIN GetCurrentObject(void*,u32);
static u32 ns_dcs;
static struct NsState* ns_dc_owners[128];
static struct NsState* ns_dc_owner(void* dc){for(u32 i=0;i<128;++i)if(ns_dc_owners[i]&&ns_dc_owners[i]->dc==dc)return ns_dc_owners[i];return 0;}
/* Native painting admits the default memory-DC mapping/clip. A custom GDI
 * transform or region is forwarded before native writes. */
static int ns_dc_plain(void* dc){
 struct NsState* s=ns_dc_owner(dc);if(!s || GetCurrentObject(dc,7)!=s->bitmap)return 0;
 i32 viewport[2],window[2];if(GetMapMode(dc)!=1||GetGraphicsMode(dc)!=1||!GetViewportOrgEx(dc,viewport)||!GetWindowOrgEx(dc,window)||viewport[0]||viewport[1]||window[0]||window[1])return 0;
 void* region=CreateRectRgn(0,0,0,0);if(!region)return 0;const i32 result=GetClipRgn(dc,region);DeleteObject(region);return result==0;
}
static void ns_dc_drop(struct NsState* s){
 /* The cached DC belongs to this logical object; its physical lease may have
  * moved to another object, including a retained attached back surface. */
 for(struct NsState* p=ns_owned;p;p=p->next_owned)if(p->storage_dc==s){p->storage_dc=0;if(p->locked>0)--p->locked;}
 if(s->dc){for(u32 i=0;i<128;++i)if(ns_dc_owners[i]==s)ns_dc_owners[i]=0;
  SelectObject(s->dc,s->old_bitmap);DeleteObject(s->bitmap);DeleteDC(s->dc);--ns_dcs;}
 s->dc=s->bitmap=s->old_bitmap=s->dc_bits=0;nd_status();
}
static i32 WIN ns_get_dc(struct NsObject* o,void** out){
 if(!out)return ND_INVALID;struct NsState* s=o->state;
 if(s->dc)return (i32)0x8876026cu;
 if(s->storage_dc || s->locked<0)return NS_BUSY;
 ns_cpu_require(s);
 if(ns_dcs>=128)return (i32)0x88760249u;
 void* dc=CreateCompatibleDC(0);if(!dc)return (i32)0x88760249u;
 u32 info[266];ns_zero(info,266);info[0]=40;info[3]=1|(s->bits<<16);info[4]=s->bits==16?3:0;
 if(s->bits==16)for(u32 i=0;i<3;++i)info[10+i]=s->masks[i];
 if(s->bits==8){info[8]=256;for(u32 i=0;i<256;++i){u32 color=s->palette?s->palette->entries[i]:0;
  info[10+i]=((color&255)<<16)|(color&0xff00)|((color>>16)&255);}}
 info[1]=s->width;info[2]=0u-s->height;void* bits=0;
 void* bitmap=CreateDIBSection(dc,info,0,&bits,0,0);
 if(!bitmap || !bits){if(bitmap)DeleteObject(bitmap);DeleteDC(dc);return (i32)0x88760249u;}
 void* old=SelectObject(dc,bitmap);
 if(!old || old==(void*)-1){DeleteObject(bitmap);DeleteDC(dc);return (i32)0x88760249u;}
 u32 row_bytes=s->width*(s->bits/8),pitch=(row_bytes+3)&~3u;
 for(u32 y=0;y<s->height;++y){u8* target=(u8*)bits+y*pitch;const u8* source=(const u8*)s->pixels+y*s->pitch;
  for(u32 x=0;x<row_bytes;++x)target[x]=source[x];}
 s->dc=dc;s->bitmap=bitmap;s->old_bitmap=old;s->dc_bits=bits;
 s->storage_dc=s;++s->locked;
 for(u32 i=0;i<128;++i)if(!ns_dc_owners[i]){ns_dc_owners[i]=s;break;}
 ++ns_dcs;*out=dc;nd_status();return 0;
}
static i32 WIN ns_release_dc(struct NsObject* o,void* dc){
 struct NsState* s=o->state;if(!s->dc)return (i32)0x8876024au;
 if(s->dc!=dc || s->storage_dc!=s)return (i32)0x8876086cu;
 if(GetCurrentObject(dc,7)!=s->bitmap)return ND_INVALID;
 if(!GdiFlush())return (i32)0x80004005u;
 u32 row_bytes=s->width*(s->bits/8),pitch=(row_bytes+3)&~3u;
 for(u32 y=0;y<s->height;++y){u8* target=(u8*)s->pixels+y*s->pitch;const u8* source=(const u8*)s->dc_bits+y*pitch;
  for(u32 x=0;x<row_bytes;++x)target[x]=source[x];}
 if(s->locked<=0){s->locked=-1;s->storage_dc=0;} /* Explicit unsupported debt. */
 ns_dc_drop(s);return 0;
}
