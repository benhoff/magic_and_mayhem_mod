/* Observe game-owned full-surface locks. Never Lock, Unlock, Query or retain COM objects. */
struct GameLock {void* object;u32 owner,generation,epoch,active,kind,flags,desc[31];};
static struct GameLock game_locks[32];
static volatile i32 game_locks_busy;
static u32 game_lock_epoch,game_lock_generation,lock_capture_count,lock_capture_bytes,lock_capture_reserved;
static char lock_capture_path[512];
static u32 lock_capture_path_length;
static void init_lock_lifecycle(void){
    u32 length=GetEnvironmentVariableA("MNM_RENDER_LOCK_CAPTURE_DIR",lock_capture_path,sizeof(lock_capture_path));
    if(length && length+28<sizeof(lock_capture_path))lock_capture_path_length=length;
}
#include "lock_diagnostics.h"
#include "lock_aliases.h"
static void game_lock_retire(void* object){
    if(!lock_capture_path_length)return;
    if(!__sync_bool_compare_and_swap(&game_locks_busy,0,1)){__atomic_store_n(&game_alias_reset_pending,1,__ATOMIC_RELEASE);__atomic_add_fetch(&game_lock_epoch,1,__ATOMIC_RELAXED);return;}
    game_alias_sync();
    /* Final Release invalidates the whole observed interface component. */
    __atomic_add_fetch(&game_lock_epoch,1,__ATOMIC_RELAXED);
    for(u32 i=0;i<32;++i)if(game_alias_same(game_locks[i].object,object))zero(game_locks+i,sizeof(game_locks[i]));
    game_alias_retire(object);
    __sync_lock_release(&game_locks_busy);
}
static void game_lock_observed(void* object,struct Table* table,void* rect,const u32* desc,u32 flags,i32 result){
    if(!lock_capture_path_length)return;
    if(result<0){lock_diagnostic("lock_failed",object,table->kind,(u32)rect,flags,result,0,0);return;}
    if(!__sync_bool_compare_and_swap(&game_locks_busy,0,1)){__atomic_add_fetch(&game_lock_epoch,1,__ATOMIC_RELAXED);return;}
    game_alias_sync();
    struct GameLock* slot=0;
    for(u32 i=0;i<32;++i)if(game_alias_same(game_locks[i].object,object)){slot=game_locks+i;break;}
    if(!slot)for(u32 i=0;i<32;++i)if(!game_locks[i].active || game_locks[i].epoch!=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED)){slot=game_locks+i;break;}
    const char* reason="lock_capacity";u32 diagnostic_desc[31];zero(diagnostic_desc,sizeof(diagnostic_desc));
    if(slot){
        zero(slot,sizeof(*slot));slot->object=object;slot->generation=++game_lock_generation;
        /* Replacing an earlier record always invalidates its pointer, even for unsupported locks. */
        u32 size=table->kind>=14?124:108;
        int valid=readable(desc,size);if(valid)copy(diagnostic_desc,desc,size);
        reason=rect?"lock_partial":flags&0x10?"lock_readonly":flags&~0x4831u?"lock_flags":!valid || desc[0]!=size?"lock_descriptor":"lock_accepted";
        if(!rect && !(flags&0x10) && !(flags&~0x4831u) && valid && desc[0]==size){
            slot->epoch=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED);slot->owner=GetCurrentThreadId();slot->kind=table->kind;slot->flags=flags;
            copy(slot->desc,desc,size);slot->active=1;
        }
    }
    lock_diagnostic(reason,object,table->kind,(u32)rect,flags,result,0,diagnostic_desc);
    __sync_lock_release(&game_locks_busy);
}
struct GameUnlock {struct Snapshot pixels;void* object;u32 generation,owner,kind,flags,primary;};
static void game_unlock_before(void* object,u32 unlock_kind,void* argument,struct GameUnlock* pending){
    zero(pending,sizeof(*pending));
    if(!lock_capture_path_length)return;
    if(!__sync_bool_compare_and_swap(&game_locks_busy,0,1)){__atomic_add_fetch(&game_lock_epoch,1,__ATOMIC_RELAXED);return;}
    game_alias_sync();
    const char* reason="unlock_unmatched";struct GameLock* matched=0;
    for(u32 i=0;i<32;++i){struct GameLock* slot=game_locks+i;
        if(!slot->active || !game_alias_same(slot->object,object) || slot->epoch!=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED))continue;
        matched=slot;pending->object=slot->object;pending->generation=slot->generation;
        if(slot->owner!=GetCurrentThreadId()){reason="unlock_owner";break;}
        if(unlock_kind>=14?argument!=0:argument!=(void*)slot->desc[9]){reason="unlock_argument";break;}
        u32* d=slot->desc;u32 width=d[3],height=d[2],bits=d[21];i32 pitch=(i32)d[4];
        if(!width || width>2048 || !height || height>2048 || !supported_format(d) || pitch==(-2147483647-1)){reason="unlock_layout";break;}
        if(bits!=8 && (!render_mask(d[22],bits) || !render_mask(d[23],bits) || !render_mask(d[24],bits) ||
           (d[22]&d[23]) || (d[22]&d[24]) || (d[23]&d[24]))){reason="unlock_masks";break;}
        u32 stride=width*(bits/8),magnitude=(u32)(pitch<0?-pitch:pitch),offset=(height-1)*magnitude,at=d[9];
        if(magnitude<stride || magnitude>32768 || (pitch<0 && at<offset) ||
           !readable((void*)(pitch<0?at-offset:at),offset+stride)){reason="unlock_memory";break;}
        if(lock_capture_count>=16 || stride*height>64*1024*1024-lock_capture_bytes-__atomic_load_n(&lock_capture_reserved,__ATOMIC_RELAXED)){reason="unlock_limit";break;}
        struct Snapshot* s=&pending->pixels;s->data=HeapAlloc(GetProcessHeap(),0,stride*height);if(!s->data){reason="unlock_allocation";break;}
        __atomic_add_fetch(&lock_capture_reserved,stride*height,__ATOMIC_RELAXED);
        s->width=width;s->height=height;s->bits=bits;s->flags=d[19];s->r=d[22];s->g=d[23];s->b=d[24];s->length=stride*height;
        for(u32 y=0;y<height;++y)copy(s->data+y*stride,(u8*)at+(i32)y*pitch,stride);
        pending->owner=slot->owner;pending->kind=slot->kind;pending->flags=slot->flags;
        pending->primary=(d[1]&1) && (d[26]&0x200);reason="unlock_copied";break;
    }
    lock_diagnostic(reason,object,matched?matched->kind:0,(u32)argument,matched?matched->flags:0,0,matched?matched->owner:0,matched?matched->desc:0);
    __sync_lock_release(&game_locks_busy);
}
static void game_unlock_after(struct GameUnlock* pending,i32 result){
    if(!lock_capture_path_length)return;
    lock_diagnostic(result<0?"unlock_failed":"unlock_succeeded",pending->object,pending->kind,0,pending->flags,result,pending->owner,0);
    if(!pending->object){if(result>=0)__atomic_add_fetch(&game_lock_epoch,1,__ATOMIC_RELAXED);return;}
    if(!__sync_bool_compare_and_swap(&game_locks_busy,0,1)){__atomic_add_fetch(&game_lock_epoch,1,__ATOMIC_RELAXED);if(pending->pixels.data)__atomic_sub_fetch(&lock_capture_reserved,pending->pixels.length,__ATOMIC_RELAXED);free_snapshot(&pending->pixels);return;}
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
            if(file!=(HANDLE)-1){u32 header[16];copy(header,"MNMLOCK1",8);
                header[2]=1;header[3]=64;header[4]=id;header[5]=(u32)pending->object;header[6]=pending->owner;
                header[7]=pending->kind;header[8]=pending->flags;header[9]=s->width;header[10]=s->height;header[11]=s->bits;
                header[12]=s->r;header[13]=s->g;header[14]=s->b;header[15]=s->length;
                if(write_all(file,header,64))write_all(file,s->data,s->length);CloseHandle(file);
            }
            /* Only directly locked primary RGB surfaces are presentation frames. No offscreen guesses. */
            if(pending->primary && s->bits!=8 && __sync_bool_compare_and_swap(&capture_busy,0,1)){
                u32 sequence=__atomic_load_n(stream+4,__ATOMIC_RELAXED);
                __atomic_store_n(stream+4,sequence+1,__ATOMIC_SEQ_CST);
                if(render_pixels((u8*)stream+64,s->width,s->height,s->data,(i32)(s->width*(s->bits/8)),s->bits,s->r,s->g,s->b,0)){
                    stream[5]=s->width;stream[6]=s->height;stream[7]=s->width*4;stream[8]=1;++stream[10];stream[9]=1;
                }
                __atomic_store_n(stream+4,sequence+2,__ATOMIC_RELEASE);__sync_lock_release(&capture_busy);
            }
        }
    }
    free_snapshot(s);__sync_lock_release(&game_locks_busy);
}
