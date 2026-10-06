API u32 WIN RenderRecover(const char*);
API u32 WIN RenderRecoveryStateForTest(u32*);
API u32 WIN GetTickCount(void);
static void rc_name(char* out,const char* prefix,u32 n){
    u32 at=0;while(prefix[at]){out[at]=prefix[at];++at;}
    const char* hex="0123456789abcdef";for(u32 i=0;i<8;++i)out[at+i]=hex[(n>>(28-i*4))&15];
    copy_bytes(out+at+8,".bin",5);
}
static void rc_mark(const char* prefix,u32 phase,u32 value){char name[64];rc_name(name,prefix,phase);pl_file(name,&value,4);}
static void rc_wait(const char* prefix,u32 phase){
    char name[64];rc_name(name,prefix,phase);u32 start=GetTickCount();
    for(;;){HANDLE f=CreateFileA(name,0x80000000,3,0,3,0x80,0);
        if(f!=(HANDLE)-1){CloseHandle(f);return;}if(GetTickCount()-start>10000)ExitProcess(410);Sleep(5);}
}
static void rc_try(const char* path,u32 expected){
    SetLastError(0x77);if(RenderRecover(path)!=expected || GetLastError()!=0x77)ExitProcess(411);
}
static void rc_guards(void){
    const char* paths[]={0,"missing.bin","commands-00000000.bin","alias-old.bin","reused-id.bin","cancelled.bin","claimed.bin","v1.bin","bad-size.bin","reserved.bin"};
    for(u32 i=0;i<sizeof(paths)/sizeof(paths[0]);++i)rc_try(paths[i],0);
    char longpath[513];for(u32 i=0;i<512;++i)longpath[i]='x';longpath[512]=0;rc_try(longpath,0);
    RenderCaptureGuardForTest(1);rc_try("commands-00000001.bin",0);RenderCaptureGuardForTest(0);
}
/* The application queries this alias once, then locks canonical and unlocks
 * alias across every recovered session. Originals share independent storage. */
static struct CsSurface rc_alias;
static u32 rc_queries;
static i32 WIN rc_query(void* object,const u8* iid,void** out){
    (void)object;(void)iid;++rc_queries;*out=&rc_alias;SetLastError(0x88);return 23;
}
static i32 WIN rc_alias_lock(void* object,void* rect,u32* d,u32 flags,HANDLE event){(void)object;return cs_lock(&cs_big,rect,d,flags,event);}
static i32 WIN rc_alias_unlock(void* object,void* arg){(void)object;return cs_unlock(&cs_big,arg);}
static void rc_alias_update(u32 color){
    u32 desc[31]={124};SetLastError(0x77);
    if(((i32 (WIN *)(void*,void*,void*,u32,HANDLE))cs_big.table[25])(&cs_big,0,desc,1,0)!=13 || GetLastError()!=0x88)ExitProcess(420);
    for(u32 y=0;y<cs_big.height;++y)for(u32 x=0;x<cs_big.width;++x)for(u32 b=0;b<4;++b)cs_big.exposed[y*desc[4]+x*4+b]=(u8)(color>>(b*8));
    SetLastError(0x77);if(((i32 (WIN *)(void*,void*))rc_alias.table[32])(&rc_alias,0)!=19 || GetLastError()!=0x88)ExitProcess(421);
    for(u32 i=0;i<cs_big.width*cs_big.height*4;++i)if(cs_big.native[i]!=(u8)(color>>((i%4)*8)))ExitProcess(422);
}
static void test_recovery(const char* mode){
    SetLastError(0x77);if(!RenderStartup() || GetLastError()!=0x77)ExitProcess(412);
    cs_setup(&cs_small,4,4,16,0);cs_setup(&cs_big,512,512,32,1);
    u32 aliases=mode[0]=='a';
    if(aliases){
        static void* table[33];table[0]=(void*)&rc_query;table[25]=(void*)&rc_alias_lock;table[32]=(void*)&rc_alias_unlock;
        cs_big.table=rc_alias.table=table;RenderInstallForTest(&cs_big,14);
        static const u8 iid[16]={0x30,0x86,0x2b,0x0b,0x35,0xad,0xd0,0x11,0x8e,0xa6,0,0x60,0x97,0x97,0xea,0x5b};void* out=0;
        SetLastError(0x77);if(((i32 (WIN *)(void*,const u8*,void**))table[0])(&cs_big,iid,&out)!=23 || out!=&rc_alias || GetLastError()!=0x88)ExitProcess(423);
    }
    u32 phases=mode[0]=='b'?16:(mode[0]=='r' || aliases)?3:2;
    for(u32 phase=0;phase<phases;++phase){
        if(phase){char path[64];rc_name(path,"commands-",phase);rc_try(path,1);
            u32 state[8];if(!RenderRecoveryStateForTest(state) || GetLastError()!=0x77)ExitProcess(418);
            for(u32 i=0;i<8;++i)if(state[i]!=(aliases && i==6?1u:0u))ExitProcess(419);
            if(!RenderStartup() || GetLastError()!=0x77)ExitProcess(413);}
        cs_update(&cs_small,0x1200+phase);
        for(u32 f=0;f<2;++f){if(aliases)rc_alias_update(0xa5000000u|((phase*2+f)*0x254713u&0xffffff));else cs_update(&cs_big,0xa5000000u|((phase*2+f)*0x254713u&0xffffff));Sleep(75);}
        if(!phase && mode[0]=='e')rc_try("commands-00000001.bin",0);
        rc_mark("drawn-",phase,phase);rc_wait("go-",phase);
        u32 failed=!phase && (mode[0]=='c' || mode[0]=='s' || mode[0]=='i' || mode[0]=='h');
        if(!phase && mode[0]=='s'){cs_update(&cs_big,0xa5ffffff);Sleep(450);}
        if(!phase && (mode[0]=='c' || mode[0]=='i'))Sleep(150);
        if(!phase && mode[0]=='h'){
            u32 d[31]={124};SetLastError(0x77);
            if(((i32 (WIN *)(void*,void*,void*,u32,HANDLE))cs_big.table[25])(&cs_big,0,d,1,0)!=13 || GetLastError()!=0x88)ExitProcess(414);
        }
        SetLastError(0x77);if(RenderShutdown(failed?0:3000)!=(failed?0u:1u) || GetLastError()!=0x77)ExitProcess(415);
        if(!phase && mode[0]=='h'){
            rc_try("commands-00000001.bin",0);
            SetLastError(0x77);if(((i32 (WIN *)(void*,void*))cs_big.table[32])(&cs_big,0)!=19 || GetLastError()!=0x88)ExitProcess(416);
        }
        if(!phase && mode[0]=='g')rc_guards();
        rc_mark("closed-",phase,phase);rc_wait("next-",phase);
    }
    if(mode[0]=='b')rc_try("commands-00000010.bin",0);
    if(aliases){if(rc_queries!=1)ExitProcess(424);pl_file("alias-query-count.bin",&rc_queries,4);}
    u32 result[3]={cs_locks,cs_unlocks,phases};pl_file("engine-counts.bin",result,12);
    SetLastError(0x77);if(RenderShutdown(0)!=1 || GetLastError()!=0x77)ExitProcess(417);ExitProcess(0);
}
