#define MNM_MENU_SELFTEST
#include "../runtime/menu/observer.c"
#include "menu_fixture_bytes.h"
#pragma section(".fixture", read, write, execute)
__declspec(allocate(".fixture")) volatile u8 fixture_space[0x300000];
static u8 object[128],display[0x30];
static u32 helper_count,host_seq,beat,command_id;
static u32* channel;
static void require(int ok,u32 code){if(!ok)ExitProcess(code);}
static void reset(void){for(u32 i=0;i<128;++i)object[i]=0;put(object,0x5c63e4);put(object+4,3);put(object+8,1);helper_count=0;}
static void THIS fixture_callback(void* p,u32 index){u8* cb=p;((void (THIS *)(void*,u32))get(cb+4))((void*)get(cb+8),index);}
static u32 THIS fixture_tick(void* p){menu_poll(p,1);menu_poll(p,0);return 0x13579bdf;}
#include "campaign-preferences-selftest.h"
void start(void){
 fixture_space[0]=1;u32 old;require(VirtualProtect((void*)0x470000,0x290000,0x40,&old),1);
 copy((void*)0x4cdf40,slider_set,sizeof(slider_set));const u8 noop[]={0xc2,4,0};copy((void*)0x4a0000,noop,3);put((void*)0x5c0030,0x4a0000);
 const u8 ftol[]={0x83,0xec,8,0xdf,0x3c,0x24,0x8b,4,0x24,0x8b,0x54,0x24,4,0x83,0xc4,8,0xc3};copy((void*)0x59bee0,ftol,sizeof(ftol));
 u8 helper[]={0xff,0x05,0,0,0,0,0xc6,0x41,0x0c,1,0xc7,0x41,0x0d,0,0,0,0,0xc3};put(helper+2,(u32)&helper_count);copy((void*)0x557510,helper,sizeof(helper));
 preferences_fixture_init();menu_init();require(menu_words&&menu_version==12&&!menu_retired,2);channel=menu_words;host_seq=channel[4];beat=channel[6];
 u8* mini=(u8*)0x6a5088;put(mini,0x5c6644);put(mini+4,17);put(mini+8,1);put(mini+0x53,2);put(mini+0x57,5);put(mini+0x4f,0x6e3800);
 put((void*)0x6e3800,0x6e3820);put((void*)0x6e3820,0x5c6674);put((void*)0x6e3824,0x4b23f0);put((void*)0x6e3828,(u32)mini);
 put((void*)0x6e3804,0x6e3840);put((void*)0x6e3840,0x5c6674);put((void*)0x6e3844,0x4b24f0);put((void*)0x6e3848,(u32)mini);
 put((void*)0x689920,5);put((void*)0x6f349c,5);put((void*)0x6f34a8,0x657ce0);put((void*)0x6f34ac,0x659408);put((void*)0x6f34b0,0x6cbb78);put((void*)0x6f34b4,(u32)mini);put((void*)0x6f34e0,(u32)mini);
 put((void*)0x659408,0x5c6a60);put((void*)0x65940c,4);put((void*)0x6cbb78,0x5c5dd8);put((void*)0x6cbb7c,2);
 copy((void*)0x4b23f0,mini_button,sizeof(mini_button));*(u8*)0x4a39a0=0xc3;menu_poll(mini,0);require(menu_ready&&get(mini_payload+16)==7,7);
 preferences_fixture_host(MNM_MENU_MINI_PREFERENCES,menu_generation,0);menu_poll(mini,1);require(menu_status==MNM_MENU_OK&&get(mini+0x33)==0x6a4948&&get(mini+0x43)==1,8);
 preferences_fixture_reset();preferences_fixture_tick(fixture_tick);require(channel[35]&&channel[MNM_MENU_V6_PREFERENCES/4+2]==3,3);
 campaign_preferences_fixture_tests(fixture_tick);
 preferences_fixture_reset();preferences_fixture_tick(fixture_tick);require(channel[35]&&channel[MNM_MENU_V6_PREFERENCES/4+2]==3,4);
 preferences_fixture_host(MNM_MENU_PREFERENCES_OK,channel[33]-1,0);preferences_fixture_tick(fixture_tick);require(menu_status==MNM_MENU_STALE&&!pref_writes,5);
 preferences_fixture_host(MNM_MENU_PREFERENCES_OK,channel[33],0);channel[MNM_MENU_V6_HOST_PREFERENCES/4+6]=2;preferences_fixture_tick(fixture_tick);require(menu_status==MNM_MENU_INVALID&&!pref_writes&&!pref_audio,6);
 ExitProcess(0);
}
