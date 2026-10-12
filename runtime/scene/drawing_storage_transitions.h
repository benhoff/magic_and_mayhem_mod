/* Diagnostic identities are observations within one process, never ownership. */
static HANDLE storage_transition_file;
static u32 storage_transition_records,storage_transition_calls,storage_transition_returns;
static u32 storage_transition_addresses[512],storage_transition_address_count;
static u32 storage_transition_epoch;
static u32 storage_address_id(u32 address){
 if(!address)return 0;
 for(u32 i=0;i<storage_transition_address_count;++i)if(storage_transition_addresses[i]==address)return i+1;
 if(storage_transition_address_count>=512)ExitProcess(99);
 storage_transition_addresses[storage_transition_address_count++]=address;return storage_transition_address_count;
}
static int storage_transitions_open(void){
 char flag[2];u32 n=GetEnvironmentVariableA("MNM_DRAW_STORAGE_TRANSITIONS",flag,2);
 if(!n)return 1;if(n!=1||flag[0]!='1'||!producer_trace_mode)return 0;
 char name[260];path(name,"canvas-storage-transitions-",0,".bin");
 storage_transition_file=CreateFileA(name,0x40000000,3,0,1,0x80,0);
 if(storage_transition_file==(HANDLE)-1){storage_transition_file=0;return 0;}
 u32 h[16]={0};copy(h,"MNMCST02",8);h[2]=1;h[3]=96;h[4]=0x40209ca7;h[5]=131072;h[6]=512;
 return write(storage_transition_file,h,64);
}
static void storage_transition_record(struct Frame* f,u32 phase,u32 result){
 if(!storage_transition_file||f->tag>2)return;
 if(phase==1)f->storage_call=++storage_transition_calls;
 else if(!f->storage_call)return;
 if(storage_transition_records>=131071)ExitProcess(99);
 u32 r[24]={0};r[0]=++storage_transition_records;r[1]=phase;r[2]=f->storage_call;r[3]=f->tag;r[4]=f->owner;
 r[5]=f->caller>=0x400000&&f->caller<0x5c0000?f->caller-0x400000:0;
 r[6]=sequence;r[7]=queue;r[8]=failures;
 struct Surface* s=object(f->ecx);if(s){r[9]=s->id;r[10]=storage_address_id(s->pixels);r[11]=s->width;r[12]=s->height;r[13]=s->stride;r[14]|=2;}
 if(readable(f->ecx,28)){r[14]|=1;r[15]=storage_address_id(get((void*)(f->ecx+4)));r[16]=storage_address_id(get((void*)(f->ecx+8)));r[17]=get((void*)(f->ecx+16));r[18]=get((void*)(f->ecx+20));r[19]=get((void*)(f->ecx+24));}
 if(phase==2){++storage_transition_returns;
  if(f->tag==2){r[20]=++storage_transition_epoch;r[21]=storage_address_id(result);}
  /* Create/release EAX is unspecified and is never exported. */
 }
 if(!write(storage_transition_file,r,96))ExitProcess(99);
}
static void storage_transitions_close(void){
 if(!storage_transition_file)return;
 u32 r[24]={0};r[0]=++storage_transition_records;r[1]=4;r[2]=storage_transition_calls;r[3]=storage_transition_returns;r[4]=storage_transition_calls-storage_transition_returns;
 r[5]=sequence;r[6]=queue;r[7]=failures;r[8]=storage_transition_address_count;r[9]=storage_transition_epoch;
 if(!write(storage_transition_file,r,96)||!CloseHandle(storage_transition_file))ExitProcess(99);storage_transition_file=0;
}
