/* No RenderInstallForTest before first DirectDrawCreate: initial hooks must be
 * installed through the actual saved-original factory path. Fixture pixels are
 * independent native storage, published only after a complete original unlock. */
API u32 WIN RenderStartup(void);
static void* ds_tables[2][7];
static void** ds_draws[2];
static u32 ds_calls,ds_surface_calls,ds_fail,ds_null,ds_depth,ds_nested,ds_worker_original;
static i32 WIN ds_unlock(void* object,void* pointer){
    if(pointer!=((struct RsSurface*)object)->state->pixels.exposed)ExitProcess(466);return rs_unlock(object,0);
}
static i32 WIN ds_surface(void* object,u32* d,void** out,void* outer){
    if((object!=ds_draws && object!=ds_draws+1) || !d || d[0]!=108 || d[2]!=16 || d[3]!=32 || !out || outer || GetLastError()!=0x77)ExitProcess(450);
    ++ds_surface_calls;SetLastError(0x88);if(rs_create_fail){rs_create_fail=0;return -1;}
    static void* table[33];table[0]=(void*)&rs_query;table[2]=(void*)&rs_release;table[25]=(void*)&rs_lock;table[32]=(void*)&ds_unlock;
    struct RsState* state=&rs_primary_state;rs_primary.table=table;rs_primary.state=state;state->live=state->refs=1;
    struct CsSurface* p=&state->pixels;p->width=32;p->height=16;p->bits=32;p->primary=1;
    p->native=HeapAlloc(GetProcessHeap(),8,32*16*4);p->exposed=HeapAlloc(GetProcessHeap(),8,(32*4+8)*16);
    if(!p->native || !p->exposed)ExitProcess(451);*out=&rs_primary;d[3]=0xcc;return 23;
}
static void ds_make_surface(void){
    u32 d[27]={108,0x1007,16,32};d[18]=32;d[19]=0x40;d[21]=32;d[22]=0xff0000;d[23]=0xff00;d[24]=0xff;d[26]=0x200;
    void* out=0;rs_create_fail=1;SetLastError(0x77);
    if(((i32 (WIN *)(void*,u32*,void**,void*))ds_draws[ds_nested?1:0][6])(ds_draws+(ds_nested?1:0),d,&out,0)!=-1 || out || GetLastError()!=0x88)ExitProcess(452);
    SetLastError(0x77);if(((i32 (WIN *)(void*,u32*,void**,void*))ds_draws[ds_nested?1:0][6])(ds_draws+(ds_nested?1:0),d,&out,0)!=23 || out!=&rs_primary || d[3]!=0xcc || GetLastError()!=0x88)ExitProcess(453);
}
static void ds_update(u32 color){
    struct CsSurface* p=&rs_primary_state.pixels;u32 d[27]={108};SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*,u32*,u32,HANDLE))rs_primary.table[25])(&rs_primary,0,d,1,0)!=13 || GetLastError()!=0x88)ExitProcess(463);
    for(u32 y=0;y<16;++y)for(u32 x=0;x<32;++x)for(u32 b=0;b<4;++b)p->exposed[y*d[4]+x*4+b]=(u8)(color>>(b*8));
    SetLastError(0x77);if(((i32 (WIN *)(void*,void*))rs_primary.table[32])(&rs_primary,(void*)d[9])!=19 || GetLastError()!=0x88)ExitProcess(464);
    for(u32 i=0;i<32*16*4;++i)if(p->native[i]!=(u8)(color>>((i%4)*8)))ExitProcess(465);++rs_frames;Sleep(75);
}
static i32 WIN ds_factory(void* guid,void** out,void* outer){
    if(guid!=(void*)0x1234 || outer!=(void*)0x5678 || !out || GetLastError()!=0x77)ExitProcess(454);
    ++ds_calls;if(rs_gate_kind==4){ds_worker_original=1;__atomic_store_n(&rs_gate_original,1,__ATOMIC_RELEASE);}
    SetLastError(0x88);if(ds_fail){ds_fail=0;return (i32)0x887600ff;}if(ds_null){ds_null=0;*out=0;return 0;}
    if(ds_nested && !ds_depth){
        ds_depth=1;void* nested=0;SetLastError(0x77);
        if(RenderCreateForTest((void*)&ds_factory,guid,&nested,outer)!=23 || nested!=ds_draws+1 || GetLastError()!=0x88)ExitProcess(455);
        ds_make_surface();ds_update(0xa5123456);SetLastError(0x88);ds_depth=0;
    }
    *out=ds_draws+(ds_depth || rs_gate_kind==4?1:0);return 23;
}
static void ds_call(i32 expected,u32 null_result){
    void* out=0;SetLastError(0x77);i32 status=RenderCreateForTest((void*)&ds_factory,(void*)0x1234,&out,(void*)0x5678);
    if(status!=expected || GetLastError()!=0x88 || (expected<0 || null_result?out!=0:out!=ds_draws+(rs_gate_kind==4?1:0)))ExitProcess(456);
}
static u32 WIN ds_worker(void* unused){
    (void)unused;u32 start=GetTickCount();while(!__atomic_load_n(&rs_gate_ready,__ATOMIC_ACQUIRE)){if(GetTickCount()-start>2000)ExitProcess(457);Sleep(0);}
    __atomic_store_n(&rs_gate_arrived,1,__ATOMIC_RELEASE);ds_fail=1;ds_call((i32)0x887600ff,0);ds_null=1;ds_call(0,1);ds_call(23,0);
    __atomic_store_n(&rs_gate_done,1,__ATOMIC_RELEASE);return 0;
}
static void test_draw_startup(const char* mode){
    for(u32 i=0;i<2;++i){ds_tables[i][6]=(void*)&ds_surface;ds_draws[i]=ds_tables[i];}
    u32 timeout=rs_mode(mode,"timeout"),cross=rs_mode(mode,"cross") || timeout;ds_nested=rs_mode(mode,"nested");
    if(rs_mode(mode,"failed")){ds_fail=1;ds_call((i32)0x887600ff,0);}if(rs_mode(mode,"null")){ds_null=1;ds_call(0,1);}
    ds_call(23,0);SetLastError(0x77);if(!RenderStartup() || GetLastError()!=0x77)ExitProcess(458);
    if(rs_mode(mode,"delayed")){
        pl_file("factory-ready.bin",&ds_calls,4);u32 ready=0;
        for(u32 i=0;i<500;++i){HANDLE file=CreateFileA("consumer-ready.bin",0x80000000,3,0,3,0x80,0);if(file!=(HANDLE)-1){CloseHandle(file);ready=1;break;}Sleep(10);}
        if(!ready)ExitProcess(467);
    }
    if(!ds_nested){ds_make_surface();ds_update(0xa5123456);}
    if(cross){rs_gate_kind=4;rs_gate_timeout=timeout;HANDLE worker=CreateThread(0,0,ds_worker,0,0,0);if(!worker)ExitProcess(459);
        ds_update(0xa5654321);if(WaitForSingleObject(worker,2000)!=0)ExitProcess(460);CloseHandle(worker);rs_gate_kind=0;
        if((ds_draws[1][6]==(void*)&ds_surface)!=timeout)ExitProcess(461);
    }else ds_update(0xa5654321);
    rs_drop(&rs_primary,0);u32 counts[6]={cs_locks,cs_unlocks,rs_queries,rs_releases,rs_dc_calls,rs_frames};pl_file("resource-counts.bin",counts,sizeof(counts));
    u32 factory_counts[5]={ds_calls,ds_surface_calls,ds_nested,ds_worker_original,rs_gate_done};pl_file("factory-counts.bin",factory_counts,sizeof(factory_counts));
    SetLastError(0x77);if(RenderShutdown(3000)!=!timeout || GetLastError()!=0x77 || RenderShutdown(0)!=!timeout || RenderStartup()!=0 || GetLastError()!=0x77)ExitProcess(462);ExitProcess(0);
}
