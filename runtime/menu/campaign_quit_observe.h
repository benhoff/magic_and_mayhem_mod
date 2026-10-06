/* Forward the registered campaign confirmation receiver; never choose an answer. */
static u32 THIS campaign_quit_answer(void* object,u32 answer){
    u32 error=GetLastError();record(21,object,answer,0);record(24,object,get((void*)0x689920),((u32)*(u8*)0x6dbc18)|((u32)*(u8*)0x6dbc19<<8));SetLastError(error);
    u32 result=((ActionFn)0x4b24f0)(object,answer);error=GetLastError();if(menu_version>=11&&answer==0&&object==(void*)0x6a5088&&get((u8*)object+0x53)==2&&get((void*)0x689920)==5&&*(u8*)0x6dbc18)defeat_quit_pending=1;record(22,object,answer,result);record(25,object,get((void*)0x689920),((u32)*(u8*)0x6dbc18)|((u32)*(u8*)0x6dbc19<<8));SetLastError(error);return result;
}
static void campaign_quit_observe(void* object){
    u8* p=object;if(p!=(u8*)0x6a5088||!readable(p,0x5b)||get(p)!=0x5c6644||get(p+4)!=17||get(p+0x53)!=2||get(p+0x57)!=5||get((void*)0x68991c)||get((void*)0x689920)!=5)return;
    u8* pair=(u8*)get(p+0x4f);if(!readable(pair,8))return;
    u8* cb=(u8*)get(pair+4);if(!readable(cb,12)||get(cb)!=0x5c6674||get(cb+8)!=(u32)object||get(cb+4)!=0x4b24f0)return;
    const u8 prefix[]={0x56,0x8b,0xf1,0x8b,0x4e,0x3b,0xc7,0x46};if(!readable((void*)0x4b24f0,8)||!equal((void*)0x4b24f0,prefix,8))return;
    u32 old,restore;if(!VirtualProtect(cb+4,4,0x40,&old))return;
    put(cb+4,(u32)&campaign_quit_answer);VirtualProtect(cb+4,4,old,&restore);
}
