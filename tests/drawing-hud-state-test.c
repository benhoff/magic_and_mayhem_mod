#include "../runtime/shadow/win32_min.h"
API u32 WIN GetTickCount(void);
API void WIN Sleep(u32);
API HANDLE WIN CreateThread(void*,u32,u32(WIN*)(void*),void*,u32,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
static u32 menu_thread,menu_screen=2;
static u32 get(const void* p){return *(const u32*)p;}
static void copy(void* a,const void* b,u32 n){u8* x=a;const u8* y=b;while(n--)*x++=*y++;}
static int readable(const void* p,u32 n){u32 m[7];return VirtualQuery(p,m,28)==28&&m[4]==0x1000&&!(m[5]&0x101)&&(u32)p+n>=(u32)p&&(u32)p+n<=m[0]+m[3];}
static u8* owned;
static u8 before[65536];
#define MNM_HUD_STATE_GLOBALS ((u32)owned+0x100)
#define MNM_HUD_STATE_INDEX ((u32)owned+0x104)
#define MNM_HUD_STATE_CLASS ((u32)owned+0x108)
#define MNM_HUD_STATE_POOL ((u32)owned+0x110)
#define MNM_HUD_STATE_CAMERA ((u32)owned+0x400)
#include "../runtime/menu/drawing_hud_state.h"
static void require(int ok,u32 code){if(!ok)ExitProcess(code);}
static void sample(void){
 copy(before,owned,65536);hud_state_sample();
 for(u32 i=0;i<65536;++i)require(before[i]==owned[i],212);
}
static void advance(void){u32 begin=GetTickCount();while(GetTickCount()-hud_state_tick<60){require(GetTickCount()-begin<2000,218);Sleep(10);}}
static u32 WIN foreign(void* ignored){(void)ignored;hud_state_sample();return 0;}
void start(void){
 menu_thread=GetCurrentThreadId();
 owned=VirtualAlloc(0,65536,0x3000,4);require(owned!=0,201);
 u32 base=(u32)owned+0x1000,actor=base+0xe4b;
 *(u32*)MNM_HUD_STATE_POOL=base;*(u32*)(MNM_HUD_STATE_POOL+4)=2;
 *(u32*)MNM_HUD_STATE_GLOBALS=actor;*(u32*)MNM_HUD_STATE_INDEX=4;*(u32*)MNM_HUD_STATE_CLASS=2;
 *(u32*)actor=1;*(u32*)(actor+0xa8)=14;*(u32*)(actor+0x174)=1;*(u32*)(actor+4)=1;*(u32*)(actor+0xe4)=100;
 *(u32*)(MNM_HUD_STATE_CAMERA+0x31)=2;
 for(u32 i=0;i<6;++i)*(u32*)(MNM_HUD_STATE_CAMERA+0x39+i*4)=i+10;
 require(hud_state_open(),202);sample();require(hud_state_count==1&&hud_state_rows[0][4]==15&&hud_state_rows[0][9]==2&&hud_state_rows[0][7]==1,203);
 sample();require(hud_state_count==1,204);
 advance();*(u32*)MNM_HUD_STATE_GLOBALS=actor+1;sample();require(hud_state_rows[1][4]==11&&hud_state_rows[1][21]==2&&hud_state_rows[1][9]==0,205);
 advance();base=(u32)owned+0x5000;actor=base;*(u32*)MNM_HUD_STATE_POOL=base;*(u32*)MNM_HUD_STATE_GLOBALS=actor;*(u32*)actor=0;*(u32*)(actor+0xa8)=10;
 sample();require(hud_state_count==3&&hud_state_rows[2][7]==2&&hud_state_rows[2][9]==1&&hud_state_rows[2][10]==10,206);
 u32 count=hud_state_count;advance();HANDLE thread=CreateThread(0,0,foreign,0,0,0);
 require(thread&&WaitForSingleObject(thread,5000)==0&&CloseHandle(thread)&&hud_state_count==count,219);
 u32 owner=menu_thread;menu_thread=0;sample();require(hud_state_count==count,220);menu_thread=owner;
 *(u32*)MNM_HUD_STATE_POOL=0xfffffff0u;sample();require(hud_state_count==4&&!(hud_state_rows[3][4]&6),221);
 hud_state_close();ExitProcess(0);
}
