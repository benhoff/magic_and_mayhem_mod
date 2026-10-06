#include "../../protocols/include/mnm/menu_v11.h"
static int defeat_quit_pending; // Native admission provenance, never an engine field.
static u8 defeat_payload[MNM_MENU_V11_DEFEAT_SIZE];
static int defeat_snapshot(void* object,u8* out){
    u8* p=object;
    if(!defeat_quit_pending||p!=(u8*)0x6db967||!readable(p,0x2b1)||get(p)!=0x5c5ebc||get(p+4)!=6||get(p+0x53)||
       get((void*)0x689920)!=5||get((void*)0x68991c)||!*(u8*)0x6dbc19)return 0;
    if(!readable((void*)0x6f349c,0x48)||get((void*)0x6f349c)!=5||get((void*)0x6f34b4)!=(u32)p||
       get((void*)0x6f34b0)!=0x6cbb78||get((void*)0x6f34ac)!=0x659408||get((void*)0x6f34a8)!=0x657ce0)return 0;
    if(get((void*)0x6cbb78)!=0x5c5dd8||get((void*)0x6cbb7c)!=2||get((void*)0x659408)!=0x5c6a60||get((void*)0x65940c)!=4||get((void*)0x657ce0)!=0x5c63e4||get((void*)0x657ce4)!=3)return 0;
    u8* pair=(u8*)get(p+0x47);u8* labels=(u8*)get(p+0x4b);u8* cb=(u8*)get(p+0x4f);
    if(!readable(pair,4)||!readable(labels,84)||!readable(cb,12)||get(cb)!=0x5c5eec||get(cb+4)!=0x4747a0||get(cb+8)!=(u32)p)return 0;
    u8* button=(u8*)get(pair);if(!readable(button,0x31)||get(button+0x25)!=(u32)cb||get(button+0x2d)!=21)return 0;
    for(u32 i=0;i<MNM_MENU_V11_DEFEAT_SIZE;++i)out[i]=0;
    put(out,1);put(out+4,5);put(out+8,5);
    if(!battle_string(out+16,256,p+0x57,256))return 0;
    for(u32 i=0;i<21;++i){u8* label=(u8*)get(labels+4*i);if(!label)continue;
        if(!readable(label,12)||!battle_string(out+272+i*256,256,(void*)get(label+8),256))return 0;
    }
    return out[16]!=0&&out[272+19*256]!=0;
}
static u32 defeat_dispatch(void* object,u32 action){
    static u8 next[MNM_MENU_V11_DEFEAT_SIZE];if(!defeat_snapshot(object,next))return MNM_MENU_UNAVAILABLE;
    if(action!=MNM_MENU_DEFEAT_OK)return MNM_MENU_UNSUPPORTED;
    record(2,object,21,0);u32 result=((ActionFn)0x4747a0)(object,21);record(3,object,21,result);return MNM_MENU_OK;
}
