/* Campaign diagnostic only: retain original Region Entry tick and Realm resume. */
static u32 entry_last[2][8];
static int entry_have[2];
static u32 entry_difficulty(u8* p){
    u32 count=get(p+0x67),selected=get(p+0x63);u8* group=(u8*)get(p+0x5f);
    if(count!=4||selected>=4||!readable(group,16))return 0xffffffff;
    u8* radio=(u8*)get(group+selected*4);
    return readable(radio,0x31)?get(radio+0x2d):0xffffffff;
}
static void entry_record(void* object,u32 phase){
    u32 error=GetLastError();u8* p=object;
    if(p==(u8*)0x6578c0&&readable(p,0x81)&&get(p)==0x5c667c&&get(p+4)==18&&readable((void*)0x6f349c,0x48)){
        u32 choice=entry_difficulty(p),caller=get(p+0x7c)|(p[0x77]<<16);
        u32 key[8]={get(p+8),get(p+0x33),get(p+0x43),p[0xc],choice,caller,get((void*)0x6f34e0),get((void*)0x6f349c)};
        u32 slot=phase-1;
        if(!entry_have[slot]||!equal(key,entry_last[slot],sizeof(key))){
            copy(entry_last[slot],key,sizeof(key));entry_have[slot]=1;record(phase==1?9:10,p,choice,caller);
        }
    }
    SetLastError(error);
}
static u32 THIS entry_tick(void* object){
    u32 error=GetLastError();entry_record(object,1);SetLastError(error);
    u32 result=((TickFn)0x5595d0)(object);error=GetLastError();entry_record(object,2);SetLastError(error);return result;
}
static void THIS campaign_enter(void* object){
    u32 error=GetLastError();campaign_record(object,3,0);SetLastError(error);
    ((void (THIS *)(void*))0x552210)(object);error=GetLastError();campaign_record(object,4,0);SetLastError(error);
}
static int install_campaign_navigation(void){
    const u8 entry_bytes[]={0x83,0xec,0x1c,0x56,0x8b,0xf1,0x57};
    const u8 resume_bytes[]={0x8b,0x81,0x37,0x0a,0,0,0x85,0xc0};u32 a,b,restore;
    if(!readable((void*)0x5c668c,4)||get((void*)0x5c668c)!=0x5595d0||
       !readable((void*)0x5c6a60,4)||get((void*)0x5c6a60)!=0x552210||
       !equal((void*)0x5595d0,entry_bytes,sizeof(entry_bytes))||!equal((void*)0x552210,resume_bytes,sizeof(resume_bytes)))return 0;
    if(!VirtualProtect((void*)0x5c668c,4,0x40,&a))return 0;
    if(!VirtualProtect((void*)0x5c6a60,4,0x40,&b)){VirtualProtect((void*)0x5c668c,4,a,&restore);return 0;}
    put((void*)0x5c668c,(u32)&entry_tick);put((void*)0x5c6a60,(u32)&campaign_enter);
    VirtualProtect((void*)0x5c6a60,4,b,&restore);VirtualProtect((void*)0x5c668c,4,a,&restore);return 1;
}
