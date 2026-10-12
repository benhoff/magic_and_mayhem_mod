#include "../../protocols/include/mnm/menu_v17.h"
static u8 lobby_payload[MNM_MENU_V17_LOBBY_SIZE];
static u32 lobby_diagnostics;
static int lobby_refuse(u8* p,u32 why){if(get(p+8)==1&&!(lobby_diagnostics&(1u<<why))){lobby_diagnostics|=1u<<why;record(42,p,why,0);}return 0;}
static int lobby_snapshot(void* object,u8* out){
 u8* p=object;if(p!=(u8*)0x6a4b1c||!readable(p-4,0x4c4)||get(p)!=0x5c689c||get(p-4)!=0x5c68cc||get(p+4)!=24||get(p+8)!=1)return 0;
 if(!readable((void*)0x6f349c,0x48)||get((void*)0x6f349c)!=5||get((void*)0x6f34e0)!=(u32)p||get((void*)0x6f34b4)!=(u32)p||get((void*)0x6f34b0)!=0x6de750||get((void*)0x6f34ac)!=0x6e0020||get((void*)0x6f34a8)!=0x657ce0||
    !readable((void*)0x6de750,12)||get((void*)0x6de750)!=0x5c660c||get((void*)0x6de754)!=15||get((void*)0x6de758)||
    !readable((void*)0x6e0020,8)||get((void*)0x6e0020)!=0x5c641c||get((void*)0x6e0024)!=22||!readable((void*)0x657ce0,8)||get((void*)0x657ce0)!=0x5c63e4||get((void*)0x657ce4)!=3)return lobby_refuse(p,1);
 if(!readable((void*)0x68991c,8)||get((void*)0x68991c)!=1||get((void*)0x689920)!=2||!readable((void*)0x6e1f0c,4)||get((void*)0x6e1f0c)||!readable((void*)0x6de6ac,12)||get((void*)0x6de6ac)!=1||get((void*)0x6de6b4)!=1||get((void*)0x6de6b0)<1||get((void*)0x6de6b0)>3)return lobby_refuse(p,2);
 if(get(p+0x8b)!=0x6903e0||!readable((void*)0x6903e0,8)||get((void*)0x6903e4)!=(u32)(p-4)||get(p+0x9b)!=15||get(p+0x8f))return lobby_refuse(p,3);
 u32 serviceTable=get((void*)0x6903e0);if(!readable((void*)serviceTable,0x48)||get((void*)(serviceTable+0x44))<0x400000||get((void*)(serviceTable+0x44))>=0x5a0000)return lobby_refuse(p,13);
 u8* callbacks=(u8*)get(p+0x4b),*buttons=(u8*)get(p+0x57);if(!readable(callbacks,20)||!readable(buttons,4))return lobby_refuse(p,4);
 u8* cb=(u8*)get(callbacks),*cancel=(u8*)get(buttons);
 if(!readable(cb,16)||get(cb)!=0x5c67f0||get(cb+4)!=0x4b7bf0||get(cb+8)||get(cb+12)!=(u32)(p-4)||!multiplayer_control(cancel,0x5c6c6c,0x59)||get(cancel+0x2d)||get(cancel+0x25)!=(u32)cb)return lobby_refuse(p,5);
 u8* headings=(u8*)get(p+0x5b);if(!readable(headings,160)||!readable((void*)0x6c3758,56)||!readable((void*)0x658f10,4*0x133))return lobby_refuse(p,6);
 u8* mapLabel=(u8*)get(headings+13*4);if(!readable(mapLabel,12))return lobby_refuse(p,7);
 for(u32 i=0;i<MNM_MENU_V17_LOBBY_SIZE;++i)out[i]=0;
 put(out,15);put(out+4,5);put(out+8,2);put(out+12,get((void*)0x6de6b0));put(out+16,multiplayer_available(cancel));put(out+20,get((void*)0x6c3758));
 copy(out+32,(void*)0x6c375c,52);
 u8 absent[32]={0};if(!battle_string(absent,32,(void*)0x6ead98,32))return lobby_refuse(p,8);
 for(u32 i=0;i<4;++i){u8* player=(u8*)(0x658f10+i*0x133),*row=out+84+i*48;
  if(!battle_string(row+16,32,player,21))return lobby_refuse(p,9);int active=!equal(row+16,absent,32);
  if((i==0&&!active)||(i&&active))return lobby_refuse(p,10);
  u32 portrait=get(player+0x15),colour=get(player+0x1d),handicap=get(player+0x19);
  if(active&&(portrait>11||colour>7||handicap>50))return lobby_refuse(p,11);
  put(row,active);put(row+4,portrait);put(row+8,colour);put(row+12,handicap);
 }
 if(!battle_string(out+276,128,(void*)0x6de59c,128)||!out[276]||!battle_string(out+404,128,(void*)get(mapLabel+8),128))return lobby_refuse(p,12);
 return 1;
}
static u32 lobby_dispatch(void* object,u32 action,u32 argument){
 u8 state[MNM_MENU_V17_LOBBY_SIZE];if(!lobby_snapshot(object,state))return MNM_MENU_UNAVAILABLE;
 u8* p=object;if(p[0xc]||get(p+0x33)||get(p+0x37)||get(p+0x3b)||get(p+0x43))return MNM_MENU_UNAVAILABLE;
 if(action!=MNM_MENU_LOBBY_CANCEL)return MNM_MENU_UNSUPPORTED;if(argument)return MNM_MENU_INVALID;if(!get(state+16))return MNM_MENU_UNAVAILABLE;
 // Callback receiver is the full object; tick/state owner is its primary menu+4.
 record(2,object,0,0);u32 result=((ActionFn)0x4b7bf0)(p-4,0);record(3,object,0,result);return MNM_MENU_OK;
}
