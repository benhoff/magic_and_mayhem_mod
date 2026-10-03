/* Owned RGB checkpoints; only application calls establish pixels and properties.
 * All access is under game_locks_busy. No observer COM calls or references. */
struct GameSurface {void* object;u32 epoch,generation,clip_known,clip,key_known,key;struct Snapshot pixels;};
static struct GameSurface game_surfaces[32];
static u32 game_surface_bytes,game_surface_generation,game_blit_count,game_blit_bytes;
#define GAME_SURFACE_LIMIT (64u*1024u*1024u)
static void game_surface_drop(struct GameSurface* s){
    if(s->pixels.data){game_surface_bytes-=s->pixels.length;free_snapshot(&s->pixels);}
    s->generation=++game_surface_generation;
}
static void game_surface_sync(void){
    game_alias_sync();
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
static void game_surface_invalidate_locked(void* object){
    for(u32 i=0;i<32;++i)if(game_surfaces[i].object && game_alias_same(object,game_surfaces[i].object))game_surface_drop(game_surfaces+i);
    for(u32 i=0;i<32;++i)if(game_alias_same(object,game_locks[i].object))zero(game_locks+i,sizeof(game_locks[i]));
}
static void game_surface_alias(void* object){
    u32 count=0;
    for(u32 i=0;i<32;++i)if(game_surfaces[i].object && game_alias_same(object,game_surfaces[i].object))++count;
    /* Independently observed interfaces may have conflicting pixel/properties.
     * Require a new checkpoint rather than choosing an arbitrary record. */
    if(count>1)for(u32 i=0;i<32;++i)if(game_surfaces[i].object && game_alias_same(object,game_surfaces[i].object)){
        game_surface_drop(game_surfaces+i);zero(game_surfaces+i,sizeof(game_surfaces[i]));
    }
}
static void game_surface_created(void* object){
    if(!game_surface_enter())return;
    for(u32 i=0;i<32;++i)if(game_surfaces[i].object && game_alias_same(object,game_surfaces[i].object)){
        game_surface_drop(game_surfaces+i);zero(game_surfaces+i,sizeof(game_surfaces[i]));
    }
    for(u32 i=0;i<32;++i)if(game_alias_same(object,game_locks[i].object))zero(game_locks+i,sizeof(game_locks[i]));
    game_alias_retire(object);
    struct GameSurface* s=game_surface_find(object,1);if(s){s->clip_known=1;s->clip=0;}
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
static void game_surface_store(void* object,struct Snapshot* pixels){
    struct GameSurface* s=game_surface_find(object,1);
    if(!s)return;
    game_surface_drop(s);
    if(pixels->bits==8 || !pixels->data || pixels->length>GAME_SURFACE_LIMIT-game_surface_bytes-__atomic_load_n(&lock_capture_reserved,__ATOMIC_RELAXED))return;
    copy(&s->pixels,pixels,sizeof(*pixels));pixels->data=0;game_surface_bytes+=s->pixels.length;
}
struct GameBlit {struct Snapshot src,dst;void *source,*target;u32 source_generation,target_generation,epoch,fields[10],valid;};
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
static void game_blit_before(void* target,void* destination,void* source,void* rectangle,u32 flags,void* effects,u32 fast,u32 x,u32 y,struct GameBlit* p){
    zero(p,sizeof(*p));p->target=target;p->source=source;
    if(!game_surface_enter())return;
    const char* reason="blit_untracked";
    struct GameSurface *src=source?game_surface_find(source,0):0,*dst=game_surface_find(target,0);
    if(!src || !dst || !src->pixels.data || !dst->pixels.data)goto done;
    reason="blit_unsupported";
    if(src==dst || effects || (fast?(flags&~0x11u):(flags&~0x01008000u)) || (!fast && (!dst->clip_known || dst->clip)))goto done;
    if(game_blit_count>=16){reason="blit_limit";goto done;}
    struct Snapshot *a=&src->pixels,*b=&dst->pixels;
    if(a->bits!=b->bits || a->r!=b->r || a->g!=b->g || a->b!=b->b || a->width>256 || a->height>256)goto done;
    struct Rect sr={0,0,(i32)a->width,(i32)a->height},dr={0,0,(i32)b->width,(i32)b->height};
    if(rectangle){if(!readable(rectangle,16))goto done;copy(&sr,rectangle,16);}
    if(!inside(&sr,a->width,a->height))goto done;
    if(fast){if(x>b->width || y>b->height)goto done;dr.left=(i32)x;dr.top=(i32)y;dr.right=dr.left+sr.right-sr.left;dr.bottom=dr.top+sr.bottom-sr.top;}
    else if(destination){if(!readable(destination,16))goto done;copy(&dr,destination,16);}
    if(!inside(&dr,b->width,b->height) || dr.right-dr.left!=sr.right-sr.left || dr.bottom-dr.top!=sr.bottom-sr.top)goto done;
    u32 keyed=fast?(flags&1)!=0:(flags&0x8000)!=0;
    if(keyed && !src->key_known)goto done;
    if(a->length+b->length*2+256>GAME_SURFACE_LIMIT-game_blit_bytes){reason="blit_limit";goto done;}
    if(!game_surface_clone(&p->src,a) || !game_surface_clone(&p->dst,b)){reason="blit_memory";game_blit_free(p);goto done;}
    p->fields[0]=1;p->fields[1]=2;p->fields[2]=(u32)sr.left;p->fields[3]=(u32)sr.top;
    p->fields[4]=(u32)sr.right;p->fields[5]=(u32)sr.bottom;
    p->fields[6]=(u32)dr.left;p->fields[7]=(u32)dr.top;p->fields[8]=keyed;p->fields[9]=keyed?src->key:0;
    p->source_generation=src->generation;p->target_generation=dst->generation;p->epoch=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED);p->valid=1;reason="blit_ready";
 done:lock_diagnostic(reason,target,0,(u32)source,flags,0,0,0);__sync_lock_release(&game_locks_busy);
}
static void game_blit_after(struct GameBlit* p,i32 result){
    if(!lock_capture_path_length)return;
    if(result<0){lock_diagnostic("blit_failed",p->target,0,(u32)p->source,0,result,0,0);game_blit_free(p);return;}
    if(!game_surface_enter()){game_blit_free(p);return;}
    struct GameSurface *src=p->source?game_surface_find(p->source,0):0,*dst=game_surface_find(p->target,0);
    if(!p->valid || game_blit_count>=16 || p->src.length+p->dst.length*2+256>GAME_SURFACE_LIMIT-game_blit_bytes || p->epoch!=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED) || !src || !dst || src->generation!=p->source_generation || dst->generation!=p->target_generation){
        game_surface_invalidate_locked(p->target);lock_diagnostic("blit_invalidated",p->target,0,(u32)p->source,0,result,0,0);goto done;
    }
    char path[544];copy(path,lock_capture_path,lock_capture_path_length);char* tail=path+lock_capture_path_length;
    copy(tail,"\\blit-",6);failure_hex(tail+6,++game_blit_count);copy(tail+14,".bin",5);
    HANDLE file=CreateFileA(path,0x40000000,1,0,1,0x80,0);u32 sequence=0,header[4];copy(header,"MNMCMD01",8);header[2]=1;header[3]=16;
    int ok=file!=(HANDLE)-1 && write_all(file,header,16) && command_create(file,&sequence,1,&p->src) && command_create(file,&sequence,2,&p->dst) && command_record(file,&sequence,3,p->fields,40,0,0);
    u32 bytes=p->src.bits/8;u32* f=p->fields;
    for(u32 y=0;y<f[5]-f[3];++y)for(u32 x=0;x<f[4]-f[2];++x){
        u8* in=p->src.data+((f[3]+y)*p->src.width+f[2]+x)*bytes;
        u8* out=p->dst.data+((f[7]+y)*p->dst.width+f[6]+x)*bytes;
        u32 value=0;for(u32 i=0;i<bytes;++i)value|=(u32)in[i]<<(8*i);
        if(!f[8] || value!=f[9])copy(out,in,bytes);
    }
    u32 source_id=1,target_id=2;
    if(ok)ok=command_record(file,&sequence,5,&target_id,4,p->dst.data,p->dst.length) && command_record(file,&sequence,6,&target_id,4,0,0) && command_record(file,&sequence,7,&source_id,4,0,0) && command_record(file,&sequence,7,&target_id,4,0,0) && command_record(file,&sequence,8,0,0,0,0);
    if(file!=(HANDLE)-1)CloseHandle(file);
    game_blit_bytes+=p->src.length+p->dst.length*2+256;
    game_surface_drop(dst);copy(&dst->pixels,&p->dst,sizeof(p->dst));p->dst.data=0;game_surface_bytes+=dst->pixels.length;
    __atomic_sub_fetch(&lock_capture_reserved,dst->pixels.length,__ATOMIC_RELAXED);
    lock_diagnostic(ok?"blit_propagated":"blit_file_failed",p->target,0,(u32)p->source,0,result,0,0);
 done:game_blit_free(p);__sync_lock_release(&game_locks_busy);
}
