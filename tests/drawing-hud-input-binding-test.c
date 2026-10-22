#include "../runtime/shadow/win32_min.h"
API u32 WIN GetTickCount(void);
API void WIN Sleep(u32);
static u32 menu_thread,menu_screen=2;
static u8* owned;
static u32 now,calls,mode;
static u32 get(const void* p){return *(const u32*)p;}
static void copy(void* a,const void* b,u32 n){u8* x=a;const u8* y=b;while(n--)*x++=*y++;}
static int readable(const void* p,u32 n){u32 m[7];return VirtualQuery(p,m,28)==28&&m[4]==0x1000&&!(m[5]&0x101)&&(u32)p+n>=(u32)p&&(u32)p+n<=m[0]+m[3];}
#define MNM_HUD_STATE_GLOBALS ((u32)owned+0x100)
#define MNM_HUD_STATE_INDEX ((u32)owned+0x104)
#define MNM_HUD_STATE_CLASS ((u32)owned+0x108)
#define MNM_HUD_STATE_POOL ((u32)owned+0x110)
#define MNM_HUD_STATE_CAMERA ((u32)owned+0x400)
#include "../runtime/menu/drawing_hud_state.h"
static void require(int value,u32 code){if(!value)ExitProcess(code);}
static u32 WIN point(u32 owner,void* output,u32 size){
 require(owner==menu_thread&&size==64,201);++calls;
 u32 r[16]={1,64,0x40209ca7,calls==4?2u:3u,owner,calls==4?2u:1u,1,0,0,0,0,1,1,now,400,300};
 if(mode==2)r[11]=2;
 if(mode==3)r[3]=0;
 if(mode==4)r[4]=owner+1;
 copy(output,r,64);
 if(calls==3)return 0; /* Deliberately poisoned failed getter output. */
 return mode==1?2:1;
}
static void emit_api(void){
 char name[1024];u32 n=GetEnvironmentVariableA("MNM_FIXTURE_DIRECTORY",name,900);require(n&&n<900,202);
 const char suffix[]="\\session.bin";copy(name+n,suffix,sizeof(suffix));
 HANDLE f=CreateFileA(name,0x40000000,3,0,1,0x80,0);require(f!=(HANDLE)-1,203);
 u32 h[16]={0},a[32]={0},t[32]={0},wrote=0;
 copy(h,"MNMDAPI1",8);h[2]=1;h[3]=128;h[4]=0x40209ca7;h[5]=262144;h[6]=1024;h[7]=1073741824;h[8]=2;
 a[0]=1;a[1]=2;a[2]=3;a[6]=menu_thread;a[8]=1;a[11]=now;a[27]=400;a[28]=300;a[29]=1;a[31]=1;
 t[0]=2;t[1]=4;t[2]=1;t[6]=menu_thread;t[21]=1;
 require(WriteFile(f,h,64,&wrote,0)&&wrote==64,204);
 require(WriteFile(f,a,128,&wrote,0)&&wrote==128,205);
 require(WriteFile(f,t,128,&wrote,0)&&wrote==128&&CloseHandle(f),206);
}
void start(void){
 char value[2];if(GetEnvironmentVariableA("MNM_HUD_BINDING_CASE",value,2)==1)mode=(u32)(value[0]-'0');
 menu_thread=GetCurrentThreadId();owned=VirtualAlloc(0,65536,0x3000,4);require(owned!=0,207);
 u32 actor=(u32)owned+0x1000;*(u32*)MNM_HUD_STATE_GLOBALS=actor;*(u32*)MNM_HUD_STATE_POOL=actor;*(u32*)(MNM_HUD_STATE_POOL+4)=1;
 *(u32*)(actor+4)=1;*(u32*)(actor+0xe4)=100;*(u32*)(actor+0xa8)=27;
 require(hud_state_open(),208);hud_state_input_point=point;now=GetTickCount();
 for(u32 i=0;i<4;++i){hud_state_sample();if(i<3){u32 begin=GetTickCount();while(GetTickCount()-hud_state_tick<60){require(GetTickCount()-begin<2000,211);Sleep(10);}}}
 hud_state_close();require(hud_state_count==4,212);require(calls==4,213);require(hud_state_rows[2][22]==8,214);
 for(u32 i=23;i<32;++i)require(!hud_state_rows[2][i],210);
 hud_state_close();emit_api();ExitProcess(0);
}
