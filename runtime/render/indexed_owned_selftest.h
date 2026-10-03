/* Original palette storage and pixels are independent of tracker snapshots. */
static void copy_bytes(void* to,const void* from,u32 count){u8* d=to;const u8* s=from;while(count--)*d++=*s++;}
struct IpPalette;
struct IpInterface {void** table;struct IpPalette* state;};
struct IpPalette {struct IpInterface base,alias;u32 refs;u8 colors[1024];};
static struct IpPalette ip_a,ip_b;static struct IpInterface* ip_attached;
static struct BsSurface* ip_front;static u32 ip_caps_calls,ip_reads,ip_writes,ip_assigns,ip_queries,ip_creates,ip_gets,ip_initializes;
static u32 ip_fail,ip_nested;
static i32 WIN ip_caps(void* object,u32* caps){(void)object;bs_entry();++ip_caps_calls;*caps=bs_mode("idx-caps")?0x46:0x54;return 23;}
static i32 WIN ip_read(void* object,u32 flags,u32 first,u32 count,u8* entries){
    bs_entry();++ip_reads;if(flags || first>=256 || count>256-first)ExitProcess(192);
    if(bs_mode("idx-read-failed"))return -1;
    copy_bytes(entries,((struct IpInterface*)object)->state->colors+first*4,count*4);return 23;
}
static i32 WIN ip_write(void* object,u32 flags,u32 first,u32 count,u8* entries){
    bs_entry();++ip_writes;if(first>=256 || count>256-first)ExitProcess(193);
    if(ip_fail){ip_fail=0;return -1;}
    if(bs_mode("idx-nested") && !ip_nested){ip_nested=1;u32 caps;SetLastError(0x77);
        if(((i32 (WIN *)(void*,u32*))((struct IpInterface*)object)->table[3])(object,&caps)!=23)ExitProcess(194);}
    copy_bytes(((struct IpInterface*)object)->state->colors+first*4,entries,count*4);
    /* The caller buffer is deliberately unusable for post-call capture. */
    for(u32 i=0;i<count*4;++i)entries[i]=0xcc;(void)flags;SetLastError(0x88);return 23;
}
static i32 WIN ip_query(void* object,const u8* guid,void** output){
    bs_entry();++ip_queries;if(guid[0]!=0x84)ExitProcess(195);struct IpPalette* p=((struct IpInterface*)object)->state;
    ++p->refs;*output=&p->alias;return 23;
}
static u32 WIN ip_release(void* object){bs_entry();struct IpPalette* p=((struct IpInterface*)object)->state;
    if(!p->refs)ExitProcess(196);return --p->refs;}
static i32 WIN ip_initialize(void* object,void* draw,u32 flags,void* entries){
    bs_entry();++ip_initializes;if(draw || flags!=0x44 || !entries)ExitProcess(197);
    copy_bytes(((struct IpInterface*)object)->state->colors,entries,1024);return 23;
}
static i32 WIN ip_assign(void* object,void* palette){
    bs_entry();++ip_assigns;struct BsSurface* surface=bs_state(object);
    if(bs_mode("idx-assign-failed"))return -1;
    if(palette)++((struct IpInterface*)palette)->state->refs;
    if(surface->palette)--((struct IpInterface*)surface->palette)->state->refs;
    surface->palette=palette;if(surface==ip_front)ip_attached=palette;return 23;
}
static i32 WIN ip_get(void* object,void** result){
    bs_entry();++ip_gets;if(object!=ip_front || !ip_attached)ExitProcess(199);
    ++ip_attached->state->refs;*result=ip_attached;return 23;
}
static i32 WIN ip_create(void* object,u32 flags,u8* colors,void** result,void* outer){
    (void)object;bs_entry();++ip_creates;if(flags!=0x44 || outer)ExitProcess(200);
    copy_bytes(ip_a.colors,colors,1024);for(u32 i=0;i<1024;++i)colors[i]=0xcc;*result=&ip_a.base;return 23;
}
static void ip_record(void){
    ++bs_draws;bs_record(ip_front);char path[]="colors-00000000.bin";static const char hex[]="0123456789abcdef";
    for(u32 i=0;i<8;++i)path[7+i]=hex[(bs_draws>>(28-i*4))&15];
    HANDLE file=CreateFileA(path,0x40000000,1,0,1,0x80,0);u32 written;
    static u8 blank[1024];u8* colors=ip_attached?ip_attached->state->colors:blank;
    if(file==(HANDLE)-1 || !WriteFile(file,colors,1024,&written,0) || written!=1024)ExitProcess(201);CloseHandle(file);
}
static void ip_set(struct IpInterface* palette){
    SetLastError(0x77);i32 status=((i32 (WIN *)(void*,void*))ip_front->table[31])(ip_front,palette);
    if(status!=(bs_mode("idx-assign-failed")?-1:23) || GetLastError()!=0x88)ExitProcess(202);ip_record();
}
static void ip_get_colors(struct IpInterface* palette,u32 first,u32 count){
    u8 colors[1024];SetLastError(0x77);i32 status=((i32 (WIN *)(void*,u32,u32,u32,void*))palette->table[4])(palette,0,first,count,colors);
    if(status!=(bs_mode("idx-read-failed")?-1:23) || GetLastError()!=0x88)ExitProcess(203);ip_record();
}
static void ip_change(struct IpInterface* palette){
    u8 colors[8]={240,20,30,0xa5,40,220,50,0x5a};u32 failed=ip_fail;SetLastError(0x77);
    if(((i32 (WIN *)(void*,u32,u32,u32,void*))palette->table[6])(palette,bs_mode("idx-flags")?1:0,1,2,colors)!=(failed?-1:23) || GetLastError()!=0x88)ExitProcess(204);ip_record();
}
static void ip_observe_caps(struct IpInterface* palette){u32 caps;SetLastError(0x77);
    if(((i32 (WIN *)(void*,u32*))palette->table[3])(palette,&caps)!=23 || GetLastError()!=0x88)ExitProcess(205);}
static void ip_test(struct BsSurface* front){
    static void *table[7],*alias[7];table[0]=(void*)&ip_query;table[2]=(void*)&ip_release;table[3]=(void*)&ip_caps;
    table[4]=(void*)&ip_read;table[5]=(void*)&ip_initialize;table[6]=(void*)&ip_write;for(u32 i=0;i<7;++i)alias[i]=table[i];
    ip_a.base.table=ip_b.base.table=table;ip_a.alias.table=ip_b.alias.table=alias;
    ip_a.base.state=ip_a.alias.state=&ip_a;ip_b.base.state=ip_b.alias.state=&ip_b;ip_a.refs=ip_b.refs=1;
    for(u32 i=0;i<256;++i)for(u32 j=0;j<4;++j){ip_a.colors[i*4+j]=(u8)(i*(j*6+1)+j*13);ip_b.colors[i*4+j]=(u8)(255-i*(j*4+3));}
    ip_front=front;
    /* The vtable was already hooked: use a fresh table containing original slots. */
    static void* surface[33];surface[25]=(void*)&bs_lock;surface[32]=(void*)&bs_unlock;
    surface[22]=(void*)&bs_description;surface[31]=(void*)&ip_assign;surface[20]=(void*)&ip_get;front->table=surface;RenderInstallForTest(front,front->kind);
    if(bs_mode("idx-offscreen"))front->primary=0;
    if(!bs_mode("idx-before-lock"))bs_seed(front);ip_record();
    if(bs_mode("idx-create")){
        static void* draw_table[7];draw_table[5]=(void*)&ip_create;void** draw=draw_table;RenderInstallForTest(&draw,4);
        u8 colors[1024];copy_bytes(colors,ip_a.colors,1024);void* result=0;SetLastError(0x77);
        if(((i32 (WIN *)(void*,u32,void*,void**,void*))draw_table[5])(&draw,0x44,colors,&result,0)!=23 || result!=&ip_a.base || GetLastError()!=0x88)ExitProcess(206);
        ip_set(&ip_a.base);
    }else{
        if(bs_mode("idx-get-palette")){ip_attached=&ip_a.base;front->palette=ip_attached;++ip_a.refs;void* result=0;SetLastError(0x77);
            if(((i32 (WIN *)(void*,void**))surface[20])(front,&result)!=23 || result!=ip_attached || GetLastError()!=0x88)ExitProcess(207);ip_record();
        }else ip_set(&ip_a.base);
        RenderInstallForTest(&ip_a.base,20);
        if(!bs_mode("idx-caps-missing"))ip_observe_caps(&ip_a.base);
        if(bs_mode("idx-partial")){ip_get_colors(&ip_a.base,0,128);ip_get_colors(&ip_a.base,128,128);}
        else ip_get_colors(&ip_a.base,0,256);
    }
    if(bs_mode("idx-before-lock")){bs_seed(front);ip_record();}
    struct IpInterface* change=&ip_a.base;
    if(bs_mode("idx-alias")){static const u8 iid[16]={0x84,0xdb,0x14,0x6c,0x33,0xa7,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60};void* result=0;SetLastError(0x77);
        if(((i32 (WIN *)(void*,const u8*,void**))table[0])(&ip_a.base,iid,&result)!=23 || result!=&ip_a.alias || GetLastError()!=0x88)ExitProcess(208);change=result;}
    if(bs_mode("idx-retry")){ip_fail=1;ip_change(change);}
    if(bs_mode("idx-detach"))ip_set(0);else ip_change(change);
    if(bs_mode("idx-reattach")){RenderInstallForTest(&ip_b.base,20);ip_observe_caps(&ip_b.base);ip_get_colors(&ip_b.base,0,256);ip_set(&ip_b.base);ip_change(&ip_a.base);ip_change(&ip_b.base);}
    if(bs_mode("idx-initialize")){u8 colors[1024];copy_bytes(colors,ip_b.colors,1024);SetLastError(0x77);
        if(((i32 (WIN *)(void*,void*,u32,void*))table[5])(&ip_a.base,0,0x44,colors)!=23 || GetLastError()!=0x88)ExitProcess(209);ip_record();ip_change(change);}
    if(bs_mode("idx-release")){
        ip_set(0);SetLastError(0x77);if(((u32 (WIN *)(void*))table[2])(&ip_a.base)!=0 || GetLastError()!=0x88)ExitProcess(214);ip_record();
        ip_set(&ip_b.base);ip_observe_caps(&ip_b.base);ip_get_colors(&ip_b.base,0,256);bs_seed(front);ip_record();
    }
    if(bs_mode("idx-descriptor")){u32 d[31]={124};SetLastError(0x77);
        if(((i32 (WIN *)(void*,void*))surface[22])(front,d)!=23 || GetLastError()!=0x88)ExitProcess(213);ip_record();ip_change(change);}
    if(bs_mode("idx-budget"))for(u32 i=0;i<16;++i)ip_change(change);
    if(bs_locks!=1u+(u32)bs_mode("idx-release") || bs_unlocks!=bs_locks || bs_descriptions!=(u32)bs_mode("idx-descriptor") || bs_queries || bs_creates || ip_queries!=(u32)bs_mode("idx-alias") || ip_creates!=(u32)bs_mode("idx-create") || ip_gets!=(u32)bs_mode("idx-get-palette"))ExitProcess(210);
    if(ip_caps_calls!=(bs_mode("idx-create")||bs_mode("idx-caps-missing")?0u:1u)+(u32)bs_mode("idx-nested")+(u32)bs_mode("idx-reattach")+(u32)bs_mode("idx-release") ||
       ip_reads!=(bs_mode("idx-create")?0u:bs_mode("idx-partial")?2u:1u)+(u32)bs_mode("idx-reattach")+(u32)bs_mode("idx-release") ||
       ip_writes!=(bs_mode("idx-detach")?0u:1u)+(u32)bs_mode("idx-retry")+2u*(u32)bs_mode("idx-reattach")+(u32)bs_mode("idx-initialize")+(u32)bs_mode("idx-descriptor")+16u*(u32)bs_mode("idx-budget"))ExitProcess(211);
    if(ip_assigns!=(bs_mode("idx-get-palette")?0u:1u)+(u32)bs_mode("idx-detach")+(u32)bs_mode("idx-reattach")+2u*(u32)bs_mode("idx-release") || ip_initializes!=(u32)bs_mode("idx-initialize"))ExitProcess(215);
    if(ip_a.refs!=(bs_mode("idx-release")?0u:1u)+(ip_attached && ip_attached->state==&ip_a)+(u32)bs_mode("idx-alias")+(u32)bs_mode("idx-get-palette") || ip_b.refs!=1u+(ip_attached && ip_attached->state==&ip_b))ExitProcess(212);
    ExitProcess(0);
}
