#include "../../protocols/include/mnm/frame_v1.h"
/* Owned native checkpoints; only application calls establish pixels and properties.
 * All access is under game_locks_busy. No observer COM calls or references. */
struct GameSurface {void* object;u32 epoch,metadata_epoch,generation,generation_origin,generation_caller,generation_owner,clip_known,clip,key_known,key,primary,layout_known,caps,back_count,back_count_known;void *back,*palette,*dc,*dc_bitmap;u32 dc_owner,dc_generation;struct Snapshot pixels;};
#define GAME_SURFACE_COUNT 128u
static struct GameSurface game_surfaces[GAME_SURFACE_COUNT];
static u32 game_surface_bytes,game_surface_generation,game_blit_count,game_blit_bytes;
#define GAME_SURFACE_LIMIT (64u*1024u*1024u)
/* Publish owned pixels only. The guard/sequence protocol is shared with the
 * legacy producer; readers never consume a partially written RGBA frame. */
static int game_surface_publish(struct GameSurface*);
static int game_surface_publish_region(struct GameSurface*,u32,u32,u32,u32,u32);
static int game_surface_colors(struct GameSurface*,u8*);
static void game_pixel_misses_sync(void);
/* Identity/generation/count of the complete frame currently in the stream.
 * A partial upload is permitted only when it advances this exact checkpoint. */
static void* game_presented_object;
static u32 game_presented_generation,game_presented_frame;
static int game_publish_pixels(const struct Snapshot* s,struct GameSurface* surface){
    if(!stream || !s->data || !__sync_bool_compare_and_swap(&capture_busy,0,1))return 0;
    game_presented_object=0;
    u32 sequence=__atomic_load_n(stream+MNM_FRAME_V1_SEQUENCE_OFFSET/4,__ATOMIC_RELAXED);
    __atomic_store_n(stream+MNM_FRAME_V1_SEQUENCE_OFFSET/4,sequence+1,__ATOMIC_SEQ_CST);
    int ok=render_pixels((u8*)stream+MNM_FRAME_V1_PIXELS_OFFSET,s->width,s->height,s->data,(i32)(s->width*(s->bits/8)),s->bits,s->r,s->g,s->b,s->bits==8?s->palette:0);
    if(ok){stream[MNM_FRAME_V1_WIDTH_OFFSET/4]=s->width;stream[MNM_FRAME_V1_HEIGHT_OFFSET/4]=s->height;stream[MNM_FRAME_V1_STRIDE_OFFSET/4]=s->width*MNM_FRAME_V1_BYTES_PER_PIXEL;stream[MNM_FRAME_V1_PIXEL_FORMAT_OFFSET/4]=MNM_FRAME_V1_PIXEL_FORMAT_RGBA8888;++stream[MNM_FRAME_V1_FRAME_COUNT_OFFSET/4];stream[MNM_FRAME_V1_STATUS_OFFSET/4]=MNM_FRAME_V1_STATUS_FRAME_PUBLISHED;
        if(surface){game_presented_object=surface->object;game_presented_generation=surface->generation;game_presented_frame=stream[MNM_FRAME_V1_FRAME_COUNT_OFFSET/4];}}
    __atomic_store_n(stream+MNM_FRAME_V1_SEQUENCE_OFFSET/4,sequence+2,__ATOMIC_RELEASE);__sync_lock_release(&capture_busy);return ok;
}
static void game_surface_drop(struct GameSurface* s){
    if(s->pixels.data){game_surface_bytes-=s->pixels.length;free_snapshot(&s->pixels);}
    s->dc=0;s->dc_bitmap=0;s->dc_owner=0;s->dc_generation=0;s->generation=++game_surface_generation;s->generation_origin=1;
}
static void game_surface_sync(void){
    game_alias_sync();game_session_sync();
    for(u32 i=0;i<32;++i)if(game_locks[i].object && game_locks[i].epoch!=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED))game_lock_clear(game_locks+i);
    for(u32 i=0;i<GAME_SURFACE_COUNT;++i){struct GameSurface* s=game_surfaces+i;if(!s->object)continue;
        if(s->metadata_epoch!=__atomic_load_n(&game_metadata_epoch,__ATOMIC_RELAXED)){
            lock_diagnostic("surface_metadata_epoch_reset",s->object,0,0,0,0,0,0);
            game_surface_drop(s);zero(s,sizeof(*s));
        }else if(s->epoch!=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED)){
            /* Keep validated shape, primary identity and properties. Only a
             * fresh complete checkpoint/full overwrite can restore pixels. */
            game_surface_drop(s);s->epoch=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED);
            lock_diagnostic("surface_pixels_epoch_reset",s->object,0,0,0,0,0,0);
        }
    }
    game_pixel_misses_sync();
}
static struct GameSurface* game_surface_find(void* object,int create){
    struct GameSurface* found=0;struct GameSurface* empty=0;
    for(u32 i=0;i<GAME_SURFACE_COUNT;++i){struct GameSurface* s=game_surfaces+i;
        if(!s->object){if(!empty)empty=s;continue;}
        if(game_alias_same(object,s->object)){if(found)return 0;found=s;}
    }
    if(!found && create && empty){found=empty;found->object=object;found->epoch=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED);found->metadata_epoch=__atomic_load_n(&game_metadata_epoch,__ATOMIC_RELAXED);found->generation=++game_surface_generation;found->generation_origin=2;}
    if(!found && create && !empty)lock_diagnostic("surface_capacity",object,0,0,0,0,0,0);
    return found;
}
static int game_surface_enter(void){
    if(!lock_capture_path_length)return 0;
    if(!game_tracker_acquire()){game_metadata_invalidate();return 0;}
    game_surface_sync();return 1;
}
static int game_surface_pixels_enter(void* object){
    if(!lock_capture_path_length)return 0;
    if(!game_tracker_acquire()){
        game_pixel_missed(object);return 0;
    }
    game_surface_sync();return 1;
}
static void game_surface_pixels_invalidate_locked(void* object){
    for(u32 i=0;i<GAME_SURFACE_COUNT;++i)if(game_surfaces[i].object && game_alias_same(object,game_surfaces[i].object))game_surface_drop(game_surfaces+i);
    for(u32 i=0;i<32;++i)if(game_alias_same(object,game_locks[i].object))game_lock_clear(game_locks+i);
}
static void game_surface_invalidate_locked(void* object){game_session_invalidate(object);game_surface_pixels_invalidate_locked(object);}
static void game_pixel_misses_sync(void){
    if(!__atomic_exchange_n(&game_pixel_misses_pending,0,__ATOMIC_ACQ_REL))return;
    for(u32 i=0;i<128;++i)if(__atomic_load_n(game_pixel_misses+i,__ATOMIC_ACQUIRE)){
        void* object=__atomic_exchange_n(game_pixel_misses+i,0,__ATOMIC_ACQ_REL);
        if(object){game_surface_invalidate_locked(object);lock_diagnostic("pixel_target_reset",object,0,0,0,0,0,0);}
    }
}
static void game_surface_alias(void* object){
    u32 count=0;
    for(u32 i=0;i<GAME_SURFACE_COUNT;++i)if(game_surfaces[i].object && game_alias_same(object,game_surfaces[i].object))++count;
    /* Independently observed interfaces may have conflicting pixel/properties.
     * Require a new checkpoint rather than choosing an arbitrary record. */
    if(count>1)game_session_invalidate(object);
    if(count>1)for(u32 i=0;i<32;++i)if(game_alias_same(object,game_locks[i].object))game_lock_clear(game_locks+i);
    if(count>1)for(u32 i=0;i<GAME_SURFACE_COUNT;++i)if(game_surfaces[i].object && game_alias_same(object,game_surfaces[i].object)){
        game_surface_drop(game_surfaces+i);zero(game_surfaces+i,sizeof(game_surfaces[i]));
    }
}
/* Both descriptor ABIs expose ddckCKSrcBlt at byte 0x40. Only the
 * DDSD_CKSRCBLT flag makes these values evidence; no observer query is needed. */
static void game_surface_descriptor_key_locked(void* object,const u32* d){
    if(!(d[1]&0x10000))return;
    struct GameSurface* s=game_surface_find(object,1);if(!s)return;
    u32 known=d[16]==d[17],key=known?d[16]:0;
    if(s->key_known!=known || s->key!=key){s->generation=++game_surface_generation;s->generation_origin=3;}
    s->key_known=known;s->key=key;
    lock_diagnostic(s->key_known?"source_key_descriptor":"source_key_range",object,0,d[16],d[17],0,0,0);
}
/* Descriptors establish shape/identity only; lpSurface is never read here. */
static void game_surface_describe_locked(void* object,u32 kind,const u32* d){
    struct GameSurface* s=game_surface_find(object,1);if(!s)return;
    u32 size=kind>=14?124:108;
    int valid=d && readable(d,size) && d[0]==size && (d[1]&0x1007)==0x1007 && d[18]==32 &&
        d[3] && d[3]<=2048 && d[2] && d[2]<=2048 &&
        ((d[19]==0x60 && d[21]==8 && !(d[22] || d[23] || d[24])) ||
         (d[19]==0x40 && (d[21]==16 || d[21]==24 || d[21]==32) && render_mask(d[22],d[21]) &&
          render_mask(d[23],d[21]) && render_mask(d[24],d[21]) &&
          !(d[22]&d[23]) && !(d[22]&d[24]) && !(d[23]&d[24]))) &&
        (kind<14 || !(d[27] || d[28] || d[29]));
    if(!valid){game_surface_invalidate_locked(object);s->layout_known=0;s->primary=0;
        lock_diagnostic("surface_metadata_rejected",object,kind,0,0,0,0,0);return;}
    /* GetSurfaceDesc is observation, not a native write. Driver methods may
     * reenter it while an admitted Blt holds owned input. Preserve generation
     * for identical known metadata; real shape/property changes still retire
     * uncertain pixels and refuse the pending operation. */
    u32 changed=!s->layout_known || s->pixels.width!=d[3] || s->pixels.height!=d[2] ||
        s->pixels.bits!=d[21] || s->pixels.flags!=d[19] || s->pixels.r!=d[22] || s->pixels.g!=d[23] || s->pixels.b!=d[24] ||
        s->caps!=d[26] || s->back_count_known!=((d[1]&0x20)!=0) ||
        s->back_count!=((d[1]&0x20)?d[5]:0) || s->primary!=((d[26]&0x200)!=0);
    struct Snapshot* pixels=&s->pixels;
    if(s->layout_known && (pixels->width!=d[3] || pixels->height!=d[2] || pixels->bits!=d[21] ||
       pixels->r!=d[22] || pixels->g!=d[23] || pixels->b!=d[24]))game_surface_invalidate_locked(object);
    for(u32 i=0;i<32;++i){struct GameLock* lock=game_locks+i;
        if(lock->active && game_alias_same(object,lock->object) &&
           (lock->desc[3]!=d[3] || lock->desc[2]!=d[2] || lock->desc[21]!=d[21] ||
            lock->desc[22]!=d[22] || lock->desc[23]!=d[23] || lock->desc[24]!=d[24])){
            game_surface_invalidate_locked(object);break;
        }
    }
    pixels->width=d[3];pixels->height=d[2];pixels->bits=d[21];pixels->flags=d[19];
    pixels->r=d[22];pixels->g=d[23];pixels->b=d[24];pixels->length=d[3]*d[2]*(d[21]/8);
    s->caps=d[26];s->back_count_known=(d[1]&0x20)!=0;s->back_count=s->back_count_known?d[5]:0;
    s->layout_known=1;s->primary=(d[26]&0x200)!=0;if(changed){s->generation=++game_surface_generation;s->generation_origin=4;}
    game_surface_descriptor_key_locked(object,d);
    lock_diagnostic("surface_metadata",object,kind,0,0,0,0,d);
}
static void game_surface_described(void* object,u32 kind,const u32* d,i32 result){
    if(result<0){lock_diagnostic("surface_metadata_failed",object,kind,0,0,result,0,0);return;}
    if(!game_surface_enter())return;
    game_surface_describe_locked(object,kind,d);game_tracker_release();
}
static void game_surface_created(void* object,u32 kind,const u32* d){
    if(!game_surface_enter())return;
    game_session_invalidate(object);
    for(u32 i=0;i<GAME_SURFACE_COUNT;++i)if(game_surfaces[i].object && game_alias_same(object,game_surfaces[i].object)){
        game_surface_drop(game_surfaces+i);zero(game_surfaces+i,sizeof(game_surfaces[i]));
    }
    for(u32 i=0;i<32;++i)if(game_alias_same(object,game_locks[i].object))game_lock_clear(game_locks+i);
    game_alias_retire(object);
    struct GameSurface* s=game_surface_find(object,1);if(s){s->clip_known=1;s->clip=0;}
    if(d)game_surface_describe_locked(object,kind,d);
    game_tracker_release();
}
static void game_surface_invalidate(void* object){
    if(!game_surface_pixels_enter(object))return;
    game_surface_invalidate_locked(object);game_tracker_release();
}
static void game_surface_clipper(void* object,void* clipper){
    if(!game_surface_enter())return;
    struct GameSurface* s=game_surface_find(object,1);
    if(s){s->clip_known=1;s->clip=clipper!=0;s->generation=++game_surface_generation;s->generation_origin=5;}
    game_tracker_release();
}
static void game_surface_key(void* object,u32 flags,int valid,const u32* key){
    if(!(flags&8) || !game_surface_enter())return;
    struct GameSurface* s=game_surface_find(object,1);
    if(s){s->key_known=valid && flags==8 && key && key[0]==key[1];s->key=s->key_known?key[0]:0;s->generation=++game_surface_generation;s->generation_origin=6;}
    game_tracker_release();
}
/* Transfer the pre-Unlock copy only after the original Unlock succeeded. */
static void game_surface_store(void* object,struct Snapshot* pixels,u32 primary,const u32* descriptor){
    struct GameSurface* s=game_surface_find(object,1);
    if(!s)return;
    game_surface_drop(s);
    if(!pixels->data || pixels->length>GAME_SURFACE_LIMIT-game_surface_bytes-__atomic_load_n(&lock_capture_reserved,__ATOMIC_RELAXED))return;
    s->caps=(descriptor[1]&1)?descriptor[26]:0;
    if(descriptor[0]>=124 && (descriptor[27] || descriptor[28] || descriptor[29]))s->caps=0;
    s->back_count_known=(descriptor[1]&0x20)!=0;s->back_count=s->back_count_known?descriptor[5]:0;
    s->generation_origin=7;s->primary=primary;s->layout_known=1;copy(&s->pixels,pixels,sizeof(*pixels));pixels->data=0;game_surface_bytes+=s->pixels.length;
}
struct GameBlit {struct Snapshot src,dst;void *source,*target;u32 source_generation,target_generation,epoch,caller,fields[10],valid,bootstrap,fill,fill_value,direct;};
static void game_blit_free(struct GameBlit* p){
    if(p->src.data)__atomic_sub_fetch(&lock_capture_reserved,p->src.length,__ATOMIC_RELAXED);
    if(p->dst.data)__atomic_sub_fetch(&lock_capture_reserved,p->dst.length,__ATOMIC_RELAXED);
    free_snapshot(&p->src);free_snapshot(&p->dst);
}
static int game_surface_clone(struct Snapshot* to,const struct Snapshot* from){
    if(from->length>GAME_SURFACE_LIMIT-game_surface_bytes-__atomic_load_n(&lock_capture_reserved,__ATOMIC_RELAXED))return 0;
    copy(to,from,sizeof(*from));to->data=HeapAlloc(GetProcessHeap(),0,from->length);if(!to->data)return 0;
    __atomic_add_fetch(&lock_capture_reserved,from->length,__ATOMIC_RELAXED);copy(to->data,from->data,from->length);return 1;
}
static int game_surface_blank(struct Snapshot* to,const struct Snapshot* shape){
    if(shape->length>GAME_SURFACE_LIMIT-game_surface_bytes-__atomic_load_n(&lock_capture_reserved,__ATOMIC_RELAXED))return 0;
    copy(to,shape,sizeof(*shape));to->data=HeapAlloc(GetProcessHeap(),8,shape->length);if(!to->data)return 0;
    __atomic_add_fetch(&lock_capture_reserved,to->length,__ATOMIC_RELAXED);return 1;
}
static int game_blit_live_only(u32 source_bytes,u32 target_bytes,u32 bits){
    return !game_session_enabled && (game_blit_count>=16 || source_bytes+target_bytes*2+(bits==8?1280u:256u)>GAME_SURFACE_LIMIT-game_blit_bytes);
}
/* DDBLT_COLORFILL uses the native dwFillColor at PE32 DDBLTFX byte 0x50.
 * A constant source is derived from arguments, never from driver readback. */
static const char* game_fill_before(struct GameSurface* dst,void* destination,void* rectangle,u32 flags,void* effects,struct GameBlit* p){
    if(!dst || !dst->layout_known)return "fill_untracked";
    if(rectangle || (flags&~0x01000400u) || !dst->clip_known || dst->clip ||
       !readable(effects,100) || ((u32*)effects)[0]!=100)return "fill_unsupported";
    struct Snapshot* b=&dst->pixels;u32 value=((u32*)effects)[20];
    if(b->bits<32 && value>>b->bits)return "fill_unsupported";
    struct Rect dr={0,0,(i32)b->width,(i32)b->height};
    if(destination){if(!readable(destination,16))return "fill_unsupported";copy(&dr,destination,16);}
    if(!inside(&dr,b->width,b->height))return "fill_unsupported";
    if(!b->data){
        if(dr.left || dr.top || (u32)dr.right!=b->width || (u32)dr.bottom!=b->height)return "fill_incomplete_initialization";
        p->bootstrap=1;
    }
    p->direct=game_blit_live_only(b->length,b->length,b->bits);p->fill_value=value;
    if(!p->direct){
        if(!game_surface_blank(&p->src,b) || !(p->bootstrap?game_surface_blank(&p->dst,b):game_surface_clone(&p->dst,b))){game_blit_free(p);return "fill_memory";}
        u32 bytes=b->bits/8;
        for(u32 at=0;at<p->src.length;at+=bytes)for(u32 j=0;j<bytes;++j)p->src.data[at+j]=(u8)(value>>(8*j));
    }
    p->fields[0]=1;p->fields[1]=2;p->fields[2]=(u32)dr.left;p->fields[3]=(u32)dr.top;
    p->fields[4]=(u32)dr.right;p->fields[5]=(u32)dr.bottom;p->fields[6]=(u32)dr.left;p->fields[7]=(u32)dr.top;
    p->target_generation=dst->generation;p->epoch=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED);p->valid=1;p->fill=1;
    return "fill_ready";
}
static void game_blit_before(void* target,void* destination,void* source,void* rectangle,u32 flags,void* effects,u32 fast,u32 x,u32 y,struct GameBlit* p){
    zero(p,sizeof(*p));p->target=target;p->source=source;
    if(!game_surface_pixels_enter(target))return;
    const char* reason="blit_untracked";
    u32 shape=0;struct Rect sr={0,0,0,0},dr={0,0,0,0};
    struct GameSurface *src=source?game_surface_find(source,0):0,*dst=game_surface_find(target,0);
    if(!fast && !source && (flags&0x400)){reason=game_fill_before(dst,destination,rectangle,flags,effects,p);goto done;}
    if(!src || !dst || !src->pixels.data || (!dst->pixels.data && !dst->layout_known))goto done;
    struct Snapshot *a=&src->pixels,*b=&dst->pixels;
    sr.right=(i32)a->width;sr.bottom=(i32)a->height;dr.right=(i32)b->width;dr.bottom=(i32)b->height;shape=1;
    reason="blit_reject_source_rect";
    if(rectangle){if(!readable(rectangle,16))goto done;copy(&sr,rectangle,16);}
    if(!inside(&sr,a->width,a->height))goto done;
    reason="blit_reject_target_rect";
    if(fast){if(x>b->width || y>b->height)goto done;dr.left=(i32)x;dr.top=(i32)y;dr.right=dr.left+sr.right-sr.left;dr.bottom=dr.top+sr.bottom-sr.top;}
    else if(destination){if(!readable(destination,16))goto done;copy(&dr,destination,16);}
    if(!inside(&dr,b->width,b->height))goto done;
    reason="blit_reject_self";if(src==dst)goto done;
    reason="blit_reject_effects";if(effects)goto done;
    reason="blit_reject_flags";if(fast?(flags&~0x11u):(flags&~0x01008000u))goto done;
    reason="blit_reject_clipper";if(!fast && (!dst->clip_known || dst->clip))goto done;
    reason="blit_reject_format";if(a->bits!=b->bits || a->r!=b->r || a->g!=b->g || a->b!=b->b || a->width>2048 || a->height>2048)goto done;
    reason="blit_reject_stretch";if(dr.right-dr.left!=sr.right-sr.left || dr.bottom-dr.top!=sr.bottom-sr.top)goto done;
    shape=0;
    u32 keyed=fast?(flags&1)!=0:(flags&0x8000)!=0;
    if(keyed && (!src->key_known || (a->bits==8 && src->key>255))){reason="blit_source_key_unobserved";goto done;}
    if(!b->data){
        /* Synthetic blank storage is permitted only when every pixel will be
         * overwritten. It is not evidence of the original destination pixels. */
        if(keyed || dr.left || dr.top || (u32)dr.right!=b->width || (u32)dr.bottom!=b->height){reason="blit_incomplete_initialization";goto done;}
        p->bootstrap=1;
    }
    p->direct=game_blit_live_only(a->length,b->length,a->bits);
    if(!p->direct && (!game_surface_clone(&p->src,a) || !(p->bootstrap?game_surface_blank(&p->dst,b):game_surface_clone(&p->dst,b)))){reason="blit_memory";game_blit_free(p);goto done;}
    p->fields[0]=1;p->fields[1]=2;p->fields[2]=(u32)sr.left;p->fields[3]=(u32)sr.top;
    p->fields[4]=(u32)sr.right;p->fields[5]=(u32)sr.bottom;
    p->fields[6]=(u32)dr.left;p->fields[7]=(u32)dr.top;p->fields[8]=keyed;p->fields[9]=keyed?src->key:0;
    p->source_generation=src->generation;p->target_generation=dst->generation;p->epoch=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED);p->valid=1;reason=p->bootstrap?"blit_bootstrap_ready":"blit_ready";
 done:
    if(dst && dst->primary && !p->valid)lock_diagnostic("primary_blit_rejected",target,0,(u32)source,flags,0,(src && src->pixels.data?1u:0u)|(dst->pixels.data?2u:0u)|(dst->layout_known?4u:0u)|(dst->clip_known?8u:0u)|(dst->clip?16u:0u),0);
    if(shape){
        u32 values[19]={(u32)target,GetCurrentThreadId(),flags,(u32)effects,(u32)sr.left,(u32)sr.top,(u32)sr.right,(u32)sr.bottom,
            (u32)dr.left,(u32)dr.top,(u32)dr.right,(u32)dr.bottom,src->pixels.width,src->pixels.height,dst->pixels.width,dst->pixels.height,dst->clip_known,dst->clip,(u32)source};
        lock_diagnostic_values(reason,values);
    }else lock_diagnostic(reason,target,0,(u32)source,flags,0,0,0);
    game_tracker_release();
}
/* Used only with owned storage under the tracker guard. */
static void game_blit_apply(const struct GameBlit* p,const struct Snapshot* src,struct Snapshot* dst){
    const u32* f=p->fields;u32 bytes=dst->bits/8,width=f[4]-f[2],height=f[5]-f[3];
    u8* first=dst->data+(f[7]*dst->width+f[6])*bytes;
    if(p->fill){
        for(u32 x=0;x<width;++x)for(u32 j=0;j<bytes;++j)first[x*bytes+j]=(u8)(p->fill_value>>(8*j));
        for(u32 y=1;y<height;++y)copy(first+y*dst->width*bytes,first,width*bytes);
        return;
    }
    for(u32 y=0;y<height;++y){const u8* in=src->data+((f[3]+y)*src->width+f[2])*bytes;u8* out=first+y*dst->width*bytes;
        if(!f[8]){copy(out,in,width*bytes);continue;}
        for(u32 x=0;x<width;++x){u32 value=0;for(u32 j=0;j<bytes;++j)value|=(u32)in[x*bytes+j]<<(8*j);
            if(value!=f[9])for(u32 j=0;j<bytes;++j)out[x*bytes+j]=in[x*bytes+j];}
    }
}
static void game_blit_after(struct GameBlit* p,i32 result){
    if(!lock_capture_path_length)return;
    if(result<0){lock_diagnostic("blit_failed",p->target,0,(u32)p->source,0,result,0,0);game_blit_free(p);return;}
    if(!game_surface_pixels_enter(p->target)){game_blit_free(p);return;}
    struct GameSurface *src=p->source?game_surface_find(p->source,0):0,*dst=game_surface_find(p->target,0);
    if(!p->valid || p->epoch!=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED) || (!p->fill && (!src || src->generation!=p->source_generation)) || !dst || dst->generation!=p->target_generation){
        u32 refusal[19]={(u32)p->target,GetCurrentThreadId(),(u32)p->source,p->valid,p->epoch,
            __atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED),p->source_generation,src?src->generation:0,
            p->target_generation,dst?dst->generation:0,p->fill,p->bootstrap,p->direct,(u32)result,src?src->generation_origin:0,dst?dst->generation_origin:0,p->caller,src?src->generation_caller:0,src?src->generation_owner:0};
        lock_diagnostic_values("blit_commit_refused",refusal);
        game_surface_invalidate_locked(p->target);lock_diagnostic("blit_invalidated",p->target,0,(u32)p->source,0,result,0,0);goto done;
    }
    if(p->direct){
        /* Original calls cannot touch this owned memory. Generation/epoch
         * checks above reject nested mutations before any in-place update.
         * Recording is finished and ordered sessions are disabled. */
        if(!dst->pixels.data){
            if(!game_surface_blank(&p->dst,&dst->pixels)){game_surface_invalidate_locked(p->target);goto done;}
            copy(&dst->pixels,&p->dst,sizeof(p->dst));p->dst.data=0;
            game_surface_bytes+=dst->pixels.length;__atomic_sub_fetch(&lock_capture_reserved,dst->pixels.length,__ATOMIC_RELAXED);
        }
        game_blit_apply(p,p->fill?0:&src->pixels,&dst->pixels);dst->generation=++game_surface_generation;dst->generation_origin=8;dst->generation_caller=p->caller;dst->generation_owner=GetCurrentThreadId();
        for(u32 i=0;i<32;++i)if(game_alias_same(p->target,game_locks[i].object))game_lock_clear(game_locks+i);
        if(dst->primary)game_surface_publish_region(dst,p->fields[6],p->fields[7],p->fields[4]-p->fields[2],p->fields[5]-p->fields[3],p->target_generation);
        lock_diagnostic("blit_recording_limit",p->target,0,(u32)p->source,0,result,0,0);
        lock_diagnostic("blit_live_in_place",p->target,0,(u32)p->source,0,result,0,0);goto done;
    }
    u32 record_bytes=p->src.length+p->dst.length*2+(p->src.bits==8?1280:256);
    int record=game_blit_count<16 && record_bytes<=GAME_SURFACE_LIMIT-game_blit_bytes;
    char path[544];copy(path,lock_capture_path,lock_capture_path_length);char* tail=path+lock_capture_path_length;
    if(record){copy(tail,"\\blit-",6);failure_hex(tail+6,++game_blit_count);copy(tail+14,".bin",5);}
    int palette_ready=p->dst.bits!=8 || game_surface_colors(dst,p->dst.palette);
    HANDLE file=record && palette_ready?CreateFileA(path,0x40000000,1,0,1,0x80,0):(HANDLE)-1;u32 sequence=0,header[4];copy(header,"MNMCMD01",8);header[2]=1;header[3]=16;
    int ok=file!=(HANDLE)-1 && write_all(file,header,16) && command_create_native(file,&sequence,1,&p->src) && command_create(file,&sequence,2,&p->dst) && command_record(file,&sequence,3,p->fields,40,0,0);
    game_session_blit_begin(p);
    game_blit_apply(p,&p->src,&p->dst);
    game_session_blit_end(p,dst->primary);
    u32 source_id=1,target_id=2;
    if(ok)ok=command_record(file,&sequence,5,&target_id,4,p->dst.data,p->dst.length) && command_record(file,&sequence,6,&target_id,4,0,0) && command_record(file,&sequence,7,&source_id,4,0,0) && command_record(file,&sequence,7,&target_id,4,0,0) && command_record(file,&sequence,8,0,0,0,0);
    if(file!=(HANDLE)-1)CloseHandle(file);
    if(record)game_blit_bytes+=record_bytes;
    for(u32 i=0;i<32;++i)if(game_alias_same(p->target,game_locks[i].object))game_lock_clear(game_locks+i);
    game_surface_drop(dst);dst->generation_origin=8;dst->generation_caller=p->caller;dst->generation_owner=GetCurrentThreadId();copy(&dst->pixels,&p->dst,sizeof(p->dst));p->dst.data=0;game_surface_bytes+=dst->pixels.length;
    __atomic_sub_fetch(&lock_capture_reserved,dst->pixels.length,__ATOMIC_RELAXED);
    if(p->bootstrap)lock_diagnostic("blit_initialized",p->target,0,(u32)p->source,0,result,0,0);
    if(dst->primary)lock_diagnostic(game_surface_publish_region(dst,p->fields[6],p->fields[7],p->fields[4]-p->fields[2],p->fields[5]-p->fields[3],p->target_generation)?"blit_presented":"blit_presentation_skipped",p->target,0,(u32)p->source,0,result,0,0);
    lock_diagnostic(!record?"blit_recording_limit":!palette_ready?"blit_palette_unobserved":ok?"blit_propagated":"blit_file_failed",p->target,0,(u32)p->source,0,result,0,0);
 done:game_blit_free(p);game_tracker_release();
}
