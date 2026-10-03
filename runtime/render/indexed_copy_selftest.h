/* Owned indexed copies/rotation: independent engine indices and per-surface palettes. */
static void ic_colors(struct BsSurface* surface){
    char path[]="colors-00000000.bin";static const char hex[]="0123456789abcdef";
    for(u32 i=0;i<8;++i)path[7+i]=hex[(bs_draws>>(28-i*4))&15];
    struct IpInterface* palette=surface->palette;static u8 blank[1024];u32 written;
    HANDLE file=CreateFileA(path,0x40000000,1,0,1,0x80,0);
    if(file==(HANDLE)-1 || !WriteFile(file,palette?palette->state->colors:blank,1024,&written,0) || written!=1024)ExitProcess(216);CloseHandle(file);
}
static void ic_palette(struct BsSurface* surface,struct IpInterface* palette,int known){
    SetLastError(0x77);if(((i32 (WIN *)(void*,void*))surface->table[31])(surface,palette)!=23 || GetLastError()!=0x88)ExitProcess(217);
    ip_observe_caps(palette);
    if(known){u8 colors[1024];SetLastError(0x77);
        if(((i32 (WIN *)(void*,u32,u32,u32,void*))palette->table[4])(palette,0,0,256,colors)!=23 || GetLastError()!=0x88)ExitProcess(218);}
}
static void ic_complete_palette(void){ip_get_colors(&ip_a.base,0,256);}
static void ic_draw(struct BsSurface* target,struct BsSurface* source,u32 fast,u32 keyed,u32 partial){
    bs_draw(target,source,fast,keyed,partial);ic_colors(bs_state(target));
}
static void ic_flip(struct BsSurface* front){bs_do_flip(front);ic_colors(front);}
static void ic_nested(void){
    if(bs_mode("idxcopy-nested-assignment")){
        SetLastError(0x77);if(((i32 (WIN *)(void*,void*))ip_front->table[31])(ip_front,&ip_b.base)!=23 || GetLastError()!=0x88)ExitProcess(219);
    }else{u8 colors[8]={90,200,30,0xaa,240,40,170,0xbb};SetLastError(0x77);
        if(((i32 (WIN *)(void*,u32,u32,u32,void*))ip_a.base.table[6])(&ip_a.base,0,1,2,colors)!=23 || GetLastError()!=0x88)ExitProcess(220);}
    SetLastError(0x88);
}
static void ic_key(struct BsSurface* source){u32 value=bs_mode("idxcopy-key-range")?256:0,key[2]={value,value};SetLastError(0x77);
    if(((i32 (WIN *)(void*,u32,void*))source->table[29])(source,8,key)!=23 || GetLastError()!=0x88)ExitProcess(221);}
static void ic_test(struct BsSurface* front,struct BsSurface* source,struct BsSurface* sprite,struct BsSurface* alias_surface){
    static void *surface[33],*palette[7],*palette_alias[7];
    surface[0]=(void*)&bs_query;surface[5]=(void*)&bs_blt;surface[7]=(void*)&bs_fast;surface[11]=(void*)&bs_flip;
    surface[12]=(void*)&bs_attached;surface[22]=(void*)&bs_description;surface[25]=(void*)&bs_lock;surface[32]=(void*)&bs_unlock;
    surface[28]=(void*)&bs_clipper;surface[29]=(void*)&bs_key;surface[31]=(void*)&ip_assign;surface[20]=(void*)&ip_get;
    front->table=source->table=sprite->table=surface;
    RenderInstallForTest(front,front->kind);RenderInstallForTest(source,source->kind);RenderInstallForTest(sprite,sprite->kind);
    palette[0]=(void*)&ip_query;palette[2]=(void*)&ip_release;palette[3]=(void*)&ip_caps;palette[4]=(void*)&ip_read;
    palette[5]=(void*)&ip_initialize;palette[6]=(void*)&ip_write;for(u32 i=0;i<7;++i)palette_alias[i]=palette[i];
    ip_a.base.table=ip_b.base.table=palette;ip_a.alias.table=ip_b.alias.table=palette_alias;
    ip_a.base.state=ip_a.alias.state=&ip_a;ip_b.base.state=ip_b.alias.state=&ip_b;ip_a.refs=ip_b.refs=1;
    for(u32 i=0;i<256;++i)for(u32 j=0;j<4;++j){ip_a.colors[i*4+j]=(u8)(i*(j*6+1)+j*13);ip_b.colors[i*4+j]=(u8)(255-i*(j*4+3));}
    /* Equal displayed colors are still distinct native key values. */
    for(u32 i=0;i<3;++i)ip_a.colors[4+i]=ip_a.colors[i];
    for(u32 y=0;y<source->height;++y)for(u32 x=0;x<source->width;++x)source->pixels[y*source->width+x]=(u8)(5+(x+y*3)%251);
    for(u32 y=0;y<front->height;++y)for(u32 x=0;x<front->width;++x)front->pixels[y*front->width+x]=(u8)(31+(x*7+y*11)%225);
    sprite->pixels[0]=sprite->pixels[3]=0;sprite->pixels[1]=sprite->pixels[2]=1;
    ip_front=front;
    int unknown=bs_mode("idxcopy-unknown-palette") || bs_mode("idxcopy-late-palette") || bs_mode("idxflip-unknown-palette") || bs_mode("idxflip-late-palette");
    if(bs_mode("idxcopy-offscreen"))front->primary=0;
    ic_palette(front,&ip_a.base,!unknown);ic_palette(source,&ip_b.base,1);
    bs_seed(source);
    if(bs_is_flip()){
        bs_back=source;
        if(bs_mode("idxflip-unseeded-front")){u32 d[31]={124};SetLastError(0x77);
            if(((i32 (WIN *)(void*,void*))surface[22])(front,d)!=23 || GetLastError()!=0x88)ExitProcess(230);}
        else bs_seed(front);ip_record();
        if(!bs_mode("idxflip-unobserved")){u32 caps[4]={4};void* out=0;SetLastError(0x77);
            if(((i32 (WIN *)(void*,void*,void**))surface[12])(front,caps,&out)!=23 || out!=source || GetLastError()!=0x88)ExitProcess(222);}
        if(bs_mode("idxflip-alias")){
            alias_surface->state=source;static const u8 iid[16]={0x81,0xdb,0x14,0x6c,0x33,0xa7,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60};void* out=0;SetLastError(0x77);
            if(((i32 (WIN *)(void*,const u8*,void**))surface[0])(source,iid,&out)!=23 || out!=alias_surface || GetLastError()!=0x88)ExitProcess(223);}
        if(bs_mode("idxflip-retry"))ic_flip(front);
        ic_flip(front);
        if(bs_mode("idxflip-late-palette"))ic_complete_palette();
        if(!bs_mode("idxflip-unobserved")){
            bs_seed(sprite);ic_key(sprite);ic_draw(source,sprite,1,1,0);ic_flip(front);ic_flip(front);
            if(!unknown){ip_change(&ip_a.base);ip_change(&ip_b.base);}
        }
        if(bs_locks!=((bs_mode("idxflip-unobserved") || bs_mode("idxflip-unseeded-front"))?2u:3u) || bs_unlocks!=bs_locks || bs_descriptions!=(u32)bs_mode("idxflip-unseeded-front") ||
           bs_attachments!=!bs_mode("idxflip-unobserved") || bs_queries!=(u32)bs_mode("idxflip-alias") || bs_creates ||
           ip_caps_calls!=2 || ip_reads!=2u-(u32)unknown+(u32)bs_mode("idxflip-late-palette") || ip_writes!=(unknown || bs_mode("idxflip-unobserved")?0u:2u))ExitProcess(224);
    }else{
        u32 d[31]={0};d[0]=front->kind>=14?124:108;SetLastError(0x77);
        if(((i32 (WIN *)(void*,void*))surface[22])(front,d)!=23 || GetLastError()!=0x88)ExitProcess(225);
        SetLastError(0x77);if(((i32 (WIN *)(void*,void*))surface[28])(front,0)!=23 || GetLastError()!=0x88)ExitProcess(226);
        struct BsSurface* target=front;
        if(bs_mode("idxcopy-alias")){static const u8 iid[16]={0x81,0xdb,0x14,0x6c,0x33,0xa7,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60};void* out=0;SetLastError(0x77);
            if(((i32 (WIN *)(void*,const u8*,void**))surface[0])(front,iid,&out)!=23 || out!=alias_surface || GetLastError()!=0x88)ExitProcess(227);target=alias_surface;}
        if(bs_mode("idxcopy-retry")){bs_fail=1;ic_draw(target,source,0,0,0);}
        if(bs_mode("idxcopy-nested-palette") || bs_mode("idxcopy-nested-assignment"))bs_copy_hook=ic_nested;
        if(bs_mode("idxcopy-keyed-bootstrap"))ic_key(source);
        ic_draw(target,source,bs_mode("idxcopy-fast") || bs_mode("idxcopy-alias"),bs_mode("idxcopy-keyed-bootstrap"),bs_mode("idxcopy-partial"));
        if(bs_mode("idxcopy-nested-assignment"))ic_draw(target,source,0,0,0);
        if(bs_mode("idxcopy-late-palette"))ic_complete_palette();
        int rejected=bs_mode("idxcopy-partial") || bs_mode("idxcopy-keyed-bootstrap");
        if(!rejected){bs_seed(sprite);ic_key(sprite);ic_draw(target,sprite,1,1,0);}
        if(bs_locks!=1u+!rejected || bs_unlocks!=bs_locks || bs_descriptions!=1 || bs_attachments || bs_queries!=(u32)bs_mode("idxcopy-alias") || bs_creates ||
           ip_caps_calls!=2 || ip_reads!=2u-(u32)unknown+(u32)bs_mode("idxcopy-late-palette") || ip_writes!=(u32)bs_mode("idxcopy-nested-palette"))ExitProcess(228);
    }
    if(ip_assigns!=2u+(u32)bs_mode("idxcopy-nested-assignment") || ip_gets || ip_queries || ip_creates || ip_initializes ||
       ip_a.refs!=(bs_mode("idxcopy-nested-assignment")?1u:2u) || ip_b.refs!=(bs_mode("idxcopy-nested-assignment")?3u:2u))ExitProcess(229);
    ExitProcess(0);
}
