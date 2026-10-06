#include "../../protocols/include/mnm/menu_v10.h"
static u8 region_payload[MNM_MENU_V7_REGION_SIZE];
static int region_snapshot(void* object,u8* out,int enter){
    u8* p=object;
    if(p!=(u8*)0x6578c0||!readable(p,0x81)||get(p)!=0x5c667c||get(p+4)!=18||get(p+8)!=1||
       get(p+0x7c)!=4||p[0x77]||get(p+0x6f)!=1||p[0x80])return 0;
    if(!readable((void*)0x6f349c,0x48)||get((void*)0x6f349c)!=4||
       get((void*)0x6f34b0)!=(u32)p||get((void*)0x6f34ac)!=0x659408||get((void*)0x6f34a8)!=0x657ce0)return 0;
    u8* realm=(u8*)0x659408;
    if(!readable(realm,0xa3b)||get(realm)!=0x5c6a60||get(realm+4)!=4||get(realm+8)||get(realm+0xa37)||
       !readable((void*)0x689920,4)||get((void*)0x689920)!=5)return 0;
    u8* parent=(u8*)0x657ce0;
    if(!readable(parent,8)||get(parent)!=0x5c63e4||get(parent+4)!=3)return 0;
    u8* cb=(u8*)get(p+0x5b),*radios=(u8*)get(p+0x53),*list=(u8*)get(p+0x5f),*buttons=(u8*)get(p+0x4b);
    u32 selected=get(p+0x63),available=0;
    if(!readable(cb,12)||get(cb)!=0x5c66ac||get(cb+4)!=0x4b3420||get(cb+8)!=(u32)p||
       !readable(radios,4*0x7e)||!readable(list,16)||!readable(buttons,2*0x59)||
       get(p+0x67)!=4||get(p+0x6b)!=4||selected>=4)return 0;
    for(u32 i=0;i<4;++i){u8* r=radios+i*0x7e;
        if(get(list+i*4)!=(u32)r||get(r)!=0x5c6d40||get(r+0x2d)!=i||get(r+0x3d)>2||get(r+0x39)!=(i==selected))return 0;
        if(get(r+8)==1&&get(r+0x3d)!=2)available|=1u<<i;
    }
    for(u32 i=0;i<2;++i){u8* b=buttons+i*0x59;
        if(get(b)!=0x5c6c6c||get(b+0x2d)!=i||get(b+0x25)!=(u32)cb||get(b+0x3d)>2)return 0;
    }
    for(u32 i=0;i<MNM_MENU_V7_REGION_SIZE;++i)out[i]=0;
    put(out,4);put(out+4,4);put(out+12,selected);put(out+16,available);
    put(out+20,get(buttons+0x59+8)==1&&get(buttons+0x59+0x3d)!=2);put(out+24,1);
    if(enter&&get(buttons+8)==1&&get(buttons+0x3d)!=2&&readable((void*)0x659f51,8)&&readable((void*)0x65b1dd,4)&&readable((void*)0x65ae1d,36)&&get((void*)0x659f55)==9){
        u32 player=get((void*)0x659f51),owner=get((void*)0x65b1dd);
        if(player<9&&owner<9&&owner!=player&&get((u8*)0x65ae1d+4*player)==1&&get((u8*)0x65ae1d+4*owner)==1)put(out+20,get(out+20)|2);
    }
    if(!battle_string(out+32,128,(void*)get(p+0x73),128)||!equal(out+32,"Celtic",7)||
       !battle_string(out+160,128,(void*)get(p+0x78),128)||!out[160])return 0;
    return 1;
}
static u32 region_dispatch(void* object,u32 action,u32 argument,int enter){
    u8 current[MNM_MENU_V7_REGION_SIZE];if(!region_snapshot(object,current,enter))return MNM_MENU_UNAVAILABLE;
    if(action==MNM_MENU_REGION_DIFFICULTY){
        if(argument>3)return MNM_MENU_INVALID;
        if(!(get(current+16)&(1u<<argument)))return MNM_MENU_UNAVAILABLE;
        typedef void (THIS *SelectFn)(void*,void*,u32);
        record(2,object,65536+argument,0);
        ((SelectFn)0x4ce730)((u8*)object+0x5f,(u8*)get((u8*)object+0x53)+argument*0x7e,0);
        record(3,object,65536+argument,0);
    }else if(action==MNM_MENU_REGION_ENTER&&enter){
        if(!(get(current+20)&2))return MNM_MENU_UNAVAILABLE;
        record(2,object,0,0);u32 result=((ActionFn)0x4b3420)(object,0);record(3,object,0,result);
    }else if(action==MNM_MENU_REGION_CANCEL){
        if(!(get(current+20)&1))return MNM_MENU_UNAVAILABLE;
        record(2,object,1,0);u32 result=((ActionFn)0x4b3420)(object,1);record(3,object,1,result);
    }else return MNM_MENU_UNSUPPORTED;
    return MNM_MENU_OK;
}
