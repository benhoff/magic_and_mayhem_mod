/* Independent fake engine with padded borrowed rows poisoned at Unlock.
 * Real PE32 hooks produce commands; expected display pixels are never inputs. */
struct CsSurface {void** table;u32 width,height,bits,primary,held;u8 *native,*exposed;};
static struct CsSurface cs_small,cs_big,cs_extra[31];
static u32 cs_locks,cs_unlocks,cs_color,cs_delay;
static void (*cs_wait_release)(void);
static i32 WIN cs_lock(void* object,void* rect,u32* d,u32 flags,HANDLE event){
    struct CsSurface* s=object;
    if(s->held || rect || flags!=1 || event || GetLastError()!=0x77)ExitProcess(290);
    if(cs_delay){u32 delay=cs_delay;cs_delay=0;pl_file("callback-entered.bin",&delay,4);
        if(cs_wait_release){cs_wait_release();cs_wait_release=0;}else Sleep(delay);}
    ++cs_locks;s->held=1;u32 row=s->width*(s->bits/8),pitch=row+8;
    for(u32 y=0;y<s->height;++y)copy_bytes(s->exposed+y*pitch,s->native+y*row,row);
    d[1]=1;d[2]=s->height;d[3]=s->width;d[4]=pitch;d[9]=(u32)s->exposed;
    d[19]=0x40;d[21]=s->bits;d[22]=s->bits==16?0xf800:0xff0000;
    d[23]=s->bits==16?0x7e0:0xff00;d[24]=0x1f*(s->bits==16)+0xff*(s->bits==32);
    d[26]=s->primary?0x200:0x40;SetLastError(0x88);return 13;
}
static i32 WIN cs_unlock(void* object,void* argument){
    struct CsSurface* s=object;if(!s->held || argument || GetLastError()!=0x77)ExitProcess(291);
    ++cs_unlocks;u32 row=s->width*(s->bits/8),pitch=row+8;
    for(u32 y=0;y<s->height;++y)copy_bytes(s->native+y*row,s->exposed+y*pitch,row);
    for(u32 i=0;i<pitch*s->height;++i)s->exposed[i]=0xcc;
    s->held=0;SetLastError(0x88);return 19;
}
static void cs_setup(struct CsSurface* s,u32 width,u32 height,u32 bits,u32 primary){
    static void* table[33];if(!table[25]){table[25]=(void*)&cs_lock;table[32]=(void*)&cs_unlock;}
    s->table=table;s->width=width;s->height=height;s->bits=bits;s->primary=primary;
    s->native=HeapAlloc(GetProcessHeap(),8,width*height*(bits/8));
    s->exposed=HeapAlloc(GetProcessHeap(),8,(width*(bits/8)+8)*height);
    if(!s->native || !s->exposed)ExitProcess(292);RenderInstallForTest(s,14);
}
static void cs_update(struct CsSurface* s,u32 color){
    u32 desc[31]={124};SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*,void*,u32,HANDLE))s->table[25])(s,0,desc,1,0)!=13 || GetLastError()!=0x88)ExitProcess(293);
    for(u32 y=0;y<s->height;++y)for(u32 x=0;x<s->width;++x)for(u32 b=0;b<s->bits/8;++b)
        s->exposed[y*desc[4]+x*(s->bits/8)+b]=(u8)(color>>(b*8));
    SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*))s->table[32])(s,0)!=19 || GetLastError()!=0x88)ExitProcess(294);
    /* Original storage is separate from the poisoned exposed rows. */
    for(u32 i=0;i<s->width*s->height*(s->bits/8);++i)
        if(s->native[i]!=(u8)(color>>((i%(s->bits/8))*8)))ExitProcess(295);
}
static void cs_delta_update(struct CsSurface* s,u32 frame){
    if(!frame){cs_update(s,0xa5000000u);return;}
    u32 desc[31]={124};SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*,void*,u32,HANDLE))s->table[25])(s,0,desc,1,0)!=13 || GetLastError()!=0x88)ExitProcess(299);
    if(frame%3){
        u32 x=frame*17%512,y=frame*29%512;
        if(frame%3==1){u32 color=0xa5000000u|(frame*0x254713u&0xffffff);for(u32 b=0;b<4;++b)s->exposed[y*desc[4]+x*4+b]=(u8)(color>>(b*8));}
        else s->exposed[y*desc[4]+x*4+3]^=0x80;
    }
    SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*))s->table[32])(s,0)!=19 || GetLastError()!=0x88)ExitProcess(299);
}
static void test_continuous(const char* mode){
    cs_setup(&cs_small,4,4,16,0);cs_setup(&cs_big,512,512,32,1);
    u32 refused=mode[0]=='v',missing=mode[0]=='m',limited=mode[0]=='l';
    u32 byte_archive=mode[0]=='b',short_archive=mode[0]=='s';
    u32 delta=mode[0]=='d';
    u32 setup=refused || delta?2:byte_archive?80:short_archive?80:5000;
    for(u32 i=0;i<setup;++i)cs_update(&cs_small,0x100+i);
    u32 frames=refused || missing?0:short_archive?2:70;
    for(u32 i=0;i<frames;++i){
        cs_color=0xa5000000u|((i*37u)&255u)<<16|((i*71u)&255u)<<8|((i*19u)&255u);
        if(delta)cs_delta_update(&cs_big,i);else cs_update(&cs_big,cs_color);
        Sleep(75); /* Fixture pacing, outside hooks. */
    }
    if(byte_archive)for(u32 i=0;i<5000;++i)cs_update(&cs_small,0x200+i);
    if(limited)for(u32 i=0;i<31;++i){cs_setup(cs_extra+i,4,4,16,0);cs_update(cs_extra+i,0x300+i);}
    if(cs_locks!=setup+frames+(byte_archive?5000u:0u)+(limited?31u:0u) || cs_unlocks!=cs_locks)ExitProcess(296);
    pl_file("engine-final.bin",cs_big.native,512*512*4);
    u32 result[3]={cs_locks,cs_unlocks,frames};pl_file("engine-counts.bin",result,12);
    SetLastError(0x77);u32 complete=RenderShutdown(3000);
    if(complete!=(refused || missing || limited?0u:1u) || GetLastError()!=0x77)ExitProcess(297);
    if(RenderShutdown(0)!=complete || GetLastError()!=0x77)ExitProcess(298);
    ExitProcess(0);
}
