/* Verified double-buffer flips within an already seeded history. Observer
 * snapshots finish before Flip; the temporary attachment reference is balanced.
 * Longer chains and field/stereo flips fail closed after successful API calls.
 */
struct HistoryFlip {void* back;struct HistorySurface *front_surface,*back_surface;struct Snapshot front,before_back;u32 ready;};
typedef i32 (WIN *AttachedSurface)(void*,const u32*,void**);
/* Separate snapshot frames stay below the PE32 stack-probe threshold without CRT. */
static __attribute__((noinline)) void history_flip_before(void* object,void* target,u32 flags,struct HistoryFlip* f){
    zero(f,sizeof(*f));struct Table* t=lookup(object);u32 d[31],bd[31],caps[4]={4,0,0,0};
    /* WAIT/NOVSYNC/DONOTWAIT change scheduling, not pixel storage. */
    if((flags&~0x29u) || (flags&0x21u)==0x21u || !surface_description(object,t,d) ||
       (d[26]&0x238u)!=0x238u || !(d[1]&0x20u) || d[5]!=1 || d[27] || d[28] || d[29] || (d[26]&4u) || !supported_format(d) || !t->original[12])return;
    struct HistorySurface* h=history_surface_resolve(object);
    if(h && h->locked)return;
    if(((AttachedSurface)t->original[12])(object,caps,&f->back)<0 || !f->back)return;
    install_table(f->back,t->kind);struct Table* bt=lookup(f->back);
    if(!bt || bt->kind!=t->kind || !surface_description(f->back,bt,bd) ||
       (bd[26]&0x1cu)!=0x1cu || (bd[26]&0x220u) || bd[27] || bd[28] || bd[29] ||
       d[2]!=bd[2] || d[3]!=bd[3] || d[19]!=bd[19] || !same(d+21,bd+21,16) ||
       !distinct_surfaces(object,t,f->back,bt))return;
    h=history_surface_resolve(f->back);if(h && h->locked)return;
    if(!snapshot(object,t,d,&f->front) || !snapshot(f->back,bt,bd,&f->before_back))return;
    f->front_surface=history_surface(object,&f->front);f->back_surface=history_surface(f->back,&f->before_back);
    if(!history_current() || !f->front_surface || !f->back_surface || f->front_surface==f->back_surface ||
       (target && history_surface_resolve(target)!=f->back_surface) ||
       !history_palette_matches(f->front_surface,&f->front) || !history_palette_matches(f->back_surface,&f->before_back))return;
    if(!history_record(5,&f->front_surface->id,4,f->front.data,f->front.length) ||
       !history_record(5,&f->back_surface->id,4,f->before_back.data,f->before_back.length))return;
    f->ready=1;
}
static __attribute__((noinline)) void history_flip_after(void* object,struct HistoryFlip* f,i32 status){
    if(status>=0 && history_current()){
        if(!f->ready)history_gap(6);
        else {
            struct Snapshot front,back;zero(&front,sizeof(front));zero(&back,sizeof(back));
            if(!snapshot(object,lookup(object),f->front_surface->desc,&front) ||
               !snapshot(f->back,lookup(f->back),f->back_surface->desc,&back))history_gap(6);
            else if(history_palette_matches(f->front_surface,&front) && history_palette_matches(f->back_surface,&back)){
                u32 ids[2]={f->front_surface->id,f->back_surface->id};
                if(history_record(11,ids,8,0,0) && history_record(5,ids,4,front.data,front.length) &&
                   history_record(5,ids+1,4,back.data,back.length) && history_record(6,ids,4,0,0)){
                    history_presented=ids[0];
                    if(front.bits==8)history_color_snapshot(f->front_surface,&front);
                    if(back.bits==8)history_color_snapshot(f->back_surface,&back);
                    ++history_operations;
                }
            }
            free_snapshot(&front);free_snapshot(&back);
        }
    }
    free_snapshot(&f->front);free_snapshot(&f->before_back);if(f->back)observer_release(f->back);
}
