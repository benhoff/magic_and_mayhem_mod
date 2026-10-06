/* Observe game-owned writable locks. Never Lock, Unlock, Query or retain COM objects. */
struct GameLock {void* object;u32 owner,generation,epoch,active,kind,flags,desc[31];void* rectangle;struct Rect region;struct Snapshot base;};
static struct GameLock game_locks[32];
static volatile i32 game_locks_busy;
static u32 game_locks_owner,game_tracker_wait_ms=8,game_tracker_wait_count;
static u32 game_lock_epoch,game_metadata_epoch,game_lock_generation,lock_capture_count,lock_capture_bytes,lock_capture_reserved;
/* A missed pixel operation writes only its target. Queue opaque identity tokens
 * for the next guarded entry; never inspect or dereference them without the guard.
 * Overflow retains the conservative whole-pixel-epoch fallback. */
static void* game_pixel_misses[128];
static u32 game_pixel_misses_pending;
/* Pixel uncertainty invalidates pending work; missed identity/property changes
 * additionally invalidate metadata. Neither path waits across application calls. */
static void game_metadata_invalidate(void){
    __atomic_add_fetch(&game_metadata_epoch,1,__ATOMIC_RELAXED);
    __atomic_add_fetch(&game_lock_epoch,1,__ATOMIC_RELAXED);
}
/* Detached partial bases are counted as pending storage and never published. */
static void game_lock_clear(struct GameLock* slot){
    if(slot->base.data)__atomic_sub_fetch(&lock_capture_reserved,slot->base.length,__ATOMIC_RELAXED);
    free_snapshot(&slot->base);zero(slot,sizeof(*slot));
}
static u32 game_session_enabled,game_session_presentations,game_session_continuous,game_session_archive,game_session_palette_resources;
struct GameSurface;struct GameBlit;struct GamePalette;
static void game_session_palette_retire(struct GamePalette*);
static int game_session_palette_registered(struct GamePalette*);
static void game_session_gap(u32);
static void game_session_sync(void);
static void game_session_invalidate(void*);
static void game_session_blit_begin(struct GameBlit*);
static void game_session_blit_end(struct GameBlit*,u32);
static void game_session_palette(struct GameSurface*);
static void game_session_palette_changed(void*);
static char lock_capture_path[512];
static u32 lock_capture_path_length;
static void init_lock_lifecycle(void){
    char wait[8];u32 n=GetEnvironmentVariableA("MNM_RENDER_TRACKER_WAIT_MS",wait,sizeof(wait));
    if(n && n<sizeof(wait)){
        u32 value=0,valid=1;for(u32 i=0;i<n;++i){if(wait[i]<'0' || wait[i]>'9'){valid=0;break;}value=value*10+(u32)(wait[i]-'0');}
        if(valid && value<=50)game_tracker_wait_ms=value;
    }
    char session[8];game_session_enabled=GetEnvironmentVariableA("MNM_RENDER_OWNED_SESSION",session,8)==1 && session[0]=='1';
    game_session_continuous=GetEnvironmentVariableA("MNM_RENDER_CONTINUOUS",session,8)==1 && session[0]=='1';
    if(game_session_continuous)game_session_enabled=1;
    char resources[8];game_session_palette_resources=game_session_continuous &&
        GetEnvironmentVariableA("MNM_RENDER_PALETTE_RESOURCES",resources,sizeof(resources))==1 && resources[0]=='1';
    game_session_archive=!game_session_continuous ||
        (GetEnvironmentVariableA("MNM_RENDER_SESSION_ARCHIVE",session,8)==1 && session[0]=='1');
    /* Explicit finite multi-frame observation; malformed values retain the
     * ordinary sample. This does not enlarge the append-only transport. */
    char frames[4];u32 frames_length=GetEnvironmentVariableA("MNM_RENDER_SESSION_PRESENTATIONS",frames,sizeof(frames));
    if(frames_length && frames_length<sizeof(frames)){
        u32 value=0,valid=1;for(u32 i=0;i<frames_length;++i){if(frames[i]<'0' || frames[i]>'9'){valid=0;break;}value=value*10+(u32)(frames[i]-'0');}
        if(valid && value>=1 && value<=32)game_session_presentations=value;
    }
    u32 length=GetEnvironmentVariableA("MNM_RENDER_LOCK_CAPTURE_DIR",lock_capture_path,sizeof(lock_capture_path));
    if(length && length+28<sizeof(lock_capture_path))lock_capture_path_length=length;
}
#include "lock_diagnostics.h"
/* Tracker work contains no original COM calls. Brief cross-thread overlap can
 * wait without serializing the game API itself. Same-thread entry never waits.
 * Sleep(0) yields; GetTickCount resolution bounds the precision of the timeout. */
static int game_tracker_acquire(void){
    if(__sync_bool_compare_and_swap(&game_locks_busy,0,1)){
        __atomic_store_n(&game_locks_owner,GetCurrentThreadId(),__ATOMIC_RELEASE);return 1;
    }
    u32 owner=__atomic_load_n(&game_locks_owner,__ATOMIC_ACQUIRE),thread=GetCurrentThreadId();
    if(owner==thread)return 0;
    __atomic_add_fetch(&game_tracker_wait_count,1,__ATOMIC_RELAXED);
    if(!game_tracker_wait_ms)return 0;
    u32 start=GetTickCount();
    do{
        for(u32 i=0;i<64;++i){
            if(__sync_bool_compare_and_swap(&game_locks_busy,0,1)){
                __atomic_store_n(&game_locks_owner,thread,__ATOMIC_RELEASE);
                lock_diagnostic("tracker_wait_acquired",0,0,owner,GetTickCount()-start,0,0,0);return 1;
            }
            __asm__ volatile("pause");
        }
        Sleep(0);
    }while(GetTickCount()-start<game_tracker_wait_ms);
    lock_diagnostic("tracker_wait_timeout",0,0,owner,GetTickCount()-start,0,0,0);return 0;
}
static void game_tracker_release(void){
    __atomic_store_n(&game_locks_owner,0,__ATOMIC_RELEASE);__sync_lock_release(&game_locks_busy);
    command_channel_pump();
}
static void game_pixel_missed(void* object){
    if(object)for(u32 i=0;i<128;++i){
        void* previous=__atomic_load_n(game_pixel_misses+i,__ATOMIC_ACQUIRE);
        /* Keep duplicate misses: coalescing with a token concurrently drained
         * by the owner could lose a later mutation of the same surface. */
        if(!previous && __sync_bool_compare_and_swap(game_pixel_misses+i,0,object)){
            __atomic_store_n(&game_pixel_misses_pending,1,__ATOMIC_RELEASE);
            lock_diagnostic("pixel_tracker_contended",object,0,0,0,0,0,0);return;
        }
    }
    __atomic_add_fetch(&game_lock_epoch,1,__ATOMIC_RELAXED);
    lock_diagnostic("pixel_miss_overflow",object,0,0,0,0,0,0);
}
#include "lock_aliases.h"
#include "lock_surfaces.h"
#include "lock_palette.h"
#include "lock_updates.h"
#include "owned_session.h"
static void game_lock_retire(void* object){
    if(!lock_capture_path_length)return;
    if(!game_tracker_acquire()){__atomic_store_n(&game_alias_reset_pending,1,__ATOMIC_RELEASE);game_metadata_invalidate();return;}
    game_surface_sync();
    /* Retire this observed interface component, preserving unrelated primary
     * metadata/checkpoints. Contended retirement still invalidates the epoch. */
    game_session_retire(object);
    for(u32 i=0;i<32;++i)if(game_alias_same(game_locks[i].object,object))game_lock_clear(game_locks+i);
    for(u32 i=0;i<GAME_SURFACE_COUNT;++i){struct GameSurface* surface=game_surfaces+i;
        if(surface->object && game_alias_same(surface->object,object)){
            game_surface_drop(surface);zero(surface,sizeof(*surface));
        }else if(surface->back && game_alias_same(surface->back,object)){
            surface->back=0;surface->generation=++game_surface_generation;surface->generation_origin=13;
        }
    }
    game_alias_retire(object);
    game_tracker_release();
}
static void game_lock_observed(void* object,struct Table* table,void* rect,const struct Rect* region,const u32* desc,u32 flags,i32 result){
    if(!lock_capture_path_length)return;
    if(result<0){lock_diagnostic("lock_failed",object,table->kind,(u32)rect,flags,result,0,0);return;}
    if(!game_tracker_acquire()){game_pixel_missed(object);return;}
    game_surface_sync();
    struct Snapshot base;zero(&base,sizeof(base));
    struct GameSurface* surface=game_surface_find(object,0);
    u32 size=table->kind>=14?124:108;
    int valid=readable(desc,size) && desc[0]==size;
    int partial=rect && region && readable(rect,16) && same(rect,region,16) && valid && !(flags&0x10) && !(flags&~0x4831u) &&
        surface && surface->pixels.data && inside(region,surface->pixels.width,surface->pixels.height) &&
        surface->pixels.width==desc[3] && surface->pixels.height==desc[2] &&
        surface->pixels.bits==desc[21] && surface->pixels.flags==desc[19] &&
        surface->pixels.r==desc[22] && surface->pixels.g==desc[23] && surface->pixels.b==desc[24];
    /* Retain a matching owned baseline privately for continuous full-lock
     * deltas. It is not authoritative while the application holds the lock;
     * pending storage accounting and normal invalidation/clear own its life. */
    int delta=game_session_continuous && !rect && valid && !(flags&0x10) && !(flags&~0x4831u) &&
        surface && surface->pixels.data && surface->pixels.width==desc[3] && surface->pixels.height==desc[2] &&
        surface->pixels.bits==desc[21] && surface->pixels.flags==desc[19] &&
        surface->pixels.r==desc[22] && surface->pixels.g==desc[23] && surface->pixels.b==desc[24];
    if(partial || delta){copy(&base,&surface->pixels,sizeof(base));surface->pixels.data=0;
        game_surface_bytes-=base.length;__atomic_add_fetch(&lock_capture_reserved,base.length,__ATOMIC_RELAXED);}
    game_surface_pixels_invalidate_locked(object);
    if(valid)game_surface_descriptor_key_locked(object,desc);
    struct GameLock* slot=0;
    for(u32 i=0;i<32;++i)if(game_alias_same(game_locks[i].object,object)){slot=game_locks+i;break;}
    if(!slot)for(u32 i=0;i<32;++i)if(!game_locks[i].active || game_locks[i].epoch!=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED)){slot=game_locks+i;break;}
    const char* reason="lock_capacity";u32 diagnostic_desc[31];zero(diagnostic_desc,sizeof(diagnostic_desc));
    if(slot){
        game_lock_clear(slot);slot->object=object;slot->generation=++game_lock_generation;
        /* Replacing an earlier record always invalidates its pointer, even for unsupported locks. */
        if(valid)copy(diagnostic_desc,desc,size);
        reason=rect?(partial?"lock_partial_accepted":"lock_partial"):flags&0x10?"lock_readonly":flags&~0x4831u?"lock_flags":!valid || desc[0]!=size?"lock_descriptor":"lock_accepted";
        if((!rect || partial) && !(flags&0x10) && !(flags&~0x4831u) && valid && desc[0]==size){
            slot->epoch=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED);slot->owner=GetCurrentThreadId();slot->kind=table->kind;slot->flags=flags;
            copy(slot->desc,desc,size);slot->active=1;
            if(partial){slot->rectangle=rect;copy(&slot->region,region,16);}
            if(base.data){copy(&slot->base,&base,sizeof(base));base.data=0;}
        }
    }
    if(slot && !slot->active)game_session_invalidate(object);
    if(base.data)__atomic_sub_fetch(&lock_capture_reserved,base.length,__ATOMIC_RELAXED);free_snapshot(&base);
    lock_diagnostic(reason,object,table->kind,(u32)rect,flags,result,0,diagnostic_desc);
    game_tracker_release();
}
struct GameUnlock {struct Snapshot pixels;void *object,*target;u32 generation,owner,kind,flags,primary;};
static void game_unlock_before(void* object,u32 unlock_kind,void* argument,struct GameUnlock* pending){
    zero(pending,sizeof(*pending));pending->target=object;
    if(!lock_capture_path_length)return;
    if(!game_tracker_acquire()){game_pixel_missed(object);return;}
    game_surface_sync();
    const char* reason="unlock_unmatched";struct GameLock* matched=0;
    for(u32 i=0;i<32;++i){struct GameLock* slot=game_locks+i;
        if(!slot->active || !game_alias_same(slot->object,object) || slot->epoch!=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED))continue;
        matched=slot;pending->object=slot->object;pending->generation=slot->generation;
        if(slot->owner!=GetCurrentThreadId()){reason="unlock_owner";break;}
        if(unlock_kind>=14?(argument!=slot->rectangle || (argument && (!readable(argument,16) || !same(argument,&slot->region,16)))):argument!=(void*)slot->desc[9]){reason="unlock_argument";break;}
        u32* d=slot->desc;u32 width=d[3],height=d[2],bits=d[21];i32 pitch=(i32)d[4];
        if(!width || width>2048 || !height || height>2048 || !supported_format(d) || pitch==(-2147483647-1)){reason="unlock_layout";break;}
        if(bits!=8 && (!render_mask(d[22],bits) || !render_mask(d[23],bits) || !render_mask(d[24],bits) ||
           (d[22]&d[23]) || (d[22]&d[24]) || (d[23]&d[24]))){reason="unlock_masks";break;}
        u32 row_width=slot->rectangle?(u32)(slot->region.right-slot->region.left):width;
        u32 rows=slot->rectangle?(u32)(slot->region.bottom-slot->region.top):height;
        u32 stride=width*(bits/8),row_stride=row_width*(bits/8),magnitude=(u32)(pitch<0?-pitch:pitch),offset=(rows-1)*magnitude,at=d[9];
        if(magnitude<stride || magnitude>32768 || (pitch<0 && at<offset) ||
           !readable((void*)(pitch<0?at-offset:at),offset+row_stride)){reason="unlock_memory";break;}
        u32 reserved=__atomic_load_n(&lock_capture_reserved,__ATOMIC_RELAXED);
        if(reserved>GAME_SURFACE_LIMIT-game_surface_bytes || stride*height>GAME_SURFACE_LIMIT-game_surface_bytes-reserved){reason="unlock_limit";break;}
        struct Snapshot* s=&pending->pixels;s->data=HeapAlloc(GetProcessHeap(),0,stride*height);if(!s->data){reason="unlock_allocation";break;}
        __atomic_add_fetch(&lock_capture_reserved,stride*height,__ATOMIC_RELAXED);
        s->width=width;s->height=height;s->bits=bits;s->flags=d[19];s->r=d[22];s->g=d[23];s->b=d[24];s->length=stride*height;
        if(slot->rectangle)copy(s->data,slot->base.data,s->length);
        for(u32 y=0;y<rows;++y){u32 target=slot->rectangle?((u32)slot->region.top+y)*stride+(u32)slot->region.left*(bits/8):y*stride;
            copy(s->data+target,(u8*)at+(i32)y*pitch,row_stride);}
        pending->owner=slot->owner;pending->kind=slot->kind;pending->flags=slot->flags;
        pending->primary=(d[1]&1) && (d[26]&0x200);reason="unlock_copied";break;
    }
    lock_diagnostic(reason,object,matched?matched->kind:0,(u32)argument,matched?matched->flags:0,0,matched?matched->owner:0,matched?matched->desc:0);
    game_tracker_release();
}
static void game_unlock_after(struct GameUnlock* pending,i32 result){
    if(!lock_capture_path_length)return;
    lock_diagnostic(result<0?"unlock_failed":"unlock_succeeded",pending->object,pending->kind,0,pending->flags,result,pending->owner,0);
    if(!pending->object){
        if(result<0)return;
        if(!game_surface_pixels_enter(pending->target))return;
        /* Unknown pixel ownership cannot change screen metadata. */
        if(game_surface_find(pending->target,0)){
            game_surface_invalidate_locked(pending->target);
            lock_diagnostic("unlock_target_invalidated",pending->target,0,0,0,result,0,0);
        }else{
            __atomic_add_fetch(&game_lock_epoch,1,__ATOMIC_RELAXED);
            lock_diagnostic("unlock_epoch_invalidated",pending->target,0,0,0,result,0,0);
        }
        game_tracker_release();return;
    }
    if(!game_tracker_acquire()){game_pixel_missed(pending->target);if(pending->pixels.data)__atomic_sub_fetch(&lock_capture_reserved,pending->pixels.length,__ATOMIC_RELAXED);free_snapshot(&pending->pixels);return;}
    game_surface_sync();
    struct GameLock* slot=0;
    for(u32 i=0;i<32;++i)if(game_locks[i].object==pending->object && game_locks[i].generation==pending->generation && game_locks[i].epoch==__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED)){slot=game_locks+i;break;}
    struct Snapshot* s=&pending->pixels;
    if(s->data)__atomic_sub_fetch(&lock_capture_reserved,s->length,__ATOMIC_RELAXED);
    if(slot && result>=0){
        slot->active=0; /* Failed Unlock retains the descriptor for a subsequent application retry. */
        if(s->data && lock_capture_count<16 && s->length<=64*1024*1024-lock_capture_bytes){
            u32 id=++lock_capture_count;lock_capture_bytes+=s->length;
            char* tail=lock_capture_path+lock_capture_path_length;copy(tail,"\\lock-",6);failure_hex(tail+6,id);copy(tail+14,".bin",5);
            HANDLE file=CreateFileA(lock_capture_path,0x40000000,1,0,1,0x80,0);
            if(file!=(HANDLE)-1){u32 header[20];u32 partial=slot->rectangle!=0;copy(header,partial?"MNMLOCK2":"MNMLOCK1",8);
                header[2]=partial?2:1;header[3]=partial?80:64;header[4]=id;header[5]=(u32)pending->object;header[6]=pending->owner;
                header[7]=pending->kind;header[8]=pending->flags;header[9]=s->width;header[10]=s->height;header[11]=s->bits;
                header[12]=s->r;header[13]=s->g;header[14]=s->b;header[15]=s->length;
                if(partial)copy(header+16,&slot->region,16);
                if(write_all(file,header,header[3]))write_all(file,s->data,s->length);CloseHandle(file);
            }
            game_update_commands(slot,s,id);
        }else if(s->data){lock_diagnostic("unlock_recording_limit",pending->object,pending->kind,0,pending->flags,result,pending->owner,0);}
    }
    /* Diagnostic file budgets never gate checkpoint maintenance or publication. */
    if(slot && result>=0 && s->data){
        game_session_unlock(slot,s,pending->primary);
        if(pending->primary && s->bits!=8)lock_diagnostic(game_publish_pixels(s,0)?"unlock_presented":"unlock_presentation_skipped",pending->object,pending->kind,0,0,result,0,0);
        game_surface_sync();game_surface_store(pending->object,s,pending->primary,slot->desc);
        struct GameSurface* surface=game_surface_find(pending->object,0);
        if(surface && surface->primary && surface->pixels.bits==8)game_surface_publish(surface);
    }
    if(slot && result>=0){if(!s->data && !pending->pixels.width)game_session_invalidate(pending->object);game_lock_clear(slot);}
    free_snapshot(s);game_tracker_release();
}
