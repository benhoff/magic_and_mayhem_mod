#include "../../protocols/include/mnm/frame_v1.h"
/* Never Lock the primary. The fake engine's independent backing pixels start
 * poisoned; only full opaque draws can establish a complete captured image. */
static char bootstrap_mode[32];
static int bs_mode(const char* text){u32 i=0;while(text[i] && text[i]==bootstrap_mode[i])++i;return !text[i] && !bootstrap_mode[i];}
static int bs_thread_case(void){return bs_mode("thread-contention") || bs_mode("thread-timeout");}
static int bs_scoped_partial(void){return bs_thread_case() || bs_mode("partial-lock-contention") || bs_mode("partial-blit-contention") || bs_mode("nested-lock-contention");}
static int bs_continuous(void){return bs_scoped_partial() || bs_mode("continuous") || bs_mode("continuous-live") || bs_mode("continuous-release") || bs_mode("continuous-unmatched") || bs_mode("continuous-contention") || bs_mode("lock-contention") || bs_mode("unknown-unlock") || bs_mode("property-contention") || bs_mode("release-contention");}
static int bs_is_indexed(void){return bootstrap_mode[0]=='i' && bootstrap_mode[1]=='d';}
static int bs_is_fill(void){return bootstrap_mode[0]=='f' && bootstrap_mode[1]=='i' && bootstrap_mode[2]=='l' && bootstrap_mode[3]=='l';}
struct BsSurface {void** table;struct BsSurface* state;u32 width,height,bits,primary,kind,held,key;u8 *pixels,*locked;void* palette;};
struct BsRect {i32 left,top,right,bottom;};
static struct BsSurface *bs_alias,*bs_created;
static u32 bs_locks,bs_unlocks,bs_descriptions,bs_queries,bs_draws,bs_creates,bs_fail,bs_nested;
static u32 bs_discard_lock,bs_restores;
static int bs_is_flip(void){return (bootstrap_mode[0]=='f' && bootstrap_mode[1]=='l') || (bs_is_indexed() && bootstrap_mode[3]=='f');}
static struct BsSurface* bs_back;static u32 bs_attachments,bs_flips,bs_attachment_mutations;
static u32* bs_stream;static HANDLE bs_events;
static struct BsSurface* bs_state(void* object){struct BsSurface* s=object;return s->state?s->state:s;}
static void bs_entry(void){if(GetLastError()!=0x77)ExitProcess(150);SetLastError(0x88);}
static u32 bs_pixel(const u8* at,u32 bytes){u32 value=0;for(u32 i=0;i<bytes;++i)value|=(u32)at[i]<<(i*8);return value;}
static void bs_put(u8* at,u32 bytes,u32 value){for(u32 i=0;i<bytes;++i)at[i]=(u8)(value>>(8*i));}
static void bs_desc(struct BsSurface* s,u32* d){
    d[1]=0x1007;d[2]=s->height;d[3]=s->width;d[18]=32;d[19]=0x40;d[21]=s->bits;
    d[22]=s->bits==16?0xf800:0xff0000;d[23]=s->bits==16?0x7e0:0xff00;d[24]=s->bits==16?0x1f:0xff;d[26]=s->primary?0x200:0x840;
    if(s->bits==8){d[19]=0x60;d[22]=d[23]=d[24]=0;}
    if(bs_mode("descriptor-key") || bs_mode("descriptor-key-range") || bs_is_fill() || bs_mode("continuous-live")){
        d[1]|=0x10000;d[16]=s->key;d[17]=s->key+bs_mode("descriptor-key-range");
    }
    if(bs_is_flip()){d[26]=s->primary?0x238:0x1c;if(s->primary){d[1]|=0x20;d[5]=bs_mode("flip-chain")?2:1;if(bs_mode("flip-count-missing"))d[1]&=~0x20u;}}
}
static i32 WIN bs_description(void* object,u32* d){
    bs_entry();++bs_descriptions;struct BsSurface* s=bs_state(object);if(d[0]!=(((struct BsSurface*)object)->kind>=14?124u:108u))ExitProcess(151);
    bs_desc(s,d);
    if(bs_mode("caps-missing"))d[1]&=~1u;
    if(bs_mode("dimensions-missing"))d[1]&=~6u;
    if(bs_mode("format-missing"))d[1]&=~0x1000u;
    if(bs_mode("bad-mask"))d[23]=d[22];
    if(bs_mode("lock-description-change") && !s->primary)d[3]=799;
    if(bs_mode("failed-description"))return -1;
    return 23;
}
static i32 WIN bs_query(void* object,const u8* guid,void** output){(void)object;bs_entry();if(guid[0]!=0x81)ExitProcess(152);++bs_queries;*output=bs_alias;return 23;}
static i32 WIN bs_create(void* object,u32* d,void** output,void* outer){
    (void)object;bs_entry();if(outer || d[0]!=124 || d[3]!=800 || d[2]!=600)ExitProcess(153);++bs_creates;*output=bs_created;
    /* Mutating the input after consuming it must not rewrite captured provenance. */
    d[1]=0;d[3]=123;return 23;
}
static i32 WIN bs_lock(void* object,void* rect,u32* d,u32 flags,HANDLE event){
    bs_entry();struct BsSurface* s=bs_state(object);if((s->primary && !bs_is_indexed()) || s->held || rect || flags!=1 || event)ExitProcess(154);
    ++bs_locks;s->held=1;u32 row=s->width*(s->bits/8),pitch=row+8;
    for(u32 y=0;y<s->height;++y)for(u32 x=0;x<row;++x)s->locked[((bs_mode("negative") || bs_mode("idx-negative") || bs_mode("idxcopy-negative") || bs_mode("idxflip-negative"))?s->height-1-y:y)*pitch+x]=s->pixels[y*row+x];
    bs_desc(s,d);d[1]|=8;d[4]=(bs_mode("negative") || bs_mode("idx-negative") || bs_mode("idxcopy-negative") || bs_mode("idxflip-negative"))?(u32)-(i32)pitch:pitch;
    d[9]=(u32)(s->locked+((bs_mode("negative") || bs_mode("idx-negative") || bs_mode("idxcopy-negative") || bs_mode("idxflip-negative"))?(s->height-1)*pitch:0));return 13;
}
static i32 WIN bs_unlock(void* object,void* argument){
    bs_entry();struct BsSurface* s=bs_state(object);u32 row=s->width*(s->bits/8),pitch=row+8;
    void* expected=s->kind>=14?0:s->locked+((bs_mode("negative") || bs_mode("idx-negative") || bs_mode("idxcopy-negative") || bs_mode("idxflip-negative"))?(s->height-1)*pitch:0);
    if(!s->held || argument!=expected)ExitProcess(155);++bs_unlocks;s->held=0;
    for(u32 y=0;y<s->height;++y)for(u32 x=0;x<row;++x)s->pixels[y*row+x]=s->locked[((bs_mode("negative") || bs_mode("idx-negative") || bs_mode("idxcopy-negative") || bs_mode("idxflip-negative"))?s->height-1-y:y)*pitch+x];
    for(u32 i=0;i<pitch*s->height;++i)s->locked[i]=0xcc;return 19;
}
static u32 WIN bs_release(void* object){(void)object;bs_entry();return 0;}
static i32 WIN bs_restore(void* object){(void)object;bs_entry();++bs_restores;return 23;}
static i32 WIN bs_clipper(void* object,void* clipper){(void)object;bs_entry();if(clipper)ExitProcess(156);return 23;}
static i32 WIN bs_key(void* object,u32 flags,u32* key){bs_entry();
    u32 expected=bs_mode("idxcopy-key-range")?256:0;if(flags!=8 || !key || key[0]!=expected || key[1]!=expected)ExitProcess(157);
    bs_state(object)->key=key[0];return 23;
}

static void (*bs_copy_hook)(void);
static i32 bs_copy(void* target,void* dest,void* source,void* rect,u32 flags,u32 fast,u32 x,u32 y){
    bs_entry();++bs_draws;if(bs_fail){bs_fail=0;return -1;}
    if(bs_copy_hook){void (*hook)(void)=bs_copy_hook;bs_copy_hook=0;hook();}
    struct BsSurface *a=bs_state(source),*b=bs_state(target);if(a->held || b->held)ExitProcess(158);
    if(bs_mode("nested-description") && !bs_nested){
        bs_nested=1;u32 d[31]={0};d[0]=((struct BsSurface*)target)->kind>=14?124:108;SetLastError(0x77);
        if(((i32 (WIN *)(void*,void*))((struct BsSurface*)target)->table[22])(target,d)!=23 || GetLastError()!=0x88)ExitProcess(159);
    }
    struct BsRect r={0,0,(i32)a->width,(i32)a->height};if(rect)r=*(struct BsRect*)rect;
    i32 dx=(i32)x,dy=(i32)y;if(!fast && dest){dx=((struct BsRect*)dest)->left;dy=((struct BsRect*)dest)->top;}
    if(!fast && !dest)dx=dy=0;
    u32 bytes=a->bits/8,keyed=fast?(flags&1)!=0:(flags&0x8000)!=0;
    for(i32 yy=r.top;yy<r.bottom;++yy)for(i32 xx=r.left;xx<r.right;++xx){
        u32 value=bs_pixel(a->pixels+((u32)yy*a->width+(u32)xx)*bytes,bytes);
        if(!keyed || value!=a->key)bs_put(b->pixels+((u32)(dy+yy-r.top)*b->width+(u32)(dx+xx-r.left))*bytes,bytes,value);
    }
    SetLastError(0x88);return 17;
}
static i32 WIN bs_blt(void* target,void* dest,void* source,void* rect,u32 flags,void* effects){
    if(!source && flags==0x01000400){
        bs_entry();++bs_draws;if(bs_fail){bs_fail=0;return -1;}
        struct BsSurface* s=bs_state(target);if(rect || !effects || ((u32*)effects)[0]!=100 || s->held)ExitProcess(197);
        struct BsRect r={0,0,(i32)s->width,(i32)s->height};if(dest)r=*(struct BsRect*)dest;
        u32 bytes=s->bits/8,value=((u32*)effects)[20];
        for(i32 y=r.top;y<r.bottom;++y)for(i32 x=r.left;x<r.right;++x)bs_put(s->pixels+((u32)y*s->width+(u32)x)*bytes,bytes,value);
        ((u32*)effects)[20]=0xdead;return 17;
    }
    if(effects)ExitProcess(160);return bs_copy(target,dest,source,rect,flags,0,0,0);
}
static i32 WIN bs_fast(void* target,u32 x,u32 y,void* source,void* rect,u32 flags){return bs_copy(target,0,source,rect,flags,1,x,y);}
static void bs_record(struct BsSurface* target){
    char delay[4];if(GetEnvironmentVariableA("MNM_COMMAND_TEST_DELAY",delay,sizeof(delay)))Sleep(250);
    char native[]="original-00000000.bin",frame[]="frame-00000000.bin";static const char hex[]="0123456789abcdef";
    for(u32 i=0;i<8;++i){native[9+i]=hex[(bs_draws>>(28-i*4))&15];frame[6+i]=native[9+i];}
    u32 length=target->width*target->height*(target->bits/8),written=0;
    HANDLE file=CreateFileA(native,0x40000000,1,0,1,0x80,0);
    if(file==(HANDLE)-1 || !WriteFile(file,target->pixels,length,&written,0) || written!=length)ExitProcess(161);CloseHandle(file);
    length=bs_stream[MNM_FRAME_V1_FRAME_COUNT_OFFSET/4]?64+bs_stream[MNM_FRAME_V1_WIDTH_OFFSET/4]*bs_stream[MNM_FRAME_V1_HEIGHT_OFFSET/4]*4:64;
    file=CreateFileA(frame,0x40000000,1,0,1,0x80,0);
    if(file==(HANDLE)-1 || !WriteFile(file,bs_stream,length,&written,0) || written!=length || bs_stream[MNM_FRAME_V1_SEQUENCE_OFFSET/4]&1)ExitProcess(162);CloseHandle(file);
    u32 event[2]={bs_draws,bs_stream[MNM_FRAME_V1_FRAME_COUNT_OFFSET/4]};if(!WriteFile(bs_events,event,8,&written,0) || written!=8)ExitProcess(163);
}
static void bs_draw(struct BsSurface* target,struct BsSurface* source,u32 fast,u32 keyed,u32 partial){
    struct BsRect sr={0,0,(i32)source->width,(i32)source->height},dr={0,0,(i32)target->width,(i32)target->height};
    if(partial){sr.right=dr.right=400;}
    u32 failed=bs_fail;SetLastError(0x77);
    i32 result=fast?((i32 (WIN *)(void*,u32,u32,void*,void*,u32))target->table[7])(target,0,0,source,partial?&sr:0,keyed?0x11:0x10):
        ((i32 (WIN *)(void*,void*,void*,void*,u32,void*))target->table[5])(target,partial?&dr:0,source,partial?&sr:0,keyed?0x1008000:0x1000000,0);
    if(result!=(failed?-1:17) || GetLastError()!=0x88)ExitProcess(164);bs_record(bs_state(target));
}
static void bs_fill(struct BsSurface* target,u32 partial){
    u32 effects[25]={100};effects[20]=target->bits==16?0x1234:0x123456;
    struct BsRect r={1,2,20,30};u32 failed=bs_fail;SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*,void*,void*,u32,void*))target->table[5])(target,partial?&r:0,0,0,0x01000400,effects)!=(failed?-1:17) || GetLastError()!=0x88)ExitProcess(198);
    bs_record(target);
}
static void bs_seed(struct BsSurface* s){
    u32 d[31]={0};d[0]=s->kind>=14?124:108;SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*,void*,u32,HANDLE))s->table[25])(s,0,d,1,0)!=13 || GetLastError()!=0x88)ExitProcess(165);
    if(bs_mode("lock-description-change")){u32 changed[31]={0};changed[0]=s->kind>=14?124:108;SetLastError(0x77);
        if(((i32 (WIN *)(void*,void*))s->table[22])(s,changed)!=23 || GetLastError()!=0x88)ExitProcess(179);}
    if(bs_discard_lock){SetLastError(0x77);
        if(((i32 (WIN *)(void*))s->table[27])(s)!=23 || GetLastError()!=0x88)ExitProcess(194);}
    SetLastError(0x77);if(((i32 (WIN *)(void*,void*))s->table[32])(s,s->kind>=14?0:(void*)d[9])!=19 || GetLastError()!=0x88)ExitProcess(166);
}
static u32 bs_thread_ready,bs_thread_waits;
static u32 WIN bs_thread_overlap(void* unused){
    (void)unused;RenderCaptureGuardForTest(1);__atomic_store_n(&bs_thread_ready,1,__ATOMIC_RELEASE);
    while(RenderCaptureWaitsForTest()==bs_thread_waits)Sleep(0);
    if(bs_mode("thread-timeout"))Sleep(50);
    RenderCaptureGuardForTest(0);return 0;
}
static void bs_thread_draw(struct BsSurface* target,struct BsSurface* source,HANDLE timing){
    __atomic_store_n(&bs_thread_ready,0,__ATOMIC_RELEASE);bs_thread_waits=RenderCaptureWaitsForTest();
    HANDLE thread=CreateThread(0,0,&bs_thread_overlap,0,0,0);if(!thread)ExitProcess(203);
    while(!__atomic_load_n(&bs_thread_ready,__ATOMIC_ACQUIRE))Sleep(0);
    unsigned long long begin,end;u32 written;
    if(!QueryPerformanceCounter(&begin))ExitProcess(204);
    bs_draw(target,source,0,0,1);
    if(!QueryPerformanceCounter(&end))ExitProcess(205);
    unsigned long long elapsed=end-begin;
    if(!WriteFile(timing,&elapsed,8,&written,0) || written!=8)ExitProcess(206);
    if(WaitForSingleObject(thread,5000)!=0)ExitProcess(207);CloseHandle(thread);
}
static struct BsSurface* bs_missed_sprite;
static void bs_miss_sprite(void){RenderCaptureGuardForTest(1);bs_seed(bs_missed_sprite);RenderCaptureGuardForTest(0);}
static i32 WIN bs_attached(void* object,u32* caps,void** out){
    bs_entry();++bs_attachments;if(!bs_state(object)->primary || caps[0]!=4)ExitProcess(180);
    *out=bs_back;return bs_mode("flip-attachment-failed")?-1:23;
}
static i32 WIN bs_add_attachment(void* object,void* other){
    bs_entry();if(!bs_state(object)->primary || other!=bs_back)ExitProcess(188);++bs_attachment_mutations;return 23;
}
static i32 WIN bs_delete_attachment(void* object,u32 flags,void* other){
    if(flags)ExitProcess(189);return bs_add_attachment(object,other);
}
static i32 WIN bs_flip(void* object,void* target,u32 flags){
    bs_entry();++bs_flips;++bs_draws;struct BsSurface* front=bs_state(object);
    if(front->held || bs_back->held || flags!=(bs_mode("flip-flags")?0x10u:1u) ||
       target!=((bs_mode("flip-alias") || bs_mode("idxflip-alias"))?(void*)bs_alias:(bs_mode("flip-target") || bs_mode("idxflip-target"))?(void*)bs_back:bs_mode("flip-wrong-target")?(void*)bs_alias:0))ExitProcess(181);
    if((bs_mode("flip-retry") || bs_mode("idxflip-retry")) && bs_flips==1)return -1;
    if(bs_mode("flip-nested")){u32 d[31]={0};d[0]=124;SetLastError(0x77);
        if(((i32 (WIN *)(void*,void*))front->table[22])(front,d)!=23)ExitProcess(182);}
    u8* data=front->pixels;front->pixels=bs_back->pixels;bs_back->pixels=data;SetLastError(0x88);return 17;
}
static void bs_do_flip(struct BsSurface* front){
    SetLastError(0x77);i32 result=((i32 (WIN *)(void*,void*,u32))front->table[11])(front,
        (bs_mode("flip-alias") || bs_mode("idxflip-alias"))?bs_alias:(bs_mode("flip-target") || bs_mode("idxflip-target"))?bs_back:bs_mode("flip-wrong-target")?bs_alias:0,bs_mode("flip-flags")?0x10:1);
    if(result!=((bs_mode("flip-retry") || bs_mode("idxflip-retry")) && bs_flips==1?-1:17) || GetLastError()!=0x88)ExitProcess(183);
    bs_record(front);
}
static void bs_test_flips(struct BsSurface* front,struct BsSurface* back,struct BsSurface* sprite){
    bs_back=back;u32 d[31]={124};SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*))back->table[22])(back,d)!=23 || GetLastError()!=0x88)ExitProcess(184);
    u32 caps[4]={4};void* out=0;
    if(!bs_mode("flip-unobserved")){SetLastError(0x77);
        i32 status=((i32 (WIN *)(void*,void*,void**))front->table[12])(front,caps,&out);
        if(status!=(bs_mode("flip-attachment-failed")?-1:23) || GetLastError()!=0x88 || out!=back)ExitProcess(185);}
    if(bs_mode("flip-mutated")){
        SetLastError(0x77);if(((i32 (WIN *)(void*,u32,void*))front->table[8])(front,0,back)!=23 || GetLastError()!=0x88)ExitProcess(190);
        SetLastError(0x77);if(((i32 (WIN *)(void*,void*))front->table[3])(front,back)!=23 || GetLastError()!=0x88)ExitProcess(191);
    }
    if((bs_mode("flip-alias") || bs_mode("idxflip-alias"))){
        static const u8 iid[16]={0x81,0xdb,0x14,0x6c,0x33,0xa7,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60};
        SetLastError(0x77);if(((i32 (WIN *)(void*,const void*,void**))back->table[0])(back,iid,&out)!=23 || out!=bs_alias || GetLastError()!=0x88)ExitProcess(187);
    }
    if((bs_mode("flip-retry") || bs_mode("idxflip-retry")))bs_do_flip(front);
    bs_do_flip(front);
    int supported=(bs_mode("flip-alias") || bs_mode("idxflip-alias")) || bs_mode("flip-budget") || bs_mode("flip-rotate") || (bs_mode("flip-target") || bs_mode("idxflip-target")) || (bs_mode("flip-retry") || bs_mode("idxflip-retry")) || bs_mode("flip-no-reseed");
    if(supported){
        if(!bs_mode("flip-no-reseed"))bs_seed(back);
        bs_do_flip(front);
        if(!bs_mode("flip-no-reseed")){bs_seed(sprite);bs_draw(back,sprite,1,0,0);bs_do_flip(front);}
    }
    if(bs_mode("flip-budget"))for(u32 i=0;i<15;++i)bs_do_flip(front);
    if(bs_locks!=(supported && !bs_mode("flip-no-reseed")?3u:1u) || bs_unlocks!=bs_locks || bs_queries!=(u32)(bs_mode("flip-alias") || bs_mode("idxflip-alias")) ||
       bs_descriptions!=2u+(u32)bs_mode("flip-nested") || bs_attachments!=!bs_mode("flip-unobserved") || bs_attachment_mutations!=(bs_mode("flip-mutated")?2u:0u))ExitProcess(186);
    ExitProcess(0);
}
#include "indexed_owned_selftest.h"
#include "indexed_copy_selftest.h"
#include "dc_selftest.h"
static void test_bootstrap(void){
    static void* table[33],*alias_table[33];table[0]=(void*)&bs_query;table[2]=(void*)&bs_release;table[5]=(void*)&bs_blt;table[7]=(void*)&bs_fast;
    table[3]=(void*)&bs_add_attachment;table[8]=(void*)&bs_delete_attachment;table[11]=(void*)&bs_flip;table[12]=(void*)&bs_attached;table[22]=(void*)&bs_description;table[25]=(void*)&bs_lock;table[32]=(void*)&bs_unlock;table[28]=(void*)&bs_clipper;table[29]=(void*)&bs_key;
    table[27]=(void*)&bs_restore;
    table[17]=(void*)&bs_get_dc;table[26]=(void*)&bs_release_dc;
    for(u32 i=0;i<33;++i)alias_table[i]=table[i];
    static struct BsSurface source,target,sprite,alias;
    source.table=target.table=sprite.table=table;source.kind=target.kind=sprite.kind=(bs_mode("legacy") || bs_mode("idx-legacy") || bs_mode("idxcopy-legacy") || bs_mode("idxflip-legacy"))?12:14;
    source.width=target.width=800;source.height=target.height=600;sprite.width=sprite.height=2;
    source.bits=target.bits=sprite.bits=bs_is_indexed()?8:(bs_mode("rgb24")||bs_mode("fill24")||bs_mode("dc-rgb24"))?24:(bs_mode("rgb32")||bs_mode("fill32")||bs_mode("dc-rgb32"))?32:16;target.primary=!bs_mode("offscreen");
    if(bs_mode("descriptor-key") || bs_mode("descriptor-key-range") || bs_is_fill() || bs_mode("continuous-live"))sprite.key=sprite.bits==16?0x7ff:0xff00;
    alias.table=alias_table;alias.state=(bs_mode("flip-alias") || bs_mode("idxflip-alias"))?&source:&target;alias.kind=11;bs_alias=&alias;
    struct BsSurface* surfaces[3]={&source,&target,&sprite};u32 bytes=source.bits/8;
    for(u32 i=0;i<3;++i){struct BsSurface* s=surfaces[i];u32 length=s->width*s->height*bytes;
        s->pixels=HeapAlloc(GetProcessHeap(),0,length);s->locked=HeapAlloc(GetProcessHeap(),0,(s->width*bytes+8)*s->height);if(!s->pixels || !s->locked)ExitProcess(167);
        for(u32 y=0;y<s->height;++y)for(u32 x=0;x<s->width;++x){
            u32 value=s->bits==16?(((x*3+y*7)&31)<<11)|(((x*5+y*11)&63)<<5)|((x+y*13)&31):
                (((x*17+y*11)&255)<<16)|(((x*3+y*19)&255)<<8)|((x^y)&255)|(s->bits==32?0x80000000u:0);
            if(i==2)value=s->bits==16?(((x+y)&1)?0x7ff:0xffff):(((x+y)&1)?0xff00:0xff00ff)|(s->bits==32?0x80000000u:0);
            bs_put(s->pixels+(y*s->width+x)*bytes,bytes,i==1 && !bs_is_indexed()?0xdead:value);
        }
    }
    char path[512];if(!GetEnvironmentVariableA("MNM_RENDER_STREAM",path,sizeof(path)))ExitProcess(168);
    HANDLE file=CreateFileA(path,0xc0000000,3,0,3,0x80,0);if(file==(HANDLE)-1)ExitProcess(169);
    HANDLE mapping=CreateFileMappingA(file,0,4,0,MNM_FRAME_V1_SIZE,0);CloseHandle(file);if(!mapping)ExitProcess(170);
    bs_stream=MapViewOfFile(mapping,2,0,0,MNM_FRAME_V1_SIZE);CloseHandle(mapping);if(!bs_stream)ExitProcess(171);
    bs_events=CreateFileA("events.bin",0x40000000,1,0,1,0x80,0);if(bs_events==(HANDLE)-1)ExitProcess(172);
    RenderInstallForTest(&source,source.kind);RenderInstallForTest(&alias,11);
    if(bs_is_indexed() && (bootstrap_mode[3]=='c' || bootstrap_mode[3]=='f'))ic_test(&target,&source,&sprite,&alias);
    if(bs_is_indexed())ip_test(&target);
    if(bs_dc_case())bs_dc_seed(&source);else bs_seed(&source);
    if(bs_mode("created")){
        static void* draw_table[7];draw_table[6]=(void*)&bs_create;void** draw=draw_table;bs_created=&target;RenderInstallForTest(&draw,4);
        u32 d[31]={124};bs_desc(&target,d);void* output=0;SetLastError(0x77);
        if(((i32 (WIN *)(void*,void*,void**,void*))draw_table[6])(&draw,d,&output,0)!=23 || GetLastError()!=0x88 || output!=&target)ExitProcess(173);
    }else{
        u32 d[31]={0};d[0]=target.kind>=14?124:108;SetLastError(0x77);
        i32 status=((i32 (WIN *)(void*,void*))table[22])(&target,d);
        if(status!=(bs_mode("failed-description")?-1:23) || GetLastError()!=0x88)ExitProcess(174);
        SetLastError(0x77);if(((i32 (WIN *)(void*,void*))table[28])(&target,0)!=23 || GetLastError()!=0x88)ExitProcess(175);
    }
    if(bs_is_flip())bs_test_flips(&target,&source,&sprite);
    struct BsSurface* destination=&target;
    if(bs_mode("alias")){
        static const u8 iid[16]={0x81,0xdb,0x14,0x6c,0x33,0xa7,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60};void* output=0;SetLastError(0x77);
        if(((i32 (WIN *)(void*,const void*,void**))table[0])(&target,iid,&output)!=23 || GetLastError()!=0x88 || output!=&alias)ExitProcess(176);destination=&alias;
    }
    if(bs_mode("keyed")){u32 key[2]={0,0};SetLastError(0x77);if(((i32 (WIN *)(void*,u32,void*))table[29])(&source,8,key)!=23 || GetLastError()!=0x88)ExitProcess(177);}
    u32 rejection=bs_mode("partial")||bs_mode("keyed")||bs_mode("caps-missing")||bs_mode("dimensions-missing")||bs_mode("format-missing")||bs_mode("bad-mask")||bs_mode("failed-description")||bs_mode("lock-description-change")||bs_mode("dc-format")||bs_mode("dc-unmatched")||bs_mode("dc-swapped");
    if(bs_mode("retry")){bs_fail=1;bs_draw(destination,&source,0,0,0);}
    if(bs_is_fill()){
        if(bs_mode("fill-failed")){bs_fail=1;bs_fill(destination,0);}
        if(bs_mode("fill-partial"))bs_fill(destination,1);
        bs_fill(destination,0);
        if(bs_mode("fill-update"))bs_fill(destination,1);
    }else bs_draw(destination,&source,bs_mode("fast")||bs_mode("alias"),bs_mode("keyed"),bs_mode("partial"));
    if(bs_mode("nested-description"))bs_draw(destination,&source,0,0,0);
    if(!rejection){bs_seed(&sprite);bs_draw(destination,&sprite,1,bs_mode("descriptor-key")||bs_mode("descriptor-key-range")||bs_is_fill(),0);}
    if(bs_mode("nested-source-contention") || bs_mode("pixel-miss-overflow")){
        if(bs_mode("nested-source-contention")){bs_missed_sprite=&source;bs_copy_hook=&bs_miss_sprite;}
        else{
            RenderCaptureGuardForTest(1);for(u32 i=0;i<43;++i)bs_seed(&sprite);RenderCaptureGuardForTest(0);
        }
        /* Uncertain source/checkpoint rejects partial updates until reseeded. */
        bs_draw(destination,&source,0,0,1);bs_draw(destination,&sprite,1,0,0);
        bs_seed(&source);bs_draw(destination,&source,0,0,0);
    }
    if(bs_mode("many-surfaces")){
        static struct BsSurface assets[65],aliases[65];static u8 native[65][8],locked[65][24];
        static const u8 iid[16]={0x81,0xdb,0x14,0x6c,0x33,0xa7,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60};
        for(u32 i=0;i<65;++i){struct BsSurface* s=assets+i;s->table=table;s->kind=14;s->bits=16;s->width=s->height=2;s->pixels=native[i];s->locked=locked[i];
            for(u32 j=0;j<8;++j)native[i][j]=(u8)(i+j*17);
            aliases[i].table=alias_table;aliases[i].state=s;aliases[i].kind=11;bs_alias=aliases+i;
            u32 d[31]={124};void* output=0;SetLastError(0x77);
            if(((i32 (WIN *)(void*,void*))table[22])(s,d)!=23 || GetLastError()!=0x88)ExitProcess(201);
            SetLastError(0x77);if(((i32 (WIN *)(void*,const void*,void**))table[0])(s,iid,&output)!=23 || output!=aliases+i || GetLastError()!=0x88)ExitProcess(202);
        }
        bs_seed(assets+64);bs_draw(destination,aliases+64,1,0,0);
    }
    if(bs_continuous()){
        if(bs_mode("partial-lock-contention")){bs_missed_sprite=&sprite;bs_miss_sprite();}
        if(bs_mode("partial-blit-contention")){
            RenderCaptureGuardForTest(1);bs_draw(&source,&sprite,1,0,0);RenderCaptureGuardForTest(0);
        }
        if(bs_mode("nested-lock-contention")){bs_missed_sprite=&sprite;bs_copy_hook=&bs_miss_sprite;}
        if(bs_mode("continuous-release")){SetLastError(0x77);if(((u32 (WIN *)(void*))sprite.table[2])(&sprite)!=0 || GetLastError()!=0x88)ExitProcess(193);}
        if(bs_mode("continuous-unmatched")){bs_discard_lock=1;bs_seed(&sprite);bs_discard_lock=0;}
        if(bs_mode("continuous-contention")){
            RenderCaptureGuardForTest(1);bs_draw(destination,&source,0,0,0);RenderCaptureGuardForTest(0);
        }
        if(bs_mode("lock-contention")){RenderCaptureGuardForTest(1);bs_seed(&sprite);RenderCaptureGuardForTest(0);}
        if(bs_mode("unknown-unlock")){
            /* Emulate a Lock performed before interception: the actual Unlock
             * is forwarded on a surface with no captured identity. */
            static struct BsSurface unknown;static u8 native[8],locked[24];u32 d[31]={124};
            unknown.table=table;unknown.width=unknown.height=2;unknown.bits=16;unknown.kind=14;unknown.pixels=native;unknown.locked=locked;
            SetLastError(0x77);if(bs_lock(&unknown,0,d,1,0)!=13 || GetLastError()!=0x88)ExitProcess(199);
            SetLastError(0x77);if(((i32 (WIN *)(void*,void*))table[32])(&unknown,0)!=19 || GetLastError()!=0x88)ExitProcess(200);
        }
        if(bs_mode("property-contention") || bs_mode("release-contention")){
            RenderCaptureGuardForTest(1);SetLastError(0x77);
            if(bs_mode("property-contention")){
                if(((i32 (WIN *)(void*,void*))target.table[28])(&target,0)!=23 || GetLastError()!=0x88)ExitProcess(195);
            }else if(((u32 (WIN *)(void*))sprite.table[2])(&sprite)!=0 || GetLastError()!=0x88)ExitProcess(196);
            RenderCaptureGuardForTest(0);
        }
        HANDLE timing=0;
        if(bs_thread_case()){
            timing=CreateFileA("timing.bin",0x40000000,1,0,1,0x80,0);unsigned long long frequency;u32 written;
            if(timing==(HANDLE)-1 || !QueryPerformanceFrequency(&frequency) || !WriteFile(timing,&frequency,8,&written,0) || written!=8)ExitProcess(208);
        }
        for(u32 i=0;i<20;++i){
            if(bs_scoped_partial()){
                if(bs_thread_case())bs_thread_draw(destination,&source,timing);
                else if(bs_mode("partial-blit-contention"))bs_draw(destination,&sprite,1,0,0);
                else bs_draw(destination,&source,0,0,1);
            }else{source.pixels[0]=(u8)i;bs_seed(&source);bs_draw(destination,&source,0,0,0);}
            if(bs_mode("continuous-live")){bs_draw(destination,&sprite,1,1,0);bs_fill(destination,1);}}
        if(bs_thread_case()){CloseHandle(timing);bs_draw(destination,&source,0,0,0);}
        if(bs_mode("continuous-live")){bs_fail=1;bs_fill(destination,0);}
    }
    if(bs_locks!=1u-bs_dc_case()+!rejection+20u*(bs_continuous()&&!bs_scoped_partial())+bs_mode("partial-lock-contention")+bs_mode("nested-lock-contention")+2u*bs_mode("nested-source-contention")+44u*bs_mode("pixel-miss-overflow")+bs_mode("continuous-unmatched")+bs_mode("lock-contention")+bs_mode("unknown-unlock")+bs_mode("many-surfaces") || bs_unlocks!=bs_locks || bs_restores!=(u32)bs_mode("continuous-unmatched") || bs_queries!=(u32)bs_mode("alias")+65u*bs_mode("many-surfaces") || bs_creates!=(u32)bs_mode("created") ||
       bs_descriptions!=(bs_mode("created")?0u:1u)+bs_dc_case()+(u32)bs_mode("nested-description")+(u32)bs_mode("lock-description-change")+65u*bs_mode("many-surfaces") || bs_draws!=1u+!rejection+bs_mode("retry")+bs_mode("nested-description")+20u*bs_continuous()+41u*bs_mode("continuous-live")+bs_mode("continuous-contention")+bs_mode("partial-blit-contention")+bs_thread_case()+3u*(bs_mode("nested-source-contention")||bs_mode("pixel-miss-overflow"))+bs_mode("fill-failed")+bs_mode("fill-partial")+bs_mode("fill-update")+bs_mode("many-surfaces"))ExitProcess(178);
    ExitProcess(0);
}
