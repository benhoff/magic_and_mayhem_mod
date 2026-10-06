#include "../../protocols/include/mnm/render_stream_v2.h"
#include "dirty_region.h"
/* Ordered opt-in production, independent of bounded diagnostic archives.
 * Every entry point holds game_locks_busy; pixels are owned snapshots only. */
struct SessionSurface {void* object;u32 id,width,height,bits,r,g,b,palette_known,dc_pending,palette_id,palette_generation;u8 palette[1024];};
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
static void game_session_gap(u32 reason){
    command_channel_fail(MNM_RENDER_COMMANDS_V1_REASON_GAP);
    if(!session_active)return;
    session_archive_gap(reason);session_active=0;
    lock_diagnostic("session_gap",0,0,reason,0,0,0,0);
}
static void game_session_sync(void){
    if(session_active && session_epoch!=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED))game_session_gap(1);
}
static int session_emit(u32 op,const void* fields,u32 fl,const void* pixels,u32 length,u32 closing){
    if(!session_active)return 0;
    if(fl>0xffffffffu-12 || length>0xffffffffu-12-fl || session_sequence==0xffffffffu ||
       12+fl+length>0xffffffffu-session_bytes){game_session_gap(2);return 0;}
    u32 bytes=12+fl+length;
    /* Archive exhaustion is a diagnostic GAP, never a live transport GAP.
     * Reserve 32 DELETEs, END and GAP in the unchanged bounded archive. */
    if(session_file && !closing && (session_archive_sequence>=4096-session_cleanup_records() ||
       bytes>64*1024*1024-session_cleanup_bytes()-session_archive_bytes)){
        if(!game_session_continuous){game_session_gap(2);return 0;}
        session_archive_gap(2);
    }
    ++session_sequence;
    if(session_file && !command_record(session_file,&session_archive_sequence,op,fields,fl,pixels,length)){
        session_archive_close();lock_diagnostic("session_file_failed",0,0,0,0,0,0,0);
        if(!game_session_continuous){command_channel_fail(MNM_RENDER_COMMANDS_V1_REASON_INVALID);session_active=0;return 0;}
    }else if(session_file)session_archive_bytes+=bytes;
    if(!command_channel_record(session_sequence,op,fields,fl,pixels,length)){
        command_channel_fail(MNM_RENDER_COMMANDS_V1_REASON_INVALID);session_archive_close();session_active=0;
        lock_diagnostic("session_channel_failed",0,0,0,0,0,0,0);return 0;
    }
    session_bytes+=bytes;return 1;
}
static int session_record(u32 op,const void* fields,u32 fl,const void* pixels,u32 length){
    if(!session_active)return 0;
    /* Finite live sequence/byte lifetimes still reserve orderly cleanup. */
    u32 limit=game_session_continuous?0xffffffffu:64*1024*1024;
    if(session_sequence>=(game_session_continuous?0xffffffffu-session_cleanup_records():4096-session_cleanup_records()) ||
       fl>limit-session_cleanup_bytes()-12 || length>limit-session_cleanup_bytes()-12-fl || 12+fl+length>limit-session_cleanup_bytes()-session_bytes){
        game_session_gap(2);return 0;
    }
    return session_emit(op,fields,fl,pixels,length,0);
}
static int session_start(void){
    if(!game_session_enabled || !lock_capture_path_length)return 0;
    if(session_started)return session_active!=0;
    session_started=1;session_epoch=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED);
    /* Continuous publication requires a successfully claimed v2 queue. Never
     * downgrade to an archive-only or append-only stream. Recovery is separate. */
    if(game_session_continuous && (!command_queue || command_channel_refused)){
        command_channel_fail(MNM_RENDER_COMMANDS_V1_REASON_INVALID);
        lock_diagnostic("session_continuous_requires_v2",0,0,0,0,0,0,0);return 0;
    }
    u32 header[4];copy(header,game_session_palette_resources?MNM_RENDER_STREAM_V2_MAGIC:"MNMCMD01",8);header[2]=game_session_palette_resources?2:1;header[3]=16;
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
    return found;
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
static struct SessionSurface* session_surface(void* object,const struct Snapshot* pixels){
    if(!session_start())return 0;
    struct SessionSurface* s=session_find(object);
    if(!pixels->data){game_session_gap(4);return 0;}
    if(s){
        if(s->dc_pending){game_session_gap(3);return 0;}
        if(s->width!=pixels->width || s->height!=pixels->height || s->bits!=pixels->bits ||
           s->r!=pixels->r || s->g!=pixels->g || s->b!=pixels->b){game_session_gap(3);return 0;}
        return s;
    }
    for(u32 i=0;i<32;++i)if(!session_surfaces[i].id){
        u32 count=pixels->width*pixels->height;if(count>16777216-session_pixels){game_session_gap(2);return 0;}
        if(game_session_continuous && session_last_id==0xffffffffu){game_session_gap(2);return 0;}
        s=session_surfaces+i;s->object=object;s->id=game_session_continuous?++session_last_id:i+1;s->width=pixels->width;s->height=pixels->height;s->bits=pixels->bits;
        s->r=pixels->r;s->g=pixels->g;s->b=pixels->b;session_pixels+=count;
        u32 fields[7]={s->id,s->width,s->height,s->bits,s->r,s->g,s->b};
        if(!session_record(1,fields,28,pixels->data,pixels->length))return 0;return s;
    }
    game_session_gap(2);return 0;
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
static void game_session_finish(void){
    if(!game_tracker_acquire())return;
    session_finish_owned();game_tracker_release();
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
static void game_session_dc_checkpoint(struct GameSurface* surface){
    if(!game_session_enabled || (session_started && !session_active))return;
    struct SessionSurface* s=session_find(surface->object);int existing=s!=0;
    if(s){
        if(!s->dc_pending){game_session_gap(3);return;}s->dc_pending=0;
    }
    s=session_surface(surface->object,&surface->pixels);if(!s)return;
    if(existing){
        u32 fields[5]={s->id,0,0,s->width,s->height};
        /* These are admitted pre-ReleaseDC bitmap input pixels, not CHECK
         * output or a replay of individual GDI operations. */
        if(!session_record(2,fields,20,surface->pixels.data,surface->pixels.length))return;
    }
    if(!session_check(s,&surface->pixels) || (surface->primary && !session_present(s)))return;
    session_done();
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
    s=session_surface(lock->object,lock->rectangle?&lock->base:after);if(!s)return;
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
                u32 region[4];
                if(!command_dirty_region(before->data,after->data,after->width,after->height,after->bits/8,region))goto present;
                r.left=(i32)region[0];r.top=(i32)region[1];r.right=(i32)region[2];r.bottom=(i32)region[3];
            }
        }
        u32 stride=after->width*(after->bits/8),bytes=(u32)(r.right-r.left)*(after->bits/8);
        if(!inside(&r,after->width,after->height) || after->length!=stride*after->height){game_session_gap(4);return;}
        u32 fields[5]={s->id,(u32)r.left,(u32)r.top,(u32)(r.right-r.left),(u32)(r.bottom-r.top)};
        u32 length=bytes*fields[4];u8* packed=0;
        const u8* pixels=after->data+(u32)r.top*stride+(u32)r.left*(after->bits/8);
        if(bytes!=stride){
            /* The pending after-snapshot is no longer in reserved storage at
             * this point. Count it as well as the partial base and scratch. */
            u32 reserved=__atomic_load_n(&lock_capture_reserved,__ATOMIC_RELAXED);
            if(reserved>GAME_SURFACE_LIMIT-game_surface_bytes ||
               after->length>GAME_SURFACE_LIMIT-game_surface_bytes-reserved ||
               length>GAME_SURFACE_LIMIT-game_surface_bytes-reserved-after->length){game_session_gap(2);return;}
            packed=HeapAlloc(GetProcessHeap(),0,length);if(!packed){game_session_gap(4);return;}
            __atomic_add_fetch(&lock_capture_reserved,length,__ATOMIC_RELAXED);
            for(u32 y=0;y<fields[4];++y)copy(packed+y*bytes,pixels+y*stride,bytes);
            pixels=packed;
        }
        int ok=session_record(2,fields,20,pixels,length);
        if(packed){HeapFree(GetProcessHeap(),0,packed);__atomic_sub_fetch(&lock_capture_reserved,length,__ATOMIC_RELAXED);}
        if(!ok)return;
    }
 present:
    if(!session_check(s,after) || (primary && !session_present(s)))return;session_done();
}
static void game_session_blit_begin(struct GameBlit* p){
    if(!game_session_enabled)return;
    if(p->fill){
        /* Only the destination has an original identity. Bootstrap before
         * pixels are synthetic and are not an original comparison checkpoint. */
        struct SessionSurface* b=session_surface(p->target,&p->dst);
        if(b && !p->bootstrap)session_check(b,&p->dst);
        return;
    }
    struct SessionSurface* a=session_surface(p->source,&p->src),*b=session_surface(p->target,&p->dst);
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
    struct SessionSurface *a=session_surface(front->object,&front->pixels),*b=session_surface(back->object,&back->pixels);
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
