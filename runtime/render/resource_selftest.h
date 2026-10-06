/* Independent COM lifetime fixture. Only observed QI establishes aliases;
 * Release destroys storage logically before the hook sees its opaque token. */
struct RsState {struct CsSurface pixels;u32 refs,live;};
struct RsSurface {void** table;struct RsState* state;};
static struct RsSurface rs_primary,rs_objects[31],rs_alias;
static struct RsState rs_primary_state,rs_states[31];
static struct RsSurface* rs_query_target;
static u32 rs_queries,rs_releases,rs_dc_calls,rs_frames;
static HANDLE rs_dc;
static i32 WIN rs_query(void* object,const void* iid,void** out){
    struct RsSurface* s=object;
    if(!s->state->live || !iid || !out || !rs_query_target || GetLastError()!=0x77)ExitProcess(310);
    ++rs_queries;++rs_query_target->state->refs;*out=rs_query_target;SetLastError(0x88);return 23;
}
static u32 WIN rs_release(void* object){
    struct RsState* s=((struct RsSurface*)object)->state;
    if(!s->live || !s->refs || GetLastError()!=0x77)ExitProcess(311);
    ++rs_releases;if(!--s->refs)s->live=0;SetLastError(0x88);return s->refs;
}
static i32 WIN rs_lock(void* object,void* rect,u32* d,u32 flags,HANDLE event){
    struct RsState* s=((struct RsSurface*)object)->state;if(!s->live)ExitProcess(312);
    return cs_lock(&s->pixels,rect,d,flags,event);
}
static i32 WIN rs_unlock(void* object,void* arg){
    struct RsState* s=((struct RsSurface*)object)->state;if(!s->live)ExitProcess(313);
    return cs_unlock(&s->pixels,arg);
}
static i32 WIN rs_get_dc(void* object,void** out){
    if(!((struct RsSurface*)object)->state->live || GetLastError()!=0x77)ExitProcess(314);
    ++rs_dc_calls;*out=rs_dc;SetLastError(0x88);return 23;
}
static void rs_setup(struct RsSurface* object,struct RsState* state,u32 width,u32 height,u32 primary){
    static void* table[33];if(!table[25]){
        table[0]=(void*)&rs_query;table[2]=(void*)&rs_release;table[17]=(void*)&rs_get_dc;
        table[25]=(void*)&rs_lock;table[32]=(void*)&rs_unlock;
    }
    if(state->pixels.native)HeapFree(GetProcessHeap(),0,state->pixels.native);
    if(state->pixels.exposed)HeapFree(GetProcessHeap(),0,state->pixels.exposed);
    for(u32 i=0;i<sizeof(*state);++i)((u8*)state)[i]=0;object->table=table;object->state=state;state->live=state->refs=1;
    struct CsSurface* p=&state->pixels;p->width=width;p->height=height;p->bits=32;p->primary=primary;
    p->native=HeapAlloc(GetProcessHeap(),8,width*height*4);p->exposed=HeapAlloc(GetProcessHeap(),8,(width*4+8)*height);
    if(!p->native || !p->exposed)ExitProcess(315);RenderInstallForTest(object,14);
}
static void rs_update(struct RsSurface* object,u32 color){
    struct CsSurface* p=&object->state->pixels;u32 d[31]={124};SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*,u32*,u32,HANDLE))object->table[25])(object,0,d,1,0)!=13 || GetLastError()!=0x88)ExitProcess(316);
    for(u32 y=0;y<p->height;++y)for(u32 x=0;x<p->width;++x)for(u32 b=0;b<4;++b)p->exposed[y*d[4]+x*4+b]=(u8)(color>>(b*8));
    SetLastError(0x77);if(((i32 (WIN *)(void*,void*))object->table[32])(object,0)!=19 || GetLastError()!=0x88)ExitProcess(317);
    for(u32 i=0;i<p->width*p->height*4;++i)if(p->native[i]!=(u8)(color>>((i%4)*8)))ExitProcess(318);
    if(p->primary){++rs_frames;Sleep(75);}
}
static void rs_drop(struct RsSurface* object,u32 expected){
    SetLastError(0x77);if(((u32 (WIN *)(void*))object->table[2])(object)!=expected || GetLastError()!=0x88)ExitProcess(319);
}
static void rs_observe_alias(struct RsSurface* from,struct RsSurface* to){
    static const u8 iid[16]={0x30,0x86,0x2b,0x0b,0x35,0xad,0xd0,0x11,0x8e,0xa6,0,0x60,0x97,0x97,0xea,0x5b};
    rs_query_target=to;void* out=0;SetLastError(0x77);
    if(((i32 (WIN *)(void*,const void*,void**))from->table[0])(from,iid,&out)!=23 || out!=to || GetLastError()!=0x88)ExitProcess(320);
}
/* Fixture pacing only, outside hooks: wait until the independent consumer
 * acknowledges DELETE before producing the next large resource. This chunk
 * keeps production nonblocking backpressure policy unchanged. */
API i32 WIN UnmapViewOfFile(const void*);
static void rs_wait_ack(u32 target){
    char path[1024];u32 n=GetEnvironmentVariableA("MNM_RENDER_COMMAND_CHANNEL",path,sizeof(path));
    if(!n || n>=sizeof(path))ExitProcess(326);
    HANDLE file=CreateFileA(path,0x80000000,3,0,3,0x80,0);
    if(file==(HANDLE)-1)ExitProcess(327);
    HANDLE mapping=CreateFileMappingA(file,0,2,0,0,0);CloseHandle(file);
    u32* control=mapping?MapViewOfFile(mapping,4,0,0,64):0;
    if(!control)ExitProcess(328);
    u32 ready=0;for(u32 i=0;i<1000;++i){
        if(__atomic_load_n(control+6,__ATOMIC_ACQUIRE)!=1)ExitProcess(329);
        if(__atomic_load_n(control+9,__ATOMIC_ACQUIRE)>=target){ready=1;break;}Sleep(10);
    }
    UnmapViewOfFile(control);CloseHandle(mapping);if(!ready)ExitProcess(330);
}
static int rs_mode(const char* mode,const char* expected){
    u32 i=0;while(expected[i] && expected[i]==mode[i])++i;return !expected[i] && !mode[i];
}
static void test_resources(const char* mode){
    u32 churn=rs_mode(mode,"churn"),pixels=rs_mode(mode,"pixels"),alias=rs_mode(mode,"alias");
    u32 untracked=rs_mode(mode,"untracked"),held=rs_mode(mode,"held"),dc=rs_mode(mode,"dc");
    u32 contention=rs_mode(mode,"contention"),conflict=rs_mode(mode,"conflict"),bounded=rs_mode(mode,"bounded");
    u32 valid=churn || pixels || alias || untracked;
    rs_setup(&rs_primary,&rs_primary_state,32,16,1);rs_update(&rs_primary,0xa5123456);
    if(churn){
        /* Fill all 32 slots, then reclaim them out of order five times. */
        for(u32 cycle=0;cycle<5;++cycle){
            for(u32 i=0;i<31;++i){rs_setup(rs_objects+i,rs_states+i,4+i%3,4,0);rs_update(rs_objects+i,0xa5010000+cycle*31+i);}
            for(u32 i=31;i>0;--i)rs_drop(rs_objects+i-1,0);
        }
        rs_drop(&rs_primary,0);
        for(u32 i=0;i<40;++i){rs_setup(&rs_primary,&rs_primary_state,32+(i%2)*32,16,1);
            rs_update(&rs_primary,0xa5000000|((i*37u)&255)<<16|((i*71u)&255)<<8|((i*19u)&255));rs_drop(&rs_primary,0);}
    }else if(pixels){
        rs_drop(&rs_primary,0);
        u32 target=16+40+32*16*4+16+16;rs_wait_ack(target);
        for(u32 i=0;i<12;++i){rs_setup(&rs_primary,&rs_primary_state,2048,1024,1);
            rs_update(&rs_primary,0xa5000000|((i*37u)&255)<<16|((i*71u)&255)<<8|((i*19u)&255));rs_drop(&rs_primary,0);target+=40+2048*1024*4+16+16;rs_wait_ack(target);}
    }else if(alias){
        for(u32 i=0;i<20;++i){
            rs_setup(rs_objects,rs_states,4+i%3,4,0);rs_alias.table=rs_objects[0].table;rs_alias.state=rs_states;
            rs_update(rs_objects,0xa5010000+i);rs_observe_alias(rs_objects,&rs_alias);
            rs_drop(rs_objects,1);rs_update(&rs_alias,0xa5020000+i);rs_drop(&rs_alias,0);
            /* The retired alias address now denotes an independent resource. */
            rs_setup(&rs_alias,rs_states,8,4,0);rs_update(&rs_alias,0xa5030000+i);rs_drop(&rs_alias,0);
        }
        rs_update(&rs_primary,0xa5654321);rs_drop(&rs_primary,0);
    }else if(untracked){
        for(u32 i=0;i<160;++i){rs_setup(rs_objects,rs_states,4,4,0);rs_drop(rs_objects,0);}
        rs_update(&rs_primary,0xa5654321);rs_drop(&rs_primary,0);
    }else{
        rs_setup(rs_objects,rs_states,4,4,0);rs_update(rs_objects,0xa5010000);
        if(held){u32 d[31]={124};SetLastError(0x77);
            if(((i32 (WIN *)(void*,void*,u32*,u32,HANDLE))rs_objects[0].table[25])(rs_objects,0,d,1,0)!=13 || GetLastError()!=0x88)ExitProcess(321);
        }
        HANDLE bitmap=0,old=0;
        if(dc){u32 info[10]={40,4,(u32)-4,(32u<<16)|1,0};void* bits=0;
            rs_dc=CreateCompatibleDC(0);bitmap=CreateDIBSection(rs_dc,info,0,&bits,0,0);
            if(!rs_dc || !bitmap || !bits)ExitProcess(322);old=SelectObject(rs_dc,bitmap);void* out=0;SetLastError(0x77);
            if(((i32 (WIN *)(void*,void**))rs_objects[0].table[17])(rs_objects,&out)!=23 || out!=rs_dc || GetLastError()!=0x88)ExitProcess(323);
        }
        if(contention)RenderCaptureGuardForTest(1);
        if(conflict){rs_observe_alias(rs_objects,&rs_primary);}
        else rs_drop(rs_objects,0);
        if(contention){RenderCaptureGuardForTest(0);rs_update(&rs_primary,0xa5654321);}
        if(dc){SelectObject(rs_dc,old);DeleteObject(bitmap);DeleteDC(rs_dc);}
        if(!held && !dc && !contention && !conflict && !bounded)ExitProcess(324);
    }
    u32 counts[6]={cs_locks,cs_unlocks,rs_queries,rs_releases,rs_dc_calls,rs_frames};pl_file("resource-counts.bin",counts,sizeof(counts));
    SetLastError(0x77);u32 complete=RenderShutdown(3000);
    if(complete!=valid || GetLastError()!=0x77 || RenderShutdown(0)!=complete || GetLastError()!=0x77)ExitProcess(325);
    ExitProcess(0);
}
