/* Opt-in diagnostic only. Forward the original custom tick without dispatch. */
static HANDLE campaign_log;
static u32 campaign_sequence,campaign_last[4][31];
static int campaign_have_state[4];
static void campaign_record(void* object,u32 phase,u32 result){
    u32 error=GetLastError();u8* p=object;
    if(!campaign_log||campaign_sequence>=256||p!=(u8*)0x659408||
       !readable(p,0x1e25)||get(p)!=0x5c6a60||get(p+4)!=4){SetLastError(error);return;}
    u32 owner=0,depth=0,parent=0,owner_id=0;
    if(readable((void*)0x6f349c,0x48)){
        owner=get((void*)0x6f34e0);depth=get((void*)0x6f349c);
        if(depth>0&&depth<16)parent=get((void*)(0x6f34a0+(depth-1)*4));
        if(readable((void*)owner,8))owner_id=get((u8*)owner+4);
    }
    u32 fields[32]={campaign_sequence+1,phase,GetCurrentThreadId(),(u32)p,get(p),get(p+4),owner,depth,
        get(p+0xa2b),get(p+0xa37),get(p+0x823),get(p+0x12),get(p+0x59),get(p+0xa23),
        get(p+0xb4d),get(p+0xb49),get(p+0xb51),get(p+8),p[0xc],get(p+0xa44),p[0xa48],
        get(p+0xa33),get(p+0xa27),parent,owner_id,get((void*)0x689920),result,error,
        get((void*)0x6f2d28),get((void*)0x6f2d34),get((void*)0x6f2d2c),get((void*)0x6f2d30)};
    /* The before/after phase is intentional: initialization changes stay visible. */
    u32 key[31];copy(key,fields+1,sizeof(key));key[25]=key[26]=0;
    u32 slot=phase-1;
    if(campaign_have_state[slot]&&equal(key,campaign_last[slot],sizeof(key))){SetLastError(error);return;}
    copy(campaign_last[slot],key,sizeof(key));campaign_have_state[slot]=1;
    u8 bytes[128];for(u32 i=0;i<32;++i)put(bytes+i*4,fields[i]);u32 wrote=0;
    if(!WriteFile(campaign_log,bytes,sizeof(bytes),&wrote,0)||wrote!=sizeof(bytes)){
        CloseHandle(campaign_log);campaign_log=0;
    }else ++campaign_sequence;
    SetLastError(error);
}
static u32 THIS campaign_tick(void* object){
    u32 error=GetLastError();campaign_record(object,1,0);SetLastError(error);
    u32 result=((TickFn)0x5517a0)(object);error=GetLastError();
    campaign_record(object,2,result);SetLastError(error);return result;
}
static int install_campaign_observe(void){
    char path[1024];u32 length=GetEnvironmentVariableA("MNM_MENU_CAMPAIGN_OBSERVE",path,sizeof(path));
    if(!length)return 1;
    const u8 bytes[]={0x81,0xec,0x08,0x01,0,0,0x33,0xc0};u32 old,restore;
    if(length>=sizeof(path)||!readable((void*)0x5c6a70,4)||get((void*)0x5c6a70)!=0x5517a0||
       !readable((void*)0x5517a0,8)||!equal((void*)0x5517a0,bytes,8)||
       !VirtualProtect((void*)0x5c6a70,4,0x40,&old))return 0;
    campaign_log=CreateFileA(path,0x40000000,1,0,1,0x80,0);
    if(campaign_log==(HANDLE)-1)campaign_log=0;
    int ok=0;
    if(campaign_log){
        const u8 header[16]={'M','N','M','C','A','M','P','2',2,0,0,0,128,0,0,0};u32 wrote=0;
        if(WriteFile(campaign_log,header,16,&wrote,0)&&wrote==16){put((void*)0x5c6a70,(u32)&campaign_tick);ok=1;}
        else {CloseHandle(campaign_log);campaign_log=0;}
    }
    VirtualProtect((void*)0x5c6a70,4,old,&restore);return ok;
}
