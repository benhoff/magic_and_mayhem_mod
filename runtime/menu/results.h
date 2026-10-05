#include "../../protocols/include/mnm/menu_v5.h"
static u8 result_payload[MNM_MENU_V5_RESULTS_SIZE];
static int result_snapshot(void* object,u8* out){
    u8* p=object;
    if(p!=(u8*)0x6deec8||!readable(p,0x85)||get(p)!=0x5c5ef4||get(p+4)!=26||
       !readable((void*)0x689920,4)||get((void*)0x689920)!=1)return 0;
    if(!readable((void*)0x6f349c,0x48))return 0;
    u32 depth=get((void*)0x6f349c);if(depth<1||depth>15||get((void*)(0x6f34a0+depth*4))!=(u32)p||get((void*)(0x6f34a0+(depth-1)*4))!=0x6cbb78)return 0;
    if(!readable((void*)0x6cbb78,8)||get((void*)0x6cbb78)!=0x5c5dd8||get((void*)0x6cbb7c)!=2)return 0;
    u8* buttons=(u8*)get(p+0x57);u8* labels=(u8*)get(p+0x4f);
    if(!readable(buttons,12)||!readable(labels,104)||get(buttons))return 0; // Spectate stays original.
    u32 actions=0;
    for(u32 i=1;i<3;++i){u8* control=(u8*)get(buttons+i*4);if(!control)continue;
        if(!readable(control,0x31)||get(control+0x2d)!=i)return 0;
        u8* cb=(u8*)get(control+0x25);if(!readable(cb,12)||get(cb)!=0x5c5f58||get(cb+4)!=0x475860||get(cb+8)!=(u32)p)return 0;
        actions|=1u<<(i-1);
    }
    if(!actions)return 0;
    for(u32 i=0;i<MNM_MENU_V5_RESULTS_SIZE;++i)out[i]=0;
    put(out,actions);put(out+4,1);put(out+8,depth);
    for(u32 row=0;row<4;++row){u8* r=out+16+row*648;
        for(u32 col=0;col<5;++col){u8* label=(u8*)get(labels+4*(6+row+col*4));
            if(!readable(label,12)||!battle_string(r+8+col*128,128,(void*)get(label+8),128))return 0;
        }
        put(r,r[8]!=0);put(r+4,0xffffffff);
    }
    return 1;
}
static u32 result_dispatch(void* object,u32 action){
    u8 current[MNM_MENU_V5_RESULTS_SIZE];if(!result_snapshot(object,current))return MNM_MENU_UNAVAILABLE;
    u32 index;
    if(action==MNM_MENU_RESULT_CONTINUE&&(get(current)&1))index=1;
    else if(action==MNM_MENU_RESULT_QUIT&&(get(current)&2))index=2;
    else return MNM_MENU_UNSUPPORTED;
    record(2,object,index,0);u32 result=((ActionFn)0x475860)(object,index);record(3,object,index,result);
    return MNM_MENU_OK;
}
