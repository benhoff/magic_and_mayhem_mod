#include "../../protocols/include/mnm/menu_v4.h"
static u8 mini_payload[MNM_MENU_V4_MINI_SIZE];
static int mini_snapshot(void* object,u8* out,int campaign){
    u8* p=object;
    if(!readable(p,0x5b)||get(p)!=0x5c6644||get(p+4)!=MNM_MENU_MINI_SCREEN||
       !readable((void*)0x68991c,8))return 0;
    if(campaign){
        if(p!=(u8*)0x6a5088||get((void*)0x68991c)||get((void*)0x689920)!=5||get(p+0x53)!=2||get(p+0x57)!=5)return 0;
    }else if(!get((void*)0x68991c)||get((void*)0x689920)==5||get(p+0x53)==4||get(p+0x57)!=3)return 0;
    if(!readable((void*)0x6f349c,0x48))return 0;
    u32 depth=get((void*)0x6f349c);if(depth<1||depth>15||get((void*)(0x6f34a0+depth*4))!=(u32)object)return 0;
    u8* parent=(u8*)get((void*)(0x6f34a0+(depth-1)*4));
    if(parent!=(u8*)0x6cbb78||!readable(parent,8)||get(parent)!=0x5c5dd8||get(parent+4)!=MNM_MENU_MINI_PARENT_SCREEN)return 0;
    if(campaign&&(depth!=5||get((void*)0x6f34a8)!=0x657ce0||get((void*)0x6f34ac)!=0x659408||!readable((void*)0x659408,12)||get((void*)0x659408)!=0x5c6a60||get((void*)0x65940c)!=4))return 0;
    // Validate the original three-word callback object registered for buttons.
    u8* pair=(u8*)get(p+0x4f);if(!readable(pair,8))return 0;
    u8* cb=(u8*)get(pair);if(!readable(cb,12)||get(cb)!=0x5c6674||get(cb+4)!=0x4b23f0||get(cb+8)!=(u32)object)return 0;
    for(u32 i=0;i<MNM_MENU_V4_MINI_SIZE;++i)out[i]=0;
    u32 modal=get(p+0x3b)!=0;
    put(out,campaign?0:1);put(out+4,modal);put(out+8,depth);put(out+12,get(parent+4));
    put(out+16,modal?0:campaign?MNM_MENU_MINI_CAN_CANCEL:MNM_MENU_MINI_CAN_CANCEL|MNM_MENU_MINI_CAN_PREFERENCES|MNM_MENU_MINI_CAN_QUIT);
    put(out+20,get((void*)0x689920));if(campaign)put(out+24,2);return 1;
}
static u32 mini_dispatch(void* object,u32 action,int campaign){
    // No dispatch or memory patch is possible in the normal build.
    if(!campaign&&!MNM_MENU_MINI_EXPERIMENTAL)return MNM_MENU_UNSUPPORTED;
    u8 current[MNM_MENU_V4_MINI_SIZE];if(!mini_snapshot(object,current,campaign)||get(current+4))return MNM_MENU_UNAVAILABLE;
    u32 index;
    if(campaign){if(action!=MNM_MENU_MINI_CANCEL)return MNM_MENU_UNSUPPORTED;index=4;}
    else if(action==MNM_MENU_MINI_CANCEL)index=2;
    else if(action==MNM_MENU_MINI_PREFERENCES)index=0;
    else if(action==MNM_MENU_MINI_QUIT)index=1;
    else return MNM_MENU_UNSUPPORTED;
    // Quit opens the original confirmation. Never call its Yes callback here.
    record(2,object,index,0);u32 result=((ActionFn)0x4b23f0)(object,index);record(3,object,index,result);
    return MNM_MENU_OK;
}
