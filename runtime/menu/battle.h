#include "../../protocols/include/mnm/menu_v2.h"
static u8 battle_payload[MNM_MENU_V2_PAYLOAD_SIZE];
static u32 battle_valid;
static int battle_string(u8* out,u32 capacity,const void* source,u32 limit){
    const u8* p=source;if(!p)return 0;
    for(u32 i=0;i<limit&&i<capacity;++i){
        if(!readable(p+i,1))return 0;
        out[i]=p[i];if(!p[i])return 1;
    }return 0;
}
static int battle_snapshot(u8* data){
    for(u32 i=0;i<MNM_MENU_V2_PAYLOAD_SIZE;++i)data[i]=0;
    u8* setup=(u8*)0x658970;
    if(!readable(setup,0x63)||get(setup)!=0x5c6534||get(setup+8)!=1||
       !readable((void*)0x6c3758,56)||!readable((void*)0x658f10,4*0x133))return 0;
    put(data,get((void*)0x6c3758));
    copy(data+MNM_MENU_V2_RULES-MNM_MENU_V2_MAP,(void*)0x6c375c,52);
    for(u32 i=0;i<4;++i){
        u8* player=(u8*)(0x658f10+i*0x133);u8* out=data+MNM_MENU_V2_PLAYERS-MNM_MENU_V2_MAP+i*48;
        if(!battle_string(out+16,32,player,21))return 0;
        u8 absent[32]={0};if(!battle_string(absent,32,(void*)0x6ead98,32))return 0;
        int active=!equal(out+16,absent,32);
        u32 portrait=get(player+0x15),colour=get(player+0x1d),handicap=get(player+0x19);
        if(active&&(portrait>11||colour>7||handicap>50))return 0;
        put(out,active);put(out+4,portrait);put(out+8,colour);put(out+12,handicap);
    }
    u32 texts=get(setup+0x53);
    if(!readable((void*)texts,56))return 0;
    u32 label=get((void*)(texts+52));if(!readable((void*)label,12))return 0;
    if(!battle_string(data+MNM_MENU_V2_MAP_NAME-MNM_MENU_V2_MAP,128,(void*)get((void*)(label+8)),128))return 0;
    u8* map=(u8*)0x690448;
    if(readable(map,0x5b)&&get(map)==0x5c6940&&get(map+8)==1){
        u8* list=(u8*)get(map+0x57);
        if(!readable(list,0x6f))return 0;
        u32 count=get(list+0x49);u8* rows=(u8*)get(list+0x45);
        if(!count||count>MNM_MENU_V2_MAX_MAPS||!readable(rows,count*0x101))return 0;
        put(data+4,count);
        for(u32 i=0;i<count;++i)
            if(!battle_string(data+MNM_MENU_V2_MAP_NAMES-MNM_MENU_V2_MAP+i*128,128,rows+i*0x101,256))return 0;
    }
    return 1;
}
static u32 THIS setup_action(void* object,u32 action){
    record(2,object,action,0);u32 result=((ActionFn)0x4ad6a0)(object,action);record(3,object,action,result);return result;
}
static u32 THIS map_action(void* object,u32 action){
    record(2,object,action,0);u32 result=((ActionFn)0x4bbbf0)(object,action);record(3,object,action,result);return result;
}
// Validate the entire transaction before calling any original control setter.
static int battle_rules(void* object,const u32* values,int apply){
    u8* p=object;if(!readable(p,0x63))return 0;
    u8* sliders=(u8*)get(p+0x5f);if(!readable(sliders,68))return 0;
    for(u32 i=0;i<17;++i){
        u8* slider=(u8*)get(sliders+4*i);
        if(!slider&&i>=13&&!get(battle_payload+60+(i-13)*48)&&values[i]==0)continue;
        if(!readable(slider,0x75))return 0;
        u8* callback=(u8*)get(slider+0x25);
        if(!readable(callback,12)||get(callback)!=0x5c6594||get(callback+4)!=0x4ad860||get(callback+8)!=(u32)object||get(slider+0x2d)!=i)return 0;
        u32 min=get(slider+0x65),max=get(slider+0x69);
        if(min>max||max>10000||values[i]<min||values[i]>max)return 0;
    }
    if(apply)for(u32 i=0;i<17;++i){
        u8* slider=(u8*)get(sliders+4*i);if(!slider)continue;
        // The original setter invokes the registered rule callback itself.
        if(get(slider+0x61)!=values[i])((ActionFn)0x4cdf40)(slider,values[i]);
    }
    return 1;
}
static u32 battle_dispatch(void* object,const u32* host,u32 screen){
    u32 action=host[3],argument=host[5];
    if(action==MNM_MENU_OPEN_SINGLE&&screen==22){quick_action(object,2);return MNM_MENU_OK;}
    if(screen==14){
        if(action==MNM_MENU_SETUP_CANCEL){setup_action(object,0);return MNM_MENU_OK;}
        if(action!=MNM_MENU_SETUP_MAP&&action!=MNM_MENU_SETUP_START&&action!=MNM_MENU_SETUP_PLAYER&&action!=MNM_MENU_SETUP_APPLY)return MNM_MENU_UNSUPPORTED;
        if(action==MNM_MENU_SETUP_PLAYER&&argument>3)return MNM_MENU_INVALID;
        if(action==MNM_MENU_SETUP_START){
            // Keep engine-owned map/players authoritative; don't launch an empty setup.
            if(!get(battle_payload)||!get(battle_payload+60))return MNM_MENU_INVALID;
            int opponent=0;for(u32 i=1;i<4;++i)opponent|=get(battle_payload+60+i*48)!=0;
            if(!opponent)return MNM_MENU_INVALID;
        }
        if(!battle_rules(object,host+6,0))return MNM_MENU_INVALID;
        battle_rules(object,host+6,1);
        if(action==MNM_MENU_SETUP_MAP)setup_action(object,2);
        else if(action==MNM_MENU_SETUP_START)setup_action(object,1);
        else if(action==MNM_MENU_SETUP_PLAYER){record(2,object,0x100+argument,0);u32 result=((ActionFn)0x4ad6f0)(object,argument);record(3,object,0x100+argument,result);}
        return MNM_MENU_OK;
    }
    if(screen==25){
        if(get((u8*)object+0x15b)!=14)return MNM_MENU_UNAVAILABLE;
        if(action==MNM_MENU_MAP_CANCEL){map_action(object,1);return MNM_MENU_OK;}
        if(action==MNM_MENU_MAP_OK){
            u8* list=(u8*)get((u8*)object+0x57);
            if(!argument||argument>get(battle_payload+4)||!readable(list,0x6f))return MNM_MENU_INVALID;
            // Clear selection before selecting; original setter can toggle a selected row.
            ((ActionFn)0x4d1060)(list,0xffffffff);((ActionFn)0x4d1060)(list,argument-1);
            map_action(object,0);return MNM_MENU_OK;
        }
    }
    return MNM_MENU_UNSUPPORTED;
}
