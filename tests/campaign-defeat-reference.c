#define MNM_MENU_SELFTEST
#include "../runtime/menu/observer.c"
#include "campaign_defeat_bytes.h"
#pragma section(".fixture", read, write, execute)
__declspec(allocate(".fixture")) volatile u8 fixture_space[0x300000];
static void require(int ok,u32 code){if(!ok)ExitProcess(code);}
static void host(u32 id,u32 action,u32 generation){u32* h=menu_words+4;u32 seq=menu_load(h);menu_store(h,seq+1);menu_store(h+3,id);menu_store(h+4,action);menu_store(h+5,generation);menu_store(h,seq+2);}
static u8 data[MNM_MENU_V11_DEFEAT_SIZE];
void start(void){
 fixture_space[0]=1;u32 old;require(VirtualProtect((void*)0x470000,0x290000,0x40,&old),1);
 copy((void*)0x4747a0,defeat_ok,sizeof(defeat_ok));copy((void*)0x557510,fade_start,sizeof(fade_start));
 const u8 ret[]={0xc3};copy((void*)0x4a39a0,ret,1);copy((void*)0x474b50,ret,1);
 u8* p=(u8*)0x6db967;put(p,0x5c5ebc);put(p+4,6);put(p+8,1);p[0x11]=1;put(p+0x47,0x6e3800);put(p+0x4b,0x6e3900);put(p+0x4f,0x6e3820);
 copy(p+0x57,"Defeat! (Forest)",17);copy(p+0x157,"You quit the battle.",21);
 put((void*)0x5c5ed4,0x474b50);put((void*)0x6e3800,0x6e3840);put((void*)0x6e3840,0x5c5f24);put((void*)0x6e3865,0x6e3820);put((void*)0x6e386d,21);
 put((void*)0x6e3820,0x5c5eec);put((void*)0x6e3824,0x4747a0);put((void*)0x6e3828,(u32)p);
 for(u32 i=0;i<21;++i){u8* label=(u8*)(0x6e4000+i*16);put((void*)(0x6e3900+4*i),(u32)label);put(label+8,i==19?(u32)(p+0x157):0x6e5000);}
 copy((void*)0x6e5000,"original display",17);
 put((void*)0x689920,5);put((void*)0x6f349c,5);put((void*)0x6f34e0,(u32)p);*(u8*)0x6dbc19=1;
 put((void*)0x6f34a8,0x657ce0);put((void*)0x6f34ac,0x659408);put((void*)0x6f34b0,0x6cbb78);put((void*)0x6f34b4,(u32)p);
 put((void*)0x657ce0,0x5c63e4);put((void*)0x657ce4,3);put((void*)0x659408,0x5c6a60);put((void*)0x65940c,4);put((void*)0x6cbb78,0x5c5dd8);put((void*)0x6cbb7c,2);
 defeat_quit_pending=1;menu_init();require(menu_words&&menu_version==11&&!menu_retired,2);require(defeat_snapshot(p,data)&&get(data)==1&&get(data+4)==5&&equal(data+16,p+0x57,17)&&equal(data+272+19*256,p+0x157,21),3);
 const u32 fields[]={0,4,0x53,0x47,0x4b,0x4f};for(u32 i=0;i<6;++i){u32 v=get(p+fields[i]);put(p+fields[i],0xffffffff);require(!defeat_snapshot(p,data),10+i);put(p+fields[i],v);}
 const u32 globals[]={0x689920,0x68991c,0x6f349c,0x6f34b0,0x6f34b4,0x6e3824,0x6e3828,0x6e3865,0x6e386d};for(u32 i=0;i<9;++i){u32 v=get((void*)globals[i]);put((void*)globals[i],0xffffffff);require(!defeat_snapshot(p,data),20+i);put((void*)globals[i],v);}
 defeat_quit_pending=0;require(!defeat_snapshot(p,data),40);defeat_quit_pending=1;
 *(u8*)0x6dbc19=0;require(!defeat_snapshot(p,data),30);*(u8*)0x6dbc19=1;
 for(u32 i=0;i<256;++i)p[0x57+i]='x';require(!defeat_snapshot(p,data),31);copy(p+0x57,"Defeat! (Forest)",17);
 put((void*)0x6e3900,0);require(defeat_snapshot(p,data)&&!data[272],32);put((void*)0x6e3900,0x6e4000);
 SetLastError(0xabc123);menu_poll(p,0);require(menu_screen==6&&menu_ready&&GetLastError()==0xabc123,33);
 host(1,MNM_MENU_DEFEAT_OK,menu_generation-1);menu_poll(p,1);require(menu_status==MNM_MENU_STALE&&!get(p+0x43),34);
 host(2,MNM_MENU_RESULT_QUIT,menu_generation);menu_poll(p,1);require(menu_status==MNM_MENU_UNSUPPORTED&&!get(p+0x43),35);
 put(p+0x3b,1);menu_poll(p,0);host(3,MNM_MENU_DEFEAT_OK,menu_generation);menu_poll(p,1);require(menu_status==MNM_MENU_UNAVAILABLE&&!get(p+0x43),36);put(p+0x3b,0);
 menu_poll(p,0);host(4,MNM_MENU_DEFEAT_OK,menu_generation);menu_poll(p,1);require(menu_status==MNM_MENU_OK&&get(p+0x43)==1&&p[0xc]==1&&get(p+0xd)==0&&GetLastError()==0xabc123,37);
 put(p+0x43,0);p[0xc]=0;menu_poll(p,1);require(!get(p+0x43)&&!p[0xc],38);
 menu_store(menu_words+5,0);host(5,MNM_MENU_DEFEAT_OK,menu_generation);menu_poll(p,1);require(menu_status==MNM_MENU_RETIRED&&!get(p+0x43),39);
 ExitProcess(0);
}
