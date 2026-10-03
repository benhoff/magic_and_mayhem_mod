/* Independent native storage; Unlock poisons every exposed byte. */
static char partial_mode[32];
struct PlSurface {void** table;u32 bits,kind,held; i32 pitch;u8 native[64],exposed[128];void* pointer;};
static struct PlSurface pl_surface,pl_target;
static u32 pl_locks,pl_unlocks,pl_step,pl_fail_lock,pl_fail_unlock,pl_expected_flags;
static void *pl_expected_rect,*pl_expected_unlock;
static int pl_mode(const char* name){u32 i=0;while(name[i] && name[i]==partial_mode[i])++i;return name[i]==partial_mode[i];}
static int pl_has_palette(void){return pl_mode("indexed-palette") || pl_mode("indexed-palette-change");}
static u8 pl_colors[1024];static u32 pl_caps,pl_reads,pl_assigns,pl_writes;
static void** pl_palette;
static i32 WIN pl_palette_caps(void* object,u32* caps){if(object!=&pl_palette || GetLastError()!=0x77)ExitProcess(236);++pl_caps;*caps=0x54;SetLastError(0x88);return 23;}
static i32 WIN pl_palette_read(void* object,u32 flags,u32 first,u32 count,u8* colors){
    if(object!=&pl_palette || GetLastError()!=0x77 || flags || first || count!=256)ExitProcess(237);++pl_reads;
    copy_bytes(colors,pl_colors,1024);SetLastError(0x88);return 23;
}
static i32 WIN pl_palette_write(void* object,u32 flags,u32 first,u32 count,u8* colors){
    if(object!=&pl_palette || GetLastError()!=0x77 || flags || first || count!=256)ExitProcess(238);++pl_writes;
    copy_bytes(pl_colors,colors,1024);for(u32 i=0;i<1024;++i)colors[i]=0xcc;SetLastError(0x88);return 23;
}
static i32 WIN pl_palette_assign(void* object,void* palette){
    if(object!=&pl_surface || palette!=&pl_palette || GetLastError()!=0x77)ExitProcess(239);++pl_assigns;SetLastError(0x88);return 23;
}
static void pl_observe_palette(void){
    static void* table[7];table[3]=(void*)&pl_palette_caps;table[4]=(void*)&pl_palette_read;table[6]=(void*)&pl_palette_write;pl_palette=table;
    for(u32 i=0;i<256;++i){pl_colors[i*4]=(u8)(i*3);pl_colors[i*4+1]=(u8)(i*7);pl_colors[i*4+2]=(u8)(255-i);pl_colors[i*4+3]=0xa5;}
    RenderInstallForTest(&pl_palette,20);u32 caps;u8 colors[1024];SetLastError(0x77);
    if(((i32 (WIN *)(void*,u32*))table[3])(&pl_palette,&caps)!=23 || GetLastError()!=0x88)ExitProcess(240);
    SetLastError(0x77);if(((i32 (WIN *)(void*,u32,u32,u32,void*))table[4])(&pl_palette,0,0,256,colors)!=23 || GetLastError()!=0x88)ExitProcess(241);
    SetLastError(0x77);if(((i32 (WIN *)(void*,void*))pl_surface.table[31])(&pl_surface,&pl_palette)!=23 || GetLastError()!=0x88)ExitProcess(242);
}
static i32 WIN pl_lock(void* object,void* rect,u32* d,u32 flags,HANDLE event){
    struct PlSurface* s=object;if(s->held || GetLastError()!=0x77 || event || rect!=pl_expected_rect || flags!=pl_expected_flags)ExitProcess(210);
    ++pl_locks;SetLastError(0x88);if(pl_fail_lock){pl_fail_lock=0;return -1;}
    s->held=1;u32 bytes=s->bits/8,stride=4*bytes,magnitude=(u32)(s->pitch<0?-s->pitch:s->pitch);
    for(u32 i=0;i<128;++i)s->exposed[i]=0xee;
    u8* top=s->exposed+(s->pitch<0?3*magnitude:0);
    for(u32 y=0;y<4;++y)copy_bytes(top+(i32)y*s->pitch,s->native+y*stride,stride);
    i32* r=rect;s->pointer=top+(r?r[1]*s->pitch+r[0]*(i32)bytes:0);
    d[1]=1;d[2]=d[3]=4;d[4]=(u32)s->pitch;d[9]=(u32)s->pointer;d[19]=s->bits==8?0x60:0x40;d[21]=s->bits;
    d[22]=s->bits==8?0:s->bits==16?0xf800:0xff0000;d[23]=s->bits==8?0:s->bits==16?0x7e0:0xff00;d[24]=s->bits==8?0:s->bits==16?0x1f:0xff;
    d[26]=(s->bits==8 && !pl_has_palette()) || (pl_mode("blit") && s==&pl_surface)?0x40:0x200;if(rect && pl_mode("layout"))d[3]=3;
    (void)flags;return 13;
}
static i32 WIN pl_unlock(void* object,void* argument){
    struct PlSurface* s=object;if(!s->held || GetLastError()!=0x77 || argument!=pl_expected_unlock)ExitProcess(211);
    ++pl_unlocks;SetLastError(0x88);if(pl_fail_unlock){pl_fail_unlock=0;return -1;}
    if(s->kind<14 && argument!=s->pointer)ExitProcess(212);
    u32 stride=4*(s->bits/8),magnitude=(u32)(s->pitch<0?-s->pitch:s->pitch);
    u8* top=s->exposed+(s->pitch<0?3*magnitude:0);
    for(u32 y=0;y<4;++y)copy_bytes(s->native+y*stride,top+(i32)y*s->pitch,stride);
    for(u32 i=0;i<128;++i)s->exposed[i]=0xcc;s->held=0;return 19;
}
static i32 WIN pl_restore(void* object){(void)object;if(GetLastError()!=0x77)ExitProcess(213);SetLastError(0x88);return 23;}
static i32 WIN pl_description(void* object,u32* d){
    if(GetLastError()!=0x77)ExitProcess(224);struct PlSurface* s=object;d[1]=0x1007;d[2]=d[3]=4;
    d[18]=32;d[19]=0x40;d[21]=s->bits;d[22]=0xf800;d[23]=0x7e0;d[24]=0x1f;d[26]=0x200;SetLastError(0x88);return 23;
}
static i32 WIN pl_clipper(void* object,void* clipper){(void)object;if(clipper || GetLastError()!=0x77)ExitProcess(225);SetLastError(0x88);return 23;}
static i32 WIN pl_blt(void* object,void* dst,void* source,void* rect,u32 flags,void* effects){
    if(object!=&pl_target || source!=&pl_surface || dst || rect || effects || flags!=0x1000000 || GetLastError()!=0x77 || pl_surface.held)ExitProcess(226);
    copy_bytes(pl_target.native,pl_surface.native,32);SetLastError(0x88);return 17;
}
static void pl_file(const char* path,const void* data,u32 size){u32 written;HANDLE f=CreateFileA(path,0x40000000,1,0,1,0x80,0);
    if(f==(HANDLE)-1 || !WriteFile(f,data,size,&written,0) || written!=size)ExitProcess(214);CloseHandle(f);}
static void (*pl_record_hook)(void);
static void pl_record(void){
    char path[]="original-00000000.bin";static const char hex[]="0123456789abcdef";
    for(u32 i=0;i<8;++i)path[9+i]=hex[(pl_step>>(28-i*4))&15];pl_file(path,pl_surface.native,16*(pl_surface.bits/8));
    if(pl_has_palette()){char colors_path[]="colors-00000000.bin";
        for(u32 i=0;i<8;++i)colors_path[7+i]=hex[(pl_step>>(28-i*4))&15];pl_file(colors_path,pl_colors,1024);}
    char frame_path[]="frame-00000000.bin";for(u32 i=0;i<8;++i)frame_path[6+i]=hex[(pl_step>>(28-i*4))&15];pl_file(frame_path,bs_stream,bs_stream[10]?128:64);if(pl_record_hook)pl_record_hook();++pl_step;
}
static void pl_cycle(i32* rect,u32 flags,u32 value){
    u32 d[31]={0};d[0]=pl_surface.kind>=14?124:108;u32 failed=pl_fail_lock;
    pl_expected_rect=rect;pl_expected_flags=flags;SetLastError(0x77);i32 status=((i32 (WIN *)(void*,void*,void*,u32,HANDLE))pl_surface.table[25])(&pl_surface,rect,d,flags,0);
    if(status!=(failed?-1:13) || GetLastError()!=0x88)ExitProcess(216);
    if(failed){pl_record();return;}
    pl_record();u32 bytes=pl_surface.bits/8;
    if(rect && pl_mode("indexed-palette-change")){u8 colors[1024];for(u32 i=0;i<1024;++i)colors[i]=(u8)(255-pl_colors[i]);
        SetLastError(0x77);if(((i32 (WIN *)(void*,u32,u32,u32,void*))pl_palette[6])(&pl_palette,0,0,256,colors)!=23 || GetLastError()!=0x88)ExitProcess(243);
        if(bs_stream[10]!=1)ExitProcess(244); /* No old base publication while locked. */
    }
    u32 width=rect?(u32)(rect[2]-rect[0]):4,height=rect?(u32)(rect[3]-rect[1]):4;
    for(u32 y=0;y<height;++y)for(u32 x=0;x<width;++x)for(u32 b=0;b<bytes;++b)
        ((u8*)pl_surface.pointer+(i32)y*pl_surface.pitch)[x*bytes+b]=(u8)((value+y*31+x*17)>>(8*b));
    if(rect && pl_mode("invalidate")){SetLastError(0x77);if(((i32 (WIN *)(void*))pl_surface.table[27])(&pl_surface)!=23 || GetLastError()!=0x88)ExitProcess(217);}
    void* argument=pl_surface.kind>=14?rect:pl_surface.pointer;
    if(rect && pl_mode("changed"))++rect[2];
    if(rect && pl_mode("argument"))argument=0;
    for(u32 attempt=0;attempt<2;++attempt){u32 fail=pl_fail_unlock;pl_expected_unlock=argument;SetLastError(0x77);
        if(((i32 (WIN *)(void*,void*))pl_surface.table[32])(&pl_surface,argument)!=(fail?-1:19) || GetLastError()!=0x88)ExitProcess(218);
        pl_record();if(!fail)break;
        /* Retry must recopy the latest live region rather than keep an earlier attempt. */
        ((u8*)pl_surface.pointer)[0]=0x1f;((u8*)pl_surface.pointer)[1]=0;
    }
}
/* Eight 8-MiB checkpoints exhaust cumulative bytes before the 16-file limit.
 * A detached 8-MiB partial base must not wrap the remaining-byte subtraction. */
static u8* pl_big_pixels;static u32 pl_big_locks,pl_big_unlocks;
static i32 WIN pl_big_lock(void* object,void* rect,u32* d,u32 flags,HANDLE event){
    (void)object;if(GetLastError()!=0x77 || flags!=1 || event)ExitProcess(230);++pl_big_locks;
    for(u32 i=0;i<8*1024*1024;++i)pl_big_pixels[i]=0x55;
    d[1]=1;d[2]=pl_mode("session-bytes")?1024:2048;d[3]=pl_mode("session-bytes")?2048:1024;d[4]=d[3]*4;d[9]=(u32)(pl_big_pixels+(rect?((i32*)rect)[1]*(i32)d[4]+((i32*)rect)[0]*4:0));
    d[19]=0x40;d[21]=32;d[22]=0xff0000;d[23]=0xff00;d[24]=0xff;d[26]=0x40;SetLastError(0x88);return 13;
}
static i32 WIN pl_big_unlock(void* object,void* rect){
    (void)object;if(GetLastError()!=0x77 || rect!=pl_expected_unlock)ExitProcess(231);++pl_big_unlocks;
    for(u32 i=0;i<8*1024*1024;++i)pl_big_pixels[i]=0xcc;SetLastError(0x88);return 19;
}
static void pl_big_test(void){
    static void* table[33];table[25]=(void*)&pl_big_lock;table[32]=(void*)&pl_big_unlock;void** object=table;
    pl_big_pixels=HeapAlloc(GetProcessHeap(),0,8*1024*1024);if(!pl_big_pixels)ExitProcess(232);RenderInstallForTest(&object,14);
    i32 region[4]={1,1,3,3};
    u32 replay=pl_mode("replay-budget"),count=replay?7:9;
    if(replay){region[1]=0;region[3]=2048;}
    for(u32 i=0;i<count;++i){u32 d[31]={124};void* rect=(replay?i!=0:i==8)?region:0;SetLastError(0x77);
        if(((i32 (WIN *)(void*,void*,void*,u32,HANDLE))table[25])(&object,rect,d,1,0)!=13 || GetLastError()!=0x88)ExitProcess(233);
        if(replay){if(rect)((u8*)d[9])[0]=(u8)i;char path[]="original-00000000.bin";static const char hex[]="0123456789abcdef";
            for(u32 j=0;j<8;++j)path[9+j]=hex[(i>>(28-j*4))&15];pl_file(path,pl_big_pixels,8*1024*1024);}
        pl_expected_unlock=rect;SetLastError(0x77);if(((i32 (WIN *)(void*,void*))table[32])(&object,rect)!=19 || GetLastError()!=0x88)ExitProcess(234);
    }
    if(pl_big_locks!=count || pl_big_unlocks!=count || bs_stream[10])ExitProcess(235);ExitProcess(0);
}
static void test_partial_lock(void){
    char path[512];if(!GetEnvironmentVariableA("MNM_RENDER_STREAM",path,512))ExitProcess(220);
    HANDLE f=CreateFileA(path,0xc0000000,3,0,3,0x80,0);if(f==(HANDLE)-1)ExitProcess(221);
    HANDLE mapping=CreateFileMappingA(f,0,4,0,64+2048*2048*4,0);CloseHandle(f);if(!mapping)ExitProcess(222);
    bs_stream=MapViewOfFile(mapping,2,0,0,64+2048*2048*4);CloseHandle(mapping);if(!bs_stream)ExitProcess(223);
    if(pl_mode("memory-budget") || pl_mode("replay-budget") || pl_mode("session-bytes"))pl_big_test();
    static void* table[33];table[25]=(void*)&pl_lock;table[32]=(void*)&pl_unlock;table[27]=(void*)&pl_restore;table[22]=(void*)&pl_description;table[28]=(void*)&pl_clipper;table[5]=(void*)&pl_blt;table[31]=(void*)&pl_palette_assign;
    pl_surface.table=table;pl_surface.kind=pl_mode("legacy")?11:pl_mode("surface2")?12:pl_mode("surface7")?17:14;
    pl_surface.bits=(pl_mode("indexed") || pl_has_palette())?8:pl_mode("rgb24")?24:pl_mode("rgb32")?32:16;
    pl_surface.pitch=(i32)(4*(pl_surface.bits/8)+8);if(pl_mode("negative"))pl_surface.pitch=-pl_surface.pitch;
    RenderInstallForTest(&pl_surface,pl_surface.kind);if(pl_has_palette())pl_observe_palette();pl_record();
    if(!pl_mode("no-base"))pl_cycle(0,1,0xf800);
    i32 rect[4]={1,1,3,3};u32 flags=pl_mode("readonly")?0x11:pl_mode("discard")?0x2001:1;
    if(pl_mode("failed-lock")){pl_fail_lock=1;pl_cycle(rect,flags,0x7e0);}
    if(pl_mode("retry"))pl_fail_unlock=1;
    pl_cycle(rect,flags,0x7e0);
    if(pl_mode("chain")){rect[0]=rect[1]=0;rect[2]=rect[3]=2;pl_cycle(rect,1,0x1f);}
    if(pl_mode("budget"))for(u32 i=0;i<18;++i)pl_cycle(rect,1,0x1f+i);
    if(pl_mode("blit")){
        pl_target.table=table;pl_target.bits=16;pl_target.kind=14;RenderInstallForTest(&pl_target,14);
        u32 d[31]={124};SetLastError(0x77);if(((i32 (WIN *)(void*,void*))table[22])(&pl_target,d)!=23 || GetLastError()!=0x88)ExitProcess(227);
        SetLastError(0x77);if(((i32 (WIN *)(void*,void*))table[28])(&pl_target,0)!=23 || GetLastError()!=0x88)ExitProcess(228);
        SetLastError(0x77);if(((i32 (WIN *)(void*,void*,void*,void*,u32,void*))table[5])(&pl_target,0,&pl_surface,0,0x1000000,0)!=17 || GetLastError()!=0x88)ExitProcess(229);
        pl_record();
    }
    u32 expected_locks=pl_mode("no-base")?1:pl_mode("budget")?20:(pl_mode("chain")||pl_mode("failed-lock"))?3:2;
    if(pl_locks!=expected_locks || pl_unlocks!=expected_locks-(pl_mode("failed-lock")?1:0)+(pl_mode("retry")?1:0))ExitProcess(219);
    if(pl_has_palette() && (pl_caps!=1 || pl_reads!=1 || pl_assigns!=1 || pl_writes!=(u32)pl_mode("indexed-palette-change")))ExitProcess(245);
    ExitProcess(0);
}
