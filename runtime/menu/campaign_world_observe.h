/* First three original gameplay ticks only; opt-in diagnostic, no simulation replacement. */
static u32 campaign_world_ticks;
static u32 THIS campaign_world_tick(void* object){
    u32 error=GetLastError();
    int sample=campaign_world_ticks<3&&object==(void*)0x6cbb78&&readable(object,0x47)&&
        get((void*)0x689920)==5&&get(object)==0x5c5dd8&&get((u8*)object+4)==2&&get((void*)0x6f34e0)==(u32)object;
    if(menu_version>=8)menu_poll(object,0);
    if(sample)record(11,object,campaign_world_ticks,0);
    SetLastError(error);u32 result=((TickFn)0x46afc0)(object);error=GetLastError();
    if(menu_version>=8)menu_poll(object,0);
    if(sample){record(12,object,campaign_world_ticks,result);++campaign_world_ticks;}
    SetLastError(error);return result;
}
static int install_campaign_world_observe(void){
    const u8 bytes[]={0x83,0xec,0x74,0x53,0x55,0x56,0x8b,0xf1};u32 old,restore;
    if(!readable((void*)0x5c5de8,4)||get((void*)0x5c5de8)!=0x46afc0||
       !readable((void*)0x46afc0,8)||!equal((void*)0x46afc0,bytes,8)||
       !VirtualProtect((void*)0x5c5de8,4,0x40,&old))return 0;
    put((void*)0x5c5de8,(u32)&campaign_world_tick);VirtualProtect((void*)0x5c5de8,4,old,&restore);return 1;
}
