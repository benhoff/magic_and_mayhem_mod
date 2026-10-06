/* Independent original fixture; attachment uses administrative control only. */
static i32 WIN cp_assign(void* object,void* palette){return mu_assign(object,palette==&mu_palette_alias?&mu_palette:palette);}
static void test_checkpoint(const char* mode){
    SetLastError(0x77);if(!RenderStartup() || GetLastError()!=0x77)ExitProcess(440);
    u32 indexed=rs_mode(mode,"indexed") || rs_mode(mode,"palette-incomplete"),bits=indexed?8:32;
    static void* table[33];table[5]=(void*)&mu_blt;table[11]=(void*)&mu_flip;table[12]=(void*)&mu_attached;table[17]=(void*)&mu_get_dc;
    table[22]=(void*)&mu_desc;table[25]=(void*)&mu_lock;table[26]=(void*)&mu_release_dc;table[28]=(void*)&mu_clipper;table[29]=(void*)&mu_key;table[31]=(void*)&cp_assign;table[32]=(void*)&mu_unlock;
    mu_front.table=mu_back.table=table;mu_front.width=mu_back.width=6;mu_front.height=mu_back.height=4;mu_front.bits=mu_back.bits=bits;mu_front.primary=1;
    RenderInstallForTest(&mu_front,14);RenderInstallForTest(&mu_back,14);
    mu_output=CreateFileA("mutation-frames.bin",0x40000000,1,0,1,0x80,0);if(mu_output==(HANDLE)-1)ExitProcess(441);
    for(u32 i=0;i<256;++i){mu_colors[i*4]=(u8)(i*3);mu_colors[i*4+1]=(u8)(i*7);mu_colors[i*4+2]=(u8)(255-i);mu_colors[i*4+3]=0xa5;}
    if(indexed){
        static void* pal[7];pal[0]=(void*)&mu_palette_query;pal[3]=(void*)&mu_caps;pal[4]=(void*)&mu_read;pal[6]=(void*)&mu_write;mu_palette=pal;RenderInstallForTest(&mu_palette,20);
        u32 caps;u8 colors[1024];SetLastError(0x77);if(((i32 (WIN *)(void*,u32*))pal[3])(&mu_palette,&caps)!=23 || GetLastError()!=0x88)ExitProcess(442);
        if(!rs_mode(mode,"palette-incomplete")){SetLastError(0x77);if(((i32 (WIN *)(void*,u32,u32,u32,void*))pal[4])(&mu_palette,0,0,256,colors)!=23 || GetLastError()!=0x88)ExitProcess(443);}
        mu_palette_alias=pal;mu_palette_refs=1;void* alias=0;
        static const u8 iid[16]={0x84,0xdb,0x14,0x6c,0x33,0xa7,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60};
        SetLastError(0x77);if(((i32 (WIN *)(void*,const u8*,void**))pal[0])(&mu_palette,iid,&alias)!=23 || alias!=&mu_palette_alias || GetLastError()!=0x88)ExitProcess(453);
        for(u32 i=0;i<2;++i){SetLastError(0x77);if(((i32 (WIN *)(void*,void*))table[31])(i?&mu_back:&mu_front,i?&mu_palette_alias:&mu_palette)!=23 || GetLastError()!=0x88)ExitProcess(444);}
    }
    u32 d[31]={124};SetLastError(0x77);if(((i32 (WIN *)(void*,void*))table[22])(&mu_back,d)!=23 || GetLastError()!=0x88)ExitProcess(445);
    SetLastError(0x77);if(((i32 (WIN *)(void*,void*))table[28])(&mu_front,0)!=23 || GetLastError()!=0x88)ExitProcess(446);
    u32 key[2]={0,0};SetLastError(0x77);if(((i32 (WIN *)(void*,u32,void*))table[29])(&mu_back,8,key)!=23 || GetLastError()!=0x88)ExitProcess(447);
    u32 caps[4]={4};void* out=0;SetLastError(0x77);if(((i32 (WIN *)(void*,void*,void**))table[12])(&mu_front,caps,&out)!=23 || out!=&mu_back || GetLastError()!=0x88)ExitProcess(448);
    mu_cycle(&mu_front,0,5,0);
    if(!rs_mode(mode,"incomplete"))mu_cycle(&mu_back,0,7,0);
    if(rs_mode(mode,"partial")){
        /* New metadata-only surface with no authoritative complete base. */
        static struct MuSurface extra;extra.table=table;extra.width=6;extra.height=4;extra.bits=32;RenderInstallForTest(&extra,14);mu_cycle(&extra,1,9,0);
    }
    if(rs_mode(mode,"capacity"))for(u32 i=0;i<31;++i){cs_setup(cs_extra+i,4,4,16,0);cs_update(cs_extra+i,0x1234+i);}
    if(rs_mode(mode,"budget")){mu_front.primary=0;mu_cycle(&mu_front,0,5,0);cs_setup(&cs_big,2048,2048,32,1);cs_update(&cs_big,0x00123456);cs_setup(&cs_small,2048,2048,32,0);cs_update(&cs_small,0x00654321);}
    if(rs_mode(mode,"uncertain")){RenderCaptureGuardForTest(1);u32 desc[31]={124};SetLastError(0x77);if(((i32 (WIN *)(void*,void*))table[22])(&mu_front,desc)!=23 || GetLastError()!=0x88)ExitProcess(455);RenderCaptureGuardForTest(0);}
    u32 held=rs_mode(mode,"held"),dc=rs_mode(mode,"dc"),valid=rs_mode(mode,"rgb") || (indexed && !rs_mode(mode,"palette-incomplete"));
    if(held){u32 desc[31]={124};SetLastError(0x77);if(((i32 (WIN *)(void*,void*,void*,u32,HANDLE))table[25])(&mu_front,0,desc,1,0)!=13 || GetLastError()!=0x88)ExitProcess(449);}
    if(dc){u32 info[13]={40,6,(u32)-4,(32u<<16)|1,3,0,0,0,0,0,0xff0000,0xff00,0xff};
        mu_dc=CreateCompatibleDC(0);mu_bitmap=CreateDIBSection(mu_dc,info,0,(void**)&mu_dib,0,0);mu_old=SelectObject(mu_dc,mu_bitmap);
        SetLastError(0x77);if(((i32 (WIN *)(void*,void**))table[17])(&mu_front,&out)!=23 || GetLastError()!=0x88)ExitProcess(450);}
    for(u32 phase=0;phase<(valid?3u:1u);++phase){
        /* Checkpoint is an additional PRESENT of the latest original pixels. */
        mu_record();rc_mark("checkpoint-ready-",phase,phase);rc_wait("resume-",phase);
        if(valid){u32 state[8];SetLastError(0x77);if(!RenderRecoveryStateForTest(state) || GetLastError()!=0x77 || state[0]!=48*(bits/8) || state[1] || state[7]!=(indexed?2u:0u))ExitProcess(454);
            mu_cycle(&mu_back,1,5+phase,1);mu_draw(0,0,0,1,0);mu_draw(1,0,17+phase,0,0);mu_draw(0,1,0,0,0);mu_swap(1);mu_cycle(&mu_front,1,9+phase,1);if(indexed){mu_palette_change(0,1,0);mu_palette_change(1,0,0);}}
        rc_mark("checkpoint-drawn-",phase,phase);rc_wait("advance-",phase);
    }
    if(held){SetLastError(0x77);if(((i32 (WIN *)(void*,void*))table[32])(&mu_front,0)!=19 || GetLastError()!=0x88)ExitProcess(451);}
    if(dc){SetLastError(0x77);if(((i32 (WIN *)(void*,void*))table[26])(&mu_front,mu_dc)!=19 || GetLastError()!=0x88)ExitProcess(452);SelectObject(mu_dc,mu_old);DeleteObject(mu_bitmap);DeleteDC(mu_dc);}
    CloseHandle(mu_output);pl_file("engine-counts.bin",mu_calls,sizeof(mu_calls));u32 cs_counts[2]={cs_locks,cs_unlocks};pl_file("checkpoint-cs-counts.bin",cs_counts,8);rc_mark("checkpoint-done-",0,0);rc_wait("exit-",0);ExitProcess(0);
}
