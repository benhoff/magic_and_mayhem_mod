/* PE32 compatibility storage. No presentation and no DirectDraw dependency.
 * Unregistered calls terminate. Admitted operations never read native GPU World
 * history: the resident World consumer remains its exclusive owner. */
API int WIN VirtualFree(void*,u32,u32);
#define NS_BUSY ((i32)0x887601aeu)
#define NS_RECT ((i32)0x88760096u)
#define NS_UNSUPPORTED ((i32)0x80004001u)
struct NsState;
struct NsObject {void** table;struct NsState* state;u32 kind;};
struct NsState {
  struct NsObject aliases[5];u32 refs[5],interfaces;struct NdObject* device;
  u32 width,height,pitch,caps,bits,masks[3];u16* pixels;u32 locked;void* lock_pointer;
  u32 key_set,key_low,key_high;struct NsState* back;
  void *dc,*bitmap,*old_bitmap,*dc_bits;struct NcClipper* clipper;
  struct NsState *previous_owned,*next_owned;struct NpPalette* palette;
};
static struct NsState* ns_owned;
static void* ns_tables[5][45];
static u32 ns_bytes,ns_creates,ns_locks,ns_copies,ns_flips;
static void ns_dc_drop(struct NsState*);
static const u8 ns_ids[4][16]={
 {0x81,0xdb,0x14,0x6c,0x33,0xa7,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60},
 {0x85,0x58,0x80,0x57,0xec,0x6e,0xcf,0x11,0x94,0x41,0xa8,0x23,0x03,0xc1,0x0e,0x27},
 {0x30,0x86,0x2b,0x0b,0x35,0xad,0xd0,0x11,0x8e,0xa6,0,0x60,0x97,0x97,0xea,0x5b},
 {0x00,0x4e,0x04,0xda,0xb2,0x69,0xd0,0x11,0xa1,0xd5,0,0xaa,0,0xb8,0xdf,0xbb}};
static void ns_zero(u32* p,u32 count){for(u32 i=0;i<count;++i)p[i]=0;}
static void ns_format(u32* p){ns_zero(p,8);p[0]=32;p[1]=0x40;p[3]=16;p[4]=0xf800;p[5]=0x7e0;p[6]=0x1f;}
static i32 WIN ns_query(struct NsObject* o,const u8* iid,void** out){
 if(!iid || !out)return ND_INVALID;*out=0;
 u32 kind=1;if(!nd_equal(iid,nd_unknown)){
  for(kind=1;kind<5 && !nd_equal(iid,ns_ids[kind-1]);++kind){}
  if(kind==5)return ND_NOINTERFACE;
 }
 *out=&o->state->aliases[kind];if(!o->state->refs[kind]++)++o->state->interfaces;return 0;
}
static u32 WIN ns_retain(struct NsObject* o){if(!o->state->refs[o->kind]++)++o->state->interfaces;return o->state->refs[o->kind];}
static u32 WIN ns_release(struct NsObject* o){
 struct NsState* s=o->state;u32 error=GetLastError(),refs=--s->refs[o->kind];
 if(!refs && !--s->interfaces){if(s->device->state->primary==s)s->device->state->primary=0;ns_dc_drop(s);if(s->palette)np_release(s->palette);if(s->clipper)nc_release(s->clipper);ns_bytes-=s->pitch*s->height;if(s->back)ns_release(&s->back->aliases[s->device->kind==3?3:1]);VirtualFree(s->pixels,0,0x8000);
  if(s->previous_owned)s->previous_owned->next_owned=s->next_owned;else ns_owned=s->next_owned;
  if(s->next_owned)s->next_owned->previous_owned=s->previous_owned;
  nd_release(s->device);HeapFree(GetProcessHeap(),0,s);--nd_surfaces;}
 nd_status();SetLastError(error);return refs;
}
static void ns_surface_format(struct NsState* s,u32* p){
 ns_zero(p,8);p[0]=32;p[1]=s->bits==8?0x60:0x40;p[3]=s->bits;
 for(u32 i=0;i<3;++i)p[4+i]=s->masks[i];
}
static u32 ns_read(const struct NsState* s,u32 x,u32 y){
 const u8* p=(const u8*)s->pixels+y*s->pitch+x*(s->bits/8);
 if(s->bits==16)return *(const u16*)p;if(s->bits==8)return *p;
 if(s->bits==32)return *(const u32*)p;return (u32)p[0]|(u32)p[1]<<8|(u32)p[2]<<16;
}
static void ns_write(struct NsState* s,u32 x,u32 y,u32 value){
 u8* p=(u8*)s->pixels+y*s->pitch+x*(s->bits/8);
 if(s->bits==16)*(u16*)p=(u16)value;else if(s->bits==8)*p=(u8)value;
 else if(s->bits==32)*(u32*)p=value;else {p[0]=(u8)value;p[1]=(u8)(value>>8);p[2]=(u8)(value>>16);}
}
static i32 WIN ns_desc(struct NsObject* o,u32* d){
 u32 size=o->kind==3?124:108;if(!d || d[0]!=size)return ND_INVALID;
 struct NsState* s=o->state;ns_zero(d,size/4);d[0]=size;d[1]=0x100f;
 d[2]=s->height;d[3]=s->width;d[4]=s->pitch;ns_surface_format(s,d+18);d[26]=s->caps;
 if(s->key_set){d[1]|=0x10000;d[16]=s->key_low;d[17]=s->key_high;}return 0;
}
static int ns_rectangle(struct NsState* s,const i32* input,i32* r){
 if(input){for(u32 i=0;i<4;++i)r[i]=input[i];}
 else {r[0]=r[1]=0;r[2]=(i32)s->width;r[3]=(i32)s->height;}
 return r[0]>=0 && r[1]>=0 && r[2]>r[0] && r[3]>r[1] && (u32)r[2]<=s->width && (u32)r[3]<=s->height;
}
static i32 WIN ns_lock(struct NsObject* o,const i32* rect,u32* d,u32 flags,HANDLE event){
 struct NsState* s=o->state;i32 r[4];
 if(!d || d[0]!=(o->kind==3?124u:108u) || event || (flags&~0x7831u))return ND_INVALID;
 if(s->locked || s->dc)return NS_BUSY;if(!ns_rectangle(s,rect,r))return NS_RECT;
 i32 result=ns_desc(o,d);if(result)return result;
 d[1]|=0x800;d[9]=(u32)s->pixels+(u32)r[1]*s->pitch+(u32)r[0]*(s->bits/8);
 s->locked=1;s->lock_pointer=(void*)d[9];if(!(++ns_locks&255))nd_status();return 0;
}
static i32 WIN ns_unlock(struct NsObject* o,void* argument){
 struct NsState* s=o->state;if(!s->locked)return (i32)0x88760248u;
 /* Surface4 accepts a rectangle, older interfaces accept the returned pointer. */
 if(o->kind!=3 && argument && argument!=s->lock_pointer)return ND_INVALID;
 s->locked=0;s->lock_pointer=0;return 0;
}
static i32 WIN ns_key(struct NsObject* o,u32 flags,const u32* key){
 if(flags!=8)return NS_UNSUPPORTED;
 struct NsState* s=o->state;if(key){if(key[0]>key[1] || (s->bits<32 && key[1]>((1u<<s->bits)-1)))return ND_INVALID;
  s->key_low=key[0];s->key_high=key[1];s->key_set=1;}else s->key_set=0;return 0;
}
static i32 WIN ns_get_key(struct NsObject* o,u32 flags,u32* key){
 if(!key || flags!=8)return ND_INVALID;if(!o->state->key_set)return (i32)0x887600d7u;
 key[0]=o->state->key_low;key[1]=o->state->key_high;return 0;
}
static i32 ns_draw(struct NsObject* dst,const i32* dr,struct NsObject* src,const i32* sr,u32 flags,const u32* fx){
 struct NsState *d=dst->state,*s=src?src->state:0;i32 rd[4],rs[4];
 for(u32 i=0;i<4;++i){rd[i]=dr?dr[i]:(i<2?0:(i==2?(i32)d->width:(i32)d->height));
  rs[i]=sr?sr[i]:(i<2?0:(s?(i==2?(i32)s->width:(i32)s->height):0));}
 long long width=(long long)rd[2]-rd[0],height=(long long)rd[3]-rd[1];
 if(width<=0 || height<=0 || (s && (rs[2]<=rs[0] || rs[3]<=rs[1])))return NS_RECT;
 if(!d->clipper && (!ns_rectangle(d,rd,rd) || (s && !ns_rectangle(s,rs,rs))))return NS_RECT;
 if(s && ((long long)rs[2]-rs[0]!=width || (long long)rs[3]-rs[1]!=height))return NS_UNSUPPORTED;
 u32* clips=0;i32 result=0;
 if(d->clipper){result=nc_snapshot(d->clipper,&clips);if(result)return result;}
 if(d->locked || d->dc || (s && (s->locked || s->dc))){result=NS_BUSY;goto done;}
 if(flags&~(0x1000000u|0x8000u|0x400u)){result=NS_UNSUPPORTED;goto done;}
 if(flags&0x400){if(s || !fx || fx[0]!=100 || (flags&0x8000)){result=ND_INVALID;goto done;}}
 else if(!s || fx){result=ND_INVALID;goto done;}
 if(s && (s->bits!=d->bits || s->masks[0]!=d->masks[0] || s->masks[1]!=d->masks[1] || s->masks[2]!=d->masks[2])){result=NS_UNSUPPORTED;goto done;}
 if((flags&0x8000) && !s->key_set){result=ND_INVALID;goto done;}
 /* Freeze the whole aliased backing, including samples outside a clipped first
  * piece. Earlier regions may remain written when a later source piece fails. */
 u8* frozen=0;struct NsState snapshot;
 if(d==s){frozen=HeapAlloc(GetProcessHeap(),0,s->pitch*s->height);if(!frozen){result=(i32)0x8007000eu;goto done;}
  for(u32 i=0;i<s->pitch*s->height;++i)frozen[i]=((u8*)s->pixels)[i];snapshot.bits=s->bits;snapshot.pitch=s->pitch;snapshot.width=s->width;snapshot.height=s->height;snapshot.key_low=s->key_low;snapshot.key_high=s->key_high;snapshot.pixels=(u16*)frozen;s=&snapshot;}
 for(u32 region=0;region<(clips?clips[2]:1u);++region){
  const i32* clip=clips?(const i32*)(clips+8+4*region):0;
  i32 left=rd[0],top=rd[1],right=rd[2],bottom=rd[3];
  if(left<0)left=0;if(top<0)top=0;if(right>(i32)d->width)right=d->width;if(bottom>(i32)d->height)bottom=d->height;
  if(clip){if(left<clip[0])left=clip[0];if(top<clip[1])top=clip[1];if(right>clip[2])right=clip[2];if(bottom>clip[3])bottom=clip[3];}
  if(left>=right || top>=bottom)continue;
  long long sx=(long long)rs[0]+left-rd[0],sy=(long long)rs[1]+top-rd[1];
  if(s && (sx<0 || sy<0 || sx+right-left>s->width || sy+bottom-top>s->height)){result=NS_RECT;break;}
  for(i32 y=top;y<bottom;++y)for(i32 x=left;x<right;++x){
   u32 value=s?ns_read(s,(u32)(sx+x-left),(u32)(sy+y-top)):fx[20];
   if(!(flags&0x8000) || value<s->key_low || value>s->key_high)ns_write(d,x,y,value);
  }
 }
 if(frozen)HeapFree(GetProcessHeap(),0,frozen);
 if(!(++ns_copies&255))nd_status();
done:if(clips)HeapFree(GetProcessHeap(),0,clips);return result;
}
static i32 WIN ns_blt(struct NsObject* dst,const i32* dr,struct NsObject* src,const i32* sr,u32 flags,const u32* fx){
 return ns_draw(dst,dr,src,sr,flags,fx);
}
static i32 WIN ns_fast(struct NsObject* dst,u32 x,u32 y,struct NsObject* src,const i32* rect,u32 flags){
 if(!src)return ND_INVALID;i32 r[4];
 for(u32 i=0;i<4;++i)r[i]=rect?rect[i]:(i<2?0:(i==2?(i32)src->state->width:(i32)src->state->height));
 long long w=(long long)r[2]-r[0],h=(long long)r[3]-r[1];
 if(w<0 || h<0 || x>0x7fffffffu || y>0x7fffffffu || x+w>dst->state->width || y+h>dst->state->height)return NS_RECT;
 if(dst->state->clipper)return (i32)0x8876023eu;
 if(!ns_rectangle(src->state,r,r))return NS_RECT;
 /* Wine's selected missing-key and unsupported-flag Fast calls copy opaque. */
 if(flags&~0x80000011u)return NS_UNSUPPORTED;
 i32 d[4]={(i32)x,(i32)y,(i32)(x+w),(i32)(y+h)};
 return ns_draw(dst,d,src,r,(flags&1) && src->state->key_set?0x8000:0,0);
}
static i32 WIN ns_caps(struct NsObject* o,u32* caps){if(!caps)return ND_INVALID;caps[0]=o->state->caps;if(o->kind==3)ns_zero(caps+1,3);return 0;}
static i32 WIN ns_attached(struct NsObject* o,const u32* caps,void** out){
 if(!caps || !out)return ND_INVALID;*out=0;if(!o->state->back || caps[0]!=4 || (o->kind==3 && (caps[1] || caps[2] || caps[3])))return (i32)0x887600ffu;
 struct NsObject* selected=&o->state->back->aliases[o->kind==3?3:1];ns_retain(selected);*out=selected;return 0;
}
static i32 WIN ns_flip(struct NsObject* o,struct NsObject* target,u32 flags){
 struct NsState* front=o->state;struct NsState* back=front->back;
 if(!back)return (i32)0x88760246u;
 if(flags&~0x1000001u)return NS_UNSUPPORTED;
 if(target && target->state!=back)return ND_INVALID;
 if(front->locked || front->dc || back->locked || back->dc)return NS_BUSY;
 u16* temporary=front->pixels;front->pixels=back->pixels;back->pixels=temporary;++ns_flips;if(!(ns_flips&255))nd_status();return 0;
}
static i32 WIN ns_get_clipper(struct NsObject* o,void** out){
 if(!out)return ND_INVALID;*out=0;if(!o->state->clipper)return (i32)0x88760238u;
 nc_retain(o->state->clipper);*out=o->state->clipper;return 0;
}
static i32 WIN ns_set_clipper(struct NsObject* o,struct NcClipper* c){
 if(c && c->table!=nc_table)return ND_INVALID;if(c)nc_retain(c);
 if(o->state->clipper)nc_release(o->state->clipper);o->state->clipper=c;return 0;
}
static i32 WIN ns_pixel_format(struct NsObject* o,u32* p){(void)o;if(!p || p[0]!=32)return ND_INVALID;ns_surface_format(o->state,p);return 0;}
#include "dc.h"
static i32 WIN ns_get_palette(struct NsObject* o,void** out){
 if(!out)return ND_INVALID;*out=0;if(!o->state->palette)return (i32)0x8876023cu;
 np_retain(o->state->palette);*out=o->state->palette;return 0;
}
static i32 WIN ns_set_palette(struct NsObject* o,struct NpPalette* p){
 if(o->state->bits!=8)return (i32)0x8876024du;
 if(p && p->table!=np_table)return ND_INVALID;if(p)np_retain(p);
 if(o->state->palette)np_release(o->state->palette);o->state->palette=p;return 0;
}
static i32 WIN ns_lost(struct NsObject* o){(void)o;return 0;}
static i32 WIN ns_restore(struct NsObject* o){return (o->state->locked || o->state->dc)?NS_BUSY:0;}
static i32 WIN ns_status(struct NsObject* o,u32 flags){if(flags!=1 && flags!=2)return ND_INVALID;return (o->state->locked || o->state->dc)?NS_BUSY:0;}
static struct NsState* ns_allocate(struct NdObject* device,u32 width,u32 height,u32 caps,u32 bits,const u32* masks){
 if(!width || !height || width>4096 || height>4096 || nd_surfaces>=512 || ((width*(bits/8)+7)&~7u)*height>256*1024*1024-ns_bytes)return 0;
 struct NsState* s=HeapAlloc(GetProcessHeap(),8,sizeof(*s));if(!s)return 0;
 /* Original distortion 005965f7 reads a DWORD but uses only its low WORD.
  * Keep that read valid at the last logical pixel without changing pitch or
  * exposing another row. This does not admit a source pixel past the surface. */
 s->bits=bits;for(u32 i=0;i<3;++i)s->masks[i]=masks[i];s->pitch=(width*(bits/8)+7)&~7u;s->pixels=VirtualAlloc(0,s->pitch*height+sizeof(u16),0x3000,4);
 if(!s->pixels){HeapFree(GetProcessHeap(),0,s);return 0;}
 for(u32 i=0;i<5;++i)s->aliases[i]=(struct NsObject){ns_tables[i],s,i};
 s->interfaces=1;s->refs[device->kind==3?3:1]=1;s->device=device;nd_retain(device);s->width=width;s->height=height;s->caps=caps;
 s->next_owned=ns_owned;if(ns_owned)ns_owned->previous_owned=s;ns_owned=s;
 ++nd_surfaces;++ns_creates;ns_bytes+=s->pitch*height;nd_status();return s;
}
/* First failed descriptor per owned launch, preserving the HRESULT policy.
 * This diagnostic is local bytes, not a pointer-bearing shared wire contract. */
static i32 ns_surface_refusal(struct NdObject* o,const u32* d,u32 reason,void* caller){
 u32 error=GetLastError(),record[39]={0x534d4e4d,0x31465255,1,156,o->kind,reason,(u32)caller,d[0]},done=0;
 for(u32 i=0;i<(d[0]==124?31u:27u);++i)record[8+i]=d[i];
 HANDLE file=CreateFileA("native-draw-surface-refusal.bin",0x40000000,0,0,1,0x80,0);
 if(file!=(HANDLE)-1){WriteFile(file,record,sizeof(record),&done,0);CloseHandle(file);}
 SetLastError(error);return NS_UNSUPPORTED;
}
static i32 WIN nd_surface(struct NdObject* o,const u32* d,void** out,void* outer){
 if(!out)return ND_INVALID;*out=0;if(outer)return (i32)0x80040110u;
 if(!d || d[0]!=(o->kind==3?124u:108u))return ND_INVALID;
 u32 flags=d[1],caps=d[26],w=d[3],h=d[2],bits=16,masks[3]={0xf800,0x7e0,0x1f};
 if(caps==0x200 || caps==0x218){
  if(o->state->primary)return (i32)0x88760234u;
  if(!o->state->coop)return (i32)0x88760212u;
  if((caps==0x200 && flags!=1) || (caps==0x218 && (flags!=0x21 || d[5]!=1)))return NS_UNSUPPORTED;
  w=o->state->width;h=o->state->height;
 }else {
  if((flags!=7 && flags!=0x1007) || (caps!=0x840 && caps!=0x40 && caps!=0x800))return ns_surface_refusal(o,d,1,__builtin_return_address(0));
  if(flags==0x1007){bits=d[21];for(u32 i=0;i<3;++i)masks[i]=d[22+i];
   int valid=bits==8?d[19]==0x60 && !masks[0] && !masks[1] && !masks[2]:
    bits==16?d[19]==0x40 && masks[2]==31 && ((masks[0]==0xf800 && masks[1]==0x7e0)||(masks[0]==0x7c00 && masks[1]==0x3e0)):
    (bits==24 || bits==32) && d[19]==0x40 && masks[0]==0xff0000 && masks[1]==0xff00 && masks[2]==0xff;
   if(d[18]!=32 || !valid || d[25])return ns_surface_refusal(o,d,2,__builtin_return_address(0));}
 }
 struct NsState* s=ns_allocate(o,w,h,caps==0x40?0x10004040:(caps==0x800?0x840:caps),bits,masks);
 if(!s)return (i32)0x8007000eu;
 if(caps==0x218){s->back=ns_allocate(o,w,h,0x14,bits,masks);if(!s->back){ns_release(&s->aliases[o->kind==3?3:1]);return (i32)0x8007000eu;}}
 if(caps==0x200 || caps==0x218)o->state->primary=s;
 *out=&s->aliases[o->kind==3?3:1];return 0;
}

typedef i32 (WIN *NdModeCallback)(u32*,void*);
static i32 WIN nd_enum_modes(struct NdObject* o,u32 flags,const u32* filter,void* context,NdModeCallback callback){
 (void)o;if(!callback || (flags!=0 && flags!=1) || (filter && ((filter[0]!=108 && filter[0]!=124) || filter[1])))return NS_UNSUPPORTED;
 u32 size=filter?filter[0]:108;
 for(u32 i=0;i<2;++i){u32 d[31];ns_zero(d,31);d[0]=size;d[1]=0x4100e;d[2]=i?600:480;d[3]=i?800:640;d[4]=d[3]*2;d[6]=60;ns_format(d+18);
  if(!callback(d,context))break;}return 0;
}
static i32 WIN nd_memory(struct NdObject* o,const u32* caps,u32* total,u32* available){
 if(!caps || (!total && !available) || caps[0]!=0x4000 || (o->kind==3 && (caps[1] || caps[2] || caps[3])))return NS_UNSUPPORTED;
 if(total)*total=256*1024*1024;if(available)*available=256*1024*1024-ns_bytes;return 0;
}
static i32 WIN nd_cooperate(struct NdObject* o,void* window,u32 flags){
 if((window && !IsWindow(window)) || (!window && flags!=8) || (flags!=8 && flags!=0x11))return ND_INVALID;o->state->window=window;o->state->coop=flags;return 0;
}
static i32 WIN nd_mode(struct NdObject* o,u32 w,u32 h,u32 depth,u32 refresh,u32 flags){
 if(o->state->primary)return NS_BUSY;
 if(o->state->coop!=0x11)return (i32)0x887600e1u;
 if(!w || !h || w>4096 || h>4096 || depth!=16 || refresh || flags)return NS_UNSUPPORTED;
 o->state->width=w;o->state->height=h;o->state->depth=depth;return 0;
}
static i32 WIN nd_mode1(struct NdObject* o,u32 w,u32 h,u32 depth){return nd_mode(o,w,h,depth,0,0);}
static i32 WIN nd_restore_mode(struct NdObject* o){o->state->width=800;o->state->height=600;o->state->depth=16;return 0;}
static void nd_init(void){
 nc_init();np_init();
 for(u32 k=0;k<4;++k){
  for(u32 slot=0;slot<28;++slot)nd_tables[k][slot]=nd_refusals_1[slot];
  nd_tables[k][0]=(void*)&nd_query;nd_tables[k][1]=(void*)&nd_retain;nd_tables[k][2]=(void*)&nd_release;
  nd_tables[k][8]=(void*)&nd_enum_modes;nd_tables[k][23]=(void*)&nd_memory;
  nd_tables[k][5]=(void*)&nd_palette;nd_tables[k][4]=(void*)&nd_clipper;nd_tables[k][6]=(void*)&nd_surface;nd_tables[k][19]=(void*)&nd_restore_mode;nd_tables[k][20]=(void*)&nd_cooperate;
  nd_tables[k][21]=k==1?(void*)&nd_mode1:(void*)&nd_mode;
 }
 for(u32 k=0;k<5;++k){
  for(u32 slot=0;slot<45;++slot)ns_tables[k][slot]=nd_refusals_2[slot];
  ns_tables[k][0]=(void*)&ns_query;ns_tables[k][1]=(void*)&ns_retain;ns_tables[k][2]=(void*)&ns_release;
  ns_tables[k][11]=(void*)&ns_flip;ns_tables[k][12]=(void*)&ns_attached;
  ns_tables[k][5]=(void*)&ns_blt;ns_tables[k][7]=(void*)&ns_fast;ns_tables[k][13]=(void*)&ns_status;
  ns_tables[k][14]=(void*)&ns_caps;ns_tables[k][16]=(void*)&ns_get_key;ns_tables[k][18]=(void*)&ns_status;
  ns_tables[k][15]=(void*)&ns_get_clipper;ns_tables[k][28]=(void*)&ns_set_clipper;
  ns_tables[k][20]=(void*)&ns_get_palette;ns_tables[k][31]=(void*)&ns_set_palette;
  ns_tables[k][21]=(void*)&ns_pixel_format;ns_tables[k][22]=(void*)&ns_desc;ns_tables[k][24]=(void*)&ns_lost;
  ns_tables[k][17]=(void*)&ns_get_dc;ns_tables[k][26]=(void*)&ns_release_dc;
  ns_tables[k][25]=(void*)&ns_lock;ns_tables[k][27]=(void*)&ns_restore;ns_tables[k][29]=(void*)&ns_key;ns_tables[k][32]=(void*)&ns_unlock;
 }
}
