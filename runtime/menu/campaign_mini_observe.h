/* Campaign Mini lifecycle observation; no pause, exit or state writes. */
static u32 campaign_mini_last[2][10];
static int campaign_mini_have[2];
static void campaign_mini_record(void* object,u32 phase){
    u8* p=object;if(p!=(u8*)0x6a5088||!readable(p,0x5b)||get(p)!=0x5c6644||get(p+4)!=17)return;
    u32 key[10]={get(p+8),get(p+0x33),get(p+0x43),get(p+0x53),get(p+0x57),get(p+0x3b),get((void*)0x68991c),get((void*)0x689920),get((void*)0x6f34e0),get((void*)0x6f349c)};
    if(campaign_mini_have[phase]&&equal(key,campaign_mini_last[phase],sizeof(key)))return;
    copy(campaign_mini_last[phase],key,sizeof(key));campaign_mini_have[phase]=1;
    record(13+phase*3,object,key[3],key[4]);
    record(14+phase*3,object,key[6],key[7]);
    record(15+phase*3,object,key[5],((u32)*(u8*)0x6dbc18)|((u32)*(u8*)0x6dbc19<<8));
}
static u32 THIS campaign_mini_tick(void* object){
    u32 error=GetLastError();if(menu_version>=9)menu_poll(object,1);campaign_mini_record(object,0);SetLastError(error);
    u32 result=((TickFn)0x5595d0)(object);error=GetLastError();if(menu_version>=9)menu_poll(object,0);campaign_mini_record(object,1);SetLastError(error);return result;
}
static u32 THIS campaign_world_resume(void* object){
    u32 error=GetLastError();record(19,object,0,0);SetLastError(error);
    u32 result=((TickFn)0x46aef0)(object);error=GetLastError();record(20,object,0,result);SetLastError(error);return result;
}
static int install_campaign_mini_observe(void){
    const u8 action[]={0x64,0xa1,0,0,0,0};
    const u8 common[]={0x83,0xec,0x1c,0x56,0x8b,0xf1,0x57,0x8b};
    const u8 resume[]={0x56,0x8b,0xf1,0xb9,0xd0,0x2d,0x6a,0};u32 old,restore;
    if(!readable((void*)0x4b23f0,6)||!equal((void*)0x4b23f0,action,6)||!readable((void*)0x5c6654,4)||get((void*)0x5c6654)!=0x5595d0||!readable((void*)0x5595d0,8)||!equal((void*)0x5595d0,common,8)||
       !readable((void*)0x5c5de4,4)||get((void*)0x5c5de4)!=0x46aef0||!readable((void*)0x46aef0,8)||!equal((void*)0x46aef0,resume,8))return 0;
    if(!VirtualProtect((void*)0x5c6654,4,0x40,&old))return 0;
    put((void*)0x5c6654,(u32)&campaign_mini_tick);VirtualProtect((void*)0x5c6654,4,old,&restore);
    if(!VirtualProtect((void*)0x5c5de4,4,0x40,&old))return 0;
    put((void*)0x5c5de4,(u32)&campaign_world_resume);VirtualProtect((void*)0x5c5de4,4,old,&restore);return 1;
}
