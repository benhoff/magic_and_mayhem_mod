/* Ordered, bounded, opt-in sessions. Every entry point holds game_locks_busy.
 * Native pixels come only from owned snapshots; identities from observed aliases. */
struct SessionSurface {void* object;u32 id,width,height,bits,r,g,b,palette_known;u8 palette[1024];};
static struct SessionSurface session_surfaces[32];
static HANDLE session_file;
static u32 session_started,session_epoch,session_sequence,session_bytes,session_operations,session_presented,session_pixels;
static void game_session_gap(u32 reason){
    if(!session_file)return;
    u32 record[4]={9,++session_sequence,4,reason};write_all(session_file,record,16);
    CloseHandle(session_file);session_file=0;
    lock_diagnostic("session_gap",0,0,reason,0,0,0,0);
}
static void game_session_sync(void){
    if(session_file && session_epoch!=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED))game_session_gap(1);
}
static int session_record(u32 op,const void* fields,u32 fl,const void* pixels,u32 length){
    if(!session_file)return 0;
    /* Reserve cleanup for 32 surfaces, END, and a GAP if a limit is reached. */
    if(session_sequence>=4062 || 12+fl+length>64*1024*1024-540-session_bytes){game_session_gap(2);return 0;}
    if(!command_record(session_file,&session_sequence,op,fields,fl,pixels,length)){
        CloseHandle(session_file);session_file=0;lock_diagnostic("session_file_failed",0,0,0,0,0,0,0);return 0;
    }
    session_bytes+=12+fl+length;return 1;
}
static int session_start(void){
    if(!game_session_enabled || !lock_capture_path_length)return 0;
    if(session_started)return session_file!=0;
    session_started=1;session_epoch=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED);
    char path[544];copy(path,lock_capture_path,lock_capture_path_length);copy(path+lock_capture_path_length,"\\session-00000001.bin",22);
    session_file=CreateFileA(path,0x40000000,1,0,1,0x80,0);
    if(session_file==(HANDLE)-1){session_file=0;lock_diagnostic("session_file_failed",0,0,0,0,0,0,0);return 0;}
    u32 header[4];copy(header,"MNMCMD01",8);header[2]=1;header[3]=16;
    if(!write_all(session_file,header,16)){CloseHandle(session_file);session_file=0;return 0;}
    session_bytes=16;lock_diagnostic("session_started",0,0,0,0,0,0,0);return 1;
}
static struct SessionSurface* session_find(void* object){
    struct SessionSurface* found=0;
    for(u32 i=0;i<32;++i)if(session_surfaces[i].id && game_alias_same(object,session_surfaces[i].object)){
        if(found){game_session_gap(4);return 0;}found=session_surfaces+i;
    }
    return found;
}
static void game_session_invalidate(void* object){if(session_file && session_find(object))game_session_gap(3);}
static struct SessionSurface* session_surface(void* object,const struct Snapshot* pixels){
    if(!session_start())return 0;
    struct SessionSurface* s=session_find(object);
    if(!pixels->data){game_session_gap(4);return 0;}
    if(s){
        if(s->width!=pixels->width || s->height!=pixels->height || s->bits!=pixels->bits ||
           s->r!=pixels->r || s->g!=pixels->g || s->b!=pixels->b){game_session_gap(3);return 0;}
        return s;
    }
    for(u32 i=0;i<32;++i)if(!session_surfaces[i].id){
        u32 count=pixels->width*pixels->height;if(count>16777216-session_pixels){game_session_gap(2);return 0;}
        s=session_surfaces+i;s->object=object;s->id=i+1;s->width=pixels->width;s->height=pixels->height;s->bits=pixels->bits;
        s->r=pixels->r;s->g=pixels->g;s->b=pixels->b;session_pixels+=count;
        u32 fields[7]={s->id,s->width,s->height,s->bits,s->r,s->g,s->b};
        if(!session_record(1,fields,28,pixels->data,pixels->length))return 0;return s;
    }
    game_session_gap(2);return 0;
}
static int session_colors(struct SessionSurface* s){
    if(s->bits!=8)return 1;
    struct GameSurface* surface=game_surface_find(s->object,0);u8 colors[1024],rgb[768];
    if(!surface || !game_surface_colors(surface,colors)){game_session_gap(5);return 0;}
    if(s->palette_known && same(colors,s->palette,1024))return 1;
    for(u32 i=0;i<256;++i)copy(rgb+i*3,colors+i*4,3);
    u32 fields[3]={s->id,0,256};if(!session_record(4,fields,12,rgb,768))return 0;
    copy(s->palette,colors,1024);s->palette_known=1;return 1;
}
static int session_check(struct SessionSurface* s,const struct Snapshot* pixels){return s && session_record(5,&s->id,4,pixels->data,pixels->length);}
static int session_present(struct SessionSurface* s){
    if(!s || !session_colors(s) || !session_record(6,&s->id,4,0,0))return 0;
    ++session_presented;return 1;
}
static void session_finish_owned(void){
    if(!session_file)return;
    game_session_sync();if(!session_file)return;
    if(!session_presented){game_session_gap(6);return;}
    for(u32 i=0;i<32;++i)if(game_locks[i].active && session_find(game_locks[i].object)){game_session_gap(3);return;}
    if(!session_file)return;
    int ok=1;
    for(u32 i=0;ok && i<32;++i)if(session_surfaces[i].id)ok=command_record(session_file,&session_sequence,7,&session_surfaces[i].id,4,0,0);
    if(ok)ok=command_record(session_file,&session_sequence,8,0,0,0,0);
    CloseHandle(session_file);session_file=0;
    lock_diagnostic(ok?"session_finished":"session_file_failed",0,0,session_operations,0,0,0,0);
}
static void game_session_finish(void){
    if(!__sync_bool_compare_and_swap(&game_locks_busy,0,1))return;
    session_finish_owned();__sync_lock_release(&game_locks_busy);
}
static void session_done(void){if(session_file && ++session_operations>=16)session_finish_owned();}
static void game_session_unlock(struct GameLock* lock,const struct Snapshot* after,u32 primary){
    if(!game_session_enabled)return;
    struct SessionSurface* s=session_find(lock->object);int existing=s!=0;
    s=session_surface(lock->object,lock->rectangle?&lock->base:after);if(!s)return;
    if(lock->rectangle || existing){
        struct Rect r={0,0,(i32)after->width,(i32)after->height};if(lock->rectangle)copy(&r,&lock->region,16);
        if(lock->rectangle && !session_check(s,&lock->base))return;
        u32 stride=after->width*(after->bits/8),bytes=(u32)(r.right-r.left)*(after->bits/8);
        for(i32 y=r.top;y<r.bottom;++y){u32 fields[5]={s->id,(u32)r.left,(u32)y,(u32)(r.right-r.left),1};
            if(!session_record(2,fields,20,after->data+(u32)y*stride+(u32)r.left*(after->bits/8),bytes))return;}
    }
    if(!session_check(s,after) || (primary && !session_present(s)))return;session_done();
}
static void game_session_blit_begin(struct GameBlit* p){
    if(!game_session_enabled)return;
    struct SessionSurface* a=session_surface(p->source,&p->src),*b=session_surface(p->target,&p->dst);
    if(!session_check(a,&p->src) || !session_check(b,&p->dst))return;
}
static void game_session_blit_end(struct GameBlit* p,u32 primary){
    if(!session_file)return;
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
    if(!session_file)return;
    struct SessionSurface *a=session_find(front->object),*b=session_find(back->object);if(!a || !b){game_session_gap(4);return;}
    u32 ids[2]={a->id,b->id};
    if(!session_record(11,ids,8,0,0) || !session_check(a,&front->pixels) || !session_check(b,&back->pixels) || !session_present(a))return;session_done();
}
static void game_session_palette(struct GameSurface* surface){
    if(!session_file || !surface->pixels.data)return;
    struct SessionSurface* s=session_find(surface->object);if(!s)return;
    if(!session_check(s,&surface->pixels) || !session_colors(s) || (surface->primary && !session_present(s)))return;session_done();
}

static void game_session_palette_changed(void* object){
    if(!session_file)return;struct GamePalette* palette=game_palette_find(object,0);if(!palette)return;
    for(u32 i=0;i<32 && session_file;++i){struct GameSurface* s=game_surfaces+i;
        if(s->object && s->palette && game_palette_find(s->palette,0)==palette && session_find(s->object)){
            if(!game_palette_complete(palette)){game_session_gap(5);return;}game_session_palette(s);
        }
    }
}
