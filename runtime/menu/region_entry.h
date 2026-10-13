#include "../../protocols/include/mnm/menu_v20.h"
static int region_grimoire_enabled;
static int region_campaign_enabled;
static u32 region_enter_handoff;
static int region_auxiliary_available(u8* p,u32 slot){
    u32 base=get(p+0x4f);if(!base||slot>2)return 0;
    u8* b=(u8*)base+slot*0x7a,*cb=(u8*)get(p+0x5b);
    return readable(b,0x7a)&&get(b)==0x5c6bd0&&get(b+8)==1&&get(b+0x2d)==2+slot&&
           get(b+0x25)==(u32)cb&&get(b+0x3d)<2;
}
static int region_grimoire_available(u8* p){return region_auxiliary_available(p,0);}
static u8 region_payload[MNM_MENU_V7_REGION_SIZE];
static int region_snapshot(void* object,u8* out,int enter){
    u8* p=(u8*)object;
    if(p!=(u8*)0x6578c0||!readable(p,0x81)||get(p)!=0x5c667c||get(p+4)!=18||get(p+8)!=1||
       get(p+0x7c)!=4||p[0x77]>1||(!region_campaign_enabled&&p[0x77])||
       !get(p+0x6f)||(!region_campaign_enabled&&get(p+0x6f)!=1)||p[0x80])return 0;
    if(!readable((void*)0x6f349c,0x48)||get((void*)0x6f349c)!=4||
       get((void*)0x6f34b0)!=(u32)p||get((void*)0x6f34ac)!=0x659408||get((void*)0x6f34a8)!=0x657ce0)return 0;
    if(region_campaign_enabled&&(get((void*)0x6f34e0)!=(u32)p||!readable((void*)0x68991c,4)||get((void*)0x68991c)))return 0;
    u8* realm=(u8*)0x659408;
    if(!readable(realm,0xa3b)||get(realm)!=0x5c6a60||get(realm+4)!=4||get(realm+8)||get(realm+0xa37)||
       !readable((void*)0x689920,4)||get((void*)0x689920)!=5)return 0;
    u32 region=get(p+0x6f),loaded=0;
    if(region_campaign_enabled&&readable(realm,0x1e25)&&get(realm+0x12)==(u32)(realm+0xa49)&&
       get(realm+0x823)&&get(realm+0x823)<=19&&readable((void*)get(realm+0x59),800*600)){
        if(region>get(realm+0x823)||get(realm+0xa44)||realm[0xa48]||
           !((equal(realm+0xa49,"Celtic",7))||(equal(realm+0xa49,"Greek",6))||(equal(realm+0xa49,"Medieval",9))))return 0;
        u32 player=get(realm+0xb49),count=get(realm+0xb4d),owner=get((u8*)0x65b1d9+region*4);
        if(!count||count>80||player>=count||owner>=count||get((u8*)0x65ae1d+player*4)>get(realm+0x823)||
           p[0x77]!=(get((u8*)0x65ae1d+player*4)!=region))return 0;
        loaded=1;
    }
    if(!loaded&&(region!=1||p[0x77]))return 0;
    if(region_campaign_enabled&&!loaded&&(!readable(realm,0x1e25)||!equal(realm+0xa49,"Celtic",7)))return 0;
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
    put(out,4);put(out+4,4);put(out+8,p[0x77]);put(out+12,selected);put(out+16,available);
    put(out+20,get(buttons+0x59+8)==1&&get(buttons+0x59+0x3d)!=2);put(out+24,region);
    if(region_grimoire_enabled&&region_grimoire_available(p))put(out+20,get(out+20)|4);
    if(region_campaign_enabled){
        if(region_auxiliary_available(p,1))put(out+20,get(out+20)|8);
        if(region_auxiliary_available(p,2))put(out+20,get(out+20)|16);
    }
    if(enter&&get(buttons+8)==1&&get(buttons+0x3d)!=2&&readable((void*)0x659f51,8)&&readable((void*)0x65b1dd,4)&&readable((void*)0x65ae1d,36)&&get((void*)0x659f55)==9){
        u32 player=get((void*)0x659f51),owner=get((void*)0x65b1dd);
        if(player<9&&owner<9&&owner!=player&&get((u8*)0x65ae1d+4*player)==1&&get((u8*)0x65ae1d+4*owner)==1)put(out+20,get(out+20)|2);
    }
    if(region_campaign_enabled&&enter&&get(buttons+8)==1&&get(buttons+0x3d)!=2&&loaded){
        u32 player=get(realm+0xb49),count=get(realm+0xb4d),owner=get((u8*)0x65b1d9+region*4);
        if(!count||count>80||player>=count||owner>=count)return 0;
        u32 special=get((u8*)0x65ae1d+player*4)==region&&owner!=player&&get((u8*)0x65ae1d+owner*4)==region,occupants=0;
        for(u32 i=0;i<count;++i)if(get((u8*)0x65ab9d+i*4)&&get((u8*)0x65ae1d+i*4)==region)++occupants;
        if(special||occupants<4){put(out+20,get(out+20)|2);put(out+28,special?3:MNM_MENU_HANDOFF_CAMPAIGN_RETURN);}
        else put(out+20,get(out+20)&~2u);
    }
    if(region_campaign_enabled&&(get(out+20)&2)&&!get(out+28))put(out+28,3);
    if(!battle_string(out+32,128,(void*)get(p+0x73),128)||
       (loaded?!((equal(out+32,"Celtic",7)&&equal(realm+0xa49,"Celtic",7))||
                 (equal(out+32,"Greek",6)&&equal(realm+0xa49,"Greek",6))||
                 (equal(out+32,"Medieval",9)&&equal(realm+0xa49,"Medieval",9))):!equal(out+32,"Celtic",7))||
       !battle_string(out+160,128,(void*)get(p+0x78),128)||!out[160])return 0;
    return 1;
}
static u32 region_dispatch(void* object,u32 action,u32 argument,int enter){
    u8 current[MNM_MENU_V7_REGION_SIZE];if(!region_snapshot(object,current,enter))return MNM_MENU_UNAVAILABLE;
    if(region_campaign_enabled&&(action==MNM_MENU_REGION_ENTER||action==MNM_MENU_REGION_CANCEL)&&argument)return MNM_MENU_INVALID;
    if(action==MNM_MENU_REGION_DIFFICULTY){
        if(argument>3)return MNM_MENU_INVALID;
        if(!(get(current+16)&(1u<<argument)))return MNM_MENU_UNAVAILABLE;
        typedef void (THIS *SelectFn)(void*,void*,u32);
        record(2,object,65536+argument,0);
        ((SelectFn)0x4ce730)((u8*)object+0x5f,(u8*)get((u8*)object+0x53)+argument*0x7e,0);
        record(3,object,65536+argument,0);
    }else if(action==MNM_MENU_REGION_ENTER&&enter){
        if(!(get(current+20)&2))return MNM_MENU_UNAVAILABLE;
        region_enter_handoff=region_campaign_enabled?get(current+28):0;
        record(2,object,0,0);u32 result=((ActionFn)0x4b3420)(object,0);record(3,object,0,result);
        if(region_campaign_enabled){
            if(region_enter_handoff==3&&(!*(u8*)0x659e50||get((void*)0x659e33)!=4))return MNM_MENU_UNAVAILABLE;
            if(region_enter_handoff==MNM_MENU_HANDOFF_CAMPAIGN_RETURN){u32 player=get((void*)0x659f51);
                if(player>=80||get((u8*)0x65af5d+player*4)!=get(current+24))return MNM_MENU_UNAVAILABLE;}
        }
    }else if(action==MNM_MENU_REGION_CANCEL){
        if(!(get(current+20)&1))return MNM_MENU_UNAVAILABLE;
        record(2,object,1,0);u32 result=((ActionFn)0x4b3420)(object,1);record(3,object,1,result);
    }else if(action==MNM_MENU_REGION_GRIMOIRE&&region_grimoire_enabled){
        if(argument)return MNM_MENU_INVALID;
        if(!(get(current+20)&4))return MNM_MENU_UNAVAILABLE;
        record(2,object,2,0);u32 result=((ActionFn)0x4b3420)(object,2);record(3,object,2,result);
    }else if(region_campaign_enabled&&(action==MNM_MENU_REGION_SPELLBOX||action==MNM_MENU_REGION_CHARACTER)){
        if(argument)return MNM_MENU_INVALID;
        u32 mask=action==MNM_MENU_REGION_SPELLBOX?8:16,index=action==MNM_MENU_REGION_SPELLBOX?3:4;
        if(!(get(current+20)&mask))return MNM_MENU_UNAVAILABLE;
        record(2,object,index,0);u32 result=((ActionFn)0x4b3420)(object,index);record(3,object,index,result);
    }else return MNM_MENU_UNSUPPORTED;
    return MNM_MENU_OK;
}
