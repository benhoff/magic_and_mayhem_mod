/* Explicit fresh-session handoff. Caller quiesces application rendering and
 * calls RenderShutdown first. No automatic retry, in-place cursor reset or
 * observer COM calls. All old leases/worker-visible storage must be retired. */
struct CommandCandidate {u32* map;u8* queue;struct CommandFile file;};
static void command_candidate_close(struct CommandCandidate* c){
    if(c->queue)HeapFree(GetProcessHeap(),0,c->queue);
    if(c->map)UnmapViewOfFile(c->map);zero(c,sizeof(*c));
}
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
    u32* p=c->map;
    if(!mnm_ring_identity(p,p[4]) || (command_file_count && p[4]<=command_files[command_file_count-1].session) ||
       mnm_ring_load(p+6)!=0 || mnm_ring_load(p+5) || mnm_ring_load(p+7) || mnm_ring_load(p+8) || mnm_ring_load(p+9))return 0;
    c->file.session=p[4];c->queue=HeapAlloc(GetProcessHeap(),0,COMMAND_QUEUE_CAPACITY);return c->queue!=0;
}
static int command_recovery_leases_quiet(void){
    if(__atomic_load_n(&lock_capture_reserved,__ATOMIC_ACQUIRE))return 0;
    for(u32 i=0;i<32;++i)if(game_locks[i].active || session_surfaces[i].dc_pending)return 0;
    for(u32 i=0;i<GAME_SURFACE_COUNT;++i)if(game_surfaces[i].dc)return 0;
    return 1;
}
static int command_recovery_quiet(void){return !game_session_enabled && command_recovery_leases_quiet();}
static void command_recovery_checkpoints(void){
    /* Epochs/generations remain monotonic. No old CPU checkpoint, alias,
     * palette/property or pending primary identity can seed the new stream. */
    session_archive_close();session_active=0;
    for(u32 i=0;i<32;++i)game_lock_clear(game_locks+i);
    for(u32 i=0;i<GAME_SURFACE_COUNT;++i){game_surface_drop(game_surfaces+i);zero(game_surfaces+i,sizeof(game_surfaces[i]));}
    zero(game_palettes,sizeof(game_palettes));zero(game_aliases,sizeof(game_aliases));
    zero(game_alias_index,sizeof(game_alias_index));game_alias_index_valid=0;game_alias_reset_pending=0;
    zero(game_pixel_misses,sizeof(game_pixel_misses));game_pixel_misses_pending=0;
    game_metadata_invalidate();game_presented_object=0;game_presented_generation=game_presented_frame=0;
    zero(session_surfaces,sizeof(session_surfaces));
    session_started=session_epoch=session_sequence=session_bytes=session_operations=session_presented=session_pixels=0;
    session_archive_sequence=session_archive_bytes=session_last_id=0;++session_archive_id;
}
static u32 command_recover(const char* path,u32 expected_session){
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
       game_surface_generation>0xffffffffu-GAME_SURFACE_COUNT || session_archive_id==0xffffffffu){game_tracker_release();goto done;}
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
    command_recovery_checkpoints();
    command_ring=writer;command_channel=next.map;command_queue=next.queue;
    command_channel_session=writer.session;command_channel_bytes=command_channel_refused=0;
    next.map=0;next.queue=0;
    command_queue_written=command_queue_read=command_queue_end=command_queue_failure=0;
    command_queue_peak=command_queue_full=command_queue_blocked=0;
    command_queue_timeout=command_queue_armed=command_queue_since=command_queue_ack=command_queue_timed_out=0;
    command_queue_configure();command_pressure_logged=0;
    command_worker_started=command_worker_stop=command_worker_joined=command_shutdown_complete=0;
    __sync_lock_release(&command_queue_draining);
    ready=command_scheduler_launch();game_session_enabled=ready;
    if(!ready){command_worker_joined=1;command_channel_close();}
    lock_diagnostic(ready?"command_recovery_started":"command_recovery_failed",0,0,writer.session,session_archive_id,0,0,0);
    game_tracker_release();
 done:if(!ready)lock_diagnostic("command_recovery_refused",0,0,stage,expected_session,command_file_count,command_worker_joined,0);
    command_candidate_close(&next);__sync_lock_release(&command_shutdown_busy);SetLastError(error);return ready;
}
__declspec(dllexport) u32 WIN RenderRecover(const char* path){return command_recover(path,0);}
#ifdef MNM_RENDER_SELFTEST
__declspec(dllexport) u32 WIN RenderRecoveryStateForTest(u32* values){
    u32 error=GetLastError();if(!game_tracker_acquire()){SetLastError(error);return 0;}
    u32 aliases=0,palettes=0;
    for(u32 i=0;i<GAME_ALIAS_COUNT;++i)if(game_aliases[i].from || game_aliases[i].to)++aliases;
    for(u32 i=0;i<32;++i)palettes+=game_palettes[i].count;
    u32 state[8]={game_surface_bytes,lock_capture_reserved,session_started,session_sequence,session_last_id,session_pixels,aliases,palettes};
    copy(values,state,sizeof(state));game_tracker_release();SetLastError(error);return 1;
}
#endif
