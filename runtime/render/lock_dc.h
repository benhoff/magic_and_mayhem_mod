/* Borrow only the bitmap selected into an application-held surface DC.
 * No observer GetDC, Lock, GetDIBits, or bitmap replacement is permitted. */
struct GameDC {void* target;struct Snapshot pixels;u32 epoch,generation;};
static void game_dc_acquired(void* object,void* dc){
    if(!lock_capture_path_length)return;
    HANDLE bitmap=dc?GetCurrentObject(dc,7):0;
    if(!game_surface_pixels_enter(object))return;
    struct GameSurface* s=game_surface_find(object,0);
    if(s && s->layout_known && !s->dc && dc && bitmap){
        game_session_dc_acquire(object);
        game_surface_pixels_invalidate_locked(object);
        s->dc=dc;s->dc_bitmap=bitmap;s->dc_owner=GetCurrentThreadId();
        s->dc_generation=s->generation;
    }else game_surface_invalidate_locked(object);
    lock_diagnostic("dc_acquired",object,0,(u32)dc,0,0,0,0);game_tracker_release();
}
static void game_dc_before(void* object,void* dc,struct GameDC* pending){
    zero(pending,sizeof(*pending));pending->target=object;
    if(!lock_capture_path_length)return;
    /* GDI may batch writes. Flush this thread before reading the DIB bits;
     * these metadata queries and the flush occur outside the tracker guard. */
    u32 dib[21]={0};HANDLE bitmap=GetCurrentObject(dc,7);
    i32 size=GetObjectA(bitmap,84,dib);int flushed=GdiFlush();
    u32 values[19]={(u32)object,GetCurrentThreadId(),(u32)dc,(u32)bitmap,(u32)size,dib[1],dib[2],dib[3],dib[4],dib[5],dib[6],dib[7],dib[8],dib[9],dib[10],dib[16],dib[17],dib[18],(u32)flushed};
    lock_diagnostic_values("dc_bitmap",values);
    if(!game_surface_pixels_enter(object))return;
    const char* reason="dc_unmatched";struct GameSurface* s=game_surface_find(object,0);
    if(!s || !s->layout_known || s->dc!=dc || s->dc_bitmap!=bitmap || s->dc_owner!=GetCurrentThreadId() ||
       s->dc_generation!=s->generation)goto done;
    reason="dc_bitmap_rejected";
    struct Snapshot* p=&s->pixels;u32 pitch=dib[3],row=p->width*(p->bits/8),height=p->height;
    i32 dib_height=(i32)dib[8];
    if(!flushed || size!=84 || dib[6]!=40 || dib[1]!=p->width || dib[2]!=height || dib[7]!=p->width ||
       (dib_height!=(i32)height && dib_height!=-(i32)height) || dib[4]!=(p->bits<<16|1) || dib[9]!=dib[4] ||
       (p->bits!=16 && p->bits!=24 && p->bits!=32) || !dib[5] || row&1 || pitch<row || pitch>32768)goto done;
    u32 r=dib[16],g=dib[17],b=dib[18];
    if(dib[10]==0){r=p->bits==16?0x7c00:0xff0000;g=p->bits==16?0x3e0:0xff00;b=p->bits==16?0x1f:0xff;}
    else if(dib[10]!=3)goto done;
    if(r!=p->r || g!=p->g || b!=p->b)goto done;
    reason="dc_memory";if(!game_surface_blank(&pending->pixels,p))goto done;
    pending->epoch=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED);pending->generation=s->generation;reason="dc_ready";
 done:lock_diagnostic(reason,object,0,(u32)dc,0,0,0,0);game_tracker_release();
    /* GetObject normalizes biHeight to positive, even for a top-down DIB.
     * Wine GetBitmapBits returns top-down native rows aligned to 16 bits.
     * Only even row-byte lengths are admitted, so the owned layout matches.
     * Do not infer orientation from GetObject or read bmBits directly. */
    if(pending->pixels.data){
        if(GetBitmapBits(bitmap,(i32)pending->pixels.length,pending->pixels.data)!=(i32)pending->pixels.length){
            __atomic_sub_fetch(&lock_capture_reserved,pending->pixels.length,__ATOMIC_RELAXED);free_snapshot(&pending->pixels);
            lock_diagnostic("dc_read_failed",object,0,(u32)dc,0,0,0,0);
        }else lock_diagnostic("dc_copied",object,0,(u32)dc,0,0,0,0);
    }
}
static void game_dc_after(struct GameDC* pending,void* dc,i32 result){
    if(!lock_capture_path_length)return;
    lock_diagnostic(result<0?"dc_release_failed":"dc_released",pending->target,0,(u32)dc,0,result,0,0);
    if(!game_surface_pixels_enter(pending->target))goto free;
    struct GameSurface* s=game_surface_find(pending->target,0);
    if(result>=0){
        if(s && pending->pixels.data && s->dc==dc && s->dc_owner==GetCurrentThreadId() &&
           s->generation==pending->generation && pending->epoch==__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED)){
            /* Preserve metadata already observed from application descriptors. */
            game_surface_drop(s);copy(&s->pixels,&pending->pixels,sizeof(s->pixels));pending->pixels.data=0;
            game_surface_bytes+=s->pixels.length;__atomic_sub_fetch(&lock_capture_reserved,s->pixels.length,__ATOMIC_RELAXED);
            game_session_dc_checkpoint(s);
            if(s->primary)game_surface_publish(s);
            lock_diagnostic("dc_checkpoint",pending->target,0,(u32)dc,0,result,0,0);
        }else game_surface_invalidate_locked(pending->target);
    }
    game_tracker_release();
 free:if(pending->pixels.data)__atomic_sub_fetch(&lock_capture_reserved,pending->pixels.length,__ATOMIC_RELAXED);free_snapshot(&pending->pixels);
}
