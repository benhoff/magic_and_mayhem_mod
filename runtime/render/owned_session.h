#include "../../protocols/include/mnm/render_stream_v5.h"
#include "dirty_region.h"
#include "gpu_copy_commands.h"
#include "../../protocols/include/mnm/resident_frame_budget.h"
/* Ordered opt-in production, independent of bounded diagnostic archives.
 * Every entry point holds game_locks_busy; pixels are owned snapshots only. */
struct SessionSurface {void* object;u32 id,width,height,bits,r,g,b,palette_known,dc_pending,palette_id,palette_generation,used;u8 palette[1024];};
static struct SessionSurface session_surfaces[32];
struct SessionPalette {u32 id,generation;u8 colors[1024];};
static struct SessionPalette session_palettes[32];
static u32 session_last_palette_id;
static u32 session_cleanup_bytes(void){return game_session_palette_resources?1180:540;}
static u32 session_cleanup_records(void){return game_session_palette_resources?66:34;}
static HANDLE session_file;
static u32 session_active,session_started,session_epoch,session_sequence,session_bytes,session_operations,session_presented,session_pixels;
static u32 session_archive_sequence,session_archive_bytes;
/* A slot is storage, not identity. Continuous CREATE IDs never repeat, even
 * when a final Release frees a slot or the application reuses a COM address. */
static u32 session_last_id;
static u32 session_archive_id=1;
static void session_archive_close(void){if(session_file)CloseHandle(session_file);session_file=0;}
static void session_archive_gap(u32 reason){
    if(!session_file)return;
    u32 record[4]={9,++session_archive_sequence,4,reason};write_all(session_file,record,16);
    session_archive_close();lock_diagnostic("session_archive_stopped",0,0,reason,0,0,0,0);
}
static void game_session_gap_at(u32 reason,const char* file,u32 line){
    command_channel_fail(MNM_RENDER_COMMANDS_V1_REASON_GAP);
    if(!session_active)return;
    session_archive_gap(reason);session_active=0;
    lock_diagnostic("session_gap",0,0,reason,game_tracker_site(file,line),0,line,0);
}
static void game_session_sync(void){
    const u32 epoch=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED);
    if(session_active && session_epoch!=epoch){
        // A bounded failure record identifies the last metadata invalidator.
        // A mismatching tag epoch is explicitly inconclusive under concurrency.
        u32 tagged=__atomic_load_n(&game_metadata_invalid_epoch,__ATOMIC_ACQUIRE);
        u32 values[19]={session_epoch,epoch,tagged,
            __atomic_load_n(&game_metadata_invalid_site,__ATOMIC_RELAXED),
            __atomic_load_n(&game_metadata_invalid_thread,__ATOMIC_RELAXED),
            GetCurrentThreadId(),__atomic_load_n(&game_metadata_epoch,__ATOMIC_RELAXED)};
        lock_diagnostic_values("session_epoch_origin",values);game_session_gap(1);
    }
}
#include "session_emission.h"
static int session_start(void){
    if(__atomic_load_n(&command_application_stop_requested,__ATOMIC_ACQUIRE) || !game_session_enabled || !lock_capture_path_length)return 0;
    if(session_started)return session_active!=0;
    session_started=1;session_epoch=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED);
    /* Continuous publication requires a successfully claimed v2/v3 queue. Never
     * downgrade to an archive-only or append-only stream. Recovery is separate. */
    if(game_session_continuous && (!command_queue || command_channel_refused)){
        command_channel_fail(MNM_RENDER_COMMANDS_V1_REASON_INVALID);
        lock_diagnostic("session_continuous_requires_ring",0,0,0,0,0,0,0);return 0;
    }
    u32 header[4];copy(header,game_resident_world?MNM_RENDER_STREAM_V5_MAGIC:game_session_palette_resources?MNM_RENDER_STREAM_V2_MAGIC:"MNMCMD01",8);header[2]=game_resident_world?5:game_session_palette_resources?2:1;header[3]=16;
    if(game_session_archive){
        char path[544];copy(path,lock_capture_path,lock_capture_path_length);copy(path+lock_capture_path_length,"\\session-00000001.bin",22);failure_hex(path+lock_capture_path_length+9,session_archive_id);
        session_file=CreateFileA(path,0x40000000,1,0,1,0x80,0);
        if(session_file==(HANDLE)-1)session_file=0;
        if(!session_file || !write_all(session_file,header,16)){
            session_archive_close();lock_diagnostic("session_file_failed",0,0,0,0,0,0,0);
            if(!game_session_continuous){command_channel_fail(MNM_RENDER_COMMANDS_V1_REASON_INVALID);return 0;}
        }else session_archive_bytes=16;
    }
    if(!command_channel_append(header,16,0,0,0,0)){
        command_channel_fail(MNM_RENDER_COMMANDS_V1_REASON_INVALID);session_archive_close();return 0;
    }
    session_bytes=16;session_active=1;
    lock_diagnostic("session_started",0,0,game_session_continuous,game_session_archive,0,0,0);return 1;
}
static struct SessionSurface* session_find(void* object){
    struct SessionSurface* found=0;
    for(u32 i=0;i<32;++i)if(session_surfaces[i].id && game_alias_same(object,session_surfaces[i].object)){
        if(found){game_session_gap(4);return 0;}found=session_surfaces+i;
    }
    if(found)found->used=session_sequence;return found;
}
static void game_session_invalidate(void* object){if(session_active && session_find(object))game_session_gap(3);}
static void game_session_retire(void* object){
    if(!session_active)return;
    if(!game_session_continuous){game_session_invalidate(object);return;}
    struct SessionSurface* s=session_find(object);if(!s || !session_active)return;
    /* Final Release supplies an opaque identity token only. Never dereference
     * the destroyed object; alias edges remain available until cleanup below.
     * Pending borrowed work cannot be turned into a successful retirement. */
    if(s->dc_pending){game_session_gap(3);return;}
    for(u32 i=0;i<32;++i)if(game_locks[i].active && game_alias_same(object,game_locks[i].object)){
        game_session_gap(3);return;
    }
    if(!session_record(7,&s->id,4,0,0))return;
    session_pixels-=s->width*s->height;zero(s,sizeof(*s));
}
/* The last native checkpoint remains allocated, but cannot be consumed while
 * the application owns its DC. Only an admitted successful release resumes it. */
static void game_session_dc_acquire(void* object){
    if(!session_active)return;
    struct SessionSurface* s=session_find(object);if(!s)return;
    for(u32 i=0;i<32;++i)if(game_locks[i].active && game_alias_same(object,game_locks[i].object)){game_session_gap(3);return;}
    if(s->dc_pending){game_session_gap(3);return;}s->dc_pending=1;
}
/* Consumer residency is separate from verified application lifetime and owned
 * CPU storage. Queued commands own their bytes and DELETE remains FIFO ordered.
 * Never evict a primary, borrowed resource or the other operand of this command. */
static struct SessionSurface* session_victim(void* keep){
    struct SessionSurface* victim=0;
    for(u32 i=0;i<32;++i){struct SessionSurface* s=session_surfaces+i;
        if(!s->id || s->dc_pending || (keep && game_alias_same(keep,s->object)))continue;
        struct GameSurface* owned=game_surface_find(s->object,0);
        if(!owned || owned->primary || owned->dc || owned->gpu_history)continue;
        u32 borrowed=0;
        for(u32 j=0;j<32;++j)if(game_locks[j].active && game_alias_same(s->object,game_locks[j].object)){borrowed=1;break;}
        if(!borrowed && (!victim || s->used<victim->used))victim=s;
    }
    return victim;
}
static struct SessionSurface* session_surface(void* object,const struct Snapshot* pixels,void* keep){
    if(!session_start())return 0;
    struct SessionSurface* s=session_find(object);
    if(!pixels->data){game_session_gap(4);return 0;}
    if(s){
        if(s->dc_pending){game_session_gap(3);return 0;}
        if(s->width!=pixels->width || s->height!=pixels->height || s->bits!=pixels->bits ||
           s->r!=pixels->r || s->g!=pixels->g || s->b!=pixels->b){game_session_gap(3);return 0;}
        return s;
    }
    struct GameSurface* owned=game_surface_find(object,0);
    if(owned && owned->gpu_history){game_session_gap(4);return 0;}
    const u32 pixel_limit=mnm_resident_source_pixel_limit(game_prepared_world);
    u32 count=pixels->width*pixels->height;
    if(count>pixel_limit || session_last_id==0xffffffffu){game_session_gap(2);return 0;}
    for(;;){
        s=0;for(u32 i=0;i<32;++i)if(!session_surfaces[i].id){s=session_surfaces+i;break;}
        if(s && count<=pixel_limit-session_pixels)break;
        struct SessionSurface* victim=game_session_continuous?session_victim(keep):0;
        if(!victim){lock_diagnostic("session_cache_admission_refused",object,0,count,session_pixels,0,0,0);game_session_gap(2);return 0;}
        u32 old_id=victim->id;
        if(!session_record(7,&old_id,4,0,0))return 0;
        lock_diagnostic("session_cache_evicted",victim->object,0,old_id,session_pixels,0,0,0);
        session_pixels-=victim->width*victim->height;zero(victim,sizeof(*victim));
    }
    s->object=object;s->id=++session_last_id;
    s->width=pixels->width;s->height=pixels->height;s->bits=pixels->bits;
    s->r=pixels->r;s->g=pixels->g;s->b=pixels->b;s->used=session_sequence;session_pixels+=count;
    u32 fields[7]={s->id,s->width,s->height,s->bits,s->r,s->g,s->b};
    if(!session_record(1,fields,28,pixels->data,pixels->length))return 0;return s;
}
static i32 (WIN *game_cpu_history)(const void*,u32,u32,u32,u32,u32);
static i32 (WIN *game_native_cpu_transfer)(void*,u32*,const u16*,u32);
static i32 (WIN *game_native_cpu_snapshot)(void*,u32*,u8*,u32);
static i32 (WIN *game_native_cpu_lease_snapshot)(void*,const u32*,u16*,u32);
static i32 (WIN *game_native_cpu_lease_transfer)(void*,u32*,const u16*,u32);
static i32 (WIN *game_native_cpu_lease_mask)(void*,const u32*,u8*,u32,u32);
__declspec(dllexport) i32 WIN RenderRequireCpuHistoryV1(void*,const void*,u32,u32,u32);
#include "../native-draw/initial_history.h"
/* Called exactly at successful CreateSurface return, before the fresh result
 * escapes the ordered copy lease. Driver-backed surfaces have no such service. */
static void game_native_primary_initial(struct GameSurface* surface,const u32* descriptor){
    if(!game_resident_world||!game_native_cpu_transfer||!descriptor||
       !readable(descriptor,108)||(descriptor[0]!=108&&descriptor[0]!=124)||
       !(descriptor[1]&1)||descriptor[26]!=0x200||(descriptor[1]&0x20)||
       surface->pixels.data||surface->gpu_history)return;
    u32 identity[8];
    if(game_native_cpu_snapshot && !game_native_cpu_snapshot(surface->object,identity,0,0) && identity[4]!=16){
        if(identity[1]<1 || identity[1]>2048 || identity[2]<1 || identity[2]>2048 ||
           (identity[4]!=8 && identity[4]!=24 && identity[4]!=32))return;
        struct Snapshot shape;zero(&shape,sizeof(shape));shape.width=identity[1];shape.height=identity[2];
        shape.bits=identity[4];shape.flags=shape.bits==8?0x60:0x40;
        shape.r=identity[5];shape.g=identity[6];shape.b=identity[7];
        shape.length=shape.width*shape.height*(shape.bits/8);struct Snapshot initial;
        if(!game_surface_blank(&initial,&shape))return;
        if(game_native_cpu_snapshot(surface->object,identity,initial.data,initial.length)){
            __atomic_sub_fetch(&lock_capture_reserved,initial.length,__ATOMIC_RELAXED);free_snapshot(&initial);return;
        }
        copy(&surface->pixels,&initial,sizeof(initial));game_surface_bytes+=initial.length;
        __atomic_sub_fetch(&lock_capture_reserved,initial.length,__ATOMIC_RELAXED);
        surface->layout_known=surface->primary=1;surface->caps=0x200;
        surface->back_count_known=1;surface->back_count=0;
        lock_diagnostic("native_primary_initial_owned",surface->object,0,identity[1],identity[2],identity[4],initial.length,0);return;
    }
    if(game_native_cpu_transfer(surface->object,identity,0,0)!=1||
       !mnm_owned_initial_identity(identity,identity[1],identity[2])||
       !readable((void*)identity[0],identity[3]*identity[2])||
       !mnm_owned_initial_zero((const u8*)identity[0],identity[1],identity[2],identity[3]))return;
    /* Primary CreateSurface descriptors normally omit shape and format. The
     * owned identity service validates tight RGB565 authority independently. */
    if(surface->layout_known&&(surface->pixels.width!=identity[1]||
       surface->pixels.height!=identity[2]||surface->pixels.bits!=16||
       surface->pixels.r!=0xf800||surface->pixels.g!=0x7e0||surface->pixels.b!=31))return;
    surface->pixels.width=identity[1];surface->pixels.height=identity[2];
    surface->pixels.bits=16;surface->pixels.flags=0x40;
    surface->pixels.r=0xf800;surface->pixels.g=0x7e0;surface->pixels.b=31;
    surface->pixels.length=identity[1]*identity[2]*2;
    surface->layout_known=1;surface->primary=1;surface->caps=0x200;
    surface->back_count_known=surface->back_count=0;
    struct Snapshot initial;
    if(!game_surface_blank(&initial,&surface->pixels))return;
    surface->pixels.data=initial.data;game_surface_bytes+=initial.length;
    __atomic_sub_fetch(&lock_capture_reserved,initial.length,__ATOMIC_RELAXED);
    lock_diagnostic("native_primary_initial_zero",surface->object,0,identity[1],identity[2],0,initial.length,0);
}
static i32 (WIN *game_native_gpu_copy)(void*,const i32*,void*,const i32*,u32,u16*,u32);
static i32 (WIN *game_native_gpu_plan)(void*,const i32*,void*,const i32*,u32,struct MnmGpuCopyPlan*,u32);
/* Called by the fixed-build scene adapter after its validated ACK restores the
 * original page guard, before any post-World UI drawing. Only the already-owned
 * current-thread full Lock may supply the opaque identity and private baseline.
 * No COM calls and no original/native World pixels are returned here. */
__declspec(dllexport) int WIN RenderResidentWorldV1(u32 pixels,u32 width,u32 height,u32 stride,
    u32 generation_low,u32 generation_high,u32 nonce_low,u32 nonce_high){
    u32 error=GetLastError();int ok=0;
    if(!game_resident_world || width!=800 || height!=600 || stride!=800 ||
       (!generation_low&&!generation_high) || (!nonce_low&&!nonce_high))return 0;
    /* The scene callback participates in the same admission domain as surface
     * callbacks. A short cursor commit must not consume the tracker-only8ms
     * budget before the World can bind. Stop/recovery must also count this
     * exported callback before touching session storage. */
    struct CommandLease callback __attribute__((cleanup(command_gate_leave)))=command_gate_enter();
    if(!callback.observe){SetLastError(error);return 0;}
    struct GameCopyLease admission __attribute__((cleanup(game_copy_leave)))=game_copy_bind_enter();
    if(!admission.observe || !game_tracker_acquire()){SetLastError(error);return 0;}
    game_surface_sync();
    struct GameLock* lock=0;
    for(u32 i=0;i<32;++i){struct GameLock* candidate=game_locks+i;
        if(candidate->active && candidate->owner==GetCurrentThreadId() &&
           candidate->epoch==__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED) &&
           !candidate->rectangle && candidate->desc[9]==pixels){if(lock)goto done;lock=candidate;}}
    if(!lock || lock->resident || lock->desc[3]!=width || lock->desc[2]!=height ||
       lock->desc[4]!=stride*2 || lock->desc[21]!=16 || lock->desc[19]!=0x40 ||
       lock->desc[22]!=0xf800 || lock->desc[23]!=0x7e0 || lock->desc[24]!=31 ||
       (!game_native_cpu_lease_snapshot && !readable((void*)pixels,width*height*2)))goto done;
    u32 length=width*height*2;
    if(!lock->base.data){
        u32 reserved=__atomic_load_n(&lock_capture_reserved,__ATOMIC_RELAXED);
        if(reserved>GAME_SURFACE_LIMIT-game_surface_bytes || length>GAME_SURFACE_LIMIT-game_surface_bytes-reserved)goto done;
        lock->base.data=HeapAlloc(GetProcessHeap(),0,length);if(!lock->base.data)goto done;
        lock->base.length=length;__atomic_add_fetch(&lock_capture_reserved,length,__ATOMIC_RELAXED);
    }
    if(lock->base.length!=length)goto done;
    lock->base.width=width;lock->base.height=height;lock->base.bits=16;lock->base.flags=0x40;
    lock->base.r=0xf800;lock->base.g=0x7e0;lock->base.b=31;
    if(game_native_cpu_lease_snapshot){
        u32 layout[4]={pixels,width,height,stride*2};
        if(game_native_cpu_lease_snapshot(lock->object,layout,(u16*)lock->base.data,length))goto done;
    }else copy(lock->base.data,(void*)pixels,length);
    struct SessionSurface* surface=session_surface(lock->object,&lock->base,0);if(!surface)goto done;
    u32 fields[5]={surface->id,generation_low,generation_high,nonce_low,nonce_high};
    if(!session_record(MNM_RENDER_STREAM_V4_OPERATION_WORLD,fields,20,0,0))goto done;
    if(!game_cpu_history || game_cpu_history((void*)pixels,1,generation_low,generation_high,nonce_low,nonce_high))goto done;
    lock->resident=1;lock->gpu_history=1;
    struct GameSurface* owned=game_surface_find(lock->object,0);if(owned)owned->gpu_history=1;
    ok=1;lock_diagnostic("resident_world_bound",lock->object,lock->kind,generation_low,generation_high,0,0,0);
 done:
    if(!ok)game_session_gap(4);game_tracker_release();SetLastError(error);return ok;
}
static struct SessionPalette* session_palette_find(struct GamePalette* p){
    for(u32 i=0;i<32;++i)if(session_palettes[i].id && session_palettes[i].generation==p->identity_generation)return session_palettes+i;
    return 0;
}
static int game_session_palette_registered(struct GamePalette* p){return game_session_palette_resources && session_palette_find(p)!=0;}
static struct SessionPalette* session_palette_resource(struct GamePalette* p){
    if(!game_palette_complete(p)){game_session_gap(5);return 0;}
    struct SessionPalette* found=session_palette_find(p);u8 rgb[768];
    if(!found){
        for(u32 i=0;i<32;++i)if(!session_palettes[i].id){found=session_palettes+i;break;}
        if(!found || session_last_palette_id==0xffffffffu){game_session_gap(2);return 0;}
        found->id=++session_last_palette_id;found->generation=p->identity_generation;
        for(u32 i=0;i<256;++i)copy(rgb+i*3,p->colors+i*4,3);
        u32 fields[2]={found->id,found->generation};
        if(!session_record(MNM_RENDER_STREAM_V2_OPERATION_PALETTE_CREATE,fields,8,rgb,768))return 0;
        copy(found->colors,p->colors,1024);p->session=session_archive_id;return found;
    }
    u32 first=0;while(first<256 && same(found->colors+first*4,p->colors+first*4,3))++first;
    if(first<256){u32 end=256;while(end>first && same(found->colors+(end-1)*4,p->colors+(end-1)*4,3))--end;
        for(u32 i=first;i<end;++i)copy(rgb+(i-first)*3,p->colors+i*4,3);
        u32 fields[4]={found->id,found->generation,first,end-first};
        if(!session_record(MNM_RENDER_STREAM_V2_OPERATION_PALETTE_UPDATE,fields,16,rgb,(end-first)*3))return 0;
    }
    copy(found->colors,p->colors,1024);return found;
}
static int session_palette_bind(struct SessionSurface* s,struct GamePalette* p){
    struct SessionPalette* palette=p?session_palette_resource(p):0;
    if(p && !palette)return 0;
    u32 id=palette?palette->id:0,generation=palette?palette->generation:0;
    if(s->palette_id==id && s->palette_generation==generation)return 1;
    u32 fields[3]={s->id,id,generation};
    if(!session_record(MNM_RENDER_STREAM_V2_OPERATION_PALETTE_BIND,fields,12,0,0))return 0;
    s->palette_id=id;s->palette_generation=generation;return 1;
}
static void game_session_palette_retire(struct GamePalette* p){
    if(!session_active || !game_session_palette_resources)return;
    struct SessionPalette* palette=session_palette_find(p);if(!palette)return;
    for(u32 i=0;i<32;++i){struct SessionSurface* s=session_surfaces+i;
        if(!s->id || s->palette_id!=palette->id)continue;
        if(s->dc_pending){game_session_gap(3);return;}
        for(u32 j=0;j<32;++j)if(game_locks[j].active && game_alias_same(s->object,game_locks[j].object)){game_session_gap(3);return;}
    }
    for(u32 i=0;i<32;++i)if(session_surfaces[i].id && session_surfaces[i].palette_id==palette->id)
        if(!session_palette_bind(session_surfaces+i,0))return;
    u32 fields[2]={palette->id,palette->generation};
    if(session_record(MNM_RENDER_STREAM_V2_OPERATION_PALETTE_DELETE,fields,8,0,0))zero(palette,sizeof(*palette));
}
static int session_colors(struct SessionSurface* s){
    if(s->bits!=8)return 1;
    struct GameSurface* surface=game_surface_find(s->object,0);u8 colors[1024],rgb[768];
    if(game_session_palette_resources){
        if(!surface){game_session_gap(5);return 0;}
        struct GamePalette* palette=game_palette_find(surface->palette,0);
        if(!palette){game_session_gap(5);return 0;}
        return session_palette_bind(s,palette) && s->palette_id;
    }
    if(!surface || !game_surface_colors(surface,colors)){game_session_gap(5);return 0;}
    if(s->palette_known && same(colors,s->palette,1024))return 1;
    u32 first=0,count=256;
    if(game_session_continuous && s->palette_known){
        /* Entry flags are not color/alpha. Send the smallest contiguous RGB
         * range containing actual changes, retaining a full initial palette. */
        while(first<256 && same(colors+first*4,s->palette+first*4,3))++first;
        u32 end=256;while(end>first && same(colors+(end-1)*4,s->palette+(end-1)*4,3))--end;
        count=end-first;
    }
    for(u32 i=0;i<count;++i)copy(rgb+i*3,colors+(first+i)*4,3);
    u32 fields[3]={s->id,first,count};if(count && !session_record(4,fields,12,rgb,count*3))return 0;
    copy(s->palette,colors,1024);s->palette_known=1;return 1;
}
static int session_check(struct SessionSurface* s,const struct Snapshot* pixels){
    if(s && s->dc_pending){game_session_gap(3);return 0;}
    /* Expected output is diagnostic traffic, never render input. Keep it in
     * the ordinary comparison sample; multi-frame live sequences retain only
     * owned CREATE/UPDATE/COPY inputs and their lifetime/presentation records. */
    if(game_session_continuous || game_session_presentations)return s!=0;
    return s && session_record(5,&s->id,4,pixels->data,pixels->length);
}
static int session_present(struct SessionSurface* s){
    if(!s || !session_colors(s) || !session_record(6,&s->id,4,0,0))return 0;
    if(session_presented!=0xffffffffu)++session_presented;return 1;
}
static void session_finish_owned(void){
    if(!session_active)return;
    game_session_sync();if(!session_active)return;
    if(!session_presented || (!game_session_continuous && game_session_presentations && session_presented<game_session_presentations)){game_session_gap(6);return;}
    for(u32 i=0;i<32;++i)if(session_surfaces[i].id && session_surfaces[i].dc_pending){game_session_gap(3);return;}
    for(u32 i=0;i<32;++i)if(game_locks[i].active && session_find(game_locks[i].object)){game_session_gap(3);return;}
    if(!session_active)return;
    int ok=1;
    for(u32 i=0;ok && i<32;++i)if(session_surfaces[i].id)
        ok=session_emit(7,&session_surfaces[i].id,4,0,0,1);
    if(game_session_palette_resources)for(u32 i=0;ok && i<32;++i)if(session_palettes[i].id){
        u32 fields[2]={session_palettes[i].id,session_palettes[i].generation};
        ok=session_emit(MNM_RENDER_STREAM_V2_OPERATION_PALETTE_DELETE,fields,8,0,0,1);
    }
    if(ok)ok=session_emit(8,0,0,0,0,1);
    if(ok)command_channel_end();else command_channel_fail(MNM_RENDER_COMMANDS_V1_REASON_INVALID);
    session_archive_close();session_active=0;
    lock_diagnostic(ok?"session_finished":"session_file_failed",0,0,session_operations,0,0,0,0);
}
static void session_done(void){
    if(!session_active)return;
    if(session_operations!=0xffffffffu)++session_operations;
    if(game_session_continuous)return;
    if(game_session_presentations){
        /* Continue the same native surface history across primary frames.
         * First presentation must arrive by64; the complete sequence by256.
         * A short sequence refuses rather than disguising a partial sample. */
        if(session_presented>=game_session_presentations ||
           (!session_presented && session_operations>=64) || session_operations>=256)session_finish_owned();
        return;
    }
    /* Retain the ordinary 16-operation sample. Startup may need more owned
     * offscreen initialization before any eligible primary PRESENT. Its
     * separate 64-operation ceiling never relaxes record/byte/ownership caps. */
    if(session_operations>=16 && (session_presented || session_operations>=64))session_finish_owned();
}
/* Use the already admitted command inputs, never a stale CPU source checkpoint.
 * The physical facade copy and FIFO COPY commit share ordered-copy admission.
 * Any lost session after physical admission is terminal, before another caller
 * could consume a successful copy without its native command. */
static void game_gpu_refuse(u32 stage,const struct GameBlit* p){
    u32 record[20]={0x474d4e4d,1,stage,(u32)p->target,(u32)p->source,p->valid,p->bootstrap,
        p->source_generation,p->target_generation,p->epoch},done=0;
    for(u32 i=0;i<10;++i)record[10+i]=p->fields[i];
    HANDLE file=CreateFileA("native-gpu-copy-refusal.bin",0x40000000,0,0,1,0x80,0);
    if(file!=(HANDLE)-1){WriteFile(file,record,sizeof(record),&done,0);CloseHandle(file);}
    ExitProcess(124);
}
static int game_gpu_emit_copy(void* context,const u32* fields){
    (void)context;return session_record(3,fields,40,0,0);
}
/* A blank CREATE is overwritten in full on the GPU before it is sampled. CPU
 * history supplies shape only. The scratch is never a CPU reader or PRESENT. */
static struct SessionSurface* game_gpu_scratch(const struct Snapshot* shape){
    struct SessionSurface* s=0;struct Snapshot blank;zero(&blank,sizeof(blank));
    const u32 count=shape->width*shape->height,limit=mnm_resident_source_pixel_limit(game_prepared_world);
    for(u32 i=0;i<32;++i)if(!session_surfaces[i].id){s=session_surfaces+i;break;}
    if(!s || session_last_id==0xffffffffu || session_pixels>limit || count>limit-session_pixels || !game_surface_blank(&blank,shape))return 0;
    zero(s,sizeof(*s));s->id=++session_last_id;s->width=shape->width;s->height=shape->height;
    s->bits=shape->bits;s->r=shape->r;s->g=shape->g;s->b=shape->b;session_pixels+=count;
    u32 fields[7]={s->id,s->width,s->height,s->bits,s->r,s->g,s->b};
    const int ok=session_record(1,fields,28,blank.data,blank.length);
    __atomic_sub_fetch(&lock_capture_reserved,blank.length,__ATOMIC_RELAXED);free_snapshot(&blank);
    return ok?s:0;
}
#include "gpu_copy_census.h"
static i32 game_gpu_blit(struct GameBlit* p){
    u32 error=GetLastError();
    if(!p->valid || !p->gpu || !game_resident_world || !game_copy_order_enabled ||
       !game_native_gpu_copy || !game_native_gpu_plan || !game_surface_pixels_enter(p->target))game_gpu_refuse(1,p);
    struct GameSurface *src=game_surface_find(p->source,0),*dst=game_surface_find(p->target,0);
    struct SessionSurface *a=session_find(p->source),*b=0,*scratch=0;
    if(!session_active || !a || !src || !dst || !src->gpu_history || !p->source_generation ||
       p->epoch!=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED) ||
       src->generation!=p->source_generation || dst->generation!=p->target_generation)game_gpu_refuse(2,p);
    if(p->bootstrap && (!game_surface_blank(&p->src,&dst->pixels) || !game_surface_blank(&p->dst,&dst->pixels)))game_gpu_refuse(3,p);
    i32 sr[4]={(i32)p->fields[2],(i32)p->fields[3],(i32)p->fields[4],(i32)p->fields[5]};
    i32 dr[4]={(i32)p->fields[6],(i32)p->fields[7],(i32)(p->fields[6]+p->fields[4]-p->fields[2]),(i32)(p->fields[7]+p->fields[5]-p->fields[3])};
    struct MnmGpuCopyPlan* plan=HeapAlloc(GetProcessHeap(),8,sizeof(*plan));if(!plan)game_gpu_refuse(6,p);SetLastError(error);
    i32 admission=game_native_gpu_plan(p->target,dr,p->source,sr,p->fields[8]?0x8000:0,plan,sizeof(*plan));
    error=GetLastError();i32 result=admission;u32 first_copy=0,last_copy=0,scratch_id=0;
    if(admission>=0){
        if(plan->source_width!=src->pixels.width || plan->source_height!=src->pixels.height ||
           plan->target_width!=dst->pixels.width || plan->target_height!=dst->pixels.height ||
           plan->keyed!=p->fields[8] || plan->key!=p->fields[9] || plan->self!=(src==dst))game_gpu_refuse(4,p);
        /* Validate before CREATE, publication, private-baseline adoption or any
         * successful application return. A source failure retains its prefix. */
        if(!mnm_gpu_copy_plan_valid(plan,1,plan->self?1:2,plan->self&&plan->count?3:0))game_gpu_refuse(4,p);
        result=(i32)plan->result;
        if(p->bootstrap){
            /* Keep the legacy full-overwrite bootstrap's unchanged private CPU
             * mask baseline; it never seeds a GPU image or satisfies a reader. */
            SetLastError(error);
            i32 baseline=game_native_gpu_copy(p->target,dr,p->source,sr,0,(u16*)p->dst.data,p->dst.length);
            error=GetLastError();if(baseline<0 || result<0 || plan->count!=1 || plan->self)game_gpu_refuse(3,p);
        }
        b=session_surface(p->target,p->bootstrap?&p->src:&dst->pixels,p->source);if(!b)game_gpu_refuse(4,p);
        if(plan->self && plan->count){scratch=game_gpu_scratch(&src->pixels);if(!scratch)game_gpu_refuse(6,p);}
        scratch_id=scratch?scratch->id:0;if(plan->count)first_copy=session_sequence+1;
        if(!mnm_gpu_copy_commands(plan,a->id,b->id,scratch_id,game_gpu_emit_copy,0))game_gpu_refuse(5,p);
        if(plan->count)last_copy=session_sequence;
        if(scratch){
            if(!session_record(7,&scratch->id,4,0,0))game_gpu_refuse(5,p);
            session_pixels-=scratch->width*scratch->height;zero(scratch,sizeof(*scratch));
        }
        if(dst->primary && (result>=0 || plan->count) && !session_present(b))game_gpu_refuse(5,p);
        if(plan->count){
            dst->gpu_history=1;dst->generation=++game_surface_generation;dst->generation_origin=8;
            dst->generation_caller=p->caller;dst->generation_owner=GetCurrentThreadId();
        }
        if(p->bootstrap){
            copy(&dst->pixels,&p->dst,sizeof(p->dst));p->dst.data=0;
            game_surface_bytes+=dst->pixels.length;__atomic_sub_fetch(&lock_capture_reserved,dst->pixels.length,__ATOMIC_RELAXED);
        }
        session_done();lock_diagnostic("resident_gpu_copy",p->target,0,(u32)p->source,plan->count,(u32)result,plan->self,0);
    }
    game_gpu_census_record(p,src,dst,plan,admission,result,a->id,b?b->id:0,scratch_id,first_copy,last_copy);
    HeapFree(GetProcessHeap(),0,plan);game_blit_free(p);p->gpu=2;game_tracker_release();SetLastError(error);return result;
}
/* Explicit CPU reader only. FIFO execution fixes the image boundary; physical
 * identity is rechecked after waiting without holding the tracker. The ordered
 * copy lease remains held by surface_dc. No per-World pixel return is enabled. */
#include "../../protocols/include/mnm/native_cpu_read_v1.h"
static u32 game_cpu_read_ticket,game_cpu_transfer_wait;
static u32 game_cpu_read_completed_counts[4];
static u32 game_cpu_read_completed_ticket;
static void game_cpu_read_completed(u32 kind){
 if(kind>=4)ExitProcess(124);++game_cpu_read_completed_counts[kind];
 game_cpu_read_completed_ticket=game_cpu_read_ticket;
}
/* Shutdown owns closed application admission and joined publication. Never do
 * diagnostic file I/O under the tracker, copy lease, or reader callback. */
static int game_cpu_read_counts_flush(void){
 if(!game_cpu_read_completed_ticket)return 1;
 u32 r[12]={0x524d4e4d,0x31304344,1,48,game_cpu_read_completed_ticket,
  game_cpu_read_completed_counts[0],game_cpu_read_completed_counts[1],game_cpu_read_completed_counts[2],game_cpu_read_completed_counts[3],0,0,0},done=0;
 HANDLE file=CreateFileA("native-cpu-read-counts.bin",0x40000000,0,0,2,0x80,0);
 if(file==(HANDLE)-1)return 0;
 int ok=WriteFile(file,r,sizeof(r),&done,0) && done==sizeof(r);
 CloseHandle(file);return ok;
}
static void game_cpu_read_refuse(u32 stage,void* object,u32 detail){
    u32 record[8]={0x434d4e4d,1,stage,(u32)object,detail,game_cpu_read_ticket,game_lock_epoch,command_channel_session},done=0;
    HANDLE file=CreateFileA("native-cpu-transfer-refusal.bin",0x40000000,0,0,1,0x80,0);
    if(file!=(HANDLE)-1){WriteFile(file,record,sizeof(record),&done,0);CloseHandle(file);}
    ExitProcess(124);
}
static void game_materialize_reader(void* object,u32 kind){
    if(!game_resident_world)return;
    u32 error=GetLastError();
    if(!game_surface_pixels_enter(object))game_cpu_read_refuse(1,object,0);
    struct GameSurface* source=game_surface_find(object,0);
    if(!source || !source->gpu_history){game_tracker_release();SetLastError(error);return;}
    u32 identity[8];
    if(!game_native_cpu_transfer)game_cpu_read_refuse(2,object,0);
    i32 query=game_native_cpu_transfer(object,identity,0,0);
    if(query<0)for(u32 i=0;i<32;++i)if(game_locks[i].active && game_alias_same(game_locks[i].object,object)){
        /* The facade returns SURFACEBUSY for overlapping protected Lock/DC
         * leases. Preserve the active checkpoint and let it return that result. */
        game_tracker_release();SetLastError(error);return;
    }
    if(query==1){source->gpu_history=0;game_tracker_release();SetLastError(error);return;}
    struct SessionSurface* surface=session_find(object);
    if(query || !surface || !session_active || game_cpu_read_ticket==0xffffffffu ||
       source->pixels.bits!=16 || identity[1]!=surface->width || identity[2]!=surface->height)game_cpu_read_refuse(3,object,(u32)query);
    struct Snapshot result;
    if(!game_surface_blank(&result,&source->pixels))game_cpu_read_refuse(4,object,0);
    const u32 ticket=++game_cpu_read_ticket,generation=source->generation,epoch=game_lock_epoch;
    u32 fields[10]={surface->id,ticket,0,command_channel_session,identity[1],identity[2],identity[4],identity[5],identity[6],identity[7]};
    __atomic_store_n(&game_cpu_transfer_wait,1,__ATOMIC_RELEASE);
    if(!session_record(MNM_RENDER_STREAM_V5_OPERATION_CPU_READ,fields,40,0,0))game_cpu_read_refuse(5,object,0);
    game_tracker_release();
    char path[544];u32 n=GetEnvironmentVariableA("MNM_RENDER_COMMAND_CHANNEL",path,512);
    if(!n || n>=512)game_cpu_read_refuse(6,object,n);copy(path+n,".cpu-reply",11);
    u32 start=GetTickCount(),ready=0;
    while(GetTickCount()-start<MNM_NATIVE_CPU_READ_V1_TIMEOUT_MS){
        if(!command_channel_identity() || command_channel_refused ||
           __atomic_load_n(command_channel+6,__ATOMIC_ACQUIRE)>=2 ||
           __atomic_load_n(command_channel+8,__ATOMIC_ACQUIRE))break;
        HANDLE file=CreateFileA(path,0x80000000,7,0,3,0x80,0);
        if(file!=(HANDLE)-1){
            u32 h[16],read=0;
            const u32 file_bytes=GetFileSize(file,0);
            if(ReadFile(file,h,64,&read,0) && read==64 && mnm_native_cpu_read_reply(h,file_bytes,fields) &&
               ReadFile(file,result.data,result.length,&read,0) && read==result.length)ready=1;
            CloseHandle(file);if(ready)break;
        }
        Sleep(1);
    }
    if(!ready || !game_surface_pixels_enter(object))game_cpu_read_refuse(7,object,GetTickCount()-start);
    source=game_surface_find(object,0);
    if(!source || source->generation!=generation || game_lock_epoch!=epoch ||
       game_native_cpu_transfer(object,identity,(const u16*)result.data,result.length))game_cpu_read_refuse(8,object,0);
    game_surface_drop_at(source,0);source->gpu_history=0;
    copy(&source->pixels,&result,sizeof(result));result.data=0;
    game_surface_bytes+=source->pixels.length;__atomic_sub_fetch(&lock_capture_reserved,source->pixels.length,__ATOMIC_RELAXED);
    /* The enclosing ordered callback still owns its CPU conversion/DC work.
     * game_copy_leave retires the reader deadline with that complete lease. */
    lock_diagnostic("resident_cpu_materialized",object,0,ticket,fields[0],0,source->pixels.length,0);
    game_cpu_read_completed(kind);
    game_tracker_release();SetLastError(error);
}
static void game_materialize_dc(void* object){game_materialize_reader(object,0);}
static void game_session_dc_checkpoint(struct GameSurface* surface){
    if(!game_session_enabled || (session_started && !session_active))return;
    struct SessionSurface* s=session_find(surface->object);int existing=s!=0;
    if(s){
        if(!s->dc_pending){game_session_gap(3);return;}s->dc_pending=0;
    }
    s=session_surface(surface->object,&surface->pixels,0);if(!s)return;
    if(existing){
        u32 fields[5]={s->id,0,0,s->width,s->height};
        /* These are admitted pre-ReleaseDC bitmap input pixels, not CHECK
         * output or a replay of individual GDI operations. */
        if(!session_record(2,fields,20,surface->pixels.data,surface->pixels.length))return;
    }
    if(!session_check(s,&surface->pixels) || (surface->primary && !session_present(s)))return;
    session_done();
}
/* Native CPU transforms are owned producer output. Commit their current bytes
 * through CREATE/UPDATE; CHECK remains a separate diagnostic, never input. */
static int game_native_blit_checkpoint(void* object,u32 caller){
 if(!game_native_cpu_snapshot || !game_session_continuous)return 0;
 struct GameSurface* dst=game_surface_find(object,0);u32 layout[8];
 if(!dst || dst->dc || !dst->layout_known || game_native_cpu_snapshot(object,layout,0,0))return 0;
 for(u32 i=0;i<32;++i)if(game_locks[i].active && game_alias_same(object,game_locks[i].object))return 0;
 struct Snapshot shape,result;copy(&shape,&dst->pixels,sizeof(shape));
 if(shape.width!=layout[1] || shape.height!=layout[2] || shape.bits!=layout[4] ||
    shape.r!=layout[5] || shape.g!=layout[6] || shape.b!=layout[7] || !game_surface_blank(&result,&shape))return 0;
 if(game_native_cpu_snapshot(object,layout,result.data,result.length)){
  __atomic_sub_fetch(&lock_capture_reserved,result.length,__ATOMIC_RELAXED);free_snapshot(&result);return 0;
 }
 game_surface_drop_at(dst,0);copy(&dst->pixels,&result,sizeof(result));
 game_surface_bytes+=result.length;__atomic_sub_fetch(&lock_capture_reserved,result.length,__ATOMIC_RELAXED);
 dst->gpu_history=0;dst->generation_origin=8;dst->generation_caller=caller;dst->generation_owner=GetCurrentThreadId();
 struct SessionSurface* prior=session_find(object);
 struct SessionSurface* surface=session_surface(object,&dst->pixels,0);
 if(!surface)return 0;
 if(prior){u32 fields[5]={surface->id,0,0,surface->width,surface->height};
  if(!session_record(2,fields,20,dst->pixels.data,dst->pixels.length))return 0;}
 if(!session_check(surface,&dst->pixels) || (dst->primary && !session_present(surface)))return 0;
 session_done();lock_diagnostic("native_cpu_blit_checkpoint",object,0,caller,0,0,0,0);return 1;
}
/* Each packed UPDATE borrows only owned input. Scratch is freed before the
 * next rectangle; all updates precede CHECK/PRESENT and use existing budgets. */
static int session_unlock_region(struct SessionSurface* s,const struct Snapshot* after,const struct Rect* region){
    struct Rect r=*region;
    u32 stride=after->width*(after->bits/8),bytes=(u32)(r.right-r.left)*(after->bits/8);
    if(!inside(&r,after->width,after->height) || after->length!=stride*after->height){game_session_gap(4);return 0;}
    u32 fields[5]={s->id,(u32)r.left,(u32)r.top,(u32)(r.right-r.left),(u32)(r.bottom-r.top)};
    u32 length=bytes*fields[4];u8* packed=0;
    const u8* pixels=after->data+(u32)r.top*stride+(u32)r.left*(after->bits/8);
    if(bytes!=stride){
        /* The pending after-snapshot is no longer in reserved storage at
         * this point. Count it as well as the partial base and scratch. */
        u32 reserved=__atomic_load_n(&lock_capture_reserved,__ATOMIC_RELAXED);
        if(reserved>GAME_SURFACE_LIMIT-game_surface_bytes ||
           after->length>GAME_SURFACE_LIMIT-game_surface_bytes-reserved ||
           length>GAME_SURFACE_LIMIT-game_surface_bytes-reserved-after->length){game_session_gap(2);return 0;}
        packed=HeapAlloc(GetProcessHeap(),0,length);if(!packed){game_session_gap(4);return 0;}
        __atomic_add_fetch(&lock_capture_reserved,length,__ATOMIC_RELAXED);
        for(u32 y=0;y<fields[4];++y)copy(packed+y*bytes,pixels+y*stride,bytes);
        pixels=packed;
    }
    int ok=session_record(2,fields,20,pixels,length);
    if(packed){HeapFree(GetProcessHeap(),0,packed);__atomic_sub_fetch(&lock_capture_reserved,length,__ATOMIC_RELAXED);}
    return ok;
}

static int session_resident_unlock(struct SessionSurface* surface,const struct Snapshot* before,const struct Snapshot* after,const u8* dirty){
    if(!before->data || before->width!=after->width || before->height!=after->height ||
       before->length!=after->length || before->bits!=16 || after->bits!=16){game_session_gap(4);return 0;}
    u32 regions[COMMAND_DIRTY_REGIONS][4];
    game_copy_stage(40);
    u32 count=command_dirty_regions_mask(before->data,after->data,after->width,after->height,2,dirty,regions);
    game_copy_stage(41);
    for(u32 i=0;i<count;++i){u32 width=regions[i][2]-regions[i][0],height=regions[i][3]-regions[i][1];
        /* Bound record/scratch bytes even for a dense full-canvas UI write. */
        for(u32 row=0;row<height;row+=32){u32 rows=height-row;if(rows>32)rows=32;u32 pixels=width*rows,length=pixels*3;
            u32 reserved=__atomic_load_n(&lock_capture_reserved,__ATOMIC_RELAXED);
            if(reserved>GAME_SURFACE_LIMIT-game_surface_bytes || after->length>GAME_SURFACE_LIMIT-game_surface_bytes-reserved ||
               length>GAME_SURFACE_LIMIT-game_surface_bytes-reserved-after->length){game_session_gap(2);return 0;}
            u8* packed=HeapAlloc(GetProcessHeap(),0,length);if(!packed){game_session_gap(4);return 0;}
            __atomic_add_fetch(&lock_capture_reserved,length,__ATOMIC_RELAXED);
            u32 fields[5]={surface->id,regions[i][0],regions[i][1]+row,width,rows};
            game_copy_stage(42);
            for(u32 y=0;y<rows;++y)for(u32 x=0;x<width;++x){u32 at=(fields[2]+y)*after->width+fields[1]+x,p=y*width+x;
                copy(packed+p*2,after->data+at*2,2);packed[pixels*2+p]=!same(before->data+at*2,after->data+at*2,2) || (dirty && (dirty[at*2] || dirty[at*2+1]));}
            game_copy_stage(43);
            int ok=session_record(MNM_RENDER_STREAM_V4_OPERATION_MASKED_UPDATE,fields,20,packed,length);
            HeapFree(GetProcessHeap(),0,packed);__atomic_sub_fetch(&lock_capture_reserved,length,__ATOMIC_RELAXED);if(!ok)return 0;
        }
    }
    return 1;
}
static void game_cpu_lease_layout(const struct GameLock* lock,u32* layout){
 layout[0]=lock->desc[9]-(lock->rectangle?(u32)lock->region.top*lock->desc[4]+(u32)lock->region.left*2:0);
 layout[1]=lock->desc[3];layout[2]=lock->desc[2];layout[3]=lock->desc[4];
}
static i32 game_cpu_lease_dirty(const struct GameLock* lock,u8* out,u32 action){
 if(!game_native_cpu_lease_mask)return 0;u32 layout[4];game_cpu_lease_layout(lock,layout);
 return game_native_cpu_lease_mask(lock->object,layout,out,out?layout[1]*layout[2]*2:0,action);
}
static u8* game_cpu_lease_mask(const struct GameLock* lock,u32 bytes){
 i32 flags=game_cpu_lease_dirty(lock,0,2);if(flags<0)game_cpu_read_refuse(16,lock->object,(u32)flags);
 if(!flags)return 0;
 u32 reserved=__atomic_load_n(&lock_capture_reserved,__ATOMIC_RELAXED);
 if(reserved>GAME_SURFACE_LIMIT-game_surface_bytes || bytes>GAME_SURFACE_LIMIT-game_surface_bytes-reserved)game_cpu_read_refuse(17,lock->object,bytes);
 u8* mask=HeapAlloc(GetProcessHeap(),0,bytes);if(!mask)game_cpu_read_refuse(18,lock->object,bytes);
 __atomic_add_fetch(&lock_capture_reserved,bytes,__ATOMIC_RELAXED);
 if(game_cpu_lease_dirty(lock,mask,0)!=1)game_cpu_read_refuse(19,lock->object,0);return mask;
}
static void game_session_unlock(struct GameLock* lock,const struct Snapshot* after,u32 primary){
    if(!game_session_enabled)return;
    struct SessionSurface* s=session_find(lock->object);int existing=s!=0;
    /* A committed full writable Unlock is complete replacement input, not
     * recovery from a missed operation. Preserve COM aliases while changing
     * the native wire identity/layout via DELETE followed by fresh CREATE. */
    if(game_session_continuous && s && !lock->rectangle &&
       (s->width!=after->width || s->height!=after->height || s->bits!=after->bits ||
        s->r!=after->r || s->g!=after->g || s->b!=after->b)){
        game_session_retire(lock->object);existing=0;
    }
    s=session_surface(lock->object,lock->rectangle?&lock->base:after,0);if(!s)return;
    if(lock->resident || lock->gpu_history){
        u8* mask=game_cpu_lease_mask(lock,after->length);
        int ok=existing && session_resident_unlock(s,&lock->base,after,mask);
        if(mask){HeapFree(GetProcessHeap(),0,mask);__atomic_sub_fetch(&lock_capture_reserved,after->length,__ATOMIC_RELAXED);}
        if(!ok)return;
        if(game_cpu_lease_dirty(lock,0,1)<0)game_cpu_read_refuse(20,lock->object,0);
        goto present;
    }
    if(lock->rectangle || existing){
        struct Rect r={0,0,(i32)after->width,(i32)after->height};if(lock->rectangle)copy(&r,&lock->region,16);
        if(lock->rectangle && !session_check(s,&lock->base))return;
        /* The quarantined owned baseline precedes this successful Unlock. Only
         * continuous existing resources may omit equal storage bytes; missing
         * or changed layouts retain the complete replacement path. */
        if(game_session_continuous && existing && !lock->rectangle){
            const struct Snapshot* before=&lock->base;
            if(before && before->data && before->length==after->length &&
               before->width==after->width && before->height==after->height &&
               before->bits==after->bits && before->r==after->r && before->g==after->g && before->b==after->b){
                u32 regions[COMMAND_DIRTY_REGIONS][4];
                u32 count=command_dirty_regions(before->data,after->data,after->width,after->height,after->bits/8,regions);
                for(u32 i=0;i<count;++i){
                    struct Rect part={(i32)regions[i][0],(i32)regions[i][1],(i32)regions[i][2],(i32)regions[i][3]};
                    if(!session_unlock_region(s,after,&part))return;
                }
                goto present;
            }
        }
        if(!session_unlock_region(s,after,&r))return;
    }
 present:
    if(!session_check(s,after) || (primary && !session_present(s)))return;session_done();
}
static void game_session_blit_begin(struct GameBlit* p){
    if(!game_session_enabled)return;
    if(p->fill){
        /* Only the destination has an original identity. Bootstrap before
         * pixels are synthetic and are not an original comparison checkpoint. */
        struct SessionSurface* b=session_surface(p->target,&p->dst,p->source);
        if(b && !p->bootstrap)session_check(b,&p->dst);
        return;
    }
    struct SessionSurface* a=session_surface(p->source,&p->src,p->target),*b=session_surface(p->target,&p->dst,p->source);
    if(!session_check(a,&p->src) || !session_check(b,&p->dst))return;
}
static void game_session_blit_end(struct GameBlit* p,u32 primary){
    if(!session_active)return;
    if(p->fill){
        struct SessionSurface* b=session_find(p->target);if(!b){game_session_gap(4);return;}
        u32 width=p->fields[4]-p->fields[2],height=p->fields[5]-p->fields[3];
        u32 fields[5]={b->id,p->fields[6],p->fields[7],width,height};
        u32 length=width*height*(p->dst.bits/8);
        /* game_fill_before derives this entire constant buffer from dwFillColor.
         * Its first rectangle-sized prefix is already tightly packed, including
         * partial fills. Never use reconstructed CHECK output as render input. */
        if(!p->src.data || length>p->src.length){game_session_gap(4);return;}
        if(!session_record(2,fields,20,p->src.data,length) || !session_check(b,&p->dst) ||
           (primary && !session_present(b)))return;
        session_done();return;
    }
    struct SessionSurface *a=session_find(p->source),*b=session_find(p->target);if(!a || !b){game_session_gap(4);return;}
    u32 fields[10];copy(fields,p->fields,40);fields[0]=a->id;fields[1]=b->id;
    if(!session_record(3,fields,40,0,0) || !session_check(b,&p->dst) || (primary && !session_present(b)))return;session_done();
}
static void game_session_flip_begin(struct GameSurface* front,struct GameSurface* back){
    if(!game_session_enabled)return;
    struct SessionSurface *a=session_surface(front->object,&front->pixels,back->object),*b=session_surface(back->object,&back->pixels,front->object);
    if(!session_check(a,&front->pixels) || !session_check(b,&back->pixels))return;
}
static void game_session_flip_end(struct GameSurface* front,struct GameSurface* back){
    if(!session_active)return;
    struct SessionSurface *a=session_find(front->object),*b=session_find(back->object);if(!a || !b){game_session_gap(4);return;}
    u32 ids[2]={a->id,b->id};
    if(!session_record(11,ids,8,0,0) || !session_check(a,&front->pixels) || !session_check(b,&back->pixels) || !session_present(a))return;session_done();
}
static void game_session_palette(struct GameSurface* surface){
    if(!session_active || !surface->pixels.data)return;
    struct SessionSurface* s=session_find(surface->object);if(!s)return;
    if(game_session_palette_resources && s->bits==8 && !surface->palette){
        if(session_palette_bind(s,0) && surface->primary)game_session_gap(5);return;
    }
    if(!session_check(s,&surface->pixels) || !session_colors(s) || (surface->primary && !session_present(s)))return;session_done();
}

static void game_session_palette_changed(void* object){
    if(!session_active)return;struct GamePalette* palette=game_palette_find(object,0);if(!palette)return;
    for(u32 i=0;i<GAME_SURFACE_COUNT && session_active;++i){struct GameSurface* s=game_surfaces+i;
        if(s->object && s->palette && game_palette_find(s->palette,0)==palette && session_find(s->object)){
            if(!game_palette_complete(palette)){game_session_gap(5);return;}game_session_palette(s);
        }
    }
}
