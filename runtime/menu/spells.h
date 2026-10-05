#include "../../protocols/include/mnm/menu_v3.h"
static u8 spell_payload[MNM_MENU_V3_SPELL_SIZE];
static u32 spell_valid;
typedef char* (THIS *SpellTextFn)(void*,u32);
static int spell_cell(u8* cell,u32 callback,void* owner){
    if(!readable(cell,0x82))return 0;
    u8* cb=(u8*)get(cell+0x25);
    return readable(cb,16)&&get(cb)==0x5c77b4&&get(cb+4)==callback&&get(cb+12)==(u32)owner;
}
static int spell_snapshot(void* object,u8* out){
    u8* p=object;if(!readable(p,0x26a)||get(p)!=0x5c775c||get(p+0x250)!=1||get(p+0x264)||get(p+0x94)!=0xffffffff||get(p+0x42))return 0;
    u32 player=get(p+0x90);if(player<0x65b250||player>=0x65b250+4*0x93a||(player-0x65b250)%0x93a)return 0;
    for(u32 i=0;i<MNM_MENU_V3_SPELL_SIZE;++i)out[i]=0;
    put(out,(player-0x65b250)/0x93a);put(out+4,get((void*)0x6f5284));
    if(!readable((void*)0x6f4c08,63*4))return 0;
    copy(out+MNM_MENU_V3_RECIPES,(void*)0x6f4c08,63*4);
    u32 seen=0;
    for(u32 a=0;a<3;++a){
        u32 n=get(p+0x84+4*a);if(n>7)return 0;put(out+MNM_MENU_V3_COUNTS+4*a,n);
        u8* cells=(u8*)get(p+0x218+4*a);
        for(u32 i=0;i<21;++i){u32 v=0xffffffff;
            if(i<n){u8* cell=cells+i*0x86;if(!spell_cell(cell,0x576380,object)||get(cell+0x7a)!=a)return 0;v=get(cell+0x7e);}
            if(v!=0xffffffff){if(v>=21||(seen&(1u<<v)))return 0;seen|=1u<<v;}
            put(out+MNM_MENU_V3_SLOTS+4*(a*21+i),v);
        }
    }
    u8* shelves=(u8*)get(p+0x214);
    for(u32 i=0;i<25;++i){u8* cell=shelves+i*0x82;if(!spell_cell(cell,0x5765d0,object))return 0;
        u32 v=get(cell+0x7a);if(v!=0xffffffff){if(v>=21||(seen&(1u<<v)))return 0;seen|=1u<<v;}
        put(out+MNM_MENU_V3_SHELVES+4*i,v);
    }
    put(out+8,seen);
    // Use original loaded name catalogs, not a guessed ID-to-recipe mapping.
    if(!readable((void*)0x6f4d48,16)||!readable((void*)0x6f4f60,16))return 0;
    for(u32 i=0;i<21;++i){
        if(!(seen&(1u<<i)))continue;
        if(!battle_string(out+MNM_MENU_V3_ITEM_NAMES+i*128,128,((SpellTextFn)0x58a680)((void*)0x6f4d48,i),128))return 0;
        for(u32 a=0;a<3;++a){u32 id=get(out+MNM_MENU_V3_RECIPES+4*(i*3+a));if(id>92)return 0;
            if(!battle_string(out+MNM_MENU_V3_SPELL_NAMES+(i*3+a)*128,128,((SpellTextFn)0x58a680)((void*)0x6f4f60,id+5),128))return 0;
        }
    }
    return 1;
}
static void spell_click(void* object,u32 address,u32 index){
    record(2,object,index,0);u32 result=((ActionFn)address)(object,index);record(3,object,index,result);
}
static u32 spell_finish(void* object,const u32* slots){
    u8* p=object;u32 seen=0,counts[3];
    for(u32 a=0;a<3;++a){counts[a]=get(spell_payload+MNM_MENU_V3_COUNTS+a*4);
        for(u32 i=0;i<21;++i){u32 v=slots[a*21+i];
            if(i>=counts[a]&&v!=0xffffffff)return MNM_MENU_INVALID;
            if(v!=0xffffffff){if(v>=21||(seen&(1u<<v))||!(get(spell_payload+8)&(1u<<v)))return MNM_MENU_INVALID;seen|=1u<<v;}
        }
    }
    if(get((void*)0x6f5288))return MNM_MENU_UNAVAILABLE;
    // Original callbacks own carried items, swaps, sprites, tooltips and sounds.
    // The flag brackets one synthetic press/release gesture on the engine thread.
    for(u32 a=0;a<3;++a)for(u32 i=0;i<counts[a];++i){u8* cell=(u8*)get(p+0x218+4*a)+i*0x86;
        if(get(cell+0x7e)==0xffffffff)continue;
        u32 shelf=0;u8* shelves=(u8*)get(p+0x214);
        while(shelf<25&&get(shelves+shelf*0x82+0x7a)!=0xffffffff)++shelf;
        if(shelf==25)return MNM_MENU_INVALID; // Prevalidated <=21 unique items in 25 shelves.
        put((void*)0x6f5288,1);spell_click(object,0x576380,0x7fe+a*7+i);
        put((void*)0x6f5288,0);spell_click(object,0x5765d0,0x7d3+shelf);
    }
    for(u32 a=0;a<3;++a)for(u32 i=0;i<counts[a];++i){u32 v=slots[a*21+i];if(v==0xffffffff)continue;
        u8* shelves=(u8*)get(p+0x214);u32 shelf=0;
        while(shelf<25&&get(shelves+shelf*0x82+0x7a)!=v)++shelf;
        if(shelf==25)return MNM_MENU_INVALID;
        put((void*)0x6f5288,1);spell_click(object,0x5765d0,0x7d3+shelf);
        put((void*)0x6f5288,0);spell_click(object,0x576380,0x7fe+a*7+i);
    }
    if(!spell_snapshot(object,spell_payload))return MNM_MENU_UNAVAILABLE;
    // Original OK/Escape callback returns to setup; its original resume loads battle.
    record(2,object,MNM_MENU_SPELL_FINISH,0);u32 result=((ActionFn)0x576810)(object,0);record(3,object,MNM_MENU_SPELL_FINISH,result);
    return MNM_MENU_OK;
}
