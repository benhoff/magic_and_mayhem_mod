#include "../../protocols/include/mnm/frame_v1.h"
#include "../shadow/win32_min.h"
#include "frame_pixels.h"
API HANDLE WIN CreateFileMappingA(HANDLE,void*,u32,u32,u32,const char*);
API void* WIN MapViewOfFile(HANDLE,u32,u32,u32,u32);
API u32 WIN GetFileSize(HANDLE,u32*);
API u32 WIN GetTickCount(void);
API u32 WIN GetCurrentThreadId(void);
API void WIN Sleep(u32);
API HANDLE WIN GetCurrentObject(HANDLE,u32);
API i32 WIN GetObjectA(HANDLE,i32,void*);
API i32 WIN GetBitmapBits(HANDLE,i32,void*);
API i32 WIN GdiFlush(void);
#define STREAM_SIZE (MNM_FRAME_V1_SIZE)
typedef i32 (WIN *Query)(void*,const u8*,void**);
typedef i32 (WIN *CreateSurface)(void*,void*,void**,void*);
typedef i32 (WIN *Blt)(void*,void*,void*,void*,u32,void*);
typedef i32 (WIN *Flip)(void*,void*,u32);
typedef i32 (WIN *Description)(void*,void*);
typedef i32 (WIN *Lock)(void*,void*,void*,u32,HANDLE);
typedef i32 (WIN *Unlock)(void*,void*);
typedef i32 (WIN *CreateDraw)(void*,void**,void*);
static CreateDraw original_create;
static u32* stream;
static u32 readback_disabled;
static volatile i32 capture_busy,table_busy;
#ifndef MNM_RENDER_SELFTEST
static u32 last_capture;
#endif
/* Shared vtables, not object addresses: records survive object Release/reuse. */
struct Table {void** vtable;void* original[33];u32 kind;};
static struct Table tables[32];static u32 table_count;
static void copy(void* to,const void* from,u32 length){
    /* PE32, no CRT and no SIMD state changes. Inputs are non-overlapping,
     * as in the previous forward byte loop. REP handles unaligned rows. */
    __asm__ volatile("cld; rep movsb" : "+D"(to), "+S"(from), "+c"(length) : : "memory", "cc");
}
static void zero(void* to,u32 length){u8* p=to;while(length--)*p++=0;}
static int same(const void* a,const void* b,u32 size){const u8* x=a;const u8* y=b;while(size--)if(*x++!=*y++)return 0;return 1;}
static int readable(const void* pointer,u32 size){
    u32 at=(u32)pointer,end=at+size;if(!at || end<at)return 0;
    while(at<end){u32 m[7];if(VirtualQuery((void*)at,m,28)!=28||m[4]!=0x1000||(m[5]&0x101))return 0;
        u32 next=m[0]+m[3];if(next<=at)return 0;at=next;}return 1;
}
static struct Table* lookup(void* object){
    void** table=*(void***)object;
    for(u32 i=0;i<__atomic_load_n(&table_count,__ATOMIC_ACQUIRE);++i)if(tables[i].vtable==table)return tables+i;
    return 0;
}
static void install_table(void*,u32);
#include "input_polling.h"
#include "media_bridge.h"
static i32 WIN input_cooperative(void* object,void* window,u32 flags){
    struct Table* table=lookup(object);i32 result=((i32 (WIN *)(void*,void*,u32))table->original[20])(object,window,flags);
    if(result>=0 && window)input_window=window;return result;
}
#include "command_channel.h"
#include "draw_capture.h"
#include "lock_lifecycle.h"
#include "lock_flip.h"
#include "lock_dc.h"
#include "command_scheduler.h"
#include "command_lifecycle.h"
#include "command_recovery.h"
static u32 guid_kind(const u8* guid){
    static const u8 ids[8][16]={
      {0x80,0xdb,0x14,0x6c,0x33,0xa7,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60},
      {0xe0,0xf3,0xa6,0xb3,0x43,0x2b,0xcf,0x11,0xa2,0xde,0,0xaa,0,0xb9,0x33,0x56},
      {0x9a,0x50,0x59,0x9c,0xbd,0x39,0xd1,0x11,0x8c,0x4a,0,0xc0,0x4f,0xd9,0x30,0xc5},
      {0xc0,0x5e,0xe6,0x15,0x9c,0x3b,0xd2,0x11,0xb9,0x2f,0,0x60,0x97,0x97,0xea,0x5b},
      {0x81,0xdb,0x14,0x6c,0x33,0xa7,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60},
      {0x85,0x58,0x80,0x57,0xec,0x6e,0xcf,0x11,0x94,0x41,0xa8,0x23,3,0xc1,0x0e,0x27},
      {0x30,0x86,0x2b,0x0b,0x35,0xad,0xd0,0x11,0x8e,0xa6,0,0x60,0x97,0x97,0xea,0x5b},
      {0x80,0x5a,0x67,0x06,0x9b,0x3b,0xd2,0x11,0xb9,0x2f,0,0x60,0x97,0x97,0xea,0x5b}};
    static const u32 kinds[8]={1,2,4,7,11,12,14,17};
    static const u8 palette[16]={0x84,0xdb,0x14,0x6c,0x33,0xa7,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60};
    if(same(guid,palette,16))return 20;
    for(u32 i=0;i<8;++i)if(same(guid,ids[i],16))return kinds[i];return 0;
}
static void install_table(void*,u32);
static i32 WIN query(void* object,const u8* guid,void** result){
    u32 entry=GetLastError();int token=history_enter();struct Table* t=lookup(object);SetLastError(entry);i32 status=((Query)t->original[0])(object,guid,result);u32 error=GetLastError();
    if(status>=0 && result && *result && guid){u32 kind=guid_kind(guid);if(kind)install_table(*result,kind);
        if(t->kind>=11 && t->kind<=17 && kind>=11 && kind<=17)game_alias_observed(object,*result,kind);
        if(t->kind==20 && kind==20)game_palette_alias(object,*result);
        if(token){if(t->kind==20)history_palette_alias(object,*result);else history_alias(object,*result);}}
    history_leave(token);SetLastError(error);return status;
}
static u32 WIN surface_release(void* object){
    u32 entry=GetLastError();int token=history_enter();struct HistorySurface* h=token?history_surface_resolve(object):0;
    struct Table* t=lookup(object);SetLastError(entry);u32 remaining=((ReleaseObject)t->original[2])(object),error=GetLastError();
    if(!remaining)game_lock_retire(object);
    if(token)history_release(h,remaining);history_leave(token);SetLastError(error);return remaining;
}
static i32 WIN create_surface(void* object,void* desc,void** result,void* outer){
    u32 entry=GetLastError();struct Table* t=lookup(object);u32 input[31];zero(input,sizeof(input));
    u32 size=t->kind>=4?124:108;int valid=lock_capture_path_length && readable(desc,size) && *(u32*)desc==size;
    if(valid)copy(input,desc,size);SetLastError(entry);
    i32 status=((CreateSurface)t->original[6])(object,desc,result,outer);u32 error=GetLastError();
    if(status>=0 && result && *result){install_table(*result,t->kind+10);game_surface_created(*result,t->kind+10,valid?input:0);}
    draw_event(6,(u32)__builtin_return_address(0),object,status>=0 && result?*result:0,0,status,0,0);
    SetLastError(error);return status;
}
static i32 WIN create_palette(void* object,u32 flags,void* entries,void** result,void* outer){
    u32 entry=GetLastError();struct Table* t=lookup(object);u8 colors[1024];
    int valid=game_palette_supported(flags) && readable(entries,1024);if(valid)copy(colors,entries,1024);SetLastError(entry);
    i32 status=((i32 (WIN *)(void*,u32,void*,void**,void*))t->original[5])(object,flags,entries,result,outer);u32 error=GetLastError();
    if(status>=0 && result && *result){install_table(*result,20);game_palette_created(*result,flags,valid?colors:0);}
    SetLastError(error);return status;
}
static i32 WIN palette_caps(void* object,u32* caps){
    u32 entry=GetLastError();struct Table* t=lookup(object);SetLastError(entry);
    i32 status=((i32 (WIN *)(void*,u32*))t->original[3])(object,caps);u32 error=GetLastError();
    if(status>=0 && readable(caps,4))game_palette_caps(object,*caps);SetLastError(error);return status;
}
static i32 WIN palette_get_entries(void* object,u32 flags,u32 first,u32 count,void* entries){
    u32 entry=GetLastError();struct Table* t=lookup(object);struct GamePaletteUpdate pending;
    game_palette_before(object,flags,first,count,entries,0,&pending);SetLastError(entry);
    i32 status=((i32 (WIN *)(void*,u32,u32,u32,void*))t->original[4])(object,flags,first,count,entries);u32 error=GetLastError();
    game_palette_after(object,first,count,entries,1,&pending,status);SetLastError(error);return status;
}
static i32 WIN surface_get_palette(void* object,void** result){
    u32 entry=GetLastError();struct Table* t=lookup(object);SetLastError(entry);
    i32 status=((i32 (WIN *)(void*,void**))t->original[20])(object,result);u32 error=GetLastError();
    if(status>=0 && result && *result){install_table(*result,20);game_surface_palette(object,*result);}
    SetLastError(error);return status;
}
static i32 WIN surface_attached(void* object,u32* caps,void** result){
    u32 entry=GetLastError();struct Table* t=lookup(object);u32 requested[4]={0};u32 size=t->kind>=14?16:4;
    int valid=readable(caps,size);if(valid)copy(requested,caps,size);SetLastError(entry);
    i32 status=((i32 (WIN *)(void*,void*,void**))t->original[12])(object,caps,result);u32 error=GetLastError();
    if(status>=0 && result && *result){install_table(*result,t->kind);game_attached_observed(object,*result,valid?requested:0);}
    SetLastError(error);return status;
}
static i32 WIN surface_add_attached(void* object,void* other){
    u32 entry=GetLastError();struct Table* t=lookup(object);SetLastError(entry);
    i32 status=((i32 (WIN *)(void*,void*))t->original[3])(object,other);u32 error=GetLastError();
    if(status>=0)game_metadata_invalidate();SetLastError(error);return status;
}
static i32 WIN surface_delete_attached(void* object,u32 flags,void* other){
    u32 entry=GetLastError();struct Table* t=lookup(object);SetLastError(entry);
    i32 status=((i32 (WIN *)(void*,u32,void*))t->original[8])(object,flags,other);u32 error=GetLastError();
    if(status>=0)game_metadata_invalidate();SetLastError(error);return status;
}
static i32 WIN surface_desc(void* object,u32* desc){
    u32 entry=GetLastError();struct Table* t=lookup(object);SetLastError(entry);
    i32 status=((Description)t->original[22])(object,desc);u32 error=GetLastError();
    game_surface_described(object,t->kind,desc,status);SetLastError(error);return status;
}

static void capture(void* object){
    if(readback_disabled || !stream || !__sync_bool_compare_and_swap(&capture_busy,0,1))return;
    u32 saved_error=GetLastError();
    struct Table* t=lookup(object);
    u32 desc[31];zero(desc,sizeof(desc));desc[0]=t->kind>=14?124:108;
    if(((Description)t->original[22])(object,desc)<0 || !(desc[26]&0x200))goto done;
#ifndef MNM_RENDER_SELFTEST
    u32 now=GetTickCount();
    if(now-last_capture<16){SetLastError(saved_error);__sync_lock_release(&capture_busy);return;}
    last_capture=now;
#endif
    zero(desc,sizeof(desc));desc[0]=t->kind>=14?124:108;
    i32 lock_result=((Lock)t->original[25])(object,0,desc,0x4810,0);
    if(lock_result<0){render_failure("primary_lock",lock_result,object,t->kind,0x4810);__atomic_store_n(stream+MNM_FRAME_V1_STATUS_OFFSET/4,MNM_FRAME_V1_STATUS_LOCK_FAILED,__ATOMIC_RELEASE);goto done;}
    u32 width=desc[3],height=desc[2],bits=desc[21],bytes=bits/8;
    i32 pitch=(i32)desc[4];u8* pixels=(u8*)desc[9];u8 palette[1024];const u8* colors=0;
    int ok=0;
    if(width && height && width<=MNM_FRAME_V1_MAX_WIDTH && height<=MNM_FRAME_V1_MAX_HEIGHT && (bits==8||bits==16||bits==24||bits==32) &&
       pitch!=(-2147483647-1) && (u32)(pitch<0?-pitch:pitch)<=32768 && (u32)(pitch<0?-pitch:pitch)>=width*bytes){
        const u8* start=pitch<0?pixels+(i32)(height-1)*pitch:pixels;
        if(readable(start,(height-1)*(u32)(pitch<0?-pitch:pitch)+width*bytes)){
            if(bits==8 && (desc[19]&0x20)){
                void* pal=0;
                typedef i32 (WIN *GetPalette)(void*,void**);
                typedef i32 (WIN *GetEntries)(void*,u32,u32,u32,void*);
                if(((GetPalette)t->original[20])(object,&pal)>=0 && pal){
                    void** vt=*(void***)pal;if(((GetEntries)vt[4])(pal,0,0,256,palette)>=0)colors=palette;
                    observer_release(pal);
                }
            }
            u32 sequence=__atomic_load_n(stream+MNM_FRAME_V1_SEQUENCE_OFFSET/4,__ATOMIC_RELAXED);
            __atomic_store_n(stream+MNM_FRAME_V1_SEQUENCE_OFFSET/4,sequence+1,__ATOMIC_SEQ_CST);
            ok=render_pixels((u8*)stream+MNM_FRAME_V1_PIXELS_OFFSET,width,height,pixels,pitch,bits,desc[22],desc[23],desc[24],colors);
            if(ok){stream[MNM_FRAME_V1_WIDTH_OFFSET/4]=width;stream[MNM_FRAME_V1_HEIGHT_OFFSET/4]=height;stream[MNM_FRAME_V1_STRIDE_OFFSET/4]=width*MNM_FRAME_V1_BYTES_PER_PIXEL;stream[MNM_FRAME_V1_PIXEL_FORMAT_OFFSET/4]=MNM_FRAME_V1_PIXEL_FORMAT_RGBA8888;++stream[MNM_FRAME_V1_FRAME_COUNT_OFFSET/4];stream[MNM_FRAME_V1_STATUS_OFFSET/4]=MNM_FRAME_V1_STATUS_FRAME_PUBLISHED;}
            else stream[MNM_FRAME_V1_STATUS_OFFSET/4]=MNM_FRAME_V1_STATUS_SURFACE_REJECTED;
            __atomic_store_n(stream+MNM_FRAME_V1_SEQUENCE_OFFSET/4,sequence+2,__ATOMIC_RELEASE);
        }
    }
    i32 unlock_result=((Unlock)t->original[32])(object,t->kind>=14?0:pixels);
    render_failure("primary_unlock",unlock_result,object,t->kind,0);
    if(!ok)__atomic_store_n(stream+MNM_FRAME_V1_STATUS_OFFSET/4,MNM_FRAME_V1_STATUS_SURFACE_REJECTED,__ATOMIC_RELEASE);
done:
    SetLastError(saved_error);__sync_lock_release(&capture_busy);
}
static i32 WIN blt(void* object,void* dest,void* source,void* rect,u32 flags,void* effects){
    u32 error=GetLastError(),caller=(u32)__builtin_return_address(0);
    struct Table* t=lookup(object);
    int token=history_enter();struct DrawCapture* c=begin_draw(object,dest,source,rect,flags,effects,1,0,0,caller);
    if(token && !c)history_gap(6);
    struct GameBlit propagated;game_blit_before(object,dest,source,rect,flags,effects,0,0,0,&propagated);
    SetLastError(error);i32 status=((Blt)t->original[5])(object,dest,source,rect,flags,effects);error=GetLastError();
    game_blit_after(&propagated,status);
    render_failure("application_blt",status,object,t->kind,flags);
    end_draw(c,object,status);draw_event(1,caller,object,source,flags,status,dest,rect);
    if(status>=0)capture(object);history_leave(token);SetLastError(error);return status;
}
static i32 WIN blt_fast(void* object,u32 x,u32 y,void* source,void* rect,u32 flags){
    u32 error=GetLastError(),caller=(u32)__builtin_return_address(0);
    struct Table* t=lookup(object);
    int token=history_enter();struct DrawCapture* c=begin_draw(object,0,source,rect,flags,0,2,x,y,caller);
    if(token && !c)history_gap(6);
    struct GameBlit propagated;game_blit_before(object,0,source,rect,flags,0,1,x,y,&propagated);
    SetLastError(error);i32 status=((BltFast)t->original[7])(object,x,y,source,rect,flags);error=GetLastError();
    game_blit_after(&propagated,status);
    render_failure("application_bltfast",status,object,t->kind,flags);
    end_draw(c,object,status);
    /* For a Fast event, left/top are x/y; right/bottom are deliberately zero. */
    u32 dest[4]={x,y,0,0};draw_event(2,caller,object,source,flags,status,dest,rect);
    if(status>=0)capture(object);history_leave(token);SetLastError(error);return status;
}
static i32 WIN flip(void* object,void* target,u32 flags){
    u32 entry=GetLastError();int token=history_enter();struct Table* t=lookup(object);struct HistoryFlip pending;
    struct GameFlip owned;game_flip_before(object,target,flags,&owned);
    if(token)history_flip_before(object,target,flags,&pending);
    SetLastError(entry);i32 status=((Flip)t->original[11])(object,target,flags);u32 error=GetLastError();
    game_flip_after(&owned,status);
    render_failure("application_flip",status,object,t->kind,flags);
    if(token)history_flip_after(object,&pending,status);
    draw_event(3,(u32)__builtin_return_address(0),object,target,flags,status,0,0);
    if(status>=0)capture(object);history_leave(token);SetLastError(error);return status;
}
static i32 WIN surface_lock(void* object,void* rect,void* desc,u32 flags,HANDLE event){
    u32 entry=GetLastError();struct Rect region;int region_valid=rect && readable(rect,16);if(region_valid)copy(&region,rect,16);
    int token=history_enter();struct Table* t=lookup(object);SetLastError(entry);i32 status=((Lock)t->original[25])(object,rect,desc,flags,event);u32 error=GetLastError();
    game_lock_observed(object,t,rect,region_valid?&region:0,desc,flags,status);
    render_failure("application_lock",status,object,t->kind,flags);
    if(token)history_lock(object,rect,desc,flags,status);
    draw_event(4,(u32)__builtin_return_address(0),object,0,flags,status,rect,0);history_leave(token);SetLastError(error);return status;
}
static i32 WIN surface_unlock(void* object,void* rect){
    u32 error=GetLastError();int token=history_enter();struct Snapshot pending;zero(&pending,sizeof(pending));
    struct Table* t=lookup(object);
    struct GameUnlock game_pending;game_unlock_before(object,t->kind,rect,&game_pending);
    if(token)history_unlock_before(object,rect,&pending);SetLastError(error);
    i32 status=((Unlock)t->original[32])(object,rect);error=GetLastError();
    game_unlock_after(&game_pending,status);
    render_failure("application_unlock",status,object,t->kind,0);
    if(token)history_unlock_after(object,&pending,status);
    draw_event(5,(u32)__builtin_return_address(0),object,0,0,status,0,0);history_leave(token);SetLastError(error);return status;
}
static i32 WIN surface_restore(void* object){
    u32 entry=GetLastError();int token=history_enter();struct Table* t=lookup(object);SetLastError(entry);
    i32 status=((i32 (WIN *)(void*))t->original[27])(object);u32 error=GetLastError();
    if(status>=0){game_surface_invalidate(object);game_surface_key(object,8,0,0);}
    if(token && status>=0 && history_find(object))history_gap(6);history_leave(token);SetLastError(error);return status;
}
static i32 WIN surface_dc(void* object,void** output){
    u32 entry=GetLastError();int token=history_enter();struct Table* t=lookup(object);SetLastError(entry);
    i32 status=((GetObject)t->original[17])(object,output);u32 error=GetLastError();
    if(status>=0)game_dc_acquired(object,readable(output,4)?*output:0);
    if(token && status>=0 && history_find(object))history_gap(6);history_leave(token);SetLastError(error);return status;
}
static i32 WIN surface_release_dc(void* object,void* dc){
    u32 entry=GetLastError();struct Table* t=lookup(object);
    struct GameDC pending;game_dc_before(object,dc,&pending);
    SetLastError(entry);i32 status=((i32 (WIN *)(void*,void*))t->original[26])(object,dc);u32 error=GetLastError();
    game_dc_after(&pending,dc,status);SetLastError(error);return status;
}
static i32 surface_property(void* object,void* value,u32 slot){
    u32 entry=GetLastError();int token=history_enter();struct Table* t=lookup(object);SetLastError(entry);
    i32 status=((i32 (WIN *)(void*,void*))t->original[slot])(object,value);u32 error=GetLastError();
    if(status>=0 && slot==28)game_surface_clipper(object,value);
    if(token && status>=0 && history_find(object))history_gap(6);history_leave(token);SetLastError(error);return status;
}
static i32 WIN surface_palette(void* object,void* palette){
    u32 entry=GetLastError();int token=history_enter();struct Table* t=lookup(object);SetLastError(entry);
    i32 status=((i32 (WIN *)(void*,void*))t->original[31])(object,palette);u32 error=GetLastError();
    if(status>=0){if(palette)install_table(palette,20);game_surface_palette(object,palette);}
    if(token && status>=0)history_set_palette(object,palette);
    history_leave(token);SetLastError(error);return status;
}
static u32 WIN palette_release(void* object){
    u32 entry=GetLastError();int token=history_enter();struct HistoryPalette* p=token?history_palette_resolve(object):0;
    struct Table* t=lookup(object);SetLastError(entry);
    u32 remaining=((ReleaseObject)t->original[2])(object),error=GetLastError();
    if(!remaining && lock_capture_path_length)game_metadata_invalidate();
    if(token)history_palette_release(p,remaining);history_leave(token);SetLastError(error);return remaining;
}
static i32 WIN palette_entries(void* object,u32 flags,u32 first,u32 count,void* entries){
    u32 entry=GetLastError();int token=history_enter();struct Table* t=lookup(object);SetLastError(entry);
    struct GamePaletteUpdate pending;game_palette_before(object,flags,first,count,entries,1,&pending);SetLastError(entry);
    i32 status=((i32 (WIN *)(void*,u32,u32,u32,void*))t->original[6])(object,flags,first,count,entries);u32 error=GetLastError();
    game_palette_after(object,first,count,entries,0,&pending,status);
    if(token && status>=0)history_palette_entries(object,flags,first,count);
    history_leave(token);SetLastError(error);return status;
}
static i32 WIN palette_initialize(void* object,void* draw,u32 flags,void* entries){
    u32 entry=GetLastError();int token=history_enter();struct Table* t=lookup(object);SetLastError(entry);
    i32 status=((i32 (WIN *)(void*,void*,u32,void*))t->original[5])(object,draw,flags,entries);u32 error=GetLastError();
    if(status>=0)game_palette_invalidated(object);
    if(token && status>=0 && history_palette_resolve(object))history_gap(5);
    history_leave(token);SetLastError(error);return status;
}
static i32 WIN surface_color_key(void* object,u32 flags,u32* key){
    u32 entry=GetLastError(),saved[2]={0};int valid=!key || readable(key,8);if(key && valid)copy(saved,key,8);
    struct Table* t=lookup(object);SetLastError(entry);
    i32 status=((i32 (WIN *)(void*,u32,void*))t->original[29])(object,flags,key);u32 error=GetLastError();
    if(status>=0)game_surface_key(object,flags,valid,key?saved:0);
    SetLastError(error);return status;
}
static i32 WIN surface_clipper(void* object,void* clipper){return surface_property(object,clipper,28);}
static i32 WIN surface_batch(void* object,void* batch,u32 count,u32 flags){
    u32 entry=GetLastError();int token=history_enter();struct Table* t=lookup(object);SetLastError(entry);
    i32 status=((i32 (WIN *)(void*,void*,u32,u32))t->original[6])(object,batch,count,flags);u32 error=GetLastError();
    if(status>=0)game_surface_invalidate(object);
    if(token && status>=0 && history_find(object))history_gap(6);history_leave(token);SetLastError(error);return status;
}
static void install_table(void* object,u32 kind){
    if(!stream || !readable(object,4) || !__sync_bool_compare_and_swap(&table_busy,0,1))return;
    if(lookup(object))goto done;
    u32 count=__atomic_load_n(&table_count,__ATOMIC_RELAXED),length=kind<10?(input_words?21:7):kind==20?7:33,protection;
    void** vt=*(void***)object;
    if(count>=32 || !readable(vt,length*4) || !VirtualProtect(vt,length*4,0x40,&protection))goto done;
    struct Table* t=tables+count;t->vtable=vt;t->kind=kind;copy(t->original,vt,length*4);
    __atomic_store_n(&table_count,count+1,__ATOMIC_RELEASE);
    __atomic_store_n(vt,(void*)&query,__ATOMIC_RELEASE);
    if(kind<10){__atomic_store_n(vt+6,(void*)&create_surface,__ATOMIC_RELEASE);
        if(input_words)__atomic_store_n(vt+20,(void*)&input_cooperative,__ATOMIC_RELEASE);
        if(lock_capture_path_length)__atomic_store_n(vt+5,(void*)&create_palette,__ATOMIC_RELEASE);
    }
    else if(kind==20){
        if(lock_capture_path_length){__atomic_store_n(vt+3,(void*)&palette_caps,__ATOMIC_RELEASE);__atomic_store_n(vt+4,(void*)&palette_get_entries,__ATOMIC_RELEASE);
        }
        __atomic_store_n(vt+2,(void*)&palette_release,__ATOMIC_RELEASE);
        __atomic_store_n(vt+5,(void*)&palette_initialize,__ATOMIC_RELEASE);
        __atomic_store_n(vt+6,(void*)&palette_entries,__ATOMIC_RELEASE);
    }else {
        __atomic_store_n(vt+5,(void*)&blt,__ATOMIC_RELEASE);
        __atomic_store_n(vt+7,(void*)&blt_fast,__ATOMIC_RELEASE);
        __atomic_store_n(vt+11,(void*)&flip,__ATOMIC_RELEASE);
        if(lock_capture_path_length){
            __atomic_store_n(vt+20,(void*)&surface_get_palette,__ATOMIC_RELEASE);
            __atomic_store_n(vt+31,(void*)&surface_palette,__ATOMIC_RELEASE);
            __atomic_store_n(vt+12,(void*)&surface_attached,__ATOMIC_RELEASE);
            __atomic_store_n(vt+3,(void*)&surface_add_attached,__ATOMIC_RELEASE);
            __atomic_store_n(vt+8,(void*)&surface_delete_attached,__ATOMIC_RELEASE);
            __atomic_store_n(vt+22,(void*)&surface_desc,__ATOMIC_RELEASE);
            __atomic_store_n(vt+6,(void*)&surface_batch,__ATOMIC_RELEASE);
            __atomic_store_n(vt+17,(void*)&surface_dc,__ATOMIC_RELEASE);
            __atomic_store_n(vt+26,(void*)&surface_release_dc,__ATOMIC_RELEASE);
            __atomic_store_n(vt+27,(void*)&surface_restore,__ATOMIC_RELEASE);
            __atomic_store_n(vt+28,(void*)&surface_clipper,__ATOMIC_RELEASE);
            __atomic_store_n(vt+29,(void*)&surface_color_key,__ATOMIC_RELEASE);
            __atomic_store_n(vt+2,(void*)&surface_release,__ATOMIC_RELEASE);
            __atomic_store_n(vt+25,(void*)&surface_lock,__ATOMIC_RELEASE);
            __atomic_store_n(vt+32,(void*)&surface_unlock,__ATOMIC_RELEASE);
        }
        if(draw_path[0]){
            __atomic_store_n(vt+2,(void*)&surface_release,__ATOMIC_RELEASE);
            __atomic_store_n(vt+6,(void*)&surface_batch,__ATOMIC_RELEASE);
            __atomic_store_n(vt+17,(void*)&surface_dc,__ATOMIC_RELEASE);
            __atomic_store_n(vt+27,(void*)&surface_restore,__ATOMIC_RELEASE);
            __atomic_store_n(vt+28,(void*)&surface_clipper,__ATOMIC_RELEASE);
            __atomic_store_n(vt+31,(void*)&surface_palette,__ATOMIC_RELEASE);
            __atomic_store_n(vt+25,(void*)&surface_lock,__ATOMIC_RELEASE);
            __atomic_store_n(vt+32,(void*)&surface_unlock,__ATOMIC_RELEASE);
        }
    }
    u32 ignored;VirtualProtect(vt,length*4,protection,&ignored);
done:__sync_lock_release(&table_busy);
}
static i32 WIN create_draw(void* guid,void** result,void* outer){
    u32 incoming=GetLastError();RenderStartup();SetLastError(incoming);
    __atomic_store_n(stream+MNM_FRAME_V1_STATUS_OFFSET/4,MNM_FRAME_V1_STATUS_INSIDE_CREATE,__ATOMIC_RELEASE); /* Entered DirectDrawCreate. */
    __atomic_add_fetch(stream+MNM_FRAME_V1_CREATE_COUNT_OFFSET/4,1,__ATOMIC_RELAXED);
    i32 status=original_create(guid,result,outer);u32 error=GetLastError();
    __atomic_store_n(stream+MNM_FRAME_V1_CREATE_HRESULT_OFFSET/4,(u32)status,__ATOMIC_RELAXED);
    if(status>=0 && result && *result)install_table(*result,1);
    u32 state=status<0?MNM_FRAME_V1_STATUS_CREATE_FAILED:result && *result && lookup(*result)?MNM_FRAME_V1_STATUS_INTERFACE_INTERCEPTED:MNM_FRAME_V1_STATUS_INTERCEPTION_FAILED;
    __atomic_store_n(stream+MNM_FRAME_V1_STATUS_OFFSET/4,state,__ATOMIC_RELEASE);
    SetLastError(error);return status;
}
#include "adapter_startup.h"
__declspec(dllexport) void RenderAnchor(void){}
/* Application orchestration calls this before process exit, outside DllMain.
 * It closes capture admission while original drawing remains installed. */
__declspec(dllexport) u32 WIN RenderShutdown(u32 milliseconds){
    u32 error=GetLastError();
    if(!game_tracker_acquire()){SetLastError(error);return 0;}
    session_finish_owned();
    if(game_session_continuous && !session_started && !__atomic_load_n(&command_queue_end,__ATOMIC_ACQUIRE))command_channel_fail(MNM_RENDER_COMMANDS_V2_REASON_GAP);
    game_session_enabled=0;game_tracker_release();
    int complete=command_scheduler_shutdown(milliseconds);SetLastError(error);return complete;
}
#ifdef MNM_RENDER_SELFTEST
__declspec(dllexport) u32 WIN RenderQueueForTest(const void* bytes,u32 length,u32 end){
    u32 error=GetLastError();command_scheduler_start();
    if(!game_tracker_acquire()){SetLastError(error);return 0;}
    int ok=command_channel_append(bytes,length,0,0,0,0);if(end)command_channel_end();
    game_tracker_release();SetLastError(error);return ok;
}
__declspec(dllexport) void WIN RenderInstallForTest(void* surface,u32 kind){u32 error=GetLastError();command_scheduler_start();install_table(surface,kind);SetLastError(error);}
/* Deterministically exercise contention without timing-dependent scheduling. */
__declspec(dllexport) void WIN RenderCaptureGuardForTest(u32 held){
    __atomic_store_n(&game_locks_owner,held?GetCurrentThreadId():0,__ATOMIC_RELEASE);
    __atomic_store_n(&game_locks_busy,held!=0,__ATOMIC_RELEASE);
}
__declspec(dllexport) u32 WIN RenderCaptureWaitsForTest(void){return __atomic_load_n(&game_tracker_wait_count,__ATOMIC_RELAXED);}
__declspec(dllexport) i32 WIN RenderCreateForTest(CreateDraw original,void* guid,void** result,void* outer){
    original_create=original;return create_draw(guid,result,outer);
}
#endif
int WIN DllMain(void* instance,u32 reason,void* reserved){
    (void)instance;(void)reserved;
    if(reason==0){game_session_finish();history_finish();command_scheduler_detach();return 1;}
    if(reason!=1)return 1;
    char path[512];u32 size=GetEnvironmentVariableA("MNM_RENDER_STREAM",path,sizeof(path));
    if(!size || size>=sizeof(path))return 1;
    HANDLE file=CreateFileA(path,0xc0000000,3,0,3,0x80,0);
    if(file==(HANDLE)-1)return 1;
    if(GetFileSize(file,0)!=STREAM_SIZE){CloseHandle(file);return 1;}
    HANDLE mapping=CreateFileMappingA(file,0,4,0,STREAM_SIZE,0);CloseHandle(file);
    if(!mapping)return 1;
    stream=MapViewOfFile(mapping,2,0,0,STREAM_SIZE);CloseHandle(mapping);
    if(!stream || !same(stream,MNM_FRAME_V1_MAGIC,MNM_FRAME_V1_MAGIC_SIZE) || stream[MNM_FRAME_V1_VERSION_OFFSET/4]!=MNM_FRAME_V1_VERSION || stream[MNM_FRAME_V1_DECLARED_SIZE_OFFSET/4]!=MNM_FRAME_V1_DECLARED_SIZE){stream=0;return 1;}
    input_init();media_init();command_channel_init();
    char no_readback[8];readback_disabled=GetEnvironmentVariableA("MNM_RENDER_NO_READBACK",no_readback,sizeof(no_readback))!=0;
    init_lock_lifecycle();
    if(lock_capture_path_length)readback_disabled=1;
    init_failure_diagnostics();init_draw_capture();
    if(game_session_continuous && !command_queue){
        command_channel_refused=1;command_channel_fail(MNM_RENDER_COMMANDS_V2_REASON_INVALID);
    }
    if(command_auto_shutdown() && !command_exit_install(GetModuleHandleA(0))){
        command_channel_refused=1;command_channel_fail(MNM_RENDER_COMMANDS_V2_REASON_INVALID);
        lock_diagnostic("command_lifecycle_install_failed",0,0,0,0,0,0,0);
    }
    stream[MNM_FRAME_V1_STATUS_OFFSET/4]=MNM_FRAME_V1_STATUS_DLL_LOADED; /* loaded, waiting for presentation */
    u32 base=(u32)GetModuleHandleA(0),protection;
    /* Staging verifies full image SHA-256. Runtime also guards its import thunk. */
    static const u8 thunk[6]={0xff,0x25,0x14,0x50,0x5c,0};
    if(base!=0x400000 || !readable((void*)(base+0x19755a),6) || !same((void*)(base+0x19755a),thunk,6)){stream[MNM_FRAME_V1_STATUS_OFFSET/4]=MNM_FRAME_V1_STATUS_HOOK_FAILED;return 1;}
    static const u8 enum_thunk[6]={0xff,0x25,0x10,0x50,0x5c,0};
    if(!readable((void*)(base+0x197560),6) || !same((void*)(base+0x197560),enum_thunk,6)){stream[MNM_FRAME_V1_STATUS_OFFSET/4]=MNM_FRAME_V1_STATUS_HOOK_FAILED;return 1;}
    void** iat=(void**)(base+0x1c5010);
    if(!readable(iat,0xc8) || !VirtualProtect(iat,0xc8,4,&protection)){stream[MNM_FRAME_V1_STATUS_OFFSET/4]=MNM_FRAME_V1_STATUS_HOOK_FAILED;return 1;}
    original_enumerate=(EnumerateDraw)iat[0];original_create=(CreateDraw)iat[1];
    original_proc_address=(ProcAddress)iat[0x31];
    iat[0]=(void*)&enumerate_draw;iat[1]=(void*)&create_draw;iat[0x31]=(void*)&proc_address;
    u32 ignored;VirtualProtect(iat,0xc8,protection,&ignored);
#ifndef MNM_RENDER_SELFTEST
    input_install(base);media_install(base);
#endif
    __atomic_store_n(stream+MNM_FRAME_V1_STATUS_OFFSET/4,MNM_FRAME_V1_STATUS_HOOK_ARMED,__ATOMIC_RELEASE);return 1;
}
