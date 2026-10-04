#include "../../protocols/include/mnm/frame_v1.h"
/* Owned native checkpoints; only application calls establish pixels and properties.
 * All access is under game_locks_busy. No observer COM calls or references. */
struct GameSurface {void* object;u32 epoch,generation,clip_known,clip,key_known,key,primary,layout_known,caps,back_count,back_count_known;void *back,*palette;struct Snapshot pixels;};
static struct GameSurface game_surfaces[32];
static u32 game_surface_bytes,game_surface_generation,game_blit_count,game_blit_bytes;
#define GAME_SURFACE_LIMIT (64u*1024u*1024u)
/* Publish owned pixels only. The guard/sequence protocol is shared with the
 * legacy producer; readers never consume a partially written RGBA frame. */
static int game_surface_publish(struct GameSurface*);
static int game_surface_colors(struct GameSurface*,u8*);
static int game_publish_pixels(const struct Snapshot* s){
    if(!stream || !s->data || !__sync_bool_compare_and_swap(&capture_busy,0,1))return 0;
    u32 sequence=__atomic_load_n(stream+MNM_FRAME_V1_SEQUENCE_OFFSET/4,__ATOMIC_RELAXED);
    __atomic_store_n(stream+MNM_FRAME_V1_SEQUENCE_OFFSET/4,sequence+1,__ATOMIC_SEQ_CST);
    int ok=render_pixels((u8*)stream+MNM_FRAME_V1_PIXELS_OFFSET,s->width,s->height,s->data,(i32)(s->width*(s->bits/8)),s->bits,s->r,s->g,s->b,s->bits==8?s->palette:0);
    if(ok){stream[MNM_FRAME_V1_WIDTH_OFFSET/4]=s->width;stream[MNM_FRAME_V1_HEIGHT_OFFSET/4]=s->height;stream[MNM_FRAME_V1_STRIDE_OFFSET/4]=s->width*MNM_FRAME_V1_BYTES_PER_PIXEL;stream[MNM_FRAME_V1_PIXEL_FORMAT_OFFSET/4]=MNM_FRAME_V1_PIXEL_FORMAT_RGBA8888;++stream[MNM_FRAME_V1_FRAME_COUNT_OFFSET/4];stream[MNM_FRAME_V1_STATUS_OFFSET/4]=MNM_FRAME_V1_STATUS_FRAME_PUBLISHED;}
    __atomic_store_n(stream+MNM_FRAME_V1_SEQUENCE_OFFSET/4,sequence+2,__ATOMIC_RELEASE);__sync_lock_release(&capture_busy);return ok;
}
static void game_surface_drop(struct GameSurface* s){
    if(s->pixels.data){game_surface_bytes-=s->pixels.length;free_snapshot(&s->pixels);}
    s->generation=++game_surface_generation;
}
static void game_surface_sync(void){
    game_alias_sync();game_session_sync();
    for(u32 i=0;i<32;++i)if(game_locks[i].object && game_locks[i].epoch!=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED))game_lock_clear(game_locks+i);
    for(u32 i=0;i<32;++i)if(game_surfaces[i].object && game_surfaces[i].epoch!=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED)){
        game_surface_drop(game_surfaces+i);zero(game_surfaces+i,sizeof(game_surfaces[i]));
    }
}
static struct GameSurface* game_surface_find(void* object,int create){
    struct GameSurface* found=0;struct GameSurface* empty=0;
    for(u32 i=0;i<32;++i){struct GameSurface* s=game_surfaces+i;
        if(!s->object){if(!empty)empty=s;continue;}
        if(game_alias_same(object,s->object)){if(found)return 0;found=s;}
    }
    if(!found && create && empty){found=empty;found->object=object;found->epoch=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED);found->generation=++game_surface_generation;}
    return found;
}
static int game_surface_enter(void){
    if(!lock_capture_path_length)return 0;
    if(!__sync_bool_compare_and_swap(&game_locks_busy,0,1)){__atomic_add_fetch(&game_lock_epoch,1,__ATOMIC_RELAXED);return 0;}
    game_surface_sync();return 1;
}
static void game_surface_pixels_invalidate_locked(void* object){
    for(u32 i=0;i<32;++i)if(game_surfaces[i].object && game_alias_same(object,game_surfaces[i].object))game_surface_drop(game_surfaces+i);
    for(u32 i=0;i<32;++i)if(game_alias_same(object,game_locks[i].object))game_lock_clear(game_locks+i);
}
static void game_surface_invalidate_locked(void* object){game_session_invalidate(object);game_surface_pixels_invalidate_locked(object);}
static void game_surface_alias(void* object){
    u32 count=0;
    for(u32 i=0;i<32;++i)if(game_surfaces[i].object && game_alias_same(object,game_surfaces[i].object))++count;
    /* Independently observed interfaces may have conflicting pixel/properties.
     * Require a new checkpoint rather than choosing an arbitrary record. */
    if(count>1)game_session_invalidate(object);
    if(count>1)for(u32 i=0;i<32;++i)if(game_alias_same(object,game_locks[i].object))game_lock_clear(game_locks+i);
    if(count>1)for(u32 i=0;i<32;++i)if(game_surfaces[i].object && game_alias_same(object,game_surfaces[i].object)){
        game_surface_drop(game_surfaces+i);zero(game_surfaces+i,sizeof(game_surfaces[i]));
    }
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
    s->layout_known=1;s->primary=(d[26]&0x200)!=0;s->generation=++game_surface_generation;
    lock_diagnostic("surface_metadata",object,kind,0,0,0,0,d);
}
static void game_surface_described(void* object,u32 kind,const u32* d,i32 result){
    if(result<0){lock_diagnostic("surface_metadata_failed",object,kind,0,0,result,0,0);return;}
    if(!game_surface_enter())return;
    game_surface_describe_locked(object,kind,d);__sync_lock_release(&game_locks_busy);
}
static void game_surface_created(void* object,u32 kind,const u32* d){
    if(!game_surface_enter())return;
    game_session_invalidate(object);
    for(u32 i=0;i<32;++i)if(game_surfaces[i].object && game_alias_same(object,game_surfaces[i].object)){
        game_surface_drop(game_surfaces+i);zero(game_surfaces+i,sizeof(game_surfaces[i]));
    }
    for(u32 i=0;i<32;++i)if(game_alias_same(object,game_locks[i].object))game_lock_clear(game_locks+i);
    game_alias_retire(object);
    struct GameSurface* s=game_surface_find(object,1);if(s){s->clip_known=1;s->clip=0;}
    if(d)game_surface_describe_locked(object,kind,d);
    __sync_lock_release(&game_locks_busy);
}
static void game_surface_invalidate(void* object){
    if(!game_surface_enter())return;
    game_surface_invalidate_locked(object);__sync_lock_release(&game_locks_busy);
}
static void game_surface_clipper(void* object,void* clipper){
    if(!game_surface_enter())return;
    struct GameSurface* s=game_surface_find(object,1);
    if(s){s->clip_known=1;s->clip=clipper!=0;s->generation=++game_surface_generation;}
    __sync_lock_release(&game_locks_busy);
}
static void game_surface_key(void* object,u32 flags,int valid,const u32* key){
    if(!(flags&8) || !game_surface_enter())return;
    struct GameSurface* s=game_surface_find(object,1);
    if(s){s->key_known=valid && flags==8 && key && key[0]==key[1];s->key=s->key_known?key[0]:0;s->generation=++game_surface_generation;}
    __sync_lock_release(&game_locks_busy);
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
    s->primary=primary;s->layout_known=1;copy(&s->pixels,pixels,sizeof(*pixels));pixels->data=0;game_surface_bytes+=s->pixels.length;
}
struct GameBlit {struct Snapshot src,dst;void *source,*target;u32 source_generation,target_generation,epoch,fields[10],valid,bootstrap;};
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
static void game_blit_before(void* target,void* destination,void* source,void* rectangle,u32 flags,void* effects,u32 fast,u32 x,u32 y,struct GameBlit* p){
    zero(p,sizeof(*p));p->target=target;p->source=source;
    if(!game_surface_enter())return;
    const char* reason="blit_untracked";
    struct GameSurface *src=source?game_surface_find(source,0):0,*dst=game_surface_find(target,0);
    if(!src || !dst || !src->pixels.data || (!dst->pixels.data && !dst->layout_known))goto done;
    reason="blit_unsupported";
    if(src==dst || effects || (fast?(flags&~0x11u):(flags&~0x01008000u)) || (!fast && (!dst->clip_known || dst->clip)))goto done;
    if(game_blit_count>=16){reason="blit_limit";goto done;}
    struct Snapshot *a=&src->pixels,*b=&dst->pixels;
    if(a->bits!=b->bits || a->r!=b->r || a->g!=b->g || a->b!=b->b || a->width>2048 || a->height>2048)goto done;
    struct Rect sr={0,0,(i32)a->width,(i32)a->height},dr={0,0,(i32)b->width,(i32)b->height};
    if(rectangle){if(!readable(rectangle,16))goto done;copy(&sr,rectangle,16);}
    if(!inside(&sr,a->width,a->height))goto done;
    if(fast){if(x>b->width || y>b->height)goto done;dr.left=(i32)x;dr.top=(i32)y;dr.right=dr.left+sr.right-sr.left;dr.bottom=dr.top+sr.bottom-sr.top;}
    else if(destination){if(!readable(destination,16))goto done;copy(&dr,destination,16);}
    if(!inside(&dr,b->width,b->height) || dr.right-dr.left!=sr.right-sr.left || dr.bottom-dr.top!=sr.bottom-sr.top)goto done;
    u32 keyed=fast?(flags&1)!=0:(flags&0x8000)!=0;
    if(keyed && (!src->key_known || (a->bits==8 && src->key>255)))goto done;
    if(!b->data){
        /* Synthetic blank storage is permitted only when every pixel will be
         * overwritten. It is not evidence of the original destination pixels. */
        if(keyed || dr.left || dr.top || (u32)dr.right!=b->width || (u32)dr.bottom!=b->height){reason="blit_incomplete_initialization";goto done;}
        p->bootstrap=1;
    }
    if(a->length+b->length*2+(a->bits==8?1280:256)>GAME_SURFACE_LIMIT-game_blit_bytes){reason="blit_limit";goto done;}
    if(!game_surface_clone(&p->src,a) || !(p->bootstrap?game_surface_blank(&p->dst,b):game_surface_clone(&p->dst,b))){reason="blit_memory";game_blit_free(p);goto done;}
    p->fields[0]=1;p->fields[1]=2;p->fields[2]=(u32)sr.left;p->fields[3]=(u32)sr.top;
    p->fields[4]=(u32)sr.right;p->fields[5]=(u32)sr.bottom;
    p->fields[6]=(u32)dr.left;p->fields[7]=(u32)dr.top;p->fields[8]=keyed;p->fields[9]=keyed?src->key:0;
    p->source_generation=src->generation;p->target_generation=dst->generation;p->epoch=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED);p->valid=1;reason=p->bootstrap?"blit_bootstrap_ready":"blit_ready";
 done:lock_diagnostic(reason,target,0,(u32)source,flags,0,0,0);__sync_lock_release(&game_locks_busy);
}
static void game_blit_after(struct GameBlit* p,i32 result){
    if(!lock_capture_path_length)return;
    if(result<0){lock_diagnostic("blit_failed",p->target,0,(u32)p->source,0,result,0,0);game_blit_free(p);return;}
    if(!game_surface_enter()){game_blit_free(p);return;}
    struct GameSurface *src=p->source?game_surface_find(p->source,0):0,*dst=game_surface_find(p->target,0);
    if(!p->valid || game_blit_count>=16 || p->src.length+p->dst.length*2+(p->src.bits==8?1280:256)>GAME_SURFACE_LIMIT-game_blit_bytes || p->epoch!=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED) || !src || !dst || src->generation!=p->source_generation || dst->generation!=p->target_generation){
        game_surface_invalidate_locked(p->target);lock_diagnostic("blit_invalidated",p->target,0,(u32)p->source,0,result,0,0);goto done;
    }
    char path[544];copy(path,lock_capture_path,lock_capture_path_length);char* tail=path+lock_capture_path_length;
    copy(tail,"\\blit-",6);failure_hex(tail+6,++game_blit_count);copy(tail+14,".bin",5);
    int palette_ready=p->dst.bits!=8 || game_surface_colors(dst,p->dst.palette);
    HANDLE file=palette_ready?CreateFileA(path,0x40000000,1,0,1,0x80,0):(HANDLE)-1;u32 sequence=0,header[4];copy(header,"MNMCMD01",8);header[2]=1;header[3]=16;
    int ok=file!=(HANDLE)-1 && write_all(file,header,16) && command_create_native(file,&sequence,1,&p->src) && command_create(file,&sequence,2,&p->dst) && command_record(file,&sequence,3,p->fields,40,0,0);
    game_session_blit_begin(p);
    u32 bytes=p->src.bits/8;u32* f=p->fields;
    for(u32 y=0;y<f[5]-f[3];++y)for(u32 x=0;x<f[4]-f[2];++x){
        u8* in=p->src.data+((f[3]+y)*p->src.width+f[2]+x)*bytes;
        u8* out=p->dst.data+((f[7]+y)*p->dst.width+f[6]+x)*bytes;
        u32 value=0;for(u32 i=0;i<bytes;++i)value|=(u32)in[i]<<(8*i);
        if(!f[8] || value!=f[9])copy(out,in,bytes);
    }
    game_session_blit_end(p,dst->primary);
    u32 source_id=1,target_id=2;
    if(ok)ok=command_record(file,&sequence,5,&target_id,4,p->dst.data,p->dst.length) && command_record(file,&sequence,6,&target_id,4,0,0) && command_record(file,&sequence,7,&source_id,4,0,0) && command_record(file,&sequence,7,&target_id,4,0,0) && command_record(file,&sequence,8,0,0,0,0);
    if(file!=(HANDLE)-1)CloseHandle(file);
    game_blit_bytes+=p->src.length+p->dst.length*2+(p->src.bits==8?1280:256);
    for(u32 i=0;i<32;++i)if(game_alias_same(p->target,game_locks[i].object))game_lock_clear(game_locks+i);
    game_surface_drop(dst);copy(&dst->pixels,&p->dst,sizeof(p->dst));p->dst.data=0;game_surface_bytes+=dst->pixels.length;
    __atomic_sub_fetch(&lock_capture_reserved,dst->pixels.length,__ATOMIC_RELAXED);
    if(p->bootstrap)lock_diagnostic("blit_initialized",p->target,0,(u32)p->source,0,result,0,0);
    if(dst->primary)lock_diagnostic(game_surface_publish(dst)?"blit_presented":"blit_presentation_skipped",p->target,0,(u32)p->source,0,result,0,0);
    lock_diagnostic(!palette_ready?"blit_palette_unobserved":ok?"blit_propagated":"blit_file_failed",p->target,0,(u32)p->source,0,result,0,0);
 done:game_blit_free(p);__sync_lock_release(&game_locks_busy);
}
