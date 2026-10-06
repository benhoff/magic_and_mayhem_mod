/* Independent small engine: padded/negative borrowed rows, argument-derived
 * native drawing and actual Wine DIBs. Expected output never feeds the hook. */
struct MuSurface {void** table;u32 width,height,bits,primary,held,variant;u8 native[256],borrowed[512];i32 pitch;void* pointer;};
static struct MuSurface mu_front,mu_back;
static void** mu_palette;
static u8 mu_colors[1024];
static u32 mu_calls[10],mu_failed,mu_frames;
static HANDLE mu_output,mu_dc,mu_bitmap,mu_old;
static u8* mu_dib;
static void mu_entry(void){if(GetLastError()!=0x77)ExitProcess(340);SetLastError(0x88);}
static u32 mu_mask(u32 bits,u32 channel){return bits==8?0:bits==16?(channel==0?0xf800:channel==1?0x7e0:0x1f):(channel==0?0xff0000:channel==1?0xff00:0xff);}
static void mu_put(u8* at,u32 bits,u32 value){for(u32 b=0;b<bits/8;++b)at[b]=(u8)(value>>(b*8));}
static u32 mu_get(const u8* at,u32 bits){u32 value=0;for(u32 b=0;b<bits/8;++b)value|=(u32)at[b]<<(b*8);return value;}
static void mu_layout(struct MuSurface* s,u32* d){
    d[1]=0x1007;d[2]=s->height;d[3]=s->width;d[18]=32;d[19]=s->bits==8?0x60:0x40;d[21]=s->bits;
    for(u32 c=0;c<3;++c)d[22+c]=mu_mask(s->bits,c);
    if(s->bits==16 && s->variant){d[22]=0x7c00;d[23]=0x3e0;}
    d[26]=s->primary?0x238:0x1c;if(s->primary){d[1]|=0x20;d[5]=1;}
}
static i32 WIN mu_desc(void* object,u32* d){mu_entry();++mu_calls[2];mu_layout(object,d);return 23;}
static i32 WIN mu_clipper(void* object,void* clip){(void)object;mu_entry();++mu_calls[3];if(clip)ExitProcess(341);return 23;}
static i32 WIN mu_key(void* object,u32 flags,u32* key){(void)object;mu_entry();++mu_calls[4];if(flags!=8 || key[0] || key[1])ExitProcess(342);return 23;}
static i32 WIN mu_attached(void* object,u32* caps,void** out){mu_entry();++mu_calls[5];if(object!=&mu_front || caps[0]!=4)ExitProcess(343);*out=&mu_back;return 23;}
static i32 WIN mu_lock(void* object,void* region,u32* d,u32 flags,HANDLE event){
    mu_entry();++mu_calls[0];if(mu_failed==1){mu_failed=0;return -1;}
    struct MuSurface* s=object;if(s->held || flags!=1 || event)ExitProcess(344);s->held=1;
    u32 row=s->width*(s->bits/8),pitch=row+4;s->pitch=-(i32)pitch;
    u8* top=s->borrowed+(s->height-1)*pitch;
    for(u32 y=0;y<s->height;++y)copy_bytes(top+(i32)y*s->pitch,s->native+y*row,row);
    i32* r=region;s->pointer=top+(r?r[1]*s->pitch+r[0]*(i32)(s->bits/8):0);
    mu_layout(s,d);d[4]=(u32)s->pitch;d[9]=(u32)s->pointer;return 13;
}
static i32 WIN mu_unlock(void* object,void* region){
    (void)region;mu_entry();++mu_calls[1];if(mu_failed==2){mu_failed=0;return -1;}
    struct MuSurface* s=object;if(!s->held)ExitProcess(345);
    u32 row=s->width*(s->bits/8),pitch=row+4;u8* top=s->borrowed+(s->height-1)*pitch;
    for(u32 y=0;y<s->height;++y)copy_bytes(s->native+y*row,top+(i32)y*s->pitch,row);
    for(u32 i=0;i<sizeof(s->borrowed);++i)s->borrowed[i]=0xcc;s->held=0;return 19;
}
static i32 WIN mu_blt(void* object,void* destination,void* source,void* rectangle,u32 flags,void* effects){
    mu_entry();++mu_calls[6];if(mu_failed==3){mu_failed=0;return -1;}
    struct MuSurface* dst=object,*src=source;i32 dr[4]={0,0,(i32)dst->width,(i32)dst->height};
    i32 sr[4]={0,0,src?(i32)src->width:0,src?(i32)src->height:0};
    if(destination)copy_bytes(dr,destination,16);if(rectangle)copy_bytes(sr,rectangle,16);
    if(dst->held || (src && src->held))ExitProcess(346);
    for(i32 y=dr[1];y<dr[3];++y)for(i32 x=dr[0];x<dr[2];++x){
        u32 value=src?mu_get(src->native+((sr[1]+y-dr[1])*src->width+sr[0]+x-dr[0])*(src->bits/8),src->bits):((u32*)effects)[20];
        if(!(flags&0x8000) || value)mu_put(dst->native+(y*dst->width+x)*(dst->bits/8),dst->bits,value);
    }
    return 17;
}
static i32 WIN mu_flip(void* object,void* target,u32 flags){
    mu_entry();++mu_calls[7];if(mu_failed==4){mu_failed=0;return -1;}
    if(object!=&mu_front || target || flags!=1 || mu_front.held || mu_back.held)ExitProcess(347);
    for(u32 i=0;i<mu_front.width*mu_front.height*(mu_front.bits/8);++i){u8 v=mu_front.native[i];mu_front.native[i]=mu_back.native[i];mu_back.native[i]=v;}return 23;
}
static i32 WIN mu_caps(void* object,u32* caps){(void)object;mu_entry();*caps=0x54;return 23;}
static i32 WIN mu_read(void* object,u32 flags,u32 first,u32 count,u8* colors){(void)object;mu_entry();if(flags || first || count!=256)ExitProcess(348);copy_bytes(colors,mu_colors,1024);return 23;}
static i32 WIN mu_write(void* object,u32 flags,u32 first,u32 count,u8* colors){
    (void)object;(void)flags;mu_entry();++mu_calls[8];if(mu_failed==5){mu_failed=0;return -1;}
    copy_bytes(mu_colors+first*4,colors,count*4);for(u32 i=0;i<count*4;++i)colors[i]=0xcc;return 23;
}
static i32 WIN mu_assign(void* object,void* palette){(void)object;mu_entry();if(palette!=&mu_palette)ExitProcess(349);return 23;}
static void** mu_palette_alias;
static u32 mu_palette_refs;
static i32 WIN mu_palette_query(void* object,const u8* iid,void** out){(void)object;(void)iid;mu_entry();++mu_palette_refs;*out=&mu_palette_alias;return 23;}
static u32 WIN mu_palette_release(void* object){(void)object;mu_entry();if(!mu_palette_refs)ExitProcess(377);return --mu_palette_refs;}
static u32 WIN mu_surface_release(void* object){(void)object;mu_entry();if(!mu_palette_refs)ExitProcess(378);--mu_palette_refs;return 0;}
static i32 WIN mu_palette_factory(void* object,u32 flags,u8* entries,void** out,void* outer){(void)object;(void)outer;mu_entry();if(flags!=0x44 || mu_palette_refs)ExitProcess(379);copy_bytes(mu_colors,entries,1024);for(u32 i=0;i<1024;++i)entries[i]=0xcc;mu_palette_refs=1;*out=&mu_palette;return 23;}
static void mu_cycle(struct MuSurface*,u32,u32,u32);
static void mu_palette_lifetimes(void){
    static const u8 iid[16]={0x84,0xdb,0x14,0x6c,0x33,0xa7,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60};
    void* alias=0;SetLastError(0x77);if(((i32 (WIN *)(void*,const u8*,void**))mu_palette[0])(&mu_palette,iid,&alias)!=23 || alias!=&mu_palette_alias || GetLastError()!=0x88)ExitProcess(380);
    SetLastError(0x77);if(((u32 (WIN *)(void*))mu_palette[2])(&mu_palette)!=3 || GetLastError()!=0x88)ExitProcess(381);
    static void* draw_table[7];draw_table[5]=(void*)&mu_palette_factory;static void** draw;draw=draw_table;RenderInstallForTest(&draw,4);
    for(u32 pass=0;pass<40;++pass){
        SetLastError(0x77);if(((u32 (WIN *)(void*))mu_front.table[2])(&mu_front) || GetLastError()!=0x88)ExitProcess(382);
        SetLastError(0x77);if(((u32 (WIN *)(void*))mu_back.table[2])(&mu_back) || GetLastError()!=0x88)ExitProcess(383);
        SetLastError(0x77);if(((u32 (WIN *)(void*))mu_palette_alias[2])(&mu_palette_alias) || GetLastError()!=0x88)ExitProcess(384);
        u8 colors[1024];for(u32 i=0;i<256;++i){colors[i*4]=(u8)(i+pass);colors[i*4+1]=(u8)(i*3+pass);colors[i*4+2]=(u8)(255-i);colors[i*4+3]=0xa5;}
        void* created=0;SetLastError(0x77);if(((i32 (WIN *)(void*,u32,u8*,void**,void*))draw_table[5])(&draw,0x44,colors,&created,0)!=23 || created!=&mu_palette || GetLastError()!=0x88 || colors[0]!=0xcc)ExitProcess(385);
        SetLastError(0x77);if(((i32 (WIN *)(void*,void*))mu_front.table[31])(&mu_front,&mu_palette)!=23 || GetLastError()!=0x88)ExitProcess(386);
        SetLastError(0x77);if(((i32 (WIN *)(void*,void*))mu_back.table[31])(&mu_back,&mu_palette)!=23 || GetLastError()!=0x88)ExitProcess(387);
        mu_palette_refs+=2;mu_cycle(&mu_front,0,5+pass,0);mu_cycle(&mu_back,0,9+pass,0);
        SetLastError(0x77);if(((i32 (WIN *)(void*,const u8*,void**))mu_palette[0])(&mu_palette,iid,&alias)!=23 || GetLastError()!=0x88)ExitProcess(388);
        SetLastError(0x77);if(((u32 (WIN *)(void*))mu_palette[2])(&mu_palette)!=3 || GetLastError()!=0x88)ExitProcess(389);
    }
}
static i32 WIN mu_get_dc(void* object,void** out){mu_entry();++mu_calls[9];struct MuSurface* s=object;if(s->held)ExitProcess(350);s->held=1;*out=mu_dc;return 23;}
static i32 WIN mu_release_dc(void* object,void* dc){
    mu_entry();++mu_calls[9];if(mu_failed==6){mu_failed=0;return -1;}
    struct MuSurface* s=object;if(!s->held || dc!=mu_dc)ExitProcess(351);
    u32 row=s->width*(s->bits/8),pitch=(row+3)&~3u;
    for(u32 y=0;y<s->height;++y)copy_bytes(s->native+y*row,mu_dib+y*pitch,row);
    for(u32 i=0;i<pitch*s->height;++i)mu_dib[i]=0xcc;s->held=0;return 19;
}
static void mu_record(void){
    u32 header[7]={mu_front.width,mu_front.height,mu_front.bits,mu_front.width*mu_front.height*(mu_front.bits/8),mu_mask(mu_front.bits,0),mu_mask(mu_front.bits,1),mu_mask(mu_front.bits,2)},written;
    if(mu_front.bits==16 && mu_front.variant){header[4]=0x7c00;header[5]=0x3e0;}
    if(!WriteFile(mu_output,header,28,&written,0) || written!=28 ||
       !WriteFile(mu_output,mu_front.native,header[3],&written,0) || written!=header[3] ||
       !WriteFile(mu_output,mu_colors,1024,&written,0) || written!=1024)ExitProcess(352);
    ++mu_frames;Sleep(8);
}
static void mu_cycle(struct MuSurface* s,u32 partial,u32 value,u32 retry){
    i32 r[4]={1,1,3,3};u32 d[31]={124};SetLastError(0x77);
    if(retry){mu_failed=1;if(((i32 (WIN *)(void*,void*,void*,u32,HANDLE))s->table[25])(s,partial?r:0,d,1,0)!=-1 || GetLastError()!=0x88)ExitProcess(353);SetLastError(0x77);}
    if(((i32 (WIN *)(void*,void*,void*,u32,HANDLE))s->table[25])(s,partial?r:0,d,1,0)!=13 || GetLastError()!=0x88)ExitProcess(354);
    for(u32 y=0;y<(partial?2:s->height);++y)for(u32 x=0;x<(partial?2:s->width);++x){
        u32 v=partial?value+x+y*3:value+x+y*7;mu_put((u8*)s->pointer+(i32)y*s->pitch+x*(s->bits/8),s->bits,v);
    }
    SetLastError(0x77);if(retry){mu_failed=2;if(((i32 (WIN *)(void*,void*))s->table[32])(s,partial?r:0)!=-1 || GetLastError()!=0x88)ExitProcess(355);SetLastError(0x77);}
    if(((i32 (WIN *)(void*,void*))s->table[32])(s,partial?r:0)!=19 || GetLastError()!=0x88)ExitProcess(356);
    if(s->primary)mu_record();
}
static void mu_draw(u32 fill,u32 keyed,u32 value,u32 retry,u32 unsupported){
    i32 r[4]={1,1,3,3};u32 fx[25]={100};fx[20]=value;
    u32 flags=unsupported?0x20000:fill?0x1000400:keyed?0x1008000:0x1000000;
    SetLastError(0x77);if(retry){mu_failed=3;if(((i32 (WIN *)(void*,void*,void*,void*,u32,void*))mu_front.table[5])(&mu_front,fill?r:0,fill?0:&mu_back,0,flags,fill?fx:0)!=-1 || GetLastError()!=0x88)ExitProcess(357);SetLastError(0x77);}
    if(((i32 (WIN *)(void*,void*,void*,void*,u32,void*))mu_front.table[5])(&mu_front,fill?r:0,fill?0:&mu_back,0,flags,fill?fx:0)!=17 || GetLastError()!=0x88)ExitProcess(358);mu_record();
}
static void mu_swap(u32 retry){
    SetLastError(0x77);if(retry){mu_failed=4;if(((i32 (WIN *)(void*,void*,u32))mu_front.table[11])(&mu_front,0,1)!=-1 || GetLastError()!=0x88)ExitProcess(359);SetLastError(0x77);}
    if(((i32 (WIN *)(void*,void*,u32))mu_front.table[11])(&mu_front,0,1)!=23 || GetLastError()!=0x88)ExitProcess(360);mu_record();
}
static void mu_palette_change(u32 flags_only,u32 retry,u32 flags){
    u8 colors[8];copy_bytes(colors,mu_colors+5*4,8);for(u32 i=0;i<2;++i){if(!flags_only)for(u32 c=0;c<3;++c)colors[i*4+c]+=17+c;colors[i*4+3]^=0x7f;}
    SetLastError(0x77);if(retry){mu_failed=5;u8 failed[8];copy_bytes(failed,colors,8);if(((i32 (WIN *)(void*,u32,u32,u32,void*))mu_palette[6])(&mu_palette,0,5,2,failed)!=-1 || GetLastError()!=0x88)ExitProcess(361);SetLastError(0x77);}
    if(((i32 (WIN *)(void*,u32,u32,u32,void*))mu_palette[6])(&mu_palette,flags,5,2,colors)!=23 || GetLastError()!=0x88)ExitProcess(362);mu_record();
}
static void test_mutations(const char* mode){
    u32 bits=mode[0]=='i' || mode[0]=='p'?8:mode[0]=='r' || mode[0]=='b' || mode[0]=='u'?32:(u32)(mode[2]-'0')*10+(u32)(mode[3]-'0');
    u32 dc=mode[0]=='d',reshape=rs_mode(mode,"reshape"),bounded=rs_mode(mode,"bounded"),partial=rs_mode(mode,"partial");
    u32 unsupported=rs_mode(mode,"unsupported"),palette_flags=rs_mode(mode,"palette-flags");
    u32 valid=!(bounded || partial || unsupported || palette_flags);
    if(partial)bits=32;
    static void* table[33];table[2]=(void*)&mu_surface_release;table[5]=(void*)&mu_blt;table[11]=(void*)&mu_flip;table[12]=(void*)&mu_attached;table[17]=(void*)&mu_get_dc;
    table[22]=(void*)&mu_desc;table[25]=(void*)&mu_lock;table[26]=(void*)&mu_release_dc;table[28]=(void*)&mu_clipper;table[29]=(void*)&mu_key;table[31]=(void*)&mu_assign;table[32]=(void*)&mu_unlock;
    mu_front.table=mu_back.table=table;mu_front.width=mu_back.width=6;mu_front.height=mu_back.height=4;mu_front.bits=mu_back.bits=bits;mu_front.primary=1;
    RenderInstallForTest(&mu_front,14);RenderInstallForTest(&mu_back,14);
    static void* pal[7];pal[0]=(void*)&mu_palette_query;pal[2]=(void*)&mu_palette_release;pal[3]=(void*)&mu_caps;pal[4]=(void*)&mu_read;pal[6]=(void*)&mu_write;mu_palette=pal;
    for(u32 i=0;i<256;++i){mu_colors[i*4]=(u8)(i*3);mu_colors[i*4+1]=(u8)(i*7);mu_colors[i*4+2]=(u8)(255-i);mu_colors[i*4+3]=0xa5;}
    RenderInstallForTest(&mu_palette,20);u32 caps;u8 colors[1024];SetLastError(0x77);if(((i32 (WIN *)(void*,u32*))pal[3])(&mu_palette,&caps)!=23 || GetLastError()!=0x88)ExitProcess(363);
    SetLastError(0x77);if(((i32 (WIN *)(void*,u32,u32,u32,void*))pal[4])(&mu_palette,0,0,256,colors)!=23 || GetLastError()!=0x88)ExitProcess(364);
    SetLastError(0x77);if(((i32 (WIN *)(void*,void*))table[31])(&mu_front,&mu_palette)!=23 || GetLastError()!=0x88)ExitProcess(365);
    char palette_profile[2];u32 palette_resources=GetEnvironmentVariableA("MNM_RENDER_PALETTE_RESOURCES",palette_profile,2)==1 && palette_profile[0]=='1';
    if(palette_resources && bits==8){mu_palette_alias=pal;mu_palette_refs=3;SetLastError(0x77);if(((i32 (WIN *)(void*,void*))table[31])(&mu_back,&mu_palette)!=23 || GetLastError()!=0x88)ExitProcess(376);}
    mu_output=CreateFileA("mutation-frames.bin",0x40000000,1,0,1,0x80,0);if(mu_output==(HANDLE)-1)ExitProcess(366);
    mu_cycle(&mu_front,0,5,0);
    if(reshape || bounded || partial){
        mu_front.width=4;mu_front.bits=16;mu_cycle(&mu_front,partial,0x1234,1);
        if(reshape){mu_front.variant=1;mu_cycle(&mu_front,0,0x4567,0);mu_front.variant=0;mu_front.width=6;mu_front.bits=24;mu_cycle(&mu_front,0,0x123456,0);mu_front.bits=8;mu_cycle(&mu_front,0,5,0);mu_palette_change(0,0,0);}
    }else if(palette_flags)mu_palette_change(0,0,1);
    else if(dc){
        u32 info[13]={40,6,(u32)-4,(bits<<16)|1,bits==24?0:3,0,0,0,0,0,mu_mask(bits,0),mu_mask(bits,1),mu_mask(bits,2)};
        mu_dc=CreateCompatibleDC(0);mu_bitmap=CreateDIBSection(mu_dc,info,0,(void**)&mu_dib,0,0);
        if(!mu_dc || !mu_bitmap || !mu_dib)ExitProcess(367);mu_old=SelectObject(mu_dc,mu_bitmap);
        for(u32 pass=0;pass<12;++pass){void* out=0;SetLastError(0x77);if(((i32 (WIN *)(void*,void**))table[17])(&mu_front,&out)!=23 || out!=mu_dc || GetLastError()!=0x88)ExitProcess(368);
            u32 pitch=(6*(bits/8)+3)&~3u;
            for(u32 y=0;y<4;++y)for(u32 x=0;x<6;++x)mu_put(mu_dib+y*pitch+x*(bits/8),bits,0x1234+pass*11+x+y*3);
            if(pass==3){mu_failed=6;SetLastError(0x77);if(((i32 (WIN *)(void*,void*))table[26])(&mu_front,mu_dc)!=-1 || GetLastError()!=0x88)ExitProcess(369);mu_put(mu_dib,bits,0x4321);}
            SetLastError(0x77);if(((i32 (WIN *)(void*,void*))table[26])(&mu_front,mu_dc)!=19 || GetLastError()!=0x88)ExitProcess(370);mu_record();
        }
        SelectObject(mu_dc,mu_old);DeleteObject(mu_bitmap);DeleteDC(mu_dc);
    }else{
        u32 d[31]={124};SetLastError(0x77);if(((i32 (WIN *)(void*,void*))table[22])(&mu_front,d)!=23 || GetLastError()!=0x88)ExitProcess(371);
        SetLastError(0x77);if(((i32 (WIN *)(void*,void*))table[28])(&mu_front,0)!=23 || GetLastError()!=0x88)ExitProcess(372);
        u32 key[2]={0,0};SetLastError(0x77);if(((i32 (WIN *)(void*,u32,void*))table[29])(&mu_back,8,key)!=23 || GetLastError()!=0x88)ExitProcess(373);
        u32 requested[4]={4};void* out=0;SetLastError(0x77);if(((i32 (WIN *)(void*,void*,void**))table[12])(&mu_front,requested,&out)!=23 || out!=&mu_back || GetLastError()!=0x88)ExitProcess(374);
        if(unsupported){mu_cycle(&mu_back,0,0x44,0);mu_draw(0,0,0,0,1);}
        else for(u32 pass=0;pass<40;++pass){
            mu_cycle(&mu_back,0,pass,pass==3);mu_cycle(&mu_back,1,5,0);
            mu_draw(0,0,0,pass==3,0);mu_draw(1,0,17+pass,0,0);mu_draw(0,1,0,0,0);mu_swap(pass==3);mu_cycle(&mu_front,1,9+pass,pass==3);
            if(bits==8){mu_palette_change(0,pass==3,0);mu_palette_change(1,0,0);}
        }
    }
    if(palette_resources && rs_mode(mode,"indexed"))mu_palette_lifetimes();
    Sleep(100);CloseHandle(mu_output);pl_file("mutation-counts.bin",mu_calls,sizeof(mu_calls));
    SetLastError(0x77);u32 completed=RenderShutdown(3000);if(completed!=valid || GetLastError()!=0x77 || RenderShutdown(0)!=completed || GetLastError()!=0x77)ExitProcess(375);ExitProcess(0);
}
