/* Diagnostic wire only; no native book actions or ownership projection. */
static HANDLE grimoire_log;
static u32 grimoire_sequence,grimoire_last[2][31];
static int grimoire_have_state[2];
static void grimoire_record(void* object,u32 phase){
    u32 error=GetLastError();u8* p=object;
    if(!grimoire_log||grimoire_sequence>=256||p!=(u8*)0x6e0088||
       !readable(p,0x1a3e)||get(p)!=0x5c6b70){SetLastError(error);return;}
    u32 owner=0,depth=0,parent=0,owner_id=0,context=0,request=0,realm=0,region=0;
    if(readable((void*)0x6f349c,0x48)){
        owner=get((void*)0x6f34e0);depth=get((void*)0x6f349c);
        if(depth>0&&depth<16)parent=get((void*)(0x6f34a0+(depth-1)*4));
        if(readable((void*)owner,8))owner_id=get((u8*)owner+4);
    }
    if(readable((void*)0x689920,4))context=get((void*)0x689920);
    if(readable((void*)0x6e1ab6,12)){request=get((void*)0x6e1ab6);realm=get((void*)0x6e1aba);region=get((void*)0x6e1abe);}
    u8* cb=(u8*)get(p+0xa00);u32 cb_type=0,cb_method=0,cb_receiver=0;
    if(readable(cb,12)){cb_type=get(cb);cb_method=get(cb+4);cb_receiver=get(cb+8);}
    u32 fields[32]={grimoire_sequence+1,phase,GetCurrentThreadId(),(u32)p,get(p),get(p+4),
        owner,depth,parent,owner_id,get(p+8),p[0xc],get(p+0xd),get(p+0x1e),get(p+0x16),get(p+0x1a),
        get(p+0x42),get(p+0x46),get(p+0x4a),get(p+0x3a),get(p+0x3e),get(p+0x1a2a),
        get(p+0x1a2e),get(p+0x1a3a),cb_type,cb_method,cb_receiver,context,request,realm,region,error};
    u32 key[31];copy(key,fields+1,sizeof(key));key[30]=0;u32 slot=phase-1;
    if(grimoire_have_state[slot]&&equal(key,grimoire_last[slot],sizeof(key))){SetLastError(error);return;}
    copy(grimoire_last[slot],key,sizeof(key));grimoire_have_state[slot]=1;
    u8 bytes[128];for(u32 i=0;i<32;++i)put(bytes+4*i,fields[i]);u32 wrote=0;
    if(!WriteFile(grimoire_log,bytes,sizeof(bytes),&wrote,0)||wrote!=sizeof(bytes)){
        CloseHandle(grimoire_log);grimoire_log=0;
    }else ++grimoire_sequence;
    SetLastError(error);
}
static u32 THIS grimoire_tick(void* object){
    u32 error=GetLastError();grimoire_record(object,1);SetLastError(error);
    u32 result=((TickFn)0x4c7890)(object);error=GetLastError();
    grimoire_record(object,2);SetLastError(error);return result;
}
static int install_grimoire_observe(void){
    char path[1024];u32 length=GetEnvironmentVariableA("MNM_MENU_GRIMOIRE_OBSERVE",path,sizeof(path));
    if(!length)return 1;
    const u8 bytes[8]={0x56,0x8b,0xf1,0x8b,0x46,0x1e,0x85,0xc0};u32 old,restore;
    if(length>=sizeof(path)||!readable((void*)0x5c6b70,0x18)||
       get((void*)0x5c6b70)!=0x4ca300||get((void*)0x5c6b74)!=0x4ca320||
       get((void*)0x5c6b80)!=0x4c7890||get((void*)0x5c6b84)!=0x4c7920||
       !readable((void*)0x4c7890,8)||!equal((void*)0x4c7890,bytes,8)||
       !VirtualProtect((void*)0x5c6b80,4,0x40,&old))return 0;
    grimoire_log=CreateFileA(path,0x40000000,1,0,1,0x80,0);
    if(grimoire_log==(HANDLE)-1)grimoire_log=0;
    int ok=0;
    if(grimoire_log){
        const u8 header[16]={'M','N','M','B','O','O','K','1',1,0,0,0,128,0,0,0};u32 wrote=0;
        if(WriteFile(grimoire_log,header,16,&wrote,0)&&wrote==16){put((void*)0x5c6b80,(u32)&grimoire_tick);ok=1;}
        else {CloseHandle(grimoire_log);grimoire_log=0;}
    }
    VirtualProtect((void*)0x5c6b80,4,old,&restore);return ok;
}
