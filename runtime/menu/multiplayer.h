#include "../../protocols/include/mnm/menu_v15.h"
static u8 multiplayer_payload[MNM_MENU_V15_MULTIPLAYER_SIZE];
static u32 multiplayer_diagnostics;
static int multiplayer_refuse(u8* p,u32 why){
    if(get(p+8)==1&&!(multiplayer_diagnostics&(1u<<why))){multiplayer_diagnostics|=1u<<why;record(40,p,why,0);}return 0;
}
static int multiplayer_control(u8* p,u32 table,u32 size){return readable(p,size)&&get(p)==table;}
static int multiplayer_available(u8* p){return get(p+8)==1&&get(p+0x39)<2&&get(p+0x3d)!=2;}
static int multiplayer_snapshot(void* object,u8* out){
    u8* p=object;int create=p==(u8*)0x6de750;
    if((!create&&p!=(u8*)0x6a5018)||!readable(p,0x70))return 0;
    if(get(p)!=(create?0x5c660c:0x5c659c)||get(p+4)!=(create?15u:16u)||get(p+8)!=1)return 0;
    if(!readable((void*)0x6f349c,0x48)||get((void*)0x6f349c)!=4||get((void*)0x6f34e0)!=(u32)p||get((void*)0x6f34b0)!=(u32)p||
       get((void*)0x6f34ac)!=0x6e0020||get((void*)0x6f34a8)!=0x657ce0||
       !readable((void*)0x657ce0,8)||get((void*)0x657ce0)!=0x5c63e4||get((void*)0x657ce4)!=3||
       !readable((void*)0x6e0020,8)||get((void*)0x6e0020)!=0x5c641c||get((void*)0x6e0024)!=22)return multiplayer_refuse(p,1);
    if(!readable((void*)0x68991c,8)||get((void*)0x68991c)!=1||get((void*)0x689920)!=2||
       !readable((void*)0x6e1f0c,4)||get((void*)0x6e1f0c))return multiplayer_refuse(p,2);
    u8* callbacks=(u8*)get(p+(create?0x5b:0x57));u8* buttons=(u8*)get(p+0x4b);
    if(!readable(callbacks,8)||!readable(buttons,8))return multiplayer_refuse(p,3);
    for(u32 i=0;i<2;++i){u8* cb=(u8*)get(callbacks+4*i);u8* b=(u8*)get(buttons+4*i);
        if(!readable(cb,12)||get(cb)!=(create?0x5c663c:0x5c65cc)||get(cb+4)!=(create?(i?0x4b1920:0x4b1840):(i?0x4af9a0:0x4af910))||get(cb+8)!=(u32)p||
           !multiplayer_control(b,0x5c6c6c,0x59)||get(b+0x2d)!=i||get(b+0x25)!=get(callbacks))return multiplayer_refuse(p,4);
    }
    u8* radios=(u8*)get(p+(create?0x57:0x53));u8* group=(u8*)get(p+0x5f);
    if(!readable(radios,12)||!readable(group,12)||get(p+0x67)!=3||get(p+0x6b)!=3||get(p+0x63)>2)return multiplayer_refuse(p,5);
    u32 transports=0,actions=0;
    for(u32 i=0;i<3;++i){u8* r=(u8*)get(radios+4*i);
        if(!multiplayer_control(r,0x5c6d40,0x7e)||get(r+0x2d)!=i||get(group+4*i)!=(u32)r)return multiplayer_refuse(p,6);
        if(multiplayer_available(r))transports|=1u<<i;
    }
    for(u32 i=0;i<2;++i)if(multiplayer_available((u8*)get(buttons+4*i)))actions|=1u<<i;
    u8* game=0,*user;
    if(create){u8* edits=(u8*)get(p+0x53);if(!readable(edits,8))return multiplayer_refuse(p,7);game=(u8*)get(edits);user=(u8*)get(edits+4);}
    else user=(u8*)get(p+0x5b);
    if(!multiplayer_control(user,0x5c6ca0,0x64)||get(user+0x60)!=15||get(user+0x25)!=get(callbacks+4)||
       !readable((void*)get(user+0x45),256))return multiplayer_refuse(p,8);
    if(create&&(!multiplayer_control(game,0x5c6ca0,0x64)||get(game+0x60)!=256||get(game+0x25)!=get(callbacks+4)||
       !readable((void*)get(game+0x45),256)))return multiplayer_refuse(p,9);
    for(u32 i=0;i<MNM_MENU_V15_MULTIPLAYER_SIZE;++i)out[i]=0;
    put(out,22);put(out+4,4);put(out+8,2);put(out+12,get(p+0x63));put(out+16,transports);put(out+20,actions);put(out+24,get(p+0x3b)!=0);put(out+28,p[0x6f]!=0);put(out+32,create?256:0);put(out+36,15);
    if((create&&!battle_string(out+48,64,(void*)get(game+0x45),64))||!battle_string(out+112,16,(void*)get(user+0x45),16))return multiplayer_refuse(p,10);
    return 1;
}
static int multiplayer_name(const u8* name,u32 size){
    for(u32 i=0;i<size;++i){if(!name[i])return i!=0;if(name[i]<32||name[i]>=127)return 0;}return 0;
}
static u32 multiplayer_dispatch(void* object,u32 action,u32 transport,const u8* names){
    u8 state[MNM_MENU_V15_MULTIPLAYER_SIZE];if(!multiplayer_snapshot(object,state))return MNM_MENU_UNAVAILABLE;
    u8* p=object;int create=get(p+4)==15;
    if(p[0xc]||get(p+0x33)||get(p+0x37)||get(p+0x43)||get(state+24)||get(state+28))return MNM_MENU_UNAVAILABLE;
    u32 index;
    if(action==MNM_MENU_MULTIPLAYER_CANCEL){if(transport)return MNM_MENU_INVALID;if(!(get(state+20)&2))return MNM_MENU_UNAVAILABLE;index=1;}
    else if(action==MNM_MENU_MULTIPLAYER_SUBMIT){
        if(transport>2||!multiplayer_name(names+64,16)||(create?!multiplayer_name(names,64):names[0]!=0))return MNM_MENU_INVALID;
        if(!(get(state+16)&(1u<<transport))||!(get(state+20)&1))return MNM_MENU_UNAVAILABLE;
        u8* game=0,*user;if(create){u8* edits=(u8*)get(p+0x53);game=(u8*)get(edits);user=(u8*)get(edits+4);}else user=(u8*)get(p+0x5b);
        if(!multiplayer_available(user)||(create&&!multiplayer_available(game)))return MNM_MENU_UNAVAILABLE;
        // Complete validation precedes the original setters and callback.
        if(create)((ActionFn)0x4cd0e0)(game,(u32)names);
        ((ActionFn)0x4cd0e0)(user,(u32)(names+64));
        typedef u32 (THIS *SelectFn)(void*,void*,u32);
        u8* radios=(u8*)get(p+(create?0x57:0x53));((SelectFn)0x4ce730)(p+0x5f,(void*)get(radios+4*transport),0);index=0;
    }else return MNM_MENU_UNSUPPORTED;
    record(2,object,index,0);u32 result=((ActionFn)(create?0x4b1840:0x4af910))(object,index);record(3,object,index,result);return MNM_MENU_OK;
}
