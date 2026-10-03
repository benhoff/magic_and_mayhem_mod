/* Opt-in, bounded evidence collection. Included after the bridge's table helpers.
 * Native pixels retain index/color-key identity; instrumentation never changes
 * an API's arguments, result, or last-error state. No observer lock spans a draw.
 */
typedef i32 (WIN *BltFast)(void*,u32,u32,void*,void*,u32);
typedef i32 (WIN *GetObject)(void*,void**);
typedef i32 (WIN *GetKey)(void*,u32,u32*);
typedef u32 (WIN *ReleaseObject)(void*);
struct Rect {i32 left,top,right,bottom;};
struct Snapshot {u32 width,height,bits,flags,r,g,b,length;u8* data;u8 palette[1024];};
struct DrawCapture {u32 header[32];struct Snapshot src,before,after;};
static char draw_path[512];
static HANDLE draw_events;
static volatile i32 draw_busy,event_busy;
static u32 draw_attempts,draw_complete,event_count;

static int write_all(HANDLE file,const void* data,u32 length){
    u32 written=0;return WriteFile(file,data,length,&written,0) && written==length;
}
static void init_draw_capture(void){
    u32 size=GetEnvironmentVariableA("MNM_RENDER_CAPTURE_DIR",draw_path,sizeof(draw_path));
    if(!size || size+20>=sizeof(draw_path)){draw_path[0]=0;return;}
    copy(draw_path+size,"\\events.bin",12);
    draw_events=CreateFileA(draw_path,0x40000000,1,0,1,0x80,0); /* CREATE_NEW */
    if(draw_events==(HANDLE)-1)draw_events=0;
    if(draw_events){u32 header[4];copy(header,"MNMDRW01",8);header[2]=1;header[3]=64;
        if(!write_all(draw_events,header,16)){CloseHandle(draw_events);draw_events=0;}}
    copy(draw_path+size,"\\blit-0001.bin",15);
}
/* Only application-facing calls are counted; snapshots call saved originals. */
static void draw_event(u32 operation,u32 caller,void* object,void* source,u32 flags,i32 result,
                       const void* dest,const void* src){
    if(!draw_path[0] || !__sync_bool_compare_and_swap(&event_busy,0,1))return;
    if(!draw_events){__sync_lock_release(&event_busy);return;}
    u32 error=GetLastError(),record[16];zero(record,sizeof(record));
    record[0]=++event_count;record[1]=operation;record[2]=caller;record[3]=(u32)object;
    record[4]=(u32)source;record[5]=flags;record[6]=(u32)result;
    if(dest && readable(dest,16)){record[7]|=1;copy(record+8,dest,16);}
    if(src && readable(src,16)){record[7]|=2;copy(record+12,src,16);}
    if(!write_all(draw_events,record,64) || event_count>=2048){CloseHandle(draw_events);draw_events=0;}
    SetLastError(error);__sync_lock_release(&event_busy);
}
static int surface_description(void* object,struct Table* table,u32* desc){
    zero(desc,124);desc[0]=table->kind>=14?124:108;
    return ((Description)table->original[22])(object,desc)>=0;
}
static int supported_format(const u32* d){
    /* Reject FOURCC, alpha and depth formats, including unrelated unions. */
    return (d[21]==8 && d[19]==0x60) ||
           ((d[21]==16 || d[21]==24 || d[21]==32) && d[19]==0x40);
}
static int inside(const struct Rect* r,u32 width,u32 height){
    return r->left>=0 && r->top>=0 && r->right>r->left && r->bottom>r->top &&
           (u32)r->right<=width && (u32)r->bottom<=height;
}
static int no_clipper(void* object,struct Table* t){
    void* clipper=0;i32 status=((GetObject)t->original[15])(object,&clipper);
    if(clipper)((ReleaseObject)(*(void***)clipper)[2])(clipper);
    return (u32)status==0x88760238 && !clipper; /* DDERR_NOCLIPPERATTACHED */
}
static int distinct_surfaces(void* a,struct Table* ta,void* b,struct Table* tb){
    static const u8 unknown[16]={0,0,0,0,0,0,0,0,0xc0,0,0,0,0,0,0,0x46};
    void *ia=0,*ib=0;int ok=0;
    if(((Query)ta->original[0])(a,unknown,&ia)>=0 && ia &&
       ((Query)tb->original[0])(b,unknown,&ib)>=0 && ib)ok=ia!=ib;
    if(ia)((ReleaseObject)(*(void***)ia)[2])(ia);
    if(ib)((ReleaseObject)(*(void***)ib)[2])(ib);
    return ok;
}
static void free_snapshot(struct Snapshot* s){if(s->data)HeapFree(GetProcessHeap(),0,s->data);s->data=0;}
static int snapshot(void* object,struct Table* t,const u32* expected,struct Snapshot* s){
    u32 d[31];zero(d,sizeof(d));d[0]=t->kind>=14?124:108;
    if(((Lock)t->original[25])(object,0,d,0x4810,0)<0)return 0;
    i32 pitch=(i32)d[4];u8* pixels=(u8*)d[9];int ok=0;
    if(d[2]!=expected[2] || d[3]!=expected[3] || d[19]!=expected[19] ||
       !same(d+21,expected+21,16) || !supported_format(d))goto done;
    u32 width=d[3],height=d[2],stride=width*(d[21]/8);
    if(!width || !height || width>2048 || height>2048 || pitch==(-2147483647-1))goto done;
    u32 magnitude=(u32)(pitch<0?-pitch:pitch);
    if(magnitude<stride || magnitude>32768)goto done;
    u32 offset=(height-1)*magnitude,at=(u32)pixels;
    if(pitch<0 && at<offset)goto done;
    if(!readable((void*)(pitch<0?at-offset:at),offset+stride))goto done;
    s->data=HeapAlloc(GetProcessHeap(),0,stride*height);if(!s->data)goto done;
    s->width=width;s->height=height;s->bits=d[21];s->flags=d[19];
    s->r=d[22];s->g=d[23];s->b=d[24];s->length=stride*height;
    for(u32 y=0;y<height;++y)copy(s->data+y*stride,pixels+(i32)y*pitch,stride);
    if(d[21]==8){
        void* palette=0;typedef i32 (WIN *GetEntries)(void*,u32,u32,u32,void*);
        if(((GetObject)t->original[20])(object,&palette)<0 || !palette)goto done;
        void** vt=*(void***)palette;
        i32 result=((GetEntries)vt[4])(palette,0,0,256,s->palette);
        ((ReleaseObject)vt[2])(palette);if(result<0)goto done;
    }
    ok=1;
done:
    if(((Unlock)t->original[32])(object,t->kind>=14?0:pixels)<0)ok=0;
    if(!ok)free_snapshot(s);return ok;
}
static void discard_draw(struct DrawCapture* c){
    if(c){free_snapshot(&c->src);free_snapshot(&c->before);free_snapshot(&c->after);HeapFree(GetProcessHeap(),0,c);}
    __sync_lock_release(&draw_busy);
}
/* Holds only the observer's busy flag over the original call, never a surface lock. */
static struct DrawCapture* begin_draw(void* object,const void* dest,void* source,const void* rect,
                                     u32 flags,void* effects,u32 operation,u32 x,u32 y,u32 caller){
    if(!draw_path[0] || !source || source==object || effects ||
       (operation==1 ? (flags&~0x09008000u) : (flags&~0x31u)) ||
       !__sync_bool_compare_and_swap(&draw_busy,0,1))return 0;
    struct DrawCapture* c=0;
    if(draw_complete || draw_attempts>=8 || !readable(source,4))goto fail;
    struct Table *ts=lookup(source),*td=lookup(object);
    if(!ts || ts->kind<10 || !td)goto fail;
    u32 sd[31],dd[31];struct Rect sr,dr;
    if(!surface_description(source,ts,sd) || !surface_description(object,td,dd))goto fail;
    if(!supported_format(sd) || sd[19]!=dd[19] || !same(sd+21,dd+21,16) ||
       !sd[3] || sd[3]>256 || !sd[2] || sd[2]>256 ||
       !dd[3] || dd[3]>2048 || !dd[2] || dd[2]>2048)goto fail;
    if(rect){if(!readable(rect,16))goto fail;copy(&sr,rect,16);}
    else {sr.left=sr.top=0;sr.right=sd[3];sr.bottom=sd[2];}
    if(!inside(&sr,sd[3],sd[2]))goto fail;
    if(operation==2){
        if(x>2048 || y>2048)goto fail;
        dr.left=x;dr.top=y;dr.right=x+sr.right-sr.left;dr.bottom=y+sr.bottom-sr.top;
    }else if(dest){if(!readable(dest,16))goto fail;copy(&dr,dest,16);}
    else {dr.left=dr.top=0;dr.right=dd[3];dr.bottom=dd[2];}
    if(!inside(&dr,dd[3],dd[2]) || dr.right-dr.left!=sr.right-sr.left ||
       dr.bottom-dr.top!=sr.bottom-sr.top || !no_clipper(object,td) || !distinct_surfaces(source,ts,object,td))goto fail;
    ++draw_attempts;
    c=HeapAlloc(GetProcessHeap(),8,sizeof(*c));if(!c)goto fail;
    u32* h=c->header;copy(h,"MNMBLT01",8);h[2]=1;h[3]=operation;h[4]=caller;h[5]=flags;
    h[6]=(flags&(operation==1?0x8000:1))!=0;
    /* The inspected engine wrappers set an exact key. Defer color-space ranges. */
    if(h[6] && (((GetKey)ts->original[16])(source,8,h+7)<0 || h[7]!=h[8] ||
                 (sd[21]<32 && h[8]>((1u<<sd[21])-1))))goto fail;
    if(!snapshot(source,ts,sd,&c->src) || !snapshot(object,td,dd,&c->before))goto fail;
    copy(h+10,&sr,16);copy(h+14,&dr,16);h[18]=sd[3];h[19]=sd[2];h[20]=dd[3];h[21]=dd[2];
    copy(h+22,sd+21,16);h[26]=c->src.length;h[27]=c->before.length;
    h[28]=sd[21]==8?1024:0;h[29]=(u32)source;h[30]=(u32)object;return c;
fail:
    discard_draw(c);return 0;
}
#include "surface_commands.h"
static void end_draw(struct DrawCapture* c,void* object,i32 result){
    if(!c)return;
    struct Table* t=lookup(object);u32 d[31];
    if(result<0 || !surface_description(object,t,d) || !snapshot(object,t,d,&c->after))goto done;
    if(!same(&c->before,&c->after,32) || (c->header[28] && !same(c->before.palette,c->after.palette,1024)))goto done;
    c->header[9]=(u32)result;
    HANDLE file=CreateFileA(draw_path,0x40000000,1,0,1,0x80,0);
    if(file!=(HANDLE)-1){
        int ok=write_all(file,c->header,128) && write_all(file,c->src.data,c->src.length) &&
               write_all(file,c->before.data,c->before.length) && write_all(file,c->after.data,c->after.length);
        if(ok && c->header[28])ok=write_all(file,c->src.palette,1024) && write_all(file,c->before.palette,1024);
        CloseHandle(file);if(ok)write_surface_commands(c);draw_complete=1; /* Never overwrite even an incomplete file. */
        (void)ok; /* The offline parser rejects truncated files. */
    }
done:discard_draw(c);
}
