#ifndef MNM_STATE_SCREEN_DEPTH
#define MNM_STATE_SCREEN_DEPTH 0x6f349c
#define MNM_STATE_TOP_SCREEN 0x6f34e0
#define MNM_STATE_WORLD 0x6cbb78
#define MNM_STATE_COUNTER 0x6c4830
#define MNM_STATE_PACING 0x6dbe95
#define MNM_STATE_EFFECT 0x6e1f00
#define MNM_STATE_ANI 0x6deffc
#endif
/* Bounded same-menu-thread scalar observation. No gameplay state mutation. */
static HANDLE semantic_state_file;
static u32 semantic_state_rows[4096][48],semantic_state_count,semantic_state_dropped,semantic_state_last_tick;
static int semantic_state_open(void){
 char name[1024];u32 n=GetEnvironmentVariableA("MNM_DRAW_SEMANTIC_STATE_FILE",name,sizeof(name));if(!n)return 1;if(n>=sizeof(name))return 0;
 semantic_state_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(semantic_state_file==(HANDLE)-1){semantic_state_file=0;return 0;}return 1;
}
static void semantic_state_sample(void){
 if(!semantic_state_file)return;u32 now=GetTickCount();if(semantic_state_count&&now-semantic_state_last_tick<50)return;
 semantic_state_last_tick=now;if(semantic_state_count>=4096){++semantic_state_dropped;return;}
 u32* r=semantic_state_rows[semantic_state_count++];r[0]=semantic_state_count;r[1]=now;r[2]=menu_thread;r[3]=menu_screen;r[4]=menu_generation;r[5]=menu_request;r[6]=menu_status;r[7]=menu_ready;
 if(readable((void*)MNM_STATE_SCREEN_DEPTH,4)&&readable((void*)MNM_STATE_TOP_SCREEN,4)){
  r[8]|=1;r[9]=get((void*)MNM_STATE_SCREEN_DEPTH);u32 top=get((void*)MNM_STATE_TOP_SCREEN);
  if(top&&readable((void*)top,12)){r[8]|=2;r[10]=get((void*)(top+4));r[11]=get((void*)(top+8));}
 }
 if(readable((void*)MNM_STATE_COUNTER,4)&&readable((void*)MNM_STATE_WORLD,0xfd66)&&get((void*)MNM_STATE_WORLD)==0x5c5dd8){r[8]|=4;r[12]=get((void*)MNM_STATE_COUNTER);r[13]=get((void*)(MNM_STATE_WORLD+0xfd62));r[14]=*(u8*)(MNM_STATE_WORLD+0xfd5a);}
 if(readable((void*)MNM_STATE_PACING,8)){r[8]|=8;r[15]=get((void*)MNM_STATE_PACING);r[16]=get((void*)(MNM_STATE_PACING+4));}
 if(readable((void*)MNM_STATE_EFFECT,4)){r[8]|=16;r[17]=get((void*)MNM_STATE_EFFECT);}
 /* Selected fields in eight48byte global ANI controllers, no retained pointers. */
 if(readable((void*)MNM_STATE_ANI,8*48)){r[8]|=32;for(u32 i=0;i<8;++i){u32 p=MNM_STATE_ANI+i*48;r[18+i*3]=get((void*)(p+8));r[19+i*3]=get((void*)(p+12));r[20+i*3]=get((void*)(p+16));}}
}
static void semantic_state_close(void){
 if(!semantic_state_file)return;u32 h[16]={0};copy(h,"MNMDSS01",8);h[2]=1;h[3]=192;h[4]=0x40209ca7;h[5]=4096;h[6]=semantic_state_count;h[7]=semantic_state_dropped;h[8]=menu_thread;h[9]=50;
 u32 wrote=0;if(!WriteFile(semantic_state_file,h,64,&wrote,0)||wrote!=64)ExitProcess(99);
 u32 bytes=semantic_state_count*192,at=0;while(at<bytes){wrote=0;if(!WriteFile(semantic_state_file,(u8*)semantic_state_rows+at,bytes-at,&wrote,0)||!wrote||wrote>bytes-at)ExitProcess(99);at+=wrote;}
 if(!CloseHandle(semantic_state_file))ExitProcess(99);semantic_state_file=0;
}
