/* Fresh-session handoff after shutdown/join. Explicit recovery discards pixels;
 * administrative checkpoint attachment preserves only independently owned,
 * complete state behind the exclusive callback gate. No observer COM calls. */
struct CommandCandidate {u32* map;u8* queue;struct CommandFile file;};
static void command_candidate_close(struct CommandCandidate* c){
    if(c->queue)HeapFree(GetProcessHeap(),0,c->queue);
    if(c->map)UnmapViewOfFile(c->map);zero(c,sizeof(*c));
}
#ifdef MNM_RENDER_SELFTEST
static int command_recovery_fault(const char* fault){
    char value[32];u32 n=GetEnvironmentVariableA("MNM_RENDER_RECOVERY_FAULT_FOR_TEST",value,sizeof(value)),length=0;
    while(fault[length])++length;return n==length && same(value,fault,length);
}
#endif
static int command_candidate_open(struct CommandCandidate* c,const char* path){
    u32 length=0;
    if(!path)return 0;
    for(;length<512;++length){if(!readable(path+length,1))return 0;if(!path[length])break;}
    if(!length || length==512)return 0;
    HANDLE file=CreateFileA(path,0xc0000000,3,0,3,0x80,0);
    if(file==(HANDLE)-1)return 0;
    u32 size=GetFileSize(file,0);
    if(size!=MNM_RENDER_COMMANDS_V2_SIZE || !command_file_identity(file,&c->file)){CloseHandle(file);return 0;}
    for(u32 i=0;i<command_file_count;++i)if(command_files[i].volume==c->file.volume &&
       command_files[i].high==c->file.high && command_files[i].low==c->file.low){CloseHandle(file);return 0;}
    HANDLE mapping=CreateFileMappingA(file,0,4,0,size,0);CloseHandle(file);if(!mapping)return 0;
    c->map=MapViewOfFile(mapping,2,0,0,size);CloseHandle(mapping);if(!c->map)return 0;
    #ifdef MNM_RENDER_SELFTEST
    if(command_recovery_fault("mapping"))return 0;
#endif
    u32* p=c->map;
    if(!mnm_ring_identity(p,p[4]) || (command_file_count && p[4]<=command_files[command_file_count-1].session) ||
       mnm_ring_load(p+6)!=0 || mnm_ring_load(p+5) || mnm_ring_load(p+7) || mnm_ring_load(p+8) || mnm_ring_load(p+9))return 0;
    c->file.session=p[4];
#ifdef MNM_RENDER_SELFTEST
    if(command_recovery_fault("allocation"))return 0;
#endif
    c->queue=HeapAlloc(GetProcessHeap(),0,COMMAND_QUEUE_CAPACITY);return c->queue!=0;
}
static int command_recovery_leases_quiet(void){
    if(__atomic_load_n(&lock_capture_reserved,__ATOMIC_ACQUIRE))return 0;
    for(u32 i=0;i<32;++i)if(game_locks[i].active || session_surfaces[i].dc_pending)return 0;
    for(u32 i=0;i<GAME_SURFACE_COUNT;++i)if(game_surfaces[i].dc)return 0;
    return 1;
}
static int command_recovery_quiet(void){return !game_session_enabled && command_recovery_leases_quiet();}
/* Called with the tracker and callback gate closed. Do not synchronize away
 * uncertainty: every observed resource must already have complete owned state. */
static int command_checkpoint_reject(u32 reason,struct GameSurface* s,u32 surfaces,u32 bytes){
    u32 v[19]={reason,s?(u32)s->object:0,surfaces,bytes,game_lock_epoch,game_metadata_epoch,
        s?s->epoch:0,s?s->metadata_epoch:0,s?s->layout_known:0,s?s->pixels.data!=0:0,
        s?s->pixels.width:0,s?s->pixels.height:0,s?s->pixels.bits:0,s?s->pixels.length:0,
        s?s->primary:0,game_pixel_misses_pending,game_alias_reset_pending};
    lock_diagnostic_values("checkpoint_admission_refused",v);return 0;
}
static int command_checkpoint_complete(void){
    if(game_pixel_misses_pending || game_alias_reset_pending)return command_checkpoint_reject(1,0,0,0);
    for(u32 i=0;i<128;++i)if(game_pixel_misses[i])return command_checkpoint_reject(1,0,0,0);
    u32 surfaces=0,pixels=0,bytes=16+12,primary=0;
    for(u32 i=0;i<GAME_SURFACE_COUNT;++i){struct GameSurface* s=game_surfaces+i;if(!s->object)continue;
        struct Snapshot* p=&s->pixels;
        if(++surfaces>GAME_SURFACE_COUNT || s->epoch!=game_lock_epoch || s->metadata_epoch!=game_metadata_epoch ||
           !s->layout_known || !p->data || !p->width || p->width>2048 || !p->height || p->height>2048 ||
           (p->bits!=8 && p->bits!=16 && p->bits!=24 && p->bits!=32) ||
           p->length!=p->width*p->height*(p->bits/8))return command_checkpoint_reject(2,s,surfaces,bytes);
        for(u32 j=0;j<i;++j)if(game_surfaces[j].object && game_alias_same(s->object,game_surfaces[j].object))return command_checkpoint_reject(3,s,surfaces,bytes);
        u32 count=p->width*p->height;if(count>16777216-pixels)return command_checkpoint_reject(4,s,surfaces,bytes);pixels+=count;
        if(bytes>COMMAND_QUEUE_CAPACITY-40 || p->length>COMMAND_QUEUE_CAPACITY-40-bytes)return command_checkpoint_reject(5,s,surfaces,bytes);bytes+=40+p->length;
        if(p->bits==8){if(!game_palette_complete(game_palette_find(s->palette,0)))return command_checkpoint_reject(6,s,surfaces,bytes);
            u32 colors=game_session_palette_resources?24:792;
            if(bytes>COMMAND_QUEUE_CAPACITY-colors)return command_checkpoint_reject(5,s,surfaces,bytes);bytes+=colors;}
        if(s->primary)++primary;
    }
    if(primary!=1)return command_checkpoint_reject(7,0,surfaces,bytes);
    for(u32 i=0;i<32;++i){struct GamePalette* p=game_palettes+i;if(!p->count)continue;
        if(p->epoch!=game_lock_epoch || !game_palette_complete(p))return command_checkpoint_reject(6,0,surfaces,bytes);
        if(game_session_palette_resources){if(bytes>COMMAND_QUEUE_CAPACITY-788)return command_checkpoint_reject(5,0,surfaces,bytes);bytes+=788;}
    }
    return bytes<=COMMAND_QUEUE_CAPACITY-session_cleanup_bytes()?1:command_checkpoint_reject(5,0,surfaces,bytes);
}
static int command_checkpoint_publish(void){
    if(!session_start())return 0;
    if(game_session_palette_resources)for(u32 i=0;i<32;++i)if(game_palettes[i].count && !session_palette_resource(game_palettes+i))return 0;
    /* All observed CPU resources were validated above and remain independently
     * owned. Materialize the complete primary before READY; subsequent commands
     * admit complete dependency baselines lazily into the same finite cache. */
    struct SessionSurface* primary=0;
    for(u32 i=0;i<GAME_SURFACE_COUNT;++i){struct GameSurface* s=game_surfaces+i;
        if(!s->object || !s->primary)continue;
        primary=session_surface(s->object,&s->pixels,0);
        if(!primary || !session_colors(primary))return 0;
    }
    return session_present(primary);
}
static void command_recovery_checkpoints(int preserve){
    /* Epochs/generations remain monotonic. Strict attachment keeps verified
     * CPU state; ordinary recovery requires fresh pixels. Wire resources
     * and pending presentation identity are always reset. */
    session_archive_close();session_active=0;
    for(u32 i=0;i<32;++i)game_lock_clear(game_locks+i);
    if(!preserve){
    /* Publication failure does not end application object lifetimes. Synchronize
     * independently observed invalidations, then discard pixels while keeping
     * current descriptors/properties and verified interface relationships. No
     * application QI/GetDesc is guaranteed to repeat after a consumer failure. */
    game_surface_sync();
    for(u32 i=0;i<GAME_SURFACE_COUNT;++i)if(game_surfaces[i].object)game_surface_drop(game_surfaces+i);
    for(u32 i=0;i<32;++i){
        if(game_palettes[i].count && game_palettes[i].epoch!=game_lock_epoch)zero(game_palettes+i,sizeof(game_palettes[i]));
        else game_palettes[i].session=0;
    }
    }else for(u32 i=0;i<32;++i)game_palettes[i].session=0;
    game_presented_object=0;game_presented_generation=game_presented_frame=0;
    zero(session_surfaces,sizeof(session_surfaces));zero(session_palettes,sizeof(session_palettes));session_last_palette_id=0;
    session_started=session_epoch=session_sequence=session_bytes=session_operations=session_presented=session_pixels=0;
    session_archive_sequence=session_archive_bytes=session_last_id=0;++session_archive_id;
}
enum { COMMAND_RECOVER_FRESH=0, COMMAND_RECOVER_CHECKPOINT=1, COMMAND_RECOVER_PREFER_CHECKPOINT=2 };
static u32 command_recover_mode(const char* path,u32 expected_session,int checkpoint){
    u32 error=GetLastError(),ready=0,stage=1;struct CommandCandidate next;zero(&next,sizeof(next));
    if(!__sync_bool_compare_and_swap(&command_shutdown_busy,0,1)){SetLastError(error);return 0;}
    if(!stream || !game_session_continuous || !lock_capture_path_length || !command_worker_joined ||
       command_worker || command_queue || command_channel || command_file_count>=16 ||
       (command_auto_shutdown() && !command_exit_installed))goto done;
    stage=2;
    if(!command_candidate_open(&next,path) || (expected_session && next.file.session!=expected_session))goto done;
    stage=3;
    if(!game_tracker_acquire())goto done;
    stage=4;
    if(!command_recovery_quiet() || game_lock_epoch==0xffffffffu || game_metadata_epoch==0xffffffffu ||
       game_surface_generation>0xffffffffu-GAME_SURFACE_COUNT || session_archive_id==0xffffffffu || (checkpoint==COMMAND_RECOVER_CHECKPOINT && !command_checkpoint_complete())){game_tracker_release();goto done;}
    /* Select once behind exclusive admission. Incomplete state can use fresh
     * observations; a failed claimed checkpoint must never retry that same file. */
    if(checkpoint==COMMAND_RECOVER_PREFER_CHECKPOINT){
        checkpoint=command_checkpoint_complete()?COMMAND_RECOVER_CHECKPOINT:COMMAND_RECOVER_FRESH;
        lock_diagnostic("command_recovery_policy",0,0,checkpoint,expected_session,0,0,0);
    }
    /* Join retired the old worker; also exclude callback-side idle pumps while
     * resetting host cursors and publishing the new mapping/queue pair. */
    if(!__sync_bool_compare_and_swap(&command_queue_draining,0,1)){game_tracker_release();goto done;}
    /* Bind only after all admission checks. If the peer changes the candidate
     * before this CAS, nothing in the old producer is reset. */
    stage=5;
    struct mnm_ring_writer writer;
    if(!mnm_ring_writer_bind(&writer,next.map,MNM_RENDER_COMMANDS_V2_SIZE)){
        __sync_lock_release(&command_queue_draining);game_tracker_release();goto done;
    }
    if(writer.session!=next.file.session){
        /* A peer changed identity during admission. Spend this claimed file
         * and the highest observed ID even though no new producer is enabled. */
        u32 previous=command_file_count?command_files[command_file_count-1].session:0;
        next.file.session=writer.session>previous?writer.session:previous;
        command_files[command_file_count++]=next.file;
        mnm_ring_fail(&writer,MNM_RENDER_COMMANDS_V2_REASON_INVALID);
        __sync_lock_release(&command_queue_draining);game_tracker_release();goto done;
    }
    command_files[command_file_count++]=next.file;
    command_recovery_checkpoints(checkpoint);
    command_ring=writer;command_channel=next.map;command_queue=next.queue;
    command_channel_session=writer.session;command_channel_bytes=command_channel_refused=0;
    next.map=0;next.queue=0;
    command_queue_written=command_queue_read=command_queue_end=command_queue_failure=0;
    command_queue_peak=command_queue_full=command_queue_blocked=0;
    command_queue_timeout=command_queue_armed=command_queue_since=command_queue_ack=command_queue_timed_out=0;
    command_queue_configure();command_pressure_logged=0;
    command_worker_started=command_worker_stop=command_worker_joined=command_shutdown_complete=0;
    __sync_lock_release(&command_queue_draining);
    game_session_enabled=1;
    ready=!checkpoint || command_checkpoint_publish();
    if(ready)ready=command_scheduler_launch();
    game_session_enabled=ready;
    /* A launched worker can observe cancellation before launch returns. Keep
     * its storage until the following normal shutdown actually joins it. */
    if(!ready && !command_worker){command_worker_joined=1;command_channel_close();}
    lock_diagnostic(ready?"command_recovery_started":"command_recovery_failed",0,0,writer.session,session_archive_id,0,0,0);
    game_tracker_release();
 done:if(!ready)lock_diagnostic("command_recovery_refused",0,0,stage,expected_session,command_file_count,command_worker_joined,0);
    command_candidate_close(&next);__sync_lock_release(&command_shutdown_busy);SetLastError(error);return ready;
}
static u32 command_recover(const char* path,u32 expected_session){return command_recover_mode(path,expected_session,0);}
__declspec(dllexport) u32 WIN RenderRecover(const char* path){return command_recover(path,0);}
#ifdef MNM_RENDER_SELFTEST
__declspec(dllexport) u32 WIN RenderRecoveryStorageForTest(u32* values){
    u32 error=GetLastError();if(!game_tracker_acquire()){SetLastError(error);return 0;}
    u32 state[6]={command_worker!=0,command_queue!=0,command_channel!=0,command_worker_joined,game_surface_bytes,lock_capture_reserved};
    copy(values,state,sizeof(state));game_tracker_release();SetLastError(error);return 1;
}
__declspec(dllexport) u32 WIN RenderRecoveryStateForTest(u32* values){
    u32 error=GetLastError();if(!game_tracker_acquire()){SetLastError(error);return 0;}
    u32 aliases=0,palettes=0;
    for(u32 i=0;i<GAME_ALIAS_COUNT;++i)if(game_aliases[i].from || game_aliases[i].to)++aliases;
    for(u32 i=0;i<32;++i)palettes+=game_palettes[i].count;
    u32 state[8]={game_surface_bytes,lock_capture_reserved,session_started,session_sequence,session_last_id,session_pixels,aliases,palettes};
    copy(values,state,sizeof(state));game_tracker_release();SetLastError(error);return 1;
}
#endif
