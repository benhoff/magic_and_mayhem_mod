#define MNM_MENU_SELFTEST
#include "../runtime/menu/observer.c"
#include "campaign_quit_bytes.h"
#pragma section(".fixture", read, write, execute)
__declspec(allocate(".fixture")) volatile u8 fixture_space[0x300000];
static void require(int ok,u32 code){if(!ok)ExitProcess(code);}
static void host(u32 id,u32 action,u32 generation){u32* h=menu_words+4;u32 seq=menu_load(h);menu_store(h,seq+1);menu_store(h+3,id);menu_store(h+4,action);menu_store(h+5,generation);menu_store(h,seq+2);}
void start(void){
 fixture_space[0]=1;u32 old;require(VirtualProtect((void*)0x466000,0x1000,0x40,&old),50);require(VirtualProtect((void*)0x470000,0x290000,0x40,&old),1);require(VirtualProtect((void*)0x46a000,0x1000,0x40,&old),2);
 copy((void*)0x4b23f0,mini_button,sizeof(mini_button));copy((void*)0x4b24f0,mini_answer,sizeof(mini_answer));
 const u8 ret[]={0xc3},ret1[]={0xc2,4,0},ret3[]={0xc2,12,0},receiver[]={0x8b,0xc1,0xc3},allocate[]={0xb8,0,0x39,0x6e,0,0xc3};
 copy((void*)0x597850,allocate,sizeof(allocate));copy((void*)0x4cef40,receiver,sizeof(receiver));copy((void*)0x4cfbd0,ret1,sizeof(ret1));copy((void*)0x4cfbe0,ret1,sizeof(ret1));copy((void*)0x4cf010,ret3,sizeof(ret3));
 copy((void*)0x4a5000,ret,1);copy((void*)0x4cefa0,ret,1);copy((void*)0x597870,ret,1);copy((void*)0x4667c0,ret1,sizeof(ret1));copy((void*)0x474b40,ret,1);copy((void*)0x474b10,ret1,sizeof(ret1));*(u8*)0x4a39a0=0xc3;
 const u8 common[]={0x83,0xec,0x1c,0x56,0x8b,0xf1,0x57,0x8b,0xfe,0x5f,0x5e,0x83,0xc4,0x1c,0xb8,0x21,0x43,0,0,0xc3};
 const u8 resume[]={0x56,0x8b,0xf1,0xb9,0xd0,0x2d,0x6a,0,0x5e,0xb8,0x21,0x43,0,0,0xc3};
 copy((void*)0x5595d0,common,sizeof(common));copy((void*)0x46aef0,resume,sizeof(resume));put((void*)0x5c6654,0x5595d0);put((void*)0x5c5de4,0x46aef0);
 u8* p=(u8*)0x6a5088;put(p,0x5c6644);put(p+4,17);put(p+8,1);put(p+0x53,2);put(p+0x57,5);put(p+0x4f,0x6e3800);
 put((void*)0x6e3800,0x6e3820);put((void*)0x6e3820,0x5c6674);put((void*)0x6e3824,0x4b23f0);put((void*)0x6e3828,(u32)p);
 put((void*)0x6e3804,0x6e3840);put((void*)0x6e3840,0x5c6674);put((void*)0x6e3844,0x4b24f0);put((void*)0x6e3848,(u32)p);
 put((void*)0x68991c,0);put((void*)0x689920,5);put((void*)0x6f349c,5);put((void*)0x6f34e0,(u32)p);
 put((void*)0x6f34a8,0x657ce0);put((void*)0x6f34ac,0x659408);put((void*)0x6f34b0,0x6cbb78);put((void*)0x6f34b4,(u32)p);
 put((void*)0x6cbb78,0x5c5dd8);put((void*)0x6cbb7c,2);put((void*)0x659408,0x5c6a60);put((void*)0x65940c,4);
 menu_init();require(menu_words&&menu_version==10&&!menu_retired,3);u8 snapshot[32];require(mini_snapshot(p,snapshot,2)&&get(snapshot)==0&&get(snapshot+16)==5&&get(snapshot+24)==2,4);
 require(!mini_snapshot(p,snapshot,0),5);
 const u32 fields[]={0,4,0x53,0x57,0x4f};for(u32 i=0;i<5;++i){u32 before=get(p+fields[i]);put(p+fields[i],0xffffffff);require(!mini_snapshot(p,snapshot,2),10+i);put(p+fields[i],before);}
 const u32 globals[]={0x68991c,0x689920,0x6f349c,0x6f34b0,0x6e3828};for(u32 i=0;i<5;++i){u32 before=get((void*)globals[i]);put((void*)globals[i],0xffffffff);require(!mini_snapshot(p,snapshot,2),20+i);put((void*)globals[i],before);}
 SetLastError(0xabc123);menu_poll(p,0);require(menu_ready&&menu_screen==17&&GetLastError()==0xabc123,30);
 host(1,MNM_MENU_MINI_PREFERENCES,menu_generation);menu_poll(p,1);require(menu_status==MNM_MENU_UNSUPPORTED&&!get(p+0x43)&&!get(p+0x33),31);
 host(2,MNM_MENU_MINI_QUIT,menu_generation);menu_poll(p,1);require(menu_status==MNM_MENU_OK&&!get(p+0x43)&&get(p+0x3b)==0x6e3900,32);
 campaign_quit_observe(p);require(get((void*)0x6e3844)==(u32)&campaign_quit_answer,44);
 ((ActionFn)get((void*)0x6e3844))(p,1);require(get(p+0x43)==1&&!get(p+0x3b)&&!*(u8*)0x6dbc18&&!*(u8*)0x6dbc19&&GetLastError()==0xabc123,45);put(p+0x43,0);
 menu_poll(p,0);host(3,MNM_MENU_MINI_QUIT,menu_generation);menu_poll(p,1);require(menu_status==MNM_MENU_OK&&get(p+0x3b)==0x6e3900,46);
 ((ActionFn)get((void*)0x6e3844))(p,0);require(get(p+0x43)==1&&!get(p+0x3b)&&*(u8*)0x6dbc18==1&&!*(u8*)0x6dbc19&&GetLastError()==0xabc123,47);put(p+0x43,0);*(u8*)0x6dbc18=0;
 u32 answer=get((void*)0x6e3844);put((void*)0x6e3848,0);require(!mini_snapshot(p,snapshot,2),48);put((void*)0x6e3848,(u32)p);put((void*)0x6e3844,0);require(!mini_snapshot(p,snapshot,2),49);put((void*)0x6e3844,answer);
 host(4,MNM_MENU_MINI_CANCEL,menu_generation-1);menu_poll(p,1);require(menu_status==MNM_MENU_STALE&&!get(p+0x43),33);
 put(p+0x3b,0x6e3900);menu_poll(p,0);host(5,MNM_MENU_MINI_QUIT,menu_generation);menu_poll(p,1);require(menu_status==MNM_MENU_UNAVAILABLE&&!get(p+0x43),34);put(p+0x3b,0);
 menu_poll(p,0);host(6,MNM_MENU_MINI_CANCEL,menu_generation);menu_poll(p,1);require(menu_status==MNM_MENU_OK&&get(p+0x43)==1&&!get(p+0x33)&&GetLastError()==0xabc123,35);
 put(p+0x43,0);menu_poll(p,1);require(!get(p+0x43)&&menu_request==6,36);
 *(u8*)0x4b24f0=0x90;require(!install_campaign_mini_observe(),51);*(u8*)0x4b24f0=mini_answer[0];
 put((void*)0x5c5de4,0x46aef1);require(!install_campaign_mini_observe()&&get((void*)0x5c6654)==0x5595d0,37);put((void*)0x5c5de4,0x46aef0);
 *(u8*)0x5595d0=0x90;require(!install_campaign_mini_observe(),38);*(u8*)0x5595d0=common[0];require(install_campaign_mini_observe(),39);
 require(((TickFn)get((void*)0x5c6654))(p)==0x4321&&GetLastError()==0xabc123,40);
 require(((TickFn)get((void*)0x5c5de4))((void*)0x6cbb78)==0x4321&&GetLastError()==0xabc123,41);require(!install_campaign_mini_observe(),42);
 menu_retired=1;host(7,MNM_MENU_MINI_CANCEL,menu_generation);menu_poll(p,1);require(menu_status==MNM_MENU_RETIRED&&!get(p+0x43),43);
 ExitProcess(0);
}
