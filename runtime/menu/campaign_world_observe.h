/* First three original gameplay ticks only; opt-in diagnostic, no simulation replacement. */
static u32 campaign_world_ticks,campaign_exit_ticks;
static void campaign_exit_record(void* object,u32 event){
    if(campaign_exit_ticks>=16)return;
    u8* p=object;if(p!=(u8*)0x6cbb78||!readable(p,0x1031b))return;
    record(event,object,((u32)p[0x100a0])|((u32)p[0x100a1]<<8),((u32)p[0x100a2])|((u32)p[0x100a3]<<8)|((u32)p[0x100a4]<<16));
}
static u32 THIS campaign_world_tick(void* object){
    u32 error=GetLastError();
    int sample=campaign_world_ticks<3&&object==(void*)0x6cbb78&&readable(object,0x47)&&
        get((void*)0x689920)==5&&get(object)==0x5c5dd8&&get((u8*)object+4)==2&&get((void*)0x6f34e0)==(u32)object;
    if(menu_version>=8)menu_poll(object,0);
    int exit_sample=object==(void*)0x6cbb78&&campaign_exit_ticks<16&&readable(object,0x100a5)&&(((u8*)object)[0x100a0]||((u8*)object)[0x100a1]);
    if(exit_sample)campaign_exit_record(object,26);
    if(sample)record(11,object,campaign_world_ticks,0);
    SetLastError(error);u32 result=((TickFn)0x46afc0)(object);error=GetLastError();
    if(menu_version>=8)menu_poll(object,0);
    if(sample){record(12,object,campaign_world_ticks,result);++campaign_world_ticks;}
    if(object==(void*)0x6cbb78)gameplay_sample();
    if(exit_sample){campaign_exit_record(object,27);u8* owner=(u8*)get((void*)0x6f34e0);if(readable(owner,8))record(28,object,get(owner),get(owner+4));++campaign_exit_ticks;}
    SetLastError(error);return result;
}
static int install_campaign_world_observe(void){
    const u8 bytes[]={0x83,0xec,0x74,0x53,0x55,0x56,0x8b,0xf1};u32 old,restore;
    if(!readable((void*)0x5c5de8,4)||get((void*)0x5c5de8)!=0x46afc0||
       !readable((void*)0x46afc0,8)||!equal((void*)0x46afc0,bytes,8)||
       !VirtualProtect((void*)0x5c5de8,4,0x40,&old))return 0;
    put((void*)0x5c5de8,(u32)&campaign_world_tick);VirtualProtect((void*)0x5c5de8,4,old,&restore);return 1;
}
