/* Opt-in passive selection and synchronous Region command-buffer admission.
 * No producer detour, delivery claim, retained actor pointer or simulation write. */
#define MNM_CAMPAIGN_ORDER_OBSERVER 1
static HANDLE order_file;
static u32 order_count,order_tick,order_owner,order_calls;
/* valid,node,header,write,used,count,index,capacity,cursor */
static void order_manager(u32* out){
 for(u32 i=0;i<9;++i)out[i]=0;
 u8* manager=(void*)0x6a2898;
 if(!readable(manager,0x38)||!get(manager+0x34))return;
 u32 capacity=get(manager+0x28),cursor=get(manager+0x30),table=get(manager+0x24);
 if(!capacity||capacity>65536||cursor>capacity||!table)return;
 u32 index=(cursor?cursor:capacity)-1,node=table+index*21;
 if(node<table||!readable((void*)node,21))return;
 u32 header=get((void*)node),size=get((void*)(node+4)),write=get((void*)(node+8)),used=get((void*)(node+12));
 if(size<20||size>1048576||used>size||header+size<header||write<header+20||write>header+size||!readable((void*)header,20))return;
 out[0]=1;out[1]=node;out[2]=header;out[3]=write;out[4]=used;out[5]=get((void*)(header+16));out[6]=index;out[7]=size;out[8]=cursor;
}
static int order_open(void){
 char path[2048];u32 n=GetEnvironmentVariableA("MNM_MENU_ORDER_ADMISSION_OBSERVE",path,sizeof(path));
 if(!n)return 1;if(n>=sizeof(path))return 0;
 const u8 producer[]={0x8b,0x41,0x34,0x85,0xc0,0x75,0x04,0x33};
 if(!readable((void*)0x537bd0,8)||!equal((void*)0x537bd0,producer,8))return 0;
 order_file=CreateFileA(path,0x40000000,1,0,1,0x80,0);
 if(order_file==(HANDLE)-1){order_file=0;return 0;}
 u32 h[4]={0},wrote=0;copy(h,"MNMCOA01",8);h[2]=3;h[3]=272;
 if(!WriteFile(order_file,h,16,&wrote,0)||wrote!=16){CloseHandle(order_file);order_file=0;return 0;}return 1;
}
static void order_sample(u32 kind,u32 call,u32 mask,u32* before){
 if(!order_file)return;
 u32 error=GetLastError(),tick=GetTickCount(),thread=GetCurrentThreadId(),r[68],now[9];
 for(u32 i=0;i<68;++i)((volatile u32*)r)[i]=0;
 if(kind==1&&order_count&&tick-order_tick<250){SetLastError(error);return;}
 if(!order_owner)order_owner=thread;
 r[64]=admission_world_returns;
 r[0]=order_count+1;r[1]=tick;r[2]=thread;r[3]=kind;r[5]=call;r[6]=mask;
 if(thread!=order_owner)r[4]|=16;
 if(readable((void*)0x6c4830,4))r[7]=get((void*)0x6c4830);
 if(readable((void*)MNM_LATCH_POOL,8)){
  u32 base=get((void*)MNM_LATCH_POOL),capacity=get((void*)(MNM_LATCH_POOL+4));
  if(base&&capacity&&capacity<=256&&base+capacity*0xe4b>=base){
   u32 a[64],b[64];
   for(u32 i=0;i<64;++i){((volatile u32*)a)[i]=0;((volatile u32*)b)[i]=0;}
   if(latch_collect(a,base,capacity)&&latch_collect(b,base,capacity)&&equal(a,b,sizeof(a))&&
      get((void*)MNM_LATCH_POOL)==base&&get((void*)(MNM_LATCH_POOL+4))==capacity){
    r[4]|=1;r[8]=capacity;copy(r+9,a+10,12);copy(r+12,a+24,64);
    /* V2: the original candidate pointer is a numeric observation, not the
     * final command receiver or an allocation generation. */
    if(readable((void*)MNM_LATCH_HIT,4)){
     u32 hit=get((void*)MNM_LATCH_HIT);
     if(!hit)r[4]|=64;
     else if(hit>=base&&(hit-base)%0xe4b==0){
      u32 slot=(hit-base)/0xe4b;
      if(slot<capacity&&(a[16+slot/32]&(1u<<(slot%32)))&&readable((void*)hit,0x178)&&get((void*)hit)==slot){r[4]|=64;r[63]=slot+1;}
     }
    }
   }
  }
 }
 if(readable((void*)0x6e984c,4)&&readable((void*)0x6903e8,4)&&readable((void*)0x5e04c8,4)){
  r[4]|=32;r[60]=get((void*)0x6e984c);r[61]=get((void*)0x6903e8);r[62]=get((void*)0x5e04c8);
 }
 order_manager(now);
 if(now[0]){r[4]|=2;r[28]=now[8];r[29]=now[6];r[30]=now[4];r[31]=now[5];r[32]=now[3]-now[2];r[33]=now[7];}
 if(kind==2&&before)copy(before,now,sizeof(now));
 if(kind==3&&before&&before[0]&&now[0]&&before[1]==now[1]&&before[2]==now[2]&&before[6]==now[6]&&before[7]==now[7]&&
    now[3]>=before[3]&&now[4]>=before[4]&&now[5]>=before[5]&&now[3]-before[3]==now[4]-before[4]){
  u32 size=now[4]-before[4],count=now[5]-before[5];r[34]=size;r[35]=count;
  if(size>96||count>8)r[4]|=8;
  else if((!size&&!count)||(size&&count&&readable((void*)before[3],size))){
   u32 offset=0,records=0;
   while(offset+12<=size){u32 length=get((void*)(before[3]+offset+8));if(length<12||length>size-offset)break;offset+=length;++records;}
   if(offset==size&&records==count){r[4]|=4;copy(r+36,(void*)before[3],size);}
  }
 }
 if(order_count>=8191)r[4]|=0x80000000u;
 u32 wrote=0;
 if(!WriteFile(order_file,r,272,&wrote,0)||wrote!=272||r[4]&0x80000000u){CloseHandle(order_file);order_file=0;}
 else{++order_count;if(kind==1)order_tick=tick;}
 SetLastError(error);
}
static void order_close(void){if(order_file){CloseHandle(order_file);order_file=0;}}
