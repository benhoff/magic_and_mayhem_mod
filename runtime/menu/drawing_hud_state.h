API u32 WIN GetCurrentThreadId(void);
API void* WIN GetProcAddress(HANDLE,const char*);
#ifndef MNM_HUD_STATE_GLOBALS
#define MNM_HUD_STATE_GLOBALS 0x656600
#define MNM_HUD_STATE_INDEX 0x657d40
#define MNM_HUD_STATE_CLASS 0x6a5134
#define MNM_HUD_STATE_POOL 0x6def58
#define MNM_HUD_STATE_CAMERA 0x689930
#endif
/* Read-only candidate hit and actual camera fields, sampled on menu owner thread.
 * Pool IDs describe observed numeric base changes, not allocations/generations. */
static HANDLE hud_state_file;
static u32 hud_state_input_enabled;
typedef u32(WIN *HudInputPoint)(u32,void*,u32);
static HudInputPoint hud_state_input_point;
static u32 hud_state_rows[4096][32],hud_state_count,hud_state_dropped,hud_state_tick;
static u32 hud_state_pool_base,hud_state_pool_id;
static int hud_state_open(void){
 char name[1024];u32 n=GetEnvironmentVariableA("MNM_DRAW_HUD_STATE_FILE",name,sizeof(name));if(!n)return 1;if(n>=sizeof(name))return 0;
 char flag[2];SetLastError(0);u32 flag_n=GetEnvironmentVariableA("MNM_DRAW_HUD_INPUT_POINT",flag,2);
 if(flag_n){if(flag_n!=1||flag[0]!='1')return 0;hud_state_input_enabled=1;}else if(GetLastError()!=203)return 0;
 hud_state_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(hud_state_file==(HANDLE)-1){hud_state_file=0;return 0;}return 1;
}
static void hud_state_sample(void){
 if(!hud_state_file||!menu_thread||menu_thread!=GetCurrentThreadId())return;u32 tick=GetTickCount();if(hud_state_count&&tick-hud_state_tick<50)return;hud_state_tick=tick;
 if(hud_state_count>=4096){++hud_state_dropped;return;}
 u32* r=hud_state_rows[hud_state_count++];r[0]=hud_state_count;r[1]=tick;r[2]=menu_thread;r[3]=menu_screen;
 if(readable((void*)MNM_HUD_STATE_GLOBALS,4)&&readable((void*)MNM_HUD_STATE_INDEX,4)&&readable((void*)MNM_HUD_STATE_CLASS,4)){
  r[4]|=1;r[5]=get((void*)MNM_HUD_STATE_CLASS);r[6]=get((void*)MNM_HUD_STATE_INDEX);
  u32 actor=get((void*)MNM_HUD_STATE_GLOBALS);r[21]=actor?2:0;
  if(readable((void*)MNM_HUD_STATE_POOL,8)){
   u32 base=get((void*)MNM_HUD_STATE_POOL),capacity=get((void*)(MNM_HUD_STATE_POOL+4));
   if(base&&capacity&&capacity<=256&&base+capacity*0xe4b>=base){
    if(base!=hud_state_pool_base){hud_state_pool_base=base;++hud_state_pool_id;}
    r[4]|=2;r[7]=hud_state_pool_id;r[8]=capacity;
    if(actor>=base&&(actor-base)%0xe4b==0&&(actor-base)/0xe4b<capacity&&readable((void*)actor,0xe4b)){
     u32 slot=(actor-base)/0xe4b;
     if(get((void*)actor)==slot){r[4]|=4;r[21]=1;r[9]=slot+1;r[10]=get((void*)(actor+0xa8));r[11]=get((void*)(actor+0x174));r[12]=get((void*)(actor+4));r[13]=get((void*)(actor+0xe4));}
    }
   }
  }
 }
 /* Existing exact static caller binds ECX689930. Readability alone is not
  * current World activation or object allocation ownership. */
 if(readable((void*)MNM_HUD_STATE_CAMERA,0x244)){
  r[4]|=8;r[14]=get((void*)(MNM_HUD_STATE_CAMERA+0x31));
  for(u32 i=0;i<6;++i)r[15+i]=get((void*)(MNM_HUD_STATE_CAMERA+0x39+i*4));
 }
 if(hud_state_input_enabled){
  if(!hud_state_input_point){void* module=GetModuleHandleA("MnmDrawing.dll");if(module)hud_state_input_point=(HudInputPoint)GetProcAddress(module,"DrawingInputPointV1");}
  u32 point[16]={0};
  if(!hud_state_input_point||!hud_state_input_point(menu_thread,point,64)){r[22]=8;return;}
  if(point[0]!=1||point[1]!=64||point[2]!=0x40209ca7||point[3]&~3u||point[4]!=menu_thread)ExitProcess(99);
  r[22]=point[3];r[23]=point[5];r[24]=point[6];r[25]=point[11];r[26]=point[12];r[27]=point[13];r[28]=point[14];r[29]=point[15];r[30]=point[7];r[31]=(point[8]?1u:0u)|(point[9]?2u:0u)|(point[10]?4u:0u);
 }
}
static void hud_state_close(void){
 if(!hud_state_file)return;u32 h[16]={0};copy(h,"MNMDHI01",8);h[2]=hud_state_input_enabled?2:1;h[3]=128;h[4]=0x40209ca7;h[5]=4096;h[6]=hud_state_count;h[7]=hud_state_dropped;h[8]=menu_thread;h[9]=50;
 u32 wrote=0;if(!WriteFile(hud_state_file,h,64,&wrote,0)||wrote!=64)ExitProcess(99);
 u32 bytes=hud_state_count*128,at=0;while(at<bytes){wrote=0;if(!WriteFile(hud_state_file,(u8*)hud_state_rows+at,bytes-at,&wrote,0)||!wrote||wrote>bytes-at)ExitProcess(99);at+=wrote;}
 if(!CloseHandle(hud_state_file))ExitProcess(99);hud_state_file=0;
}
