#include "../../protocols/include/mnm/menu_v18.h"
static u8 character_payload[MNM_MENU_V18_CHARACTER_SIZE];
static int character_snapshot(void* object,u8* out){
    u8* p=(u8*)object,*realm=(u8*)0x659408;
    if(p!=(u8*)0x6c5148||!readable(p,0x120)||get(p)!=0x5c66b4||get(p+4)!=20||get(p+8)!=1||p[0xc]||
       get(p+0x33)||get(p+0x37)||get(p+0x3b)||get(p+0x43)||get(p+0x11b)||
       !readable((void*)0x68991c,8)||get((void*)0x68991c)||get((void*)0x689920)!=5||
       !readable((void*)0x6f349c,0x48)||get((void*)0x6f34e0)!=(u32)p)return 0;
    u32 depth=get((void*)0x6f349c);if(depth!=4&&depth!=5)return 0;
    if(get((void*)(0x6f34a0+depth*4))!=(u32)p||get((void*)0x6f34a8)!=0x657ce0||get((void*)0x6f34ac)!=(u32)realm)return 0;
    if(!readable((void*)0x657ce0,8)||get((void*)0x657ce0)!=0x5c63e4||get((void*)0x657ce4)!=3)return 0;
    if(!readable(realm,0x1e25)||get(realm)!=0x5c6a60||get(realm+4)!=4||get(realm+0xa2b)||get(realm+0xa37)||
       get(realm+0xa44)||realm[0xa48]||get(realm+0x12)!=(u32)(realm+0xa49)||!get(realm+0x823)||get(realm+0x823)>19||
       !equal(realm+0xa49,"Celtic",7)||!readable((void*)get(realm+0x59),800*600))return 0;
    u32 parent=4;
    if(depth==5){u8* entry=(u8*)0x6578c0;parent=18;
        if(get((void*)0x6f34b0)!=(u32)entry||!readable(entry,0x81)||get(entry)!=0x5c667c||get(entry+4)!=18||
           get(entry+0x7c)!=4||get(entry+0x6f)<2||get(entry+0x6f)>get(realm+0x823)||entry[0x80])return 0;
    }
    u8* cb=(u8*)get(p+0x6f),*buttons=(u8*)get(p+0x73),*adjust=(u8*)get(p+0x87);
    u8* bars=(u8*)get(p+0x7f),*gems=(u8*)get(p+0x83);
    if(!readable(cb,12)||get(cb)!=0x5c66e4||get(cb+4)!=0x4b5470||get(cb+8)!=(u32)p||
       !readable(buttons,2*0x59)||!readable(adjust,12*0x7a)||!readable(bars,3*0x2c)||!readable(gems,3*0x2c))return 0;
    // Original getter scans 80 active, non-secondary wizard records.
    u32 wizard=80;
    if(!readable((void*)0x65b250,80*0x93a))return 0;
    for(u32 i=0;i<80;++i){u8* w=(u8*)0x65b250+i*0x93a;if(get(w)&&!get(w+8)){wizard=i;break;}}
    if(wizard==80||wizard>=get(realm+0xb4d)||get(realm+0xb4d)>80)return 0;
    u8* w=(u8*)0x65b250+wizard*0x93a;
    u32 experience=get(p+0xa7);if(experience>1000000000||experience>get(w+0x58)||get(w+0x58)>1000000000)return 0;
    for(u32 i=0;i<2;++i){u8* b=buttons+i*0x59;
        if(get(b)!=0x5c6c6c||get(b+0x25)!=(u32)cb||get(b+0x2d)!=i||get(b+8)!=1||get(b+0x3d)>1)return 0;
    }
    for(u32 i=0;i<MNM_MENU_V18_CHARACTER_SIZE;++i)out[i]=0;
    put(out,parent);put(out+4,depth);put(out+8,5);put(out+12,wizard);put(out+16,experience);put(out+20,3);
    const u32 limits[6]={200,800,40,7,7,7},steps[6]={5,20,1,1,1,1};u32 mask=0;
    for(u32 i=0;i<6;++i){u8* cell=(i<3?bars:gems)+(i%3)*0x2c;u8* row=out+32+i*24;
        u32 opening=get(cell+(i<3?0x24:0x18)),value=get(cell+(i<3?0x28:0x1c)),maximum=get(cell+(i<3?0x20:0x14));
        u32 step=get(p+(i<3?0xab+i*0x10:0xdb)),down=get(p+0xeb+i*8),up=get(p+0xef+i*8);
        if(maximum!=limits[i]||step!=steps[i]||opening>value||value>maximum||(value-opening)%step||
           opening!=get(w+(i<3?0x3c+i*4:0xf0+(i-3)*4))||down>1000000||up>1000000)return 0;
        put(row,opening);put(row+4,value);put(row+8,maximum);put(row+12,step);put(row+16,down);put(row+20,up);
        for(u32 direction=0;direction<2;++direction){u32 index=i*2+direction;u8* b=adjust+index*0x7a;
            if(get(b)!=0x5c6bd0||get(b+0x25)!=(u32)cb||get(b+0x2d)!=25+index||get(b+0x3d)>2)return 0;
            if(get(b+8)==1&&get(b+0x3d)!=2&&
               (direction?(value+step<=maximum&&up<=experience):(value>=opening+step&&down<=1000000000-experience)))mask|=1u<<index;
        }
    }
    put(out+24,mask);
    // Bounded original Realm label; portrait/progression mapping is not guessed.
    if(!battle_string(out+176,32,realm+0xa49+0x84c+wizard*16,16)||!out[176])return 0;
    return 1;
}
static u32 character_dispatch(void* object,u32 action,u32 argument){
    u8 snapshot[MNM_MENU_V18_CHARACTER_SIZE];if(!character_snapshot(object,snapshot))return MNM_MENU_UNAVAILABLE;
    u32 index;
    if(action==MNM_MENU_CHARACTER_ADJUST){if(argument>=12)return MNM_MENU_INVALID;
        if(!(get(snapshot+24)&(1u<<argument)))return MNM_MENU_UNAVAILABLE;
        index=25+argument;
    }else if(action==MNM_MENU_CHARACTER_OK||action==MNM_MENU_CHARACTER_CANCEL){
        if(argument)return MNM_MENU_INVALID;
        index=action==MNM_MENU_CHARACTER_OK?0:1;
    }else return MNM_MENU_UNSUPPORTED;
    record(2,object,index,0);u32 result=((ActionFn)0x4b5470)(object,index);record(3,object,index,result);
    return MNM_MENU_OK;
}
