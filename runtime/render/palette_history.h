/* Palette observation holds no COM references. Entries are RGB+reserved flags;
 * flags never become alpha. Slots retire when no tracked surface uses them. */
struct HistoryPalette {u32 used,live,count;void* aliases[16];u8 entries[1024];};
static struct HistoryPalette history_palettes[16];
static struct HistoryPalette* history_palette_find(void* object){
    for(u32 i=0;i<16;++i)if(history_palettes[i].live)
        for(u32 j=0;j<history_palettes[i].count;++j)if(history_palettes[i].aliases[j]==object)return history_palettes+i;
    return 0;
}
static struct HistoryPalette* history_palette_resolve(void* object){
    struct HistoryPalette* p=history_palette_find(object);if(p)return p;
    struct Table* t=lookup(object);void* identity=0;
    static const u8 unknown[16]={0,0,0,0,0,0,0,0,0xc0,0,0,0,0,0,0,0x46};
    if(!t || t->kind!=20 || ((Query)t->original[0])(object,unknown,&identity)<0 || !identity){history_gap(4);return 0;}
    p=history_palette_find(identity);observer_release(identity);if(!p)return 0;
    if(p->count>=16){history_gap(2);return 0;}p->aliases[p->count++]=object;return p;
}
static int palette_rgb_equal(const u8* a,const u8* b,u32 first,u32 count){
    for(u32 i=first;i<first+count;++i)if(!same(a+i*4,b+i*4,3))return 0;return 1;
}
static int palette_read(void* object,u8* entries){
    struct Table* t=lookup(object);u32 caps=0;
    return t && t->kind==20 && ((i32 (WIN *)(void*,u32*))t->original[3])(object,&caps)>=0 &&
        (caps&4) && !(caps&~0xfcu) &&
        ((i32 (WIN *)(void*,u32,u32,u32,void*))t->original[4])(object,0,0,256,entries)>=0;
}
static void history_palette_retire(struct HistoryPalette* p){
    if(!p)return;
    for(u32 i=0;i<16;++i)if(history_surfaces[i].live && history_surfaces[i].palette==p)return;
    p->live=0;
}
static struct HistoryPalette* history_palette_observe(void* object){
    install_table(object,20);struct Table* t=lookup(object);void* identity=0;
    static const u8 unknown[16]={0,0,0,0,0,0,0,0,0xc0,0,0,0,0,0,0,0x46};
    if(!t || t->kind!=20 || ((Query)t->original[0])(object,unknown,&identity)<0 || !identity){history_gap(4);return 0;}
    struct Table* canonical=lookup(identity);int covered=canonical && canonical->kind==20;
    struct HistoryPalette* p=history_palette_find(identity);observer_release(identity);
    if(!covered){history_gap(4);return 0;}
    if(p){
        if(!history_palette_find(object)){
            if(p->count>=16){history_gap(2);return 0;}p->aliases[p->count++]=object;
        }
        u8 now[1024];if(!palette_read(object,now) || !palette_rgb_equal(now,p->entries,0,256)){history_gap(5);return 0;}
        return p;
    }
    for(u32 i=0;i<16;++i)if(!history_palettes[i].used){
        p=history_palettes+i;p->used=p->live=1;p->aliases[p->count++]=object;
        if(identity!=object)p->aliases[p->count++]=identity;
        if(!palette_read(object,p->entries)){history_gap(5);return 0;}return p;
    }
    history_gap(2);return 0;
}
static int history_palette_matches(struct HistorySurface* h,const struct Snapshot* s){
    if(s->bits!=8)return 1;
    if(!h->palette || !h->palette->live || !palette_rgb_equal(s->palette,h->palette->entries,0,256)){history_gap(5);return 0;}
    return 1;
}
static int history_color_snapshot(struct HistorySurface* h,const struct Snapshot* s){
    if(!history_palette_matches(h,s))return 0;
    u8* rgba=HeapAlloc(GetProcessHeap(),0,s->width*s->height*4);if(!rgba){history_gap(2);return 0;}
    int ok=render_pixels(rgba,s->width,s->height,s->data,s->width,8,0,0,0,s->palette) &&
        history_record(10,&h->id,4,rgba,s->width*s->height*4);
    HeapFree(GetProcessHeap(),0,rgba);if(!ok && history_current())history_gap(5);return ok;
}
static int history_color_check(struct HistorySurface* h){
    if(h->locked){history_gap(3);return 0;}
    struct Snapshot s;zero(&s,sizeof(s));
    if(!snapshot(h->canonical,lookup(h->canonical),h->desc,&s)){history_gap(5);return 0;}
    int ok=history_color_snapshot(h,&s);free_snapshot(&s);return ok;
}
static int history_palette_emit(struct HistorySurface* h,u32 first,u32 count,int check){
    u32 f[3]={h->id,first,count};u8 rgb[768];
    for(u32 i=0;i<count;++i)copy(rgb+i*3,h->palette->entries+(first+i)*4,3);
    return history_record(4,f,12,rgb,count*3) && (!check || history_color_check(h));
}
static int history_palette_bind(struct HistorySurface* h,int check){
    struct Table* t=lookup(h->canonical);void* palette=0;
    if(((GetObject)t->original[20])(h->canonical,&palette)<0 || !palette){history_gap(5);return 0;}
    struct HistoryPalette* p=history_palette_observe(palette);observer_release(palette);
    if(!p || !history_current())return 0;
    struct HistoryPalette* previous=h->palette;h->palette=p;history_palette_retire(previous);
    return history_palette_emit(h,0,256,check);
}
static void history_set_palette(void* object,void* palette){
    struct HistorySurface* h=history_surface_resolve(object);if(!h)return;
    if(h->desc[21]!=8){history_gap(6);return;}
    if(!palette){history_gap(5);return;}
    if(!history_palette_bind(h,1))return;
    if(history_presented==h->id)history_record(6,&h->id,4,0,0);
    ++history_operations;
}
static void history_palette_alias(void* object,void* alias){
    struct HistoryPalette* p=history_palette_resolve(object);if(!p || history_palette_find(alias)==p)return;
    struct Table* t=lookup(alias);
    if(history_palette_find(alias) || !t || t->kind!=20){history_gap(4);return;}
    if(p->count>=16){history_gap(2);return;}p->aliases[p->count++]=alias;
}
static void history_palette_entries(void* object,u32 flags,u32 first,u32 count){
    struct HistoryPalette* p=history_palette_resolve(object);if(!p)return;
    if(flags || !count || first>=256 || count>256-first){history_gap(5);return;}
    u8 now[1024];if(!palette_read(object,now) || !palette_rgb_equal(now,p->entries,0,first) ||
        !palette_rgb_equal(now,p->entries,first+count,256-first-count)){history_gap(5);return;}
    copy(p->entries,now,1024);
    for(u32 i=0;i<16;++i){struct HistorySurface* h=history_surfaces+i;
        if(h->live && h->palette==p){
            if(!history_palette_emit(h,first,count,1))return;
            if(history_presented==h->id && !history_record(6,&h->id,4,0,0))return;
        }
    }
    ++history_operations;
}
static void history_palette_release(struct HistoryPalette* p,u32 remaining){
    if(!p || remaining)return;
    p->live=0;
    for(u32 i=0;i<16;++i)if(history_surfaces[i].live && history_surfaces[i].palette==p){history_gap(5);return;}
}
