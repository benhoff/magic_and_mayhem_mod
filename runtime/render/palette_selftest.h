/* Indexed Surface4 fixtures and two palette interfaces sharing COM identity. */
struct PPalette;
struct PInterface {void** table;struct PPalette* palette;};
struct PPalette {struct PInterface base,alias;u32 refs;u8 entries[1024];};
struct PSurface {void** table;u32 refs,locked;u8 pixels[8];struct PInterface* palette;};
static u32 p_last;
static struct PPalette pa,pb;static struct PSurface ps,pd;static char p_mode[32];
static i32 WIN p_query(void* object,const u8* guid,void** out){
    struct PInterface* p=object;++p->palette->refs;*out=guid[0]?&p->palette->alias:&p->palette->base;SetLastError(0x88);return 0;
}
static u32 WIN p_release(void* object){struct PPalette* p=((struct PInterface*)object)->palette;if(!p->refs)ExitProcess(71);--p->refs;SetLastError(0x88);return p->refs;}
static i32 WIN p_caps(void* object,u32* caps){(void)object;*caps=p_mode[0]=='c'?0x402:0x44;return 0;}
static i32 WIN p_entries(void* object,u32 flags,u32 first,u32 count,u8* out){
    struct PPalette* p=((struct PInterface*)object)->palette;
    if(flags || first>=256 || count>256-first)ExitProcess(72);
    if(p_mode[0]=='e')return -1;
    for(u32 i=0;i<count*4;++i)out[i]=p->entries[first*4+i];return 0;
}
static i32 WIN p_initialize(void* object,void* draw,u32 flags,void* entries){
    (void)object;if(draw!=(void*)0x5678 || flags!=0x44 || entries!=(void*)0x1234 || GetLastError()!=0x77)ExitProcess(90);
    SetLastError(0x88);return 17;
}
static i32 WIN p_set_entries(void* object,u32 flags,u32 first,u32 count,const u8* entries){
    struct PPalette* p=((struct PInterface*)object)->palette;
    if(GetLastError()!=0x77 || !entries)ExitProcess(73);
    if(p_mode[0]=='f'){SetLastError(0x88);return -1;}
    if(first>=256 || count>256-first)ExitProcess(74);
    for(u32 i=0;i<count*4;++i)p->entries[first*4+i]=entries[i];
    (void)flags;SetLastError(0x88);return 17;
}
static i32 WIN p_surface_query(void* object,const u8* guid,void** out){(void)guid;struct PSurface* s=object;++s->refs;*out=object;return 0;}
static u32 WIN p_surface_release(void* object){
    struct PSurface* s=object;if(!s->refs)ExitProcess(75);
    if(!--s->refs && s->palette){p_release(s->palette);s->palette=0;}
    SetLastError(0x88);return s->refs;
}
static i32 WIN p_description(void* object,u32* d){
    struct PSurface* s=object;if(!s->refs || d[0]!=124)ExitProcess(76);
    d[2]=2;d[3]=4;d[4]=4;d[9]=(u32)s->pixels;d[19]=0x60;d[21]=8;d[26]=0x40;return 0;
}
static i32 WIN p_lock(void* object,void* rect,u32* d,u32 flags,HANDLE event){
    struct PSurface* s=object;(void)flags;(void)event;if(rect || s->locked)ExitProcess(77);
    s->locked=1;return p_description(object,d);
}
static i32 WIN p_unlock(void* object,void* arg){struct PSurface* s=object;if(arg || !s->locked)ExitProcess(78);s->locked=0;SetLastError(0x88);return 0;}
static i32 WIN p_get_palette(void* object,void** out){
    struct PSurface* s=object;if(!s->palette){*out=0;return -1;}
    ++s->palette->palette->refs;*out=s->palette;return 0;
}
static i32 WIN p_set_palette(void* object,struct PInterface* palette){
    struct PSurface* s=object;if(GetLastError()!=0x77)ExitProcess(79);
    if(p_mode[0]=='a'){SetLastError(0x88);return -1;}
    if(palette)++palette->palette->refs;if(s->palette)p_release(s->palette);s->palette=palette;SetLastError(0x88);return 17;
}
static i32 WIN p_blt(void* object,void* dst,void* source,void* rect,u32 flags,void* fx){
    struct PSurface *d=object,*s=source;if(dst || rect || fx || s->locked || d->locked || GetLastError()!=0x77)ExitProcess(80);
    for(u32 i=0;i<8;++i)if(!(flags&0x8000) || s->pixels[i])d->pixels[i]=s->pixels[i];SetLastError(0x88);return 17;
}
static void p_draw(void){
    SetLastError(0x77);if(((i32 (WIN *)(void*,void*,void*,void*,u32,void*))pd.table[5])(&pd,0,&ps,0,0x1008000,0)!=17 || GetLastError()!=0x88)ExitProcess(81);
}
static void p_assign(struct PSurface* s,struct PInterface* p){
    SetLastError(0x77);i32 result=((i32 (WIN *)(void*,void*))s->table[31])(s,p);
    if(result!=(p_mode[0]=='a'?-1:17) || GetLastError()!=0x88)ExitProcess(82);
}
static void p_change(struct PInterface* p,u32 flags){
    static const u8 colors[8]={220,10,30,0xaa,40,210,60,0xbb},last[8]={30,40,240,0xaa,250,180,20,0xbb};SetLastError(0x77);
    i32 result=((i32 (WIN *)(void*,u32,u32,u32,const void*))p->table[6])(p,flags,1,2,p_last?last:colors);
    if(result!=(p_mode[0]=='f'?-1:17) || GetLastError()!=0x88)ExitProcess(83);
}
static void test_palette_history(void){
    static void *pal[7],*alias[7],*surface[33];
    pal[0]=(void*)&p_query;pal[2]=(void*)&p_release;pal[3]=(void*)&p_caps;pal[4]=(void*)&p_entries;pal[5]=(void*)&p_initialize;pal[6]=(void*)&p_set_entries;
    for(u32 i=0;i<7;++i)alias[i]=pal[i];
    surface[0]=(void*)&p_surface_query;surface[2]=(void*)&p_surface_release;surface[5]=(void*)&p_blt;
    surface[15]=(void*)&draw_clipper;surface[16]=(void*)&draw_key;surface[20]=(void*)&p_get_palette;
    surface[22]=(void*)&p_description;surface[25]=(void*)&p_lock;surface[31]=(void*)&p_set_palette;surface[32]=(void*)&p_unlock;
    pa.base.table=pal;pa.base.palette=&pa;pa.alias.table=alias;pa.alias.palette=&pa;
    pb.base.table=pal;pb.base.palette=&pb;pb.alias.table=alias;pb.alias.palette=&pb;
    pa.refs=3;pb.refs=1;ps.table=pd.table=surface;ps.refs=pd.refs=1;ps.palette=pd.palette=&pa.base;
    for(u32 i=0;i<256;++i){pa.entries[i*4]=(u8)i;pa.entries[i*4+1]=(u8)(255-i);pa.entries[i*4+2]=(u8)(i*3);pa.entries[i*4+3]=(u8)i;
        pb.entries[i*4]=(u8)(255-i);pb.entries[i*4+1]=(u8)i;pb.entries[i*4+2]=77;pb.entries[i*4+3]=0xff;}
    for(u32 i=0;i<3;++i)pa.entries[4+i]=pa.entries[i]; /* Two colors equal; only index 0 is keyed. */
    for(u32 i=0;i<8;++i){ps.pixels[i]=(u8)(i%4);pd.pixels[i]=3;}
    RenderInstallForTest(&ps,14);RenderInstallForTest(&pd,14);
    GetEnvironmentVariableA("MNM_PALETTE_SELFTEST",p_mode,sizeof(p_mode));
    /* Unsupported caps must be rejected after seeding snapshots, not before. */
    char initial=p_mode[0];if(initial=='e')p_mode[0]=0;
    p_draw();p_mode[0]=initial;if(pa.refs!=3 || pb.refs!=1)ExitProcess(84);
    if(p_mode[0]=='c')goto cleanup;
    if(p_mode[0]=='e'){p_change(&pa.base,0);goto cleanup;}
    static const u8 iid[16]={0x84,0xdb,0x14,0x6c,0x33,0xa7,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60};
    void* out=0;SetLastError(0x77);
    if(((i32 (WIN *)(void*,const u8*,void**))pal[0])(&pa.base,iid,&out) || out!=&pa.alias || GetLastError()!=0x88)ExitProcess(85);
    if(p_mode[0]=='i'){
        SetLastError(0x77);if(((i32 (WIN *)(void*,void*,u32,void*))alias[5])(out,(void*)0x5678,0x44,(void*)0x1234)!=17 || GetLastError()!=0x88)ExitProcess(91);
        goto cleanup_alias;
    }
    if(p_mode[0]=='d'){p_assign(&pd,0);goto cleanup_alias;}
    if(p_mode[0]=='u'){pa.entries[4]^=0x80;p_draw();goto cleanup_alias;}
    p_change(out,p_mode[0]=='b'?1:0);p_draw();
    p_assign(&pd,&pb.base);p_change(out,0);p_draw();
    p_assign(&ps,&pb.base);p_change(&pb.base,0);p_draw();
    if(p_mode[0]=='r'){p_assign(&ps,&pa.base);p_change(out,0);p_draw();}
    /* Indexed CPU write preserves raw key/index identity and records no RGB upload. */
    u32 desc[31]={0};desc[0]=124;SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*,u32*,u32,HANDLE))surface[25])(&ps,0,desc,0x4801,0))ExitProcess(86);
    ps.pixels[0]=2;SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*))surface[32])(&ps,0) || GetLastError()!=0x88)ExitProcess(87);p_draw();
    if(p_mode[0]=='p'){p_last=1;p_change(pd.palette,0);}
cleanup_alias:
    ((u32 (WIN *)(void*))alias[2])(&pa.alias);
cleanup:
    ((u32 (WIN *)(void*))surface[2])(&ps);((u32 (WIN *)(void*))surface[2])(&pd);
    if(pa.refs!=1 || pb.refs!=1 || ps.locked || pd.locked)ExitProcess(88);
    ((u32 (WIN *)(void*))pal[2])(&pa.base);((u32 (WIN *)(void*))pal[2])(&pb.base);
    if(pa.refs || pb.refs)ExitProcess(89);
}
