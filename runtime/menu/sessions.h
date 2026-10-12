#include "../../protocols/include/mnm/menu_v16.h"
static u8 sessions_payload[MNM_MENU_V16_SESSIONS_SIZE];
static u32 sessions_diagnostics;
static int sessions_refuse(u8* p,u32 why){if(get(p+8)==1&&!(sessions_diagnostics&(1u<<why))){sessions_diagnostics|=1u<<why;record(41,p,why,0);}return 0;}
static int sessions_snapshot(void* object,u8* out){
 u8* p=object;if(p!=(u8*)0x6de838||!readable(p,0x66)||get(p)!=0x5c65d4||get(p+4)!=21||get(p+8)!=1)return 0;
 if(!readable((void*)0x6f349c,0x48)||get((void*)0x6f349c)!=5||get((void*)0x6f34e0)!=(u32)p||get((void*)0x6f34b4)!=(u32)p||get((void*)0x6f34b0)!=0x6a5018||get((void*)0x6f34ac)!=0x6e0020||get((void*)0x6f34a8)!=0x657ce0||
    !readable((void*)0x6a5018,12)||get((void*)0x6a5018)!=0x5c659c||get((void*)0x6a501c)!=16||get((void*)0x6a5020)||
    !readable((void*)0x6e0020,8)||!readable((void*)0x657ce0,8)||get((void*)0x6e0020)!=0x5c641c||get((void*)0x6e0024)!=22||get((void*)0x657ce0)!=0x5c63e4||get((void*)0x657ce4)!=3)return sessions_refuse(p,1);
 if(!readable((void*)0x68991c,8)||get((void*)0x68991c)!=1||get((void*)0x689920)!=2||!readable((void*)0x6e1f0c,4)||!readable((void*)0x6de6b0,4)||get((void*)0x6e1f0c)||get((void*)0x6de6b0)<1||get((void*)0x6de6b0)>3)return sessions_refuse(p,2);
 u8* callbacks=(u8*)get(p+0x4f),*buttons=(u8*)get(p+0x4b),*list=(u8*)get(p+0x53);
 if(!readable(callbacks,8)||!readable(buttons,8)||!multiplayer_control(list,0x5c6db8,0x6f)||get(list+0x25)!=get(callbacks+4))return sessions_refuse(p,3);
 u32 actions=0;
 for(u32 i=0;i<2;++i){u8* cb=(u8*)get(callbacks+4*i),*b=(u8*)get(buttons+4*i);
  if(!readable(cb,12)||get(cb)!=0x5c6604||get(cb+4)!=(i?0x4b0a30:0x4b0a00)||get(cb+8)!=(u32)p||!multiplayer_control(b,0x5c6c6c,0x59)||get(b+0x2d)!=i||get(b+0x25)!=get(callbacks))return sessions_refuse(p,4);
  if(multiplayer_available(b))actions|=1u<<i;
 }
 u32 count=get(list+0x49),selected=get(list+0x5b);u8* rows=(u8*)get(list+0x45);
 if(count>100||(count&&!readable(rows,count*257))||(selected!=0xffffffffu&&selected>=count))return sessions_refuse(p,5);
 for(u32 i=0;i<MNM_MENU_V16_SESSIONS_SIZE;++i)out[i]=0;
 put(out,16);put(out+4,5);put(out+8,2);put(out+12,get((void*)0x6de6b0));put(out+16,actions);put(out+20,p[0x5f]!=0);put(out+24,p[0x65]!=0);put(out+28,p[0x64]!=0);put(out+32,count);put(out+36,selected==0xffffffffu?0:selected+1);
 for(u32 i=0;i<count;++i)if(!battle_string(out+40+i*128,128,rows+i*257,128)||!out[40+i*128])return sessions_refuse(p,6);
 return 1;
}
static u32 sessions_dispatch(void* object,u32 action,u32 row){
 static u8 state[MNM_MENU_V16_SESSIONS_SIZE];if(!sessions_snapshot(object,state))return MNM_MENU_UNAVAILABLE;
 u8* p=object;if(p[0xc]||p[0x12]||get(p+0x33)||get(p+0x37)||get(p+0x3b)||get(p+0x43)||get(state+24)||get(state+28))return MNM_MENU_UNAVAILABLE;
 u32 index;
 if(action==MNM_MENU_SESSION_CANCEL){if(row)return MNM_MENU_INVALID;if(!(get(state+16)&2))return MNM_MENU_UNAVAILABLE;index=1;}
 else if(action==MNM_MENU_SESSION_JOIN){if(!row||row>get(state+32))return MNM_MENU_INVALID;if(!(get(state+16)&1)||!get(state+20))return MNM_MENU_UNAVAILABLE;
  // Original name lookup remains authoritative; row selection changes only after all guards.
  u8* list=(u8*)get(p+0x53);((ActionFn)0x4d1060)(list,0xffffffffu);((ActionFn)0x4d1060)(list,row-1);index=0;
 }else return MNM_MENU_UNSUPPORTED;
 record(2,object,index,0);u32 result=((ActionFn)0x4b0a00)(object,index);record(3,object,index,result);return MNM_MENU_OK;
}
