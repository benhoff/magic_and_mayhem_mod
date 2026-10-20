#include "../../protocols/include/mnm/input_v1.h"
#define MNM_CAMPAIGN_INPUT_ADMISSION_OBSERVER 1
/* Opt-in read-only World-return diagnostics. Numeric slots are observations,
 * never retained actor pointers or original allocation generations. */
static HANDLE admission_file;
static u32 *admission_input;
static u32 admission_count,admission_previous[32],admission_last_tick;
static int admission_open(void){
 char path[2048];u32 n=GetEnvironmentVariableA("MNM_MENU_INPUT_ADMISSION_OBSERVE",path,sizeof(path));
 if(!n)return 1;if(n>=sizeof(path))return 0;
 admission_file=CreateFileA(path,0x40000000,1,0,1,0x80,0);
 if(admission_file==(HANDLE)-1){admission_file=0;return 0;}
 u8 header[16]={'M','N','M','C','A','I','0','1',2,0,0,0,128,0,0,0};u32 wrote;
 if(!WriteFile(admission_file,header,16,&wrote,0)||wrote!=16){CloseHandle(admission_file);admission_file=0;return 0;}
 n=GetEnvironmentVariableA("MNM_RENDER_INPUT",path,sizeof(path));
 if(n&&n<sizeof(path)){
  HANDLE file=CreateFileA(path,0x80000000,3,0,3,0x80,0);
  if(file!=(HANDLE)-1){
   if(GetFileSize(file,0)==MNM_INPUT_V1_SIZE){
    HANDLE map=CreateFileMappingA(file,0,2,0,MNM_INPUT_V1_SIZE,0);
    if(map){admission_input=MapViewOfFile(map,4,0,0,MNM_INPUT_V1_SIZE);CloseHandle(map);}
   }CloseHandle(file);
  }
 }
 return 1;
}
static void admission_sample(u32 kind,u32 mask,const u32* mouse){
 if(!admission_file)return;
 u32 error=GetLastError(),r[32]={0},tick=GetTickCount();
 r[0]=admission_count+1;r[1]=tick;r[2]=GetCurrentThreadId();r[3]=kind;
 u8* bar=(void*)0x6a2dd0;
 if(readable(bar,0x1925)){
  r[4]|=1;r[5]=get(bar+0x1518);r[6]=get(bar+0x1908);
  u8* actor=(u8*)get(bar+0x18ec);
  if(readable(actor,4)&&gameplay_creature(get(actor))==actor){r[4]|=2;r[7]=get(actor)+1;}
 }
 if(readable((void*)0x6903e8,4)&&readable((void*)0x5e04c8,4)&&
    readable((void*)0x6a5134,4)&&readable((void*)0x657d40,4)&&readable((void*)0x641fbc,4)){
  r[4]|=4;r[8]=get((void*)0x6903e8);r[9]=get((void*)0x5e04c8);
  r[10]=get((void*)0x6a5134);r[11]=get((void*)0x657d40);r[12]=get((void*)0x641fbc);
 }
 if(admission_input&&equal(admission_input,MNM_INPUT_V1_MAGIC,8)&&
    admission_input[2]==1&&admission_input[3]==MNM_INPUT_V1_SIZE){
  u32 sequence=__atomic_load_n(admission_input+4,__ATOMIC_ACQUIRE);
  if(!(sequence&1)){
   r[13]=sequence;
   for(u32 i=0;i<3;++i)r[14+i]=__atomic_load_n(admission_input+5+i,__ATOMIC_RELAXED);
   r[17]=__atomic_load_n(admission_input+17,__ATOMIC_RELAXED);
   r[18]=__atomic_load_n(admission_input+18,__ATOMIC_RELAXED);
   r[19]=__atomic_load_n(admission_input+8,__ATOMIC_RELAXED);r[20]=__atomic_load_n(admission_input+9,__ATOMIC_RELAXED);
   __atomic_thread_fence(__ATOMIC_ACQUIRE);
   if(sequence==__atomic_load_n(admission_input+4,__ATOMIC_ACQUIRE))r[4]|=8;
   else for(u32 i=13;i<=20;++i)r[i]=0;
  }
 }
 if(readable((void*)0x6c4830,4)){r[4]|=16;r[21]=get((void*)0x6c4830);}
 r[22]=gameplay_sequence;
 if(readable((void*)0x6c5484,28)&&readable((void*)0x6cbb64,4)){
  r[4]|=32;r[23]=get((void*)0x6c5488);r[24]=get((void*)0x6c5484);r[25]=get((void*)0x6cbb64);
  r[26]=get((void*)0x6c5494);r[27]=get((void*)0x6c5498);r[28]=get((void*)0x6c549c);
 }
 if(mouse){r[4]|=64;r[29]=mask;r[30]=(mouse[0]&65535)|((mouse[1]&65535)<<16);
  r[31]=(mouse[3]?1u:0u)|(mouse[4]?2u:0u)|(mouse[5]?4u:0u)|(mouse[6]?8u:0u);}

 /* Ignore heartbeat sequence alone when deduplicating. Keep at least one
  * bounded sample every500ms, and all observed key/selection changes. */
 int same=admission_count&&r[3]==admission_previous[3]&&r[4]==admission_previous[4];
 for(u32 i=5;i<=20;++i)if(i!=13&&r[i]!=admission_previous[i])same=0;
 if(kind==1&&same&&tick-admission_last_tick<500){SetLastError(error);return;}
 if(admission_count>=4095)r[4]|=0x80000000u;
 u32 wrote=0;
 if(!WriteFile(admission_file,r,128,&wrote,0)||wrote!=128||r[4]&0x80000000u){CloseHandle(admission_file);admission_file=0;}
 else{++admission_count;copy(admission_previous,r,128);admission_last_tick=tick;}
 SetLastError(error);
}
static void admission_close(void){
 if(admission_file){CloseHandle(admission_file);admission_file=0;}
 if(admission_input){UnmapViewOfFile(admission_input);admission_input=0;}
}
