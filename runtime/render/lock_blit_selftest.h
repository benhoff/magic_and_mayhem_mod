/* Independent fake engine: padded lock storage is poisoned at Unlock; original
 * blits read separate native backing storage and dump their own expected output. */
static char lock_blit_mode[32];
static int lb_mode(const char* name){u32 i=0;while(name[i] && name[i]==lock_blit_mode[i])++i;return !name[i] && !lock_blit_mode[i];}
struct LbSurface {void** table;struct LbSurface* state;u32 width,height,bits,held,clip,key_active,key,kind;u8 pixels[64],locked[80];};
struct LbRect {i32 left,top,right,bottom;};
static struct LbSurface *lb_alias,*lb_created_surface;
static u32 lb_locks,lb_unlocks,lb_blits,lb_queries,lb_keys,lb_clippers,lb_releases,lb_fail_next,lb_creates;
static struct LbSurface* lb_state(void* object){struct LbSurface* s=object;return s->state?s->state:s;}
static void lb_entry(void){if(GetLastError()!=0x77)ExitProcess(110);SetLastError(0x88);}
static u32 lb_pixel(const u8* at,u32 bytes){u32 value=0;for(u32 i=0;i<bytes;++i)value|=(u32)at[i]<<(i*8);return value;}
static void lb_put(u8* at,u32 bytes,u32 value){for(u32 i=0;i<bytes;++i)at[i]=(u8)(value>>(i*8));}
static i32 WIN lb_lock(void* object,void* rect,u32* d,u32 flags,HANDLE event){
    lb_entry();struct LbSurface* s=lb_state(object);if(s->held || rect || flags!=1 || event)ExitProcess(111);
    ++lb_locks;s->held=1;u32 row=s->width*(s->bits/8),pitch=row+2;
    for(u32 y=0;y<s->height;++y)for(u32 x=0;x<row;++x)s->locked[y*pitch+x]=s->pixels[y*row+x];
    d[1]=1;d[2]=s->height;d[3]=s->width;d[4]=pitch;d[9]=(u32)s->locked;d[19]=0x40;d[21]=s->bits;
    d[22]=s->bits==16?0xf800:0xff0000;d[23]=s->bits==16?0x7e0:0xff00;d[24]=s->bits==16?0x1f:0xff;d[26]=0x840;return 13;
}
static i32 WIN lb_unlock(void* object,void* argument){
    lb_entry();struct LbSurface* s=lb_state(object);if(!s->held || argument)ExitProcess(112);
    ++lb_unlocks;
    if(lb_mode("unlock-restore") && s->width==2){
        SetLastError(0x77);if(((i32 (WIN *)(void*))s->table[27])(s)!=23 || GetLastError()!=0x88)ExitProcess(133);
    }
    s->held=0;u32 row=s->width*(s->bits/8);
    for(u32 y=0;y<s->height;++y)for(u32 x=0;x<row;++x){s->pixels[y*row+x]=s->locked[y*(row+2)+x];s->locked[y*(row+2)+x]=0xcc;}
    return 19;
}
static i32 WIN lb_query(void* object,const u8* guid,void** output){
    (void)object;lb_entry();if(guid[0]!=0x81)ExitProcess(113);++lb_queries;*output=lb_alias;return 23;
}
static i32 WIN lb_clipper(void* object,void* clipper){lb_entry();++lb_clippers;lb_state(object)->clip=clipper!=0;return 23;}
static i32 WIN lb_key(void* object,u32 flags,u32* key){
    lb_entry();++lb_keys;if(flags!=8)ExitProcess(114);if(lb_fail_next){lb_fail_next=0;return -1;}
    struct LbSurface* s=lb_state(object);s->key_active=key!=0;s->key=key?key[0]:0;return 23;
}
static i32 WIN lb_create(void* object,void* desc,void** result,void* outer){
    (void)object;lb_entry();if(desc!=(void*)0x1234 || outer)ExitProcess(130);
    ++lb_creates;*result=lb_created_surface;return 23;
}
static i32 WIN lb_restore(void* object){(void)object;lb_entry();return 23;}
static u32 WIN lb_release(void* object){(void)object;lb_entry();++lb_releases;return 0;}
static void lb_seed(struct LbSurface* s);
static i32 lb_copy(void* object,void* destination,void* source,void* rectangle,u32 flags,u32 fast,u32 x,u32 y){
    lb_entry();++lb_blits;if(lb_fail_next){lb_fail_next=0;return -1;}
    struct LbSurface *a=lb_state(source),*b=lb_state(object);if(a->held || b->held)ExitProcess(115);
    if(lb_mode("reentrant"))lb_seed(a);
    struct LbRect r={0,0,(i32)a->width,(i32)a->height};if(rectangle)r=*(struct LbRect*)rectangle;
    i32 dx=(i32)x,dy=(i32)y;if(!fast && destination){dx=((struct LbRect*)destination)->left;dy=((struct LbRect*)destination)->top;}
    if(!fast && !destination)dx=dy=0;
    u32 keyed=fast?(flags&1)!=0:(flags&0x8000)!=0,bytes=a->bits/8;
    if(r.left<0 || r.top<0 || dx<0 || dy<0 || r.right>(i32)a->width || r.bottom>(i32)a->height || dx+r.right-r.left>(i32)b->width || dy+r.bottom-r.top>(i32)b->height)return 17;
    for(i32 yy=r.top;yy<r.bottom;++yy)for(i32 xx=r.left;xx<r.right;++xx){
        u32 value=lb_pixel(a->pixels+((u32)yy*a->width+(u32)xx)*bytes,bytes);
        if(!keyed || !a->key_active || value!=a->key)lb_put(b->pixels+((u32)(dy+yy-r.top)*b->width+(u32)(dx+xx-r.left))*bytes,bytes,value);
    }
    return 17;
}
static i32 WIN lb_blt(void* object,void* destination,void* source,void* rect,u32 flags,void* effects){if(effects)ExitProcess(116);return lb_copy(object,destination,source,rect,flags,0,0,0);}
static i32 WIN lb_fast(void* object,u32 x,u32 y,void* source,void* rect,u32 flags){return lb_copy(object,0,source,rect,flags,1,x,y);}
static void lb_seed(struct LbSurface* s){
    typedef i32 (WIN *L)(void*,void*,void*,u32,HANDLE);typedef i32 (WIN *U)(void*,void*);
    u32 d[31]={124};SetLastError(0x77);if(((L)s->table[25])(s,0,d,1,0)!=13 || GetLastError()!=0x88)ExitProcess(117);
    SetLastError(0x77);if(((U)s->table[32])(s,0)!=19 || GetLastError()!=0x88)ExitProcess(118);
}
static void lb_draw(struct LbSurface* dst,struct LbSurface* src,u32 fast,u32 flags){
    typedef i32 (WIN *B)(void*,void*,void*,void*,u32,void*);typedef i32 (WIN *F)(void*,u32,u32,void*,void*,u32);
    struct LbSurface* a=lb_state(src);struct LbRect r={0,0,(i32)a->width,(i32)a->height},d={a->width==2?1:0,a->width==2?1:0,a->width==2?3:4,a->width==2?3:3};
    if(lb_mode("bounds"))r.left=-1;
    if(lb_mode("subrect")){r.left=1;d.right=2;}
    u32 fail=lb_fail_next;SetLastError(0x77);
    i32 status=fast?((F)dst->table[7])(dst,(u32)d.left,(u32)d.top,src,&r,flags):((B)dst->table[5])(dst,&d,src,&r,flags,0);
    if(status!=(fail?-1:17) || GetLastError()!=0x88)ExitProcess(119);
    char path[]="original-00000000.bin";static const char hex[]="0123456789abcdef";
    for(u32 i=0;i<8;++i)path[9+i]=hex[(lb_blits>>(28-4*i))&15];
    struct LbSurface* state=lb_state(dst);u32 written=0,length=state->width*state->height*(state->bits/8);
    HANDLE file=CreateFileA(path,0x40000000,1,0,1,0x80,0);
    if(file==(HANDLE)-1 || !WriteFile(file,state->pixels,length,&written,0) || written!=length)ExitProcess(129);CloseHandle(file);
}
static void test_lock_blits(void){
    static void* table[33],*alias_table[33];table[0]=(void*)&lb_query;table[2]=(void*)&lb_release;table[5]=(void*)&lb_blt;table[7]=(void*)&lb_fast;
    table[25]=(void*)&lb_lock;table[32]=(void*)&lb_unlock;table[28]=(void*)&lb_clipper;table[29]=(void*)&lb_key;table[27]=(void*)&lb_restore;
    for(u32 i=0;i<33;++i)alias_table[i]=table[i];
    u32 bits=lb_mode("rgb24")?24:lb_mode("rgb32")?32:16,bytes=bits/8;
    static struct LbSurface a,b,c,alias;
    a.table=b.table=c.table=table;a.width=a.height=2;b.width=c.width=4;b.height=c.height=3;a.bits=b.bits=c.bits=bits;a.kind=b.kind=c.kind=14;
    alias.table=alias_table;alias.state=&a;alias.kind=11;lb_alias=&alias;
    u32 values[4]={0xf800,0,0x7e0,0xffff};if(bits>16){values[0]=0x80aabbcc;values[1]=0;values[2]=0x112233;values[3]=0x445566;}
    for(u32 i=0;i<4;++i)lb_put(a.pixels+i*bytes,bytes,values[i]);
    for(u32 i=0;i<12;++i){lb_put(b.pixels+i*bytes,bytes,0x1f);lb_put(c.pixels+i*bytes,bytes,0x1234);}
    RenderInstallForTest(&a,14);RenderInstallForTest(&alias,11);
    if(lb_mode("created")){
        static void* draw_table[7];draw_table[6]=(void*)&lb_create;void** draw=draw_table;lb_created_surface=&b;
        RenderInstallForTest(&draw,4);void* result=0;SetLastError(0x77);
        if(((i32 (WIN *)(void*,void*,void**,void*))draw_table[6])(&draw,(void*)0x1234,&result,0)!=23 || result!=&b || GetLastError()!=0x88)ExitProcess(131);
    }
    if(!lb_mode("untracked"))lb_seed(&a);lb_seed(&b);lb_seed(&c);
    typedef i32 (WIN *P)(void*,void*);typedef i32 (WIN *K)(void*,u32,void*);
    if(!lb_mode("created")){SetLastError(0x77);if(((P)table[28])(&b,lb_mode("clipper")?(void*)1:0)!=23 || GetLastError()!=0x88)ExitProcess(120);}
    SetLastError(0x77);if(((P)table[28])(&c,0)!=23 || GetLastError()!=0x88)ExitProcess(121);
    u32 keyed=lb_mode("keyed")||lb_mode("alias")||lb_mode("key-failed")||lb_mode("key-removed"),key[2]={0,0};
    if(keyed){SetLastError(0x77);if(((K)table[29])(&a,8,key)!=23 || GetLastError()!=0x88)ExitProcess(122);}
    if(lb_mode("key-failed")||lb_mode("key-removed")){
        u32 failed=lb_mode("key-failed");lb_fail_next=failed;key[0]=key[1]=0xf800;SetLastError(0x77);
        if(((K)table[29])(&a,8,failed?key:0)!=(failed?-1:23) || GetLastError()!=0x88)ExitProcess(123);
    }
    struct LbSurface* source=&a;
    if(lb_mode("alias-conflict")){SetLastError(0x77);if(((K)alias_table[29])(&alias,8,key)!=23 || GetLastError()!=0x88)ExitProcess(132);}
    if(lb_mode("alias")||lb_mode("release")||lb_mode("alias-conflict")){
        static const u8 iid[16]={0x81,0xdb,0x14,0x6c,0x33,0xa7,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60};void* result=0;
        SetLastError(0x77);if(((i32 (WIN *)(void*,const void*,void**))table[0])(&a,iid,&result)!=23 || result!=&alias || GetLastError()!=0x88)ExitProcess(124);source=&alias;
    }
    if(lb_mode("release")){SetLastError(0x77);if(((u32 (WIN *)(void*))alias_table[2])(&alias)!=0 || GetLastError()!=0x88)ExitProcess(125);}
    if(lb_mode("restore")){SetLastError(0x77);if(((i32 (WIN *)(void*))table[27])(&b)!=23 || GetLastError()!=0x88)ExitProcess(126);}
    if(lb_mode("failed")){lb_fail_next=1;lb_draw(&b,source,0,0x1000000);}
    if(lb_mode("unsupported")||lb_mode("reseed")){lb_draw(&b,source,0,0x4000000);if(lb_mode("reseed"))lb_seed(&b);}
    if(lb_mode("self"))lb_draw(&b,&b,0,0x1000000);
    lb_draw(&b,source,keyed,keyed?0x11:0x1000000);
    u32 draw_count=1+(lb_mode("failed")||lb_mode("unsupported")||lb_mode("reseed")||lb_mode("self"));
    if(lb_mode("update")){lb_put(a.pixels,bytes,0x7ff);lb_seed(&a);lb_draw(&b,&a,1,0x10);++draw_count;}
    if(lb_mode("chain")||lb_mode("update")||lb_mode("rgb24")||lb_mode("rgb32")){lb_draw(&c,&b,1,0x10);++draw_count;}
    if(lb_mode("budget"))for(u32 i=0;i<17;++i){lb_draw(&c,&b,1,0x10);++draw_count;}
    if(lb_blits!=draw_count || lb_queries!=(lb_mode("alias")||lb_mode("release")||lb_mode("alias-conflict")) || lb_keys!=(keyed+(lb_mode("key-failed")||lb_mode("key-removed"))+lb_mode("alias-conflict")) || lb_clippers!=2u-lb_mode("created") || lb_creates!=(u32)lb_mode("created") || lb_releases!=(u32)lb_mode("release") || lb_locks!=3u-lb_mode("untracked")+lb_mode("reseed")+lb_mode("update")+lb_mode("reentrant") || lb_unlocks!=lb_locks)ExitProcess(127);
    struct LbSurface* expected=(lb_mode("chain")||lb_mode("update")||lb_mode("rgb24")||lb_mode("rgb32")||lb_mode("budget"))?&c:&b;
    HANDLE file=CreateFileA("expected.bin",0x40000000,1,0,1,0x80,0);u32 written=0,length=expected->width*expected->height*bytes;
    if(file==(HANDLE)-1 || !WriteFile(file,expected->pixels,length,&written,0) || written!=length)ExitProcess(128);CloseHandle(file);ExitProcess(0);
}
