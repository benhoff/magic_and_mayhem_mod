/* Synthetic Surface2/Surface4 aliases with shared COM identity/refcount. */
struct HState;
struct HSurface {void** table;struct HState* state;u32 modern;};
struct HState {struct HSurface base,alias;u16* pixels;u32 width,height,refs,locked; i32 pitch;};
static struct HState hs,hd;static u32 h_updates,h_queries,h_failed_unlock;static char h_mode[32];
static i32 WIN h_query(void* object,const u8* guid,void** out){
    struct HState* s=((struct HSurface*)object)->state;
    if(!s->refs)ExitProcess(41);
    ++s->refs;++h_queries;*out=guid[0]? (void*)&s->alias:(void*)&s->base;SetLastError(0x88);return 0;
}
static u32 WIN h_release(void* object){
    struct HState* s=((struct HSurface*)object)->state;
    if(!s->refs)ExitProcess(42);--s->refs;SetLastError(0x88);return s->refs;
}
static i32 WIN h_desc(void* object,u32* d){
    struct HSurface* p=object;struct HState* s=p->state;
    if(d[0]!=(p->modern?124u:108u) || !s->refs)ExitProcess(43);
    d[2]=s->height;d[3]=s->width;d[4]=(u32)s->pitch;d[9]=(u32)s->pixels;
    d[19]=0x40;d[21]=16;d[22]=0xf800;d[23]=0x7e0;d[24]=0x1f;d[26]=0x40;return 0;
}
static i32 WIN h_lock(void* object,void* rect,u32* d,u32 flags,HANDLE event){
    struct HState* s=((struct HSurface*)object)->state;(void)rect;(void)event;
    if(s->locked || !s->refs)ExitProcess(44);
    if(flags==0x4801 && h_mode[0]=='l'){SetLastError(0x88);return (i32)0x8876021c;}
    if(flags==0x4801 && GetLastError()!=0x77)ExitProcess(45);
    if(flags==0x4801 && h_mode[0]=='r'){
        void* alias=0;static const u8 unknown[16]={0};
        if(((i32 (WIN *)(void*,const u8*,void**))s->base.table[0])(&s->base,unknown,&alias))ExitProcess(46);
        ((u32 (WIN *)(void*))(*(void***)alias)[2])(alias);
    }
    s->locked=1;i32 status=h_desc(object,d);SetLastError(0x88);return status;
}
static i32 WIN h_unlock(void* object,void* arg){
    struct HSurface* p=object;struct HState* s=p->state;
    if(!s->locked || (p->modern?arg!=0:arg!=s->pixels))ExitProcess(47);
    if(h_mode[0]=='u' && GetLastError()==0x77 && !h_failed_unlock++){
        SetLastError(0x88);return (i32)0x8876021c;
    }
    s->locked=0;SetLastError(0x88);return 0;
}
static i32 WIN h_blt(void* object,i32* dest,void* source,i32* rect,u32 flags,void* effects){
    struct HState *d=((struct HSurface*)object)->state,*s=((struct HSurface*)source)->state;
    if(effects || d->locked || s->locked || !d->refs || !s->refs || GetLastError()!=0x77)ExitProcess(48);
    for(i32 y=rect[1];y<rect[3];++y)for(i32 x=rect[0];x<rect[2];++x){
        u16 v=*(u16*)((u8*)s->pixels+y*s->pitch+x*2);
        if(!(flags&0x8000) || v)*(u16*)((u8*)d->pixels+(dest[1]+y-rect[1])*d->pitch+(dest[0]+x-rect[0])*2)=v;
    }
    SetLastError(0x88);return 17;
}
static i32 WIN h_flip(void* object,void* target,u32 flags){(void)object;(void)target;(void)flags;SetLastError(0x88);return 23;}
static i32 WIN h_restore(void* object){(void)object;if(GetLastError()!=0x77)ExitProcess(60);SetLastError(0x88);return 19;}
static i32 WIN h_property(void* object,void* value){(void)object;if(value!=(void*)0x1234 || GetLastError()!=0x77)ExitProcess(61);SetLastError(0x88);return 19;}
static i32 WIN h_dc(void* object,void** out){(void)object;if(GetLastError()!=0x77)ExitProcess(62);*out=(void*)0x1234;SetLastError(0x88);return 19;}
static void h_install(void** table){
    table[0]=(void*)&h_query;table[2]=(void*)&h_release;table[5]=(void*)&h_blt;table[11]=(void*)&h_flip;
    table[17]=(void*)&h_dc;table[27]=(void*)&h_restore;table[31]=(void*)&h_property;
    table[15]=(void*)&draw_clipper;table[16]=(void*)&draw_key;table[22]=(void*)&h_desc;
    table[25]=(void*)&h_lock;table[32]=(void*)&h_unlock;
}
static void h_copy(struct HSurface* source){
    i32 rect[4]={0,0,3,2},dest[4]={1,1,4,3};SetLastError(0x77);
    if(((i32 (WIN *)(void*,i32*,void*,i32*,u32,void*))hd.base.table[5])(&hd.base,dest,source,rect,0x1000000,0)!=17 || GetLastError()!=0x88)ExitProcess(49);
}
static void h_put(struct HSurface* object,u16 pixel,void* rect){
    u32 d[31]={0};d[0]=object->modern?124:108;SetLastError(0x77);
    i32 status=((i32 (WIN *)(void*,void*,u32*,u32,HANDLE))object->table[25])(object,rect,d,0x4801,0);
    if(GetLastError()!=0x88)ExitProcess(50);if(status<0)return;
    *(u16*)d[9]=pixel;++h_updates;SetLastError(0x77);
    status=((i32 (WIN *)(void*,void*))object->table[32])(object,object->modern?0:(void*)d[9]);
    if(GetLastError()!=0x88)ExitProcess(51);
    if(status<0){SetLastError(0x77);status=((i32 (WIN *)(void*,void*))object->table[32])(object,object->modern?0:(void*)d[9]);}
    if(status || GetLastError()!=0x88)ExitProcess(52);
}
static void h_drop(struct HSurface* object,u32 count){
    SetLastError(0x77);if(((u32 (WIN *)(void*))object->table[2])(object)!=count || GetLastError()!=0x88)ExitProcess(53);
}
static void test_surface_history(void){
    static void *old[33],*modern[33];h_install(old);h_install(modern);
    static u16 src[8]={0x7e0,0,0xffff,0xeeee,0xffff,0,0xf800,0xeeee},dst[15];
    for(u32 i=0;i<15;++i)dst[i]=0x1f;
    hs.base.table=old;hs.base.state=&hs;hs.alias.table=modern;hs.alias.state=&hs;hs.alias.modern=1;
    hs.pixels=src+4;hs.width=3;hs.height=2;hs.pitch=-8;hs.refs=1;
    hd.base.table=modern;hd.base.state=&hd;hd.base.modern=1;hd.alias=hd.base;
    hd.pixels=dst;hd.width=4;hd.height=3;hd.pitch=10;hd.refs=1;
    RenderInstallForTest(&hs.base,12);RenderInstallForTest(&hd.base,14);
    GetEnvironmentVariableA("MNM_HISTORY_SELFTEST",h_mode,sizeof(h_mode));h_copy(&hs.base);
    if(hs.refs!=1 || hd.refs!=1)ExitProcess(54); /* Observer QI/Release is balanced. */
    static const u8 surface4[16]={0x30,0x86,0x2b,0x0b,0x35,0xad,0xd0,0x11,0x8e,0xa6,0,0x60,0x97,0x97,0xea,0x5b};
    void* alias=0;SetLastError(0x77);
    if(((i32 (WIN *)(void*,const u8*,void**))old[0])(&hs.base,surface4,&alias) || alias!=&hs.alias || GetLastError()!=0x88)ExitProcess(55);
    if(h_mode[0]=='s'){
        SetLastError(0x77);if(((i32 (WIN *)(void*))modern[27])(&hd.base)!=19 || GetLastError()!=0x88)ExitProcess(63);
    }else if(h_mode[0]=='c'){
        SetLastError(0x77);if(((i32 (WIN *)(void*,void*))modern[31])(&hd.base,(void*)0x1234)!=19 || GetLastError()!=0x88)ExitProcess(64);
    }else if(h_mode[0]=='d' && h_mode[1]=='c'){
        void* out=0;SetLastError(0x77);if(((i32 (WIN *)(void*,void**))modern[17])(&hd.base,&out)!=19 || out!=(void*)0x1234 || GetLastError()!=0x88)ExitProcess(65);
    }else if(h_mode[0]=='t'){
        u32 desc[31]={0};desc[0]=108;SetLastError(0x77);
        if(((i32 (WIN *)(void*,void*,u32*,u32,HANDLE))old[25])(&hs.base,0,desc,0x4801,0) || GetLastError()!=0x88)ExitProcess(66);
        return; /* Process detach must reject an outstanding application lock. */
    }else if(h_mode[0]=='f'){
        SetLastError(0x77);if(((i32 (WIN *)(void*,void*,u32))modern[11])(&hd.base,0,0)!=23 || GetLastError()!=0x88)ExitProcess(56);
    }else{
        i32 rect[4]={0,0,1,1};h_put(alias,0xffe0,h_mode[0]=='p'?rect:0);h_copy(alias);
    }
    u32 d[31]={0};d[0]=108;SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*,u32*,u32,HANDLE))old[25])(&hs.base,0,d,0x4810,0) || GetLastError()!=0x88)ExitProcess(58);
    SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*))old[32])(&hs.base,(void*)d[9]) || GetLastError()!=0x88)ExitProcess(59);
    h_drop(alias,1);h_drop(&hs.base,0);
    if(h_mode[0]=='g'){ /* Lose ordering by changing native memory outside any recorded lock. */
        hd.pixels[0]=0xf81f;
    }
    /* Reuse the released object's address; the capture must allocate a fresh ID. */
    hs.refs=1;hs.pixels[0]=0xffff;h_copy(&hs.base);
    h_put(&hs.base,0xf81f,0);h_copy(&hs.base);
    if(h_mode[0]=='b')for(u32 i=0;i<20;++i)h_copy(&hs.base);
    if(h_mode[0]=='d' && h_mode[1]=='e'){
        if(hs.refs!=1 || hd.refs!=1 || hs.locked || hd.locked)ExitProcess(67);
        return; /* Detach footer cleans replay resources, without extra COM calls. */
    }
    h_drop(&hs.base,0);h_drop(&hd.base,0);
    if(hs.refs || hd.refs || hs.locked || hd.locked || !h_queries)ExitProcess(57);
}
