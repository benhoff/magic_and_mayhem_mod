/* Independent fake originals: actual hooks, poisoned borrowed buffers and FIFO
 * cache pressure. Expected pixels are recorded from original fixture storage. */
static struct MuSurface ws_bank[64];
static void ws_init(struct MuSurface* s,u32 primary){
    static void* table[33];if(!table[25]){table[5]=(void*)&mu_blt;table[11]=(void*)&mu_flip;table[12]=(void*)&mu_attached;table[17]=(void*)&mu_get_dc;
        table[22]=(void*)&mu_desc;table[25]=(void*)&mu_lock;table[26]=(void*)&mu_release_dc;table[28]=(void*)&mu_clipper;table[32]=(void*)&mu_unlock;}
    s->table=table;s->width=6;s->height=4;s->bits=32;s->primary=primary;RenderInstallForTest(s,14);
    SetLastError(0x77);if(((i32 (WIN *)(void*,void*))table[28])(s,0)!=23 || GetLastError()!=0x88)ExitProcess(470);
}
static void ws_copy(struct MuSurface* dst,struct MuSurface* src){
    SetLastError(0x77);if(((i32 (WIN *)(void*,void*,void*,void*,u32,void*))dst->table[5])(dst,0,src,0,0x1000000,0)!=17 || GetLastError()!=0x88)ExitProcess(471);
    if(dst->primary)mu_record();
}
static void ws_hold(struct MuSurface* s){
    u32 desc[31]={124};SetLastError(0x77);if(((i32 (WIN *)(void*,void*,void*,u32,HANDLE))s->table[25])(s,0,desc,1,0)!=13 || GetLastError()!=0x88)ExitProcess(472);
}
static void ws_unlock(struct MuSurface* s){
    SetLastError(0x77);if(((i32 (WIN *)(void*,void*))s->table[32])(s,0)!=19 || GetLastError()!=0x88)ExitProcess(473);
}
static void test_working_set(const char* mode){
    u32 held=rs_mode(mode,"held-skip"),full=rs_mode(mode,"held-full"),dc=rs_mode(mode,"dc-skip"),alias=rs_mode(mode,"alias"),pixels=rs_mode(mode,"pixels");
    ws_init(&mu_front,1);mu_output=CreateFileA("mutation-frames.bin",0x40000000,1,0,1,0x80,0);if(mu_output==(HANDLE)-1)ExitProcess(474);
    mu_cycle(&mu_front,0,0x123456,0);
    if(alias){
        static struct RsSurface objects[64];static struct RsState states[65];
        for(u32 i=0;i<64;++i){rs_setup(objects+i,states+i,6,4,0);rs_update(objects+i,0xa5010000+i);}
        rs_alias.table=objects[0].table;rs_alias.state=states;rs_observe_alias(objects,&rs_alias);
        rs_update(&rs_alias,0xa5020000);rs_drop(objects,1);rs_update(&rs_alias,0xa5030000);rs_drop(&rs_alias,0);
        rs_setup(&rs_alias,states+64,6,4,0);rs_update(&rs_alias,0xa5040000);rs_drop(&rs_alias,0);
        for(u32 i=1;i<64;++i)rs_drop(objects+i,0);
        u32 counts[2]={rs_queries,rs_releases};pl_file("alias-counts.bin",counts,8);
    }else if(pixels){
        static struct CsSurface large[5];
        for(u32 i=0;i<5;++i){cs_setup(large+i,2048,2048,16,0);cs_update(large+i,0x1000+i);Sleep(400);}
        for(u32 i=0;i<2;++i){cs_update(large+i,0x2000+i);Sleep(400);}
    }else{
        for(u32 i=0;i<64;++i){
            ws_init(ws_bank+i,0);mu_cycle(ws_bank+i,0,0x110000+i,0);
            if(i==30 && (held || full || dc)){
                if(held)ws_hold(ws_bank);
                if(full)for(u32 j=0;j<31;++j)ws_hold(ws_bank+j);
                if(dc){
                    u32 info[13]={40,6,(u32)-4,(32u<<16)|1,3,0,0,0,0,0,0xff0000,0xff00,0xff};
                    mu_dc=CreateCompatibleDC(0);mu_bitmap=CreateDIBSection(mu_dc,info,0,(void**)&mu_dib,0,0);if(!mu_dc || !mu_bitmap || !mu_dib)ExitProcess(475);mu_old=SelectObject(mu_dc,mu_bitmap);
                    copy_bytes(mu_dib,ws_bank[0].native,96);void* out=0;SetLastError(0x77);
                    if(((i32 (WIN *)(void*,void**))ws_bank[0].table[17])(ws_bank,&out)!=23 || out!=mu_dc || GetLastError()!=0x88)ExitProcess(476);
                }
            }
        }
        if(full)for(u32 j=0;j<31;++j)ws_unlock(ws_bank+j);
        if(held){mu_put((u8*)ws_bank[0].pointer,32,0x778899);ws_unlock(ws_bank);}
        if(dc){
            for(u32 i=0;i<24;++i)mu_put(mu_dib+i*4,32,0x224400+i);
            SetLastError(0x77);if(((i32 (WIN *)(void*,void*))ws_bank[0].table[26])(ws_bank,mu_dc)!=19 || GetLastError()!=0x88)ExitProcess(477);
            SelectObject(mu_dc,mu_old);DeleteObject(mu_bitmap);DeleteDC(mu_dc);ws_copy(&mu_front,ws_bank);
        }
        if(rs_mode(mode,"copies")){
            for(u32 i=0;i<128;++i){u32 src=i*17%64,dst=(src+31)%64;
                if(i%3==0)mu_cycle(ws_bank+dst,1,0x330000+i,1);
                ws_copy(ws_bank+dst,ws_bank+src);ws_copy(&mu_front,ws_bank+dst);
            }
            ws_init(&mu_back,0);mu_cycle(&mu_back,0,0xabcdef,0);u32 caps[4]={4};void* out=0;SetLastError(0x77);
            if(((i32 (WIN *)(void*,void*,void**))mu_front.table[12])(&mu_front,caps,&out)!=23 || out!=&mu_back || GetLastError()!=0x88)ExitProcess(478);
            for(u32 i=0;i<64;++i)mu_cycle(ws_bank+i,0,0x550000+i,0);
            mu_swap(1);mu_swap(0);
        }
    }
    mu_cycle(&mu_front,0,0x654321,0);CloseHandle(mu_output);
    u32 counts[10];copy_bytes(counts,mu_calls,sizeof(counts));pl_file("mutation-counts.bin",counts,sizeof(counts));
    u32 cs_counts[2]={cs_locks,cs_unlocks};pl_file("cs-counts.bin",cs_counts,8);
    SetLastError(0x77);u32 complete=RenderShutdown(3000);if(complete!=(full?0u:1u) || GetLastError()!=0x77)ExitProcess(479);
    if(RenderShutdown(0)!=complete || GetLastError()!=0x77)ExitProcess(480);
    u32 storage[6];if(!RenderRecoveryStorageForTest(storage) || GetLastError()!=0x77 || storage[0] || storage[1] || storage[2] || storage[5])ExitProcess(481);
    pl_file("storage.bin",storage,sizeof(storage));ExitProcess(0);
}
