/* Only observed two-buffer chains. No COM calls or pixel reads here. */
static u32 game_flip_count,game_flip_bytes;
static void game_attached_observed(void* object,void* back,const u32* caps){
    if(!game_surface_enter())return;
    struct GameSurface* front=game_surface_find(object,1);
    if(front && caps && caps[0]==4 && !(caps[1] || caps[2] || caps[3])){
        front->back=back;front->generation=++game_surface_generation;
        lock_diagnostic("flip_attachment",object,0,(u32)back,0,0,0,0);
    }
    __sync_lock_release(&game_locks_busy);
}
struct GameFlip {void *front,*back;u32 epoch,front_generation,back_generation,valid;};
static void game_flip_before(void* object,void* target,u32 flags,struct GameFlip* pending){
    zero(pending,sizeof(*pending));pending->front=object;
    if(!game_surface_enter())return;
    struct GameSurface* front=game_surface_find(object,0);
    struct GameSurface* back=front && front->back?game_surface_find(front->back,0):0;
    const char* reason="flip_untracked";
    if(!front || !back || front==back || !front->primary || !front->layout_known || !back->pixels.data)goto done;
    reason="flip_unsupported";
    if(flags&~1u || !front->back_count_known || front->back_count!=1 ||
       (front->caps&0x238)!=0x238 || (back->caps&0x1c)!=0x1c ||
       (target && !game_alias_same(target,back->object)))goto done;
    struct Snapshot *a=&front->pixels,*b=&back->pixels;
    if(a->width!=b->width || a->height!=b->height || a->bits!=b->bits || a->r!=b->r || a->g!=b->g || a->b!=b->b)goto done;
    for(u32 i=0;i<32;++i)if(game_locks[i].active && game_locks[i].epoch==__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED) &&
        (game_alias_same(game_locks[i].object,object) || game_alias_same(game_locks[i].object,back->object)))goto done;
    if(game_flip_count>=16){reason="flip_limit";goto done;}
    pending->back=back->object;pending->epoch=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED);
    pending->front_generation=front->generation;pending->back_generation=back->generation;
    pending->valid=1;reason="flip_ready";
 done:lock_diagnostic(reason,object,0,(u32)target,flags,0,0,0);__sync_lock_release(&game_locks_busy);
}
/* The swap is already committed. Both pre-swap inputs remain owned: the old
 * front is now back and the old back is now front. Palettes stay on identities. */
static void game_flip_commands(struct GameSurface* front,struct GameSurface* back){
    if(front->pixels.bits!=8 || !front->pixels.data || !back->pixels.data)return;
    struct Snapshot before;copy(&before,&back->pixels,sizeof(before));
    if(!game_surface_colors(front,before.palette))return;
    u32 length=before.length+front->pixels.length*2+1280;
    if(length>GAME_SURFACE_LIMIT-game_flip_bytes)return;
    char path[544];copy(path,lock_capture_path,lock_capture_path_length);char* tail=path+lock_capture_path_length;
    copy(tail,"\\flip-",6);failure_hex(tail+6,game_flip_count);copy(tail+14,".bin",5);
    HANDLE file=CreateFileA(path,0x40000000,1,0,1,0x80,0);u32 sequence=0,header[4],ids[2]={1,2};
    copy(header,"MNMCMD01",8);header[2]=1;header[3]=16;
    int ok=file!=(HANDLE)-1 && write_all(file,header,16) && command_create(file,&sequence,1,&before) &&
        command_create_native(file,&sequence,2,&front->pixels) && command_record(file,&sequence,11,ids,8,0,0) &&
        command_record(file,&sequence,5,ids,4,front->pixels.data,front->pixels.length) && command_record(file,&sequence,6,ids,4,0,0) &&
        command_record(file,&sequence,7,ids,4,0,0) && command_record(file,&sequence,7,ids+1,4,0,0) && command_record(file,&sequence,8,0,0,0,0);
    if(file!=(HANDLE)-1)CloseHandle(file);game_flip_bytes+=length;
    lock_diagnostic(ok?"flip_recorded":"flip_file_failed",front->object,0,(u32)back->object,0,0,0,0);
}
static void game_flip_after(struct GameFlip* pending,i32 result){
    if(!lock_capture_path_length)return;
    if(result<0){lock_diagnostic("flip_failed",pending->front,0,0,0,result,0,0);return;}
    if(!game_surface_enter())return;
    struct GameSurface *front=game_surface_find(pending->front,0),*back=game_surface_find(pending->back,0);
    if(!pending->valid || game_flip_count>=16 || !front || !back ||
       pending->epoch!=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED) ||
       pending->front_generation!=front->generation || pending->back_generation!=back->generation){
        __atomic_add_fetch(&game_lock_epoch,1,__ATOMIC_RELAXED);game_surface_sync();
        lock_diagnostic("flip_invalidated",pending->front,0,0,0,result,0,0);
    }else{
        game_session_flip_begin(front,back);
        struct Snapshot old;copy(&old,&front->pixels,sizeof(old));copy(&front->pixels,&back->pixels,sizeof(old));copy(&back->pixels,&old,sizeof(old));
        front->generation=++game_surface_generation;back->generation=++game_surface_generation;++game_flip_count;
        game_flip_commands(front,back);game_session_flip_end(front,back);
        lock_diagnostic(game_surface_publish(front)?"flip_presented":"flip_presentation_skipped",pending->front,0,(u32)pending->back,0,result,0,0);
    }
    __sync_lock_release(&game_locks_busy);
}
