/* Read-only whole-pool latch observations, independent of the current hit. */
#ifndef MNM_LATCH_POOL
#define MNM_LATCH_POOL 0x6def58
#define MNM_LATCH_HIT 0x656600
#define MNM_LATCH_COUNT 0x6e984c
#define MNM_LATCH_MODE 0x6903e8
#endif
typedef u32 (WIN *LatchInputPoint)(u32,void*,u32);
static LatchInputPoint latch_input_point;
static HANDLE latch_file;
static u32 latch_rows[2048][64],latch_count,latch_dropped,latch_foreign,latch_owner,latch_tick;
static u32 latch_pool_base,latch_pool_id;
static int latch_open(void){
 char name[1024];u32 n=GetEnvironmentVariableA("MNM_DRAW_ACTOR_LATCH_FILE",name,sizeof(name));
 if(!n)return GetLastError()==203;
 if(n>=sizeof(name)||latch_file)return 0;
 latch_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);
 if(latch_file==(HANDLE)-1){latch_file=0;return 0;}return 1;
}
static int latch_collect(u32* r,u32 base,u32 capacity){
 for(u32 slot=0;slot<capacity;++slot){
  u32 actor=base+slot*0xe4b;
  if(!readable((void*)actor,0x178))return 0;
  u32 active=get((void*)(actor+4));if(!active)continue;
  if(get((void*)actor)!=slot)return 0;
  u32 a=get((void*)(actor+0x68)),b=get((void*)(actor+0x70)),bit=1u<<(slot%32),word=slot/32;
  r[16+word]|=bit;++r[10];
  if(a){r[24+word]|=bit;++r[11];}
  if(b){r[32+word]|=bit;++r[12];}
  if((a||b)&&!r[57]){
   r[57]=slot+1;r[58]=get((void*)(actor+0xa8));r[59]=get((void*)(actor+0x174));
   r[60]=active;r[61]=get((void*)(actor+0xe4));r[62]=a;r[63]=b;
  }
 }
 return 1;
}
static void latch_sample(u32 kind,u32 call,u32 phase){
 if(!latch_file)return;
 u32 thread=GetCurrentThreadId();
 if(!latch_owner)__sync_bool_compare_and_swap(&latch_owner,0,thread);
 if(thread!=latch_owner){__sync_fetch_and_add(&latch_foreign,1);return;}
 u32 tick=GetTickCount();if(!kind&&latch_count&&tick-latch_tick<50)return;
 if(!kind)latch_tick=tick;
 if(latch_count>=2048){++latch_dropped;return;}
 u32* r=latch_rows[latch_count++];r[0]=latch_count;r[1]=tick;r[2]=thread;r[3]=menu_screen;r[4]=kind;r[5]=call;r[6]=phase;
 if(readable((void*)MNM_LATCH_POOL,8)){
  u32 base=get((void*)MNM_LATCH_POOL),capacity=get((void*)(MNM_LATCH_POOL+4));
  if(base&&capacity&&capacity<=256&&base+capacity*0xe4b>=base){
   u32 first[64],second[64];
   for(u32 i=0;i<64;++i){((volatile u32*)first)[i]=0;((volatile u32*)second)[i]=0;}
   if(latch_collect(first,base,capacity)&&latch_collect(second,base,capacity)&&
      get((void*)MNM_LATCH_POOL)==base&&get((void*)(MNM_LATCH_POOL+4))==capacity&&
      equal(first,second,sizeof(first))){
    if(base!=latch_pool_base){latch_pool_base=base;++latch_pool_id;}
    r[7]|=9;r[8]=latch_pool_id;r[9]=capacity;
    copy(r+10,first+10,12);copy(r+16,first+16,96);copy(r+57,first+57,28);
    if(readable((void*)MNM_LATCH_HIT,4)){
     u32 hit=get((void*)MNM_LATCH_HIT);r[56]=hit?2:0;
     if(hit>=base&&(hit-base)%0xe4b==0&&(hit-base)/0xe4b<capacity&&
        readable((void*)hit,0x178)&&get((void*)hit)==(hit-base)/0xe4b){r[56]=1;r[15]=(hit-base)/0xe4b+1;}
    }else r[56]=3;
   }
  }
 }
 if(readable((void*)MNM_LATCH_COUNT,4)&&readable((void*)MNM_LATCH_MODE,4)){
  r[7]|=2;r[13]=get((void*)MNM_LATCH_COUNT);r[14]=get((void*)MNM_LATCH_MODE);
 }
 if(!latch_input_point){void* module=GetModuleHandleA("MnmDrawing.dll");if(module)latch_input_point=(LatchInputPoint)GetProcAddress(module,"DrawingInputPointV1");}
 if(latch_input_point&&latch_input_point(thread,r+40,64))r[7]|=4;
}
static void latch_close(void){
 if(!latch_file)return;
 u32 h[16]={0};copy(h,"MNMDLA01",8);h[2]=1;h[3]=256;h[4]=0x40209ca7;h[5]=2048;h[6]=latch_count;h[7]=latch_dropped;h[8]=latch_owner;h[9]=latch_foreign;h[10]=50;
 u32 wrote=0;if(!WriteFile(latch_file,h,64,&wrote,0)||wrote!=64)ExitProcess(99);
 u32 size=latch_count*256,at=0;while(at<size){wrote=0;if(!WriteFile(latch_file,(u8*)latch_rows+at,size-at,&wrote,0)||!wrote||wrote>size-at)ExitProcess(99);at+=wrote;}
 if(!CloseHandle(latch_file))ExitProcess(99);latch_file=0;
}
