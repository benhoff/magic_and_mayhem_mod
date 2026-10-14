#include "../../protocols/include/mnm/menu_v21.h"
static int spell_campaign_enabled;
static u8 spell_context_payload[MNM_MENU_V21_SPELL_CONTEXT_SIZE];
static int spell_context(void* object,u8* out){
    u8* p=(u8*)object;for(u32 i=0;i<MNM_MENU_V21_SPELL_CONTEXT_SIZE;++i)out[i]=0;
    if(!readable(p,0x26a))return 0;
    u32 mode=get(p+0x250);if(mode==1){put(out,1);return 1;}
    if(mode||!spell_campaign_enabled||p!=(u8*)0x6f2aa0||get(p)!=0x5c775c||get(p+4)!=7||
       !readable((void*)0x68991c,8)||get((void*)0x68991c)||get((void*)0x689920)!=5||
       !readable((void*)0x6f349c,0x48)||get((void*)0x6f34e0)!=(u32)p)return 0;
    u32 depth=get((void*)0x6f349c);if(depth!=4&&depth!=5)return 0;
    if(get((void*)(0x6f34a0+depth*4))!=(u32)p||get((void*)0x6f34a8)!=0x657ce0||get((void*)0x6f34ac)!=0x659408||
       !readable((void*)0x657ce0,8)||get((void*)0x657ce0)!=0x5c63e4||get((void*)0x657ce4)!=3)return 0;
    u8* realm=(u8*)0x659408;
    if(!readable(realm,0x1e25)||get(realm)!=0x5c6a60||get(realm+4)!=4||get(realm+0xa2b)||get(realm+0xa37)||get(realm+0xa44)||realm[0xa48]||
       get(realm+0x12)!=(u32)(realm+0xa49)||!get(realm+0x823)||get(realm+0x823)>19||
       !(equal(realm+0xa49,"Celtic",7)||equal(realm+0xa49,"Greek",6)||equal(realm+0xa49,"Medieval",9))||
       !readable((void*)get(realm+0x59),800*600)||!get(realm+0xb4d)||get(realm+0xb4d)>80)return 0;
    u32 player=get(p+0x90);if(player<0x65b250||player>=0x65b250+get(realm+0xb4d)*0x93a||(player-0x65b250)%0x93a||
       !readable((void*)player,0x93a)||!get((void*)player)||get((void*)(player+8)))return 0;
    u32 parent=4;
    if(depth==5){u8* entry=(u8*)0x6578c0;parent=18;
        if(get((void*)0x6f34b0)!=(u32)entry||!readable(entry,0x81)||get(entry)!=0x5c667c||get(entry+4)!=18||get(entry+0x7c)!=4||
           !get(entry+0x6f)||get(entry+0x6f)>get(realm+0x823)||entry[0x80])return 0;
    }
    put(out+4,parent);put(out+8,depth);put(out+12,5);return 1;
}
