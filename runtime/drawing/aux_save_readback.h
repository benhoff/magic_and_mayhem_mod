#include "../../protocols/include/mnm/drawing_aux_save_readback_v1.h"
/* Explicit extra driver calls. Never hold journal_lock across those calls or
 * the Scene getter. The parent API remains pending until this probe finishes. */
static u32 aux_save_enabled,aux_save_records,aux_save_attempts,aux_save_images;
static u32 aux_save_refused,aux_save_budget_skips,aux_save_pending;
static HANDLE aux_save_file;
static int aux_save_recording(void){return __atomic_load_n(&aux_save_enabled,__ATOMIC_ACQUIRE)!=0;}
static void aux_save_emit(u32* r){r[0]=++aux_save_records;if(aux_save_records>MNM_AUX_SAVE_LIMIT+1||!write(aux_save_file,r,MNM_AUX_SAVE_RECORD_BYTES))ExitProcess(99);}
static int aux_save_open(void){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW_AUX_SAVE_READBACK",flag,2);
 if(!n)return 1;if(n!=1||flag[0]!='1'||!physical_enabled||!producer_points_enabled)return 0;
 char name[600];path(name,"aux-save-readback.bin");aux_save_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);
 if(aux_save_file==(HANDLE)-1)return 0;
 u32 h[16]={0};copy(h,MNM_AUX_SAVE_MAGIC,8);h[2]=1;h[3]=256;h[4]=0x40209ca7;
 h[5]=256;h[6]=30;h[7]=50;h[8]=5;h[9]=1;h[10]=40;h[11]=40;h[12]=3200;
 if(!write(aux_save_file,h,64))return 0;__atomic_store_n(&aux_save_enabled,1,__ATOMIC_RELEASE);return 1;
}
static int aux_save_pair(const u32* p){
 return p[0]==64&&p[1]==1&&p[16]==64&&p[17]==1&&
  (p[2]&31)==19&&(p[18]&31)==19&&p[6]==5&&p[22]==1&&
  p[7]==40&&p[8]==40&&p[23]==800&&p[24]==600&&p[4]==p[20];
}
static void aux_save_capture(struct Context* c,void* object,struct Table* t){
 if(!aux_save_recording()||!c->observed||c->r[1]!=1||c->r[2]!=7||
  (i32)c->r[8]<0||c->r[21]!=16||c->r[22]||c->r[23]||
  c->r[26]-c->r[24]!=40||c->r[27]-c->r[25]!=40||!t||
  !(t->kind==11||t->kind==12||t->kind==13||t->kind==14||t->kind==17))return;
 u32 error=GetLastError();u8 fp[512] __attribute__((aligned(16)));snapshot_fp(fp);
 ProducerPoint getter=__atomic_load_n(&producer_point,__ATOMIC_ACQUIRE);
 u32 r[64]={0},after[32]={0};void* data=0;u32 desc[31]={0};
 r[1]=1;r[2]=c->r[29];r[3]=c->r[6];r[4]=GetTickCount();r[5]=c->r[8];r[6]=c->r[9];r[7]=t->kind;
 if(!getter||!getter(object,(void*)c->args[2],r+24,128)||!aux_save_pair(r+24))goto restore;
 if(r[28]<30||r[28]>50)goto restore;
 EnterCriticalSection(journal_lock);
 if(!aux_save_recording()){LeaveCriticalSection(journal_lock);goto restore;}
 if(aux_save_attempts>=256){++aux_save_budget_skips;LeaveCriticalSection(journal_lock);goto restore;}
 ++aux_save_attempts;++aux_save_pending;LeaveCriticalSection(journal_lock);
 typedef i32(WIN *Desc)(void*,void*);typedef i32(WIN *Lock)(void*,void*,void*,u32,HANDLE);typedef i32(WIN *Unlock)(void*,void*);
 desc[0]=t->kind>=14?124:108;r[9]=1;r[12]=((Desc)t->original[22])(object,desc);
 if((i32)r[12]<0){r[8]=MNM_AUX_SAVE_DESCRIPTOR;goto complete;}
 if(desc[3]!=40||desc[2]!=40||(desc[26]&0x200)||desc[21]!=16||desc[22]!=0xf800||desc[23]!=0x7e0||desc[24]!=31){r[8]=MNM_AUX_SAVE_FORMAT;goto complete;}
 if(!aux_save_recording()){r[8]=MNM_AUX_SAVE_POINT;goto complete;}
 desc[0]=t->kind>=14?124:108;r[10]=1;r[13]=((Lock)t->original[25])(object,0,desc,0x4810,0);
 if((i32)r[13]<0){r[8]=MNM_AUX_SAVE_LOCK;goto complete;}
 r[16]=desc[3];r[17]=desc[2];r[18]=desc[4];r[19]=desc[21];r[20]=desc[22];r[21]=desc[23];r[22]=desc[24];
 if(r[16]!=40||r[17]!=40||r[19]!=16||r[20]!=0xf800||r[21]!=0x7e0||r[22]!=31||r[18]<80||r[18]>32768)r[8]=MNM_AUX_SAVE_FORMAT;
 else if(!readable((void*)desc[9],39*r[18]+80))r[8]=MNM_AUX_SAVE_MEMORY;
 else if(!(data=VirtualAlloc(0,3200,0x3000,4)))r[8]=MNM_AUX_SAVE_ALLOC;
 else for(u32 y=0;y<40;++y)copy((u8*)data+y*80,(void*)(desc[9]+y*r[18]),80);
 r[11]=1;r[14]=((Unlock)t->original[32])(object,t->kind>=14?0:(void*)desc[9]);
 if((i32)r[14]<0)r[8]=MNM_AUX_SAVE_UNLOCK;
 if(!getter(object,(void*)c->args[2],after,128)||!aux_save_pair(after)||after[10]!=r[34])r[8]=MNM_AUX_SAVE_CHANGED;
 r[56]=after[2];r[57]=after[3];r[58]=after[4];r[59]=after[6];r[60]=after[10];r[61]=after[12];
complete:
 EnterCriticalSection(journal_lock);
 if(aux_save_recording()){
  if(!r[8]&&data){char name[600],suffix[]="aux-save-oracle-000001.565";u32 n=aux_save_images+1;for(i32 i=21;i>=16;--i){suffix[i]=(char)('0'+n%10);n/=10;}path(name,suffix);
   HANDLE f=CreateFileA(name,0x40000000,3,0,1,0x80,0);int okay=f!=(HANDLE)-1;if(okay){okay=write(f,data,3200);if(!CloseHandle(f))okay=0;}
   if(okay){r[15]=++aux_save_images;r[23]=3200;}else r[8]=MNM_AUX_SAVE_WRITE;
  }
  if(r[8])++aux_save_refused;aux_save_emit(r);
 }
 if(!aux_save_pending)ExitProcess(99);--aux_save_pending;LeaveCriticalSection(journal_lock);
restore:
 if(data)VirtualFree(data,0,0x8000);restore_fp(fp);SetLastError(error);
}
static void aux_save_close(u32 reason){
 if(!aux_save_file)return;__atomic_store_n(&aux_save_enabled,0,__ATOMIC_RELEASE);
 u32 r[64]={0};r[1]=4;r[2]=reason;r[3]=GetCurrentThreadId();r[4]=GetTickCount();
 r[5]=aux_save_attempts;r[6]=aux_save_images;r[7]=aux_save_refused;r[8]=aux_save_budget_skips;r[9]=aux_save_pending;r[10]=pending;
 aux_save_emit(r);if(!CloseHandle(aux_save_file))ExitProcess(99);aux_save_file=0;
}
