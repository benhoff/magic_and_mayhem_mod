/* Opt-in bounded native-pixel history. Every observed gap makes the file unreplayable.
 * Application locks are never reacquired while held; all observer COM releases
 * bypass hooks. No references are retained and no dead objects are dereferenced.
 */
struct HistoryPalette;
struct HistorySurface {struct HistoryPalette* palette;void* canonical;u32 id,live;void* aliases[16];u32 aliases_count,desc[31],locked,writable;};
static struct HistorySurface history_surfaces[16];
static HANDLE history_handle;
static HANDLE history_current(void){return __atomic_load_n(&history_handle,__ATOMIC_ACQUIRE);}
static void history_set(HANDLE file){__atomic_store_n(&history_handle,file,__ATOMIC_RELEASE);}
static volatile i32 history_busy,history_invalid;
static u32 history_started,history_sequence,history_bytes,history_operations,history_presented;
static int history_wants_draw(void){return history_current() && !__atomic_load_n(&history_invalid,__ATOMIC_ACQUIRE);}
static void history_gap(u32 reason){
    if(!history_current())return;
    u32 record[4]={9,++history_sequence,4,reason};write_all(history_current(),record,16);
    CloseHandle(history_current());history_set(0);
}
static int history_enter(void){
    if(!history_current())return 0;
    if(!__sync_bool_compare_and_swap(&history_busy,0,1)){__atomic_store_n(&history_invalid,1,__ATOMIC_RELEASE);return 0;}
    if(__atomic_load_n(&history_invalid,__ATOMIC_ACQUIRE)){history_gap(1);__sync_lock_release(&history_busy);return 0;}return 1;
}
static void history_finish_owned(void);
static void history_leave(int token){
    if(!token)return;
    if(__atomic_load_n(&history_invalid,__ATOMIC_ACQUIRE))history_gap(1);
    else if(history_current()){
        u32 live=0;for(u32 i=0;i<16;++i)live+=history_surfaces[i].live;
        if(!live || history_operations>=16)history_finish_owned();
    }
    __sync_lock_release(&history_busy);
}
static int history_record(u32 op,const void* fields,u32 fl,const void* bytes,u32 bl){
    if(!history_current())return 0;
    if(history_sequence>=240 || history_bytes>64*1024*1024-12 || fl+bl>64*1024*1024-history_bytes-12){history_gap(2);return 0;}
    if(!command_record(history_current(),&history_sequence,op,fields,fl,bytes,bl)){CloseHandle(history_current());history_set(0);return 0;}
    history_bytes+=12+fl+bl;return 1;
}
static struct HistorySurface* history_find(void* object){
    for(u32 i=0;i<16;++i)if(history_surfaces[i].live)
        for(u32 j=0;j<history_surfaces[i].aliases_count;++j)if(history_surfaces[i].aliases[j]==object)return history_surfaces+i;
    return 0;
}
static struct HistorySurface* history_surface_resolve(void* object){
    struct HistorySurface* h=history_find(object);if(h)return h;
    struct Table* t=lookup(object);void* identity=0;
    static const u8 unknown[16]={0,0,0,0,0,0,0,0,0xc0,0,0,0,0,0,0,0x46};
    if(!t || t->kind<11 || t->kind>17 || ((Query)t->original[0])(object,unknown,&identity)<0 || !identity){history_gap(4);return 0;}
    h=history_find(identity);observer_release(identity);if(!h)return 0;
    if(h->aliases_count>=16){history_gap(2);return 0;}h->aliases[h->aliases_count++]=object;return h;
}
#include "palette_history.h"
static void history_finish_owned(void){
    if(!history_current())return;
    if(__atomic_load_n(&history_invalid,__ATOMIC_ACQUIRE)){history_gap(1);return;}
    for(u32 i=0;i<16;++i)if(history_surfaces[i].live && history_surfaces[i].locked){history_gap(3);return;}
    for(u32 i=0;i<16;++i)if(history_surfaces[i].live){
        if(!history_record(7,&history_surfaces[i].id,4,0,0))return;history_surfaces[i].live=0;
    }
    if(history_record(8,0,0,0,0)){CloseHandle(history_current());history_set(0);}
}
static struct HistorySurface* history_surface(void* object,const struct Snapshot* s){
    struct HistorySurface* found=history_find(object);if(found)return found;
    struct Table* t=lookup(object);void* identity=0;
    static const u8 unknown[16]={0,0,0,0,0,0,0,0,0xc0,0,0,0,0,0,0,0x46};
    if(!t || ((Query)t->original[0])(object,unknown,&identity)<0 || !identity){history_gap(4);return 0;}
    /* Compare identity only while the returned reference is still held. */
    found=history_find(identity);
    struct Table* canonical=lookup(identity);
    u32 canonical_kind=canonical?canonical->kind:0;
    observer_release(identity);
    if(found){
        if(found->aliases_count>=16){history_gap(2);return 0;}
        found->aliases[found->aliases_count++]=object;return found;
    }
    for(u32 i=0;i<16;++i)if(!history_surfaces[i].id){
        struct HistorySurface* h=history_surfaces+i;
        h->id=i+1;h->live=1;h->canonical=identity;h->desc[19]=s->bits==8?0x60:0x40;h->desc[2]=s->height;h->desc[3]=s->width;h->desc[21]=s->bits;
        h->desc[22]=s->r;h->desc[23]=s->g;h->desc[24]=s->b;h->aliases[0]=object;h->aliases_count=1;
        // Canonical identity must use an already intercepted surface vtable.
        if(canonical_kind<11 || canonical_kind>17){history_gap(4);return 0;}
        if(identity!=object)h->aliases[h->aliases_count++]=identity;
        u32 fields[7]={h->id,s->width,s->height,s->bits,s->bits==8?0:s->r,s->bits==8?0:s->g,s->bits==8?0:s->b};
        if(!history_record(1,fields,28,s->data,s->length))return 0;
        if(s->bits==8 && (!history_palette_bind(h,0) || !history_color_snapshot(h,s)))return 0;return h;
    }
    history_gap(2);return 0;
}
static void history_draw(const struct DrawCapture* c,void* object){
    if(!history_started){
        char enabled[4];if(GetEnvironmentVariableA("MNM_RENDER_HISTORY",enabled,4)!=1 || enabled[0]!='1')return;
        history_started=1;__sync_bool_compare_and_swap(&history_busy,0,1);char path[512];u32 n=0,last=0;
        while(draw_path[n]){path[n]=draw_path[n];if(path[n]=='\\')last=n;++n;}
        if(!last || last+19>=sizeof(path))return;
        copy(path+last,"\\history-0001.bin",18);
        history_set(CreateFileA(path,0x40000000,1,0,1,0x80,0));if(history_current()==(HANDLE)-1){history_set(0);return;}
        u32 header[4];copy(header,"MNMCMD01",8);header[2]=1;header[3]=16;
        if(!write_all(history_current(),header,16)){CloseHandle(history_current());history_set(0);return;}history_bytes=16;
    }
    if(!history_current())return;
    struct HistorySurface *src=history_surface((void*)c->header[29],&c->src),*dst=history_surface(object,&c->before);
    if(!history_current() || !src || !dst)return;
    if(!history_palette_matches(src,&c->src) || !history_palette_matches(dst,&c->before) || !history_palette_matches(dst,&c->after))return;
    if(src==dst || src->locked || dst->locked){history_gap(3);return;}
    /* Before checks detect unexplained native changes instead of uploading them. */
    if(!history_record(5,&src->id,4,c->src.data,c->src.length) ||
       !history_record(5,&dst->id,4,c->before.data,c->before.length))return;
    u32 f[10]={src->id,dst->id,c->header[10],c->header[11],c->header[12],c->header[13],
               c->header[14],c->header[15],c->header[6],c->header[6]?c->header[7]:0};
    if(!history_record(3,f,40,0,0) || !history_record(5,&dst->id,4,c->after.data,c->after.length) ||
       !history_record(6,&dst->id,4,0,0))return;
    history_presented=dst->id;
    if(dst->desc[21]==8 && !history_color_check(dst))return;
    ++history_operations;
}
static void history_alias(void* object,void* alias){
    struct HistorySurface* h=history_find(object);if(!history_current() || !h || history_find(alias)==h)return;
    if(history_find(alias)){history_gap(4);return;}
    struct Table* t=lookup(alias);
    if(!t || t->kind<11 || t->kind>17){history_gap(4);return;}
    if(h->aliases_count>=16){history_gap(2);return;}h->aliases[h->aliases_count++]=alias;
}
static void history_lock(void* object,const void* rect,const u32* d,u32 flags,i32 status){
    if(status<0)return;
    struct HistorySurface* h=history_surface_resolve(object);if(!history_current() || !h)return;
    if(rect || !readable(d,108) || h->locked || (flags&~0x4831u) || !supported_format(d) ||
       d[2]!=h->desc[2] || d[3]!=h->desc[3] || !same(d+21,h->desc+21,16)){history_gap(3);return;}
    copy(h->desc,d,108);h->locked=1;h->writable=!(flags&0x10);
}
static void history_unlock_before(void* object,void* arg,struct Snapshot* out){
    struct Snapshot* s=out;zero(s,sizeof(*s));struct HistorySurface* h=history_find(object);
    if(!history_current() || !h)return;
    struct Table* t=lookup(object);
    if(!h->locked || (t->kind>=14?arg!=0:arg!=(void*)h->desc[9])){history_gap(3);return;}
    if(!h->writable)return;
    u32* d=h->desc;u32 width=d[3],height=d[2],stride=width*(d[21]/8);i32 pitch=(i32)d[4];u32 at=d[9];
    if(!width || width>2048 || !height || height>2048 || pitch==(-2147483647-1)){history_gap(3);return;}
    u32 magnitude=(u32)(pitch<0?-pitch:pitch),offset=(height-1)*magnitude;
    if(magnitude<stride || magnitude>32768 || (pitch<0 && at<offset) ||
       !readable((void*)(pitch<0?at-offset:at),offset+stride)){history_gap(3);return;}
    s->data=HeapAlloc(GetProcessHeap(),0,stride*height);if(!s->data){history_gap(2);return;}
    s->width=width;s->height=height;s->bits=d[21];s->length=stride*height;
    for(u32 y=0;y<height;++y)copy(s->data+y*stride,(u8*)at+(i32)y*pitch,stride);return;
}
static void history_unlock_after(void* object,struct Snapshot* s,i32 status){
    struct HistorySurface* h=history_find(object);
    if(history_current() && h && status>=0){
        h->locked=0;
        if(s->data){u32 f[5]={h->id,0,0,s->width,s->height};
            if(history_record(2,f,20,s->data,s->length)){
                struct Snapshot after;zero(&after,sizeof(after));
                if(!snapshot(object,lookup(object),h->desc,&after))history_gap(6);
                else {if(history_palette_matches(h,&after))history_record(5,&h->id,4,after.data,after.length);free_snapshot(&after);
                    if(history_current() && h->desc[21]==8)history_color_check(h);}
                ++history_operations;
            }}
    }
    free_snapshot(s);
}
static void history_release(struct HistorySurface* h,u32 remaining){
    if(!history_current() || !h || remaining)return;
    if(h->locked){history_gap(3);return;}
    history_record(7,&h->id,4,0,0);h->live=0;history_palette_retire(h->palette);
}
static void history_finish(void){
    int token=history_enter();if(!token)return;
    history_finish_owned();history_leave(token);
}
