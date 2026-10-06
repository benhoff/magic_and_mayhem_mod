/* Application-observed 256-entry RGB palettes. All access uses game_locks_busy. */
struct GamePalette {void* aliases[16];u32 count,epoch,generation,identity_generation,session,caps,known[8];u8 colors[1024];};
static struct GamePalette game_palettes[32];
static u32 game_palette_identity_next;
static int game_palette_supported(u32 caps){return (caps&0x44)==0x44 && !(caps&~0x54u);}
static struct GamePalette* game_palette_find(void* object,int create){
    if(!object)return 0;struct GamePalette* empty=0;
    u32 epoch=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED);
    for(u32 i=0;i<32;++i){struct GamePalette* p=game_palettes+i;
        if(p->count && p->epoch!=epoch)zero(p,sizeof(*p));
        if(!p->count){if(!empty)empty=p;continue;}
        for(u32 j=0;j<p->count;++j)if(p->aliases[j]==object)return p;
    }
    if(create && empty){
        if(game_palette_identity_next==0xffffffffu){game_session_gap(2);return 0;}
        empty->identity_generation=++game_palette_identity_next;empty->aliases[0]=object;empty->count=1;empty->epoch=epoch;empty->generation=++game_surface_generation;return empty;}
    if(create && game_session_palette_resources)game_session_gap(2);
    return 0;
}
static int game_palette_complete(const struct GamePalette* p){
    if(!p || !game_palette_supported(p->caps))return 0;
    for(u32 i=0;i<8;++i)if(p->known[i]!=0xffffffffu)return 0;return 1;
}
static int game_surface_colors(struct GameSurface* surface,u8* colors){
    struct GamePalette* p=game_palette_find(surface->palette,0);if(!game_palette_complete(p))return 0;
    copy(colors,p->colors,1024);return 1;
}
static int game_surface_publish(struct GameSurface* surface){
    struct Snapshot* pixels=&surface->pixels;
    if(pixels->bits==8){
        struct GamePalette* p=game_palette_find(surface->palette,0);
        if(!game_palette_complete(p) || !pixels->data)return 0;
        copy(pixels->palette,p->colors,1024);
    }
    if(!game_publish_pixels(pixels,surface))return 0;
    if(pixels->bits==8)lock_diagnostic("indexed_presented",surface->object,0,(u32)surface->palette,0,0,0,0);return 1;
}
static int game_surface_publish_region(struct GameSurface* surface,u32 left,u32 top,u32 width,u32 height,u32 previous_generation){
    struct Snapshot* s=&surface->pixels;
    if(!stream || !s->data)return 0;
    if(!width || !height || left>s->width || top>s->height || width>s->width-left || height>s->height-top)return 0;
    if(game_presented_object!=surface->object || game_presented_generation!=previous_generation ||
       (width==s->width && height==s->height))return game_surface_publish(surface);
    if(s->bits==8 && !game_surface_colors(surface,s->palette))return 0;
    if(!__sync_bool_compare_and_swap(&capture_busy,0,1))return 0;
    if(stream[MNM_FRAME_V1_FRAME_COUNT_OFFSET/4]!=game_presented_frame){
        __sync_lock_release(&capture_busy);return game_surface_publish(surface);
    }
    u32 sequence=__atomic_load_n(stream+MNM_FRAME_V1_SEQUENCE_OFFSET/4,__ATOMIC_RELAXED);
    __atomic_store_n(stream+MNM_FRAME_V1_SEQUENCE_OFFSET/4,sequence+1,__ATOMIC_SEQ_CST);
    u32 bytes=s->bits/8;int ok=1;
    for(u32 y=0;y<height && ok;++y){
        u8* out=(u8*)stream+MNM_FRAME_V1_PIXELS_OFFSET+((top+y)*s->width+left)*4;
        const u8* in=s->data+((top+y)*s->width+left)*bytes;
        ok=render_pixels(out,width,1,in,(i32)(s->width*bytes),s->bits,s->r,s->g,s->b,s->bits==8?s->palette:0);
    }
    if(ok){++stream[MNM_FRAME_V1_FRAME_COUNT_OFFSET/4];stream[MNM_FRAME_V1_STATUS_OFFSET/4]=MNM_FRAME_V1_STATUS_FRAME_PUBLISHED;
        game_presented_generation=surface->generation;game_presented_frame=stream[MNM_FRAME_V1_FRAME_COUNT_OFFSET/4];}
    else game_presented_object=0;
    __atomic_store_n(stream+MNM_FRAME_V1_SEQUENCE_OFFSET/4,sequence+2,__ATOMIC_RELEASE);__sync_lock_release(&capture_busy);
    lock_diagnostic(ok?"primary_region_presented":"primary_region_failed",surface->object,0,width,height,0,0,0);return ok;
}
static void game_palette_republish(struct GamePalette* palette){
    struct GameSurface* primary=0;
    for(u32 i=0;i<GAME_SURFACE_COUNT;++i){struct GameSurface* s=game_surfaces+i;
        if(!s->object || !s->primary || !s->pixels.data || s->pixels.bits!=8)continue;
        if(game_palette_find(s->palette,0)!=palette)continue;
        if(primary){lock_diagnostic("palette_ambiguous",s->object,0,0,0,0,0,0);return;}primary=s;
    }
    if(primary)game_surface_publish(primary);
}
static void game_surface_palette(void* object,void* palette){
    if(!game_surface_enter())return;struct GameSurface* s=game_surface_find(object,1);
    if(s){s->palette=palette;s->generation=++game_surface_generation;s->generation_origin=12;
        lock_diagnostic("palette_attached",object,0,(u32)palette,0,0,0,0);
        game_session_palette(s);if(s->primary && s->pixels.bits==8)game_surface_publish(s);}
    game_tracker_release();
}
static void game_palette_alias(void* object,void* alias){
    if(!game_surface_enter())return;struct GamePalette* p=game_palette_find(object,1),*other=game_palette_find(alias,0);
    if(p && other!=p){
        if(other){
            if(game_session_palette_resources && (game_session_palette_registered(p) || game_session_palette_registered(other))){game_session_gap(4);game_tracker_release();return;}
            if(p->count+other->count>16)goto overflow;
            for(u32 i=0;i<other->count;++i)p->aliases[p->count++]=other->aliases[i];
            zero(other,sizeof(*other));zero(p->known,sizeof(p->known));p->caps=0;
        }else{if(p->count>=16)goto overflow;p->aliases[p->count++]=alias;}
        p->generation=++game_surface_generation;
    }
    game_tracker_release();return;
 overflow:__atomic_add_fetch(&game_lock_epoch,1,__ATOMIC_RELAXED);game_tracker_release();
}
static void game_palette_caps(void* object,u32 caps){
    if(!game_surface_enter())return;struct GamePalette* p=game_palette_find(object,1);
    if(p){if(!game_palette_supported(caps) || !game_palette_supported(p->caps))zero(p->known,sizeof(p->known));
        p->caps=caps;p->generation=++game_surface_generation;
        if(!game_palette_complete(p))game_session_palette_changed(object);
        lock_diagnostic(game_palette_supported(caps)?"palette_caps":"palette_caps_rejected",object,20,0,caps,0,0,0);}
    game_tracker_release();
}
struct GamePaletteUpdate {u32 epoch,generation,identity_generation,valid;u8 colors[1024];};
static void game_palette_before(void* object,u32 flags,u32 first,u32 count,void* entries,int input,struct GamePaletteUpdate* pending){
    zero(pending,sizeof(*pending));if(!game_surface_enter())return;
    struct GamePalette* p=game_palette_find(object,1);
    pending->epoch=__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED);pending->generation=p?p->generation:0;pending->identity_generation=p?p->identity_generation:0;
    pending->valid=p && game_palette_supported(p->caps) && !flags && count && first<256 && count<=256-first;
    if(input && pending->valid){pending->valid=readable(entries,count*4);if(pending->valid)copy(pending->colors,entries,count*4);}
    game_tracker_release();
}
static void game_palette_after(void* object,u32 first,u32 count,void* entries,int output,struct GamePaletteUpdate* pending,i32 result){
    if(result<0)return;if(!game_surface_enter())return;
    struct GamePalette* p=game_palette_find(object,1);
    if(p){
        int valid=pending->valid && pending->epoch==__atomic_load_n(&game_lock_epoch,__ATOMIC_RELAXED) && pending->generation==p->generation && pending->identity_generation==p->identity_generation;
        if(output && valid){valid=readable(entries,count*4);if(valid)copy(pending->colors,entries,count*4);}
        if(valid){copy(p->colors+first*4,pending->colors,count*4);for(u32 i=first;i<first+count;++i)p->known[i/32]|=1u<<(i%32);}
        else zero(p->known,sizeof(p->known));
        p->generation=++game_surface_generation;
        lock_diagnostic(valid?"palette_entries":"palette_invalidated",object,20,first,count,result,0,0);
        game_session_palette_changed(object);
        if(valid)game_palette_republish(p);
    }
    game_tracker_release();
}
static void game_palette_created(void* object,u32 caps,const u8* colors){
    if(!game_surface_enter())return;
    if(game_session_palette_resources && game_palette_find(object,0)){game_session_gap(3);game_tracker_release();return;}
    struct GamePalette* p=game_palette_find(object,1);
    if(p){zero(p->known,sizeof(p->known));p->caps=caps;p->generation=++game_surface_generation;
        if(colors && game_palette_supported(caps)){copy(p->colors,colors,1024);for(u32 i=0;i<8;++i)p->known[i]=0xffffffffu;}}
    game_tracker_release();
}
static void game_palette_invalidated(void* object){
    if(!game_surface_enter())return;struct GamePalette* p=game_palette_find(object,0);
    if(p){zero(p->known,sizeof(p->known));p->caps=0;p->generation=++game_surface_generation;game_session_palette_changed(object);}
    game_tracker_release();
}

/* Final original Release has already destroyed the COM object. Tokens only. */
static void game_palette_retired(void* object){
    if(!game_surface_enter())return;
    struct GamePalette* p=game_palette_find(object,0);
    if(p){
        game_session_palette_retire(p);
        for(u32 i=0;i<GAME_SURFACE_COUNT;++i){struct GameSurface* s=game_surfaces+i;
            if(s->object && s->palette && game_palette_find(s->palette,0)==p){s->palette=0;s->generation=++game_surface_generation;s->generation_origin=12;}}
        zero(p,sizeof(*p));
    }
    game_tracker_release();
}
