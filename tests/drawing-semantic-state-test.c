#include "../runtime/shadow/win32_min.h"
API u32 WIN GetTickCount(void);
API void WIN Sleep(u32);
static u32 menu_thread=18,menu_screen=17,menu_generation=2,menu_request=3,menu_status=1,menu_ready=1;
static u32 get(const void* p){return *(const u32*)p;}
static void copy(void* a,const void* b,u32 n){u8* x=a;const u8* y=b;while(n--)*x++=*y++;}
static int readable(const void* p,u32 n){u32 m[7];return VirtualQuery(p,m,28)==28&&m[4]==0x1000&&!(m[5]&0x101)&&((u32)p+n)<=(m[0]+m[3]);}
static u8* owned;
#define MNM_STATE_SCREEN_DEPTH ((u32)owned)
#define MNM_STATE_TOP_SCREEN ((u32)owned+4)
#define MNM_STATE_WORLD ((u32)owned+0x1000)
#define MNM_STATE_COUNTER ((u32)owned+8)
#define MNM_STATE_CAMERA ((u32)owned+12)
#define MNM_STATE_EFFECT ((u32)owned+20)
#define MNM_STATE_ANI ((u32)owned+32)
#include "../runtime/menu/drawing_semantic_state.h"
static void require(int ok,u32 code){if(!ok)ExitProcess(code);}
void start(void){
 owned=VirtualAlloc(0,0x20000,0x3000,4);require(owned!=0,201);
 *(u32*)MNM_STATE_SCREEN_DEPTH=5;*(u32*)MNM_STATE_TOP_SCREEN=((u32)owned+0x800);*(u32*)((u32)owned+0x804)=17;*(u32*)((u32)owned+0x808)=1;
 *(u32*)MNM_STATE_WORLD=0x5c5dd8;*(u32*)MNM_STATE_COUNTER=40;*(u32*)(MNM_STATE_WORLD+0xfd62)=40;*(u8*)(MNM_STATE_WORLD+0xfd5a)=2;
 *(u32*)MNM_STATE_CAMERA=100;*(u32*)(MNM_STATE_CAMERA+4)=200;*(u32*)MNM_STATE_EFFECT=9;
 for(u32 i=0;i<8;++i)for(u32 j=0;j<3;++j)*(u32*)(MNM_STATE_ANI+i*48+8+j*4)=i*3+j;
 require(semantic_state_open(),202);semantic_state_sample();require(semantic_state_count==1&&semantic_state_rows[0][8]==63&&semantic_state_rows[0][12]==40&&semantic_state_rows[0][14]==2,203);
 semantic_state_sample();require(semantic_state_count==1,204);u32 start_tick=GetTickCount();while(GetTickCount()-semantic_state_last_tick<60){require(GetTickCount()-start_tick<2000,218);Sleep(10);}semantic_state_sample();require(semantic_state_count==2,205);
 require(*(u32*)MNM_STATE_COUNTER==40&&*(u32*)(MNM_STATE_WORLD+0xfd62)==40,206);semantic_state_close();ExitProcess(0);
}
