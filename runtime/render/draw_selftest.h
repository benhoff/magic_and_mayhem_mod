/* Fake surfaces exercise the real hook, native snapshots and original-call ABI. */
struct TestSurface {void** table;u16* pixels;u32 width,height; i32 pitch;u32 locked;};
static u32 draw_locks,draw_unlocks,draw_calls,draw_lock_attempts;
static i32 WIN draw_query(void* object,const void* guid,void** out){(void)guid;*out=object;return 0;}
static u32 WIN draw_release(void* object){(void)object;return 1;}
static i32 WIN draw_description(void* object,u32* d){
    struct TestSurface* s=object;d[2]=s->height;d[3]=s->width;d[4]=(u32)s->pitch;d[9]=(u32)s->pixels;
    d[19]=0x40;d[21]=16;d[22]=0xf800;d[23]=0x07e0;d[24]=0x001f;d[26]=0x40;
    SetLastError(0x99);return 0;
}
static i32 WIN draw_lock(void* object,void* rect,u32* d,u32 flags,HANDLE event){
    struct TestSurface* s=object;(void)event;
    if(rect || flags!=0x4810 || s->locked)ExitProcess(10);
    if(++draw_lock_attempts==3){SetLastError(0x99);return (i32)0x8876021c;} /* Post-draw surface busy. */
    ++s->locked;++draw_locks;return draw_description(object,d);
}
static i32 WIN draw_unlock(void* object,void* pointer){
    struct TestSurface* s=object;
    if(s->locked!=1 || pointer!=s->pixels)ExitProcess(11); /* Surface2 ABI */
    --s->locked;++draw_unlocks;SetLastError(0x99);return 0;
}
static i32 WIN draw_clipper(void* object,void** out){(void)object;*out=0;return (i32)0x88760238;}
static i32 WIN draw_key(void* object,u32 flags,u32* key){
    (void)object;if(flags!=8)ExitProcess(12);key[0]=key[1]=0;return 0;
}
static i32 draw_copy(struct TestSurface* dest,u32 x,u32 y,struct TestSurface* src,const i32* rect,int keyed){
    if(GetLastError()!=0x77 || src->locked || dest->locked)ExitProcess(13);
    ++draw_calls;
    for(i32 row=rect[1];row<rect[3];++row)for(i32 col=rect[0];col<rect[2];++col){
        u16 pixel=*(u16*)((u8*)src->pixels+row*src->pitch+col*2);
        if(!keyed || pixel)*(u16*)((u8*)dest->pixels+(y+row-rect[1])*dest->pitch+(x+col-rect[0])*2)=pixel;
    }
    SetLastError(0x88);return 17;
}
static i32 WIN draw_blt(void* object,i32* dest,void* src,i32* rect,u32 flags,void* effects){
    if(effects)ExitProcess(14);return draw_copy(object,dest[0],dest[1],src,rect,(flags&0x8000)!=0);
}
static i32 WIN draw_fast(void* object,u32 x,u32 y,void* src,i32* rect,u32 flags){
    return draw_copy(object,x,y,src,rect,(flags&1)!=0);
}
static void test_draw_capture(void){
    static void* table[33];
    table[0]=(void*)&draw_query;table[2]=(void*)&draw_release;table[5]=(void*)&draw_blt;table[7]=(void*)&draw_fast;
    table[15]=(void*)&draw_clipper;table[16]=(void*)&draw_key;table[22]=(void*)&draw_description;
    table[25]=(void*)&draw_lock;table[32]=(void*)&draw_unlock;
    u16 source[8]={0x07e0,0,0xffff,0xeeee,0xffff,0,0xf800,0xeeee};
    u16 destination[15];for(u32 i=0;i<15;++i)destination[i]=0x001f;
    struct TestSurface src={table,source+4,3,2,-8,0},dst={table,destination,4,3,10,0};
    RenderInstallForTest(&src,12);RenderInstallForTest(&dst,12);
    i32 rect[4]={0,0,3,2},dest[4]={1,1,4,3};char mode[8];
    GetEnvironmentVariableA("MNM_DRAW_SELFTEST_MODE",mode,sizeof(mode));
    typedef i32 (WIN *TestBlt)(void*,i32*,void*,i32*,u32,void*);
    typedef i32 (WIN *TestFast)(void*,u32,u32,void*,i32*,u32);
    /* An unsupported flag must be forwarded without attempting snapshots. */
    SetLastError(0x77);
    if(((TestBlt)table[5])(&dst,dest,&src,rect,0x200,0)!=17 || GetLastError()!=0x88 || draw_locks)ExitProcess(15);
    SetLastError(0x77);
    if(((TestBlt)table[5])(&dst,dest,&src,rect,0x1000000,0)!=17 || GetLastError()!=0x88 || draw_locks!=2 || draw_unlocks!=2)ExitProcess(20);
    for(u32 i=0;i<15;++i)destination[i]=0x001f;
    SetLastError(0x77);
    i32 result=mode[0]=='b'?((TestBlt)table[5])(&dst,dest,&src,rect,0x1000000,0):
                           ((TestFast)table[7])(&dst,1,1,&src,rect,0x11);
    if(result!=17 || GetLastError()!=0x88 || draw_locks!=5 || draw_unlocks!=5 || draw_calls!=3)ExitProcess(16);
    if(destination[7]!=(mode[0]=='b'?0:0x001f) || destination[6]!=0xffff || destination[8]!=0xf800)ExitProcess(17);
    /* Completed capture is bounded, and application lock calls still forward. */
    typedef i32 (WIN *TestLock)(void*,void*,u32*,u32,HANDLE);
    typedef i32 (WIN *TestUnlock)(void*,void*);
    u32 desc[31]={0};desc[0]=108;
    if(((TestLock)table[25])(&dst,0,desc,0x4810,0) || ((TestUnlock)table[32])(&dst,dst.pixels))ExitProcess(18);
    SetLastError(0x77);
    if(((TestBlt)table[5])(&dst,dest,&src,rect,0x1000000,0)!=17 || draw_locks!=6 || draw_unlocks!=6 || GetLastError()!=0x88)ExitProcess(19);
}
