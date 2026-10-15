API u32 WIN GetTickCount(void);
/* All rows are recorder metadata; no disjointness/alias or ownership proof. */
static u32 source_pending_recorded_lease,source_pending_admissions;
static u32 source_pending_last_point[16];
static void source_pending_dump(struct Surface* source,const u32* metadata,u32 reason,const char* magic,u32 number,u32 admission){
 u32 h[16]={0},count=0;for(u32 i=0;i<MAX_FRAMES;++i)if(frames[i].sp)++count;
 copy(h,magic,8);h[2]=admission?2:1;h[3]=64;h[4]=0x40209ca7;h[5]=MAX_FRAMES;h[6]=count;h[7]=source_lease_id;h[8]=source->id;h[9]=GetCurrentThreadId();h[10]=GetTickCount();h[11]=sequence;h[12]=queue;h[13]=metadata[12];h[14]=reason;h[15]=admission;
 char name[260];path(name,admission?"source-pending-admission-":"source-pending-refusal-",number,".bin");HANDLE file=CreateFileA(name,0x40000000,0,0,1,0x80,0);if(file==(HANDLE)-1){if(admission)ExitProcess(99);return;}
 if(!write(file,h,64)){CloseHandle(file);if(admission)ExitProcess(99);return;}count=0;
 for(u32 i=0;i<MAX_FRAMES;++i)if(frames[i].sp){
  u32 r[16]={0};r[0]=++count;r[1]=i+1;r[2]=frames[i].tag;r[3]=frames[i].owner;r[4]=frames[i].primitive;r[14]=frames[i].sp==0xffffffffu;
  if(frames[i].caller>=0x400000&&frames[i].caller<0x5c0000)r[5]=frames[i].caller-0x400000;
  for(u32 j=0;j<MAX_SURFACES;++j)if(surfaces[j].object&&surfaces[j].object==frames[i].ecx){r[6]=surfaces[j].id;break;}
  if(frames[i].record&&readable((u32)frames[i].record,96)){const u32* record=(const u32*)frames[i].record;r[7]=record[2];r[8]=record[3];r[9]=record[4];r[13]=1;}
  r[10]=frames[i].ecx==source->object;r[11]=r[8]&&r[8]==source->id;r[12]=r[9]&&r[9]==source->id;
  if(!write(file,r,64)){if(admission)ExitProcess(99);break;}
 }
 if(!CloseHandle(file)&&admission)ExitProcess(99);
}

static void source_pending_refusal(struct Surface* source,const u32* metadata,u32 reason){
 if(!source_lease_active||source_lease_canvas!=source->id||source_lease_owner!=GetCurrentThreadId()||source_pending_recorded_lease==source_lease_id)return;
 source_pending_recorded_lease=source_lease_id;source_pending_dump(source,metadata,reason,"MNMSPF01",source_lease_id,0);
}
static void source_pending_admission(struct Surface* source,const u32* metadata){
 if(!source_lease_protected(source)||source_pending_check(source,1)||metadata[12]<2)ExitProcess(99);
 if(++source_pending_admissions>4096)ExitProcess(99);
 source_pending_dump(source,metadata,3,"MNMSPA01",source_pending_admissions,1);copy(source_pending_last_point,metadata,64);
}

/* Exact previous byte-copy admission point; no fresh original sampling. */
u32 WIN SceneDrawingSourceAdmissionV1(const u32* point,u32 size){
 u32 error=GetLastError(),result=0;
 if(size==64&&producer_trace_mode&&readable((u32)point,64)){
  PRODUCER_TRACE_SCOPE;
  if(source_lease_active&&source_lease_owner==GetCurrentThreadId()&&source_pending_admissions&&point[6]==source_lease_canvas){
   u32 same=1;for(u32 i=0;i<16;++i)if(point[i]!=source_pending_last_point[i])same=0;
   if(same)result=source_pending_admissions;
  }
 }
 SetLastError(error);return result;
}
