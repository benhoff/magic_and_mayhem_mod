#include "grimoire_campaign.h"
static u32 grimoire_guard_failure;
static u8 grimoire_payload[MNM_MENU_V14_GRIMOIRE_SIZE];
static int grimoire_snapshot(void* object,u8* out){
    if(grimoire_campaign_enabled)return grimoire_campaign_snapshot(object,out);
    u8* p=(u8*)object;grimoire_guard_failure=1;
    if(p!=(u8*)0x6e0088||!readable(p,0x1a3e)||get(p)!=0x5c6b70||get(p+4)||get(p+8)||
       !readable((void*)0x6f349c,0x48)||get((void*)0x6f349c)!=5||get((void*)0x6f34e0)!=(u32)p||
       get((void*)0x6f34b4)!=(u32)p||get((void*)0x6f34b0)!=0x6578c0||
       get((void*)0x6f34ac)!=0x659408||get((void*)0x6f34a8)!=0x657ce0||
       !readable((void*)0x689920,4)||get((void*)0x689920)!=5||
       get((void*)0x6e1aba)!=1||get((void*)0x6e1abe)!=1||!((get(p+0x42)==0&&get(p+0x46)<=4)||(get(p+0x42)==1&&get(p+0x46)==1))||get(p+0x4a)!=1)return 0;
    grimoire_guard_failure=2;
    u8* parent=(u8*)0x6578c0,*realm=(u8*)0x659408,*main=(u8*)0x657ce0;
    if(!readable(parent,0x81)||get(parent)!=0x5c667c||get(parent+4)!=18||get(parent+8)!=0||
       get(parent+0x7c)!=4||parent[0x77]||get(parent+0x6f)!=1||parent[0x80]||
       !readable(realm,0xa3b)||get(realm)!=0x5c6a60||get(realm+4)!=4||get(realm+8)||get(realm+0xa37)||
       !readable(main,8)||get(main)!=0x5c63e4||get(main+4)!=3)return 0;
    grimoire_guard_failure=3;
    u8* cb=(u8*)get(p+0xa00),*catalog=(u8*)get(p+0x16);
    if(!readable(cb,12)||get(cb)!=0x5c6b8c||get(cb+4)!=0x4c77c0||get(cb+8)!=(u32)p||
       !readable(catalog,0x13dc)||!readable((void*)get(p+0x1a),0x960)||
       !readable((void*)get(p+0x1a3a),4)||get(p+0x1a2a)>32)return 0;
    grimoire_guard_failure=4;
    u32 mask=0;
    for(u32 i=0;i<124;++i){u8* e=(u8*)get(catalog+12+4*i);if(!e)continue;
        if(!readable(e,32))return 0;
        u32 id=get(e+4);
        if(id>4||(mask&(1u<<id))||(id&&(!readable((void*)get(e),1)||!readable((void*)get(e+12),1))))return 0;
        mask|=1u<<id;
    }
    grimoire_guard_failure=6;u32 found=0;
    for(u32 i=0;i<124;++i){u8* e=(u8*)get(catalog+0x1fc+12+4*i);if(!e)continue;
        if(!readable(e,32))return 0;
        if(get(e+4)==1){if(found||!readable((void*)get(e),1)||!readable((void*)get(e+12),1))return 0;found=1;}
    }
    if(!found)return 0;
    mask|=32;
    grimoire_guard_failure=5;
    if(mask!=63)return 0;
    for(u32 i=0;i<MNM_MENU_V14_GRIMOIRE_SIZE;++i)out[i]=0;
    put(out,get(p+0x42));put(out+4,get(p+0x46));put(out+8,get(p+0x4a));put(out+12,mask);put(out+16,18);put(out+20,5);put(out+24,5);
    grimoire_guard_failure=0;return 1;
}
static u32 grimoire_dispatch(void* object,u32 action,u32 argument){
    u8 state[MNM_MENU_V14_GRIMOIRE_SIZE];if(!grimoire_snapshot(object,state))return MNM_MENU_UNAVAILABLE;
    u8* p=(u8*)object;if(p[0xc]||get(p+0x1e)||get(p+0x3a)||get(p+0x3e)||get(p+0x1a2e))return MNM_MENU_UNAVAILABLE;
    u32 original;
    if(action==MNM_MENU_GRIMOIRE_CLOSE){if(argument)return MNM_MENU_INVALID;original=3999;}
    else if(action==MNM_MENU_GRIMOIRE_SELECT){if(grimoire_campaign_enabled){if(argument>=MNM_MENU_V22_BOOK_ENTRIES)return MNM_MENU_INVALID;if(get(grimoire_campaign_payload+32+4*argument)>4)return MNM_MENU_UNAVAILABLE;original=argument;}
        else {if(argument>5)return MNM_MENU_INVALID;if(!(get(state+12)&(1u<<argument)))return MNM_MENU_UNAVAILABLE;original=argument==5?125:argument;}}
    else return MNM_MENU_UNSUPPORTED;
    record(2,object,original,0);u32 result=((ActionFn)0x4c77c0)(object,original);record(3,object,original,result);return MNM_MENU_OK;
}
