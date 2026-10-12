API u32 WIN GetTickCount(void);
/* Called under the byte getter's trace scope, immediately before return2. */
static u32 source_pending_recorded_lease;
static void source_pending_refusal(struct Surface* source,const u32* metadata,u32 reason){
 if(!source_lease_active||source_lease_canvas!=source->id||source_lease_owner!=GetCurrentThreadId()||source_pending_recorded_lease==source_lease_id)return;
 source_pending_recorded_lease=source_lease_id;
 u32 h[16]={0},count=0;for(u32 i=0;i<MAX_FRAMES;++i)if(frames[i].sp)++count;
 copy(h,"MNMSPF01",8);h[2]=1;h[3]=64;h[4]=0x40209ca7;h[5]=MAX_FRAMES;h[6]=count;h[7]=source_lease_id;h[8]=source->id;h[9]=GetCurrentThreadId();h[10]=GetTickCount();h[11]=sequence;h[12]=queue;h[13]=metadata[12];h[14]=reason;
 char name[260];path(name,"source-pending-refusal-",source_lease_id,".bin");HANDLE file=CreateFileA(name,0x40000000,0,0,1,0x80,0);if(file==(HANDLE)-1)return;
 if(!write(file,h,64)){CloseHandle(file);return;}count=0;
 for(u32 i=0;i<MAX_FRAMES;++i)if(frames[i].sp){
  u32 r[16]={0};r[0]=++count;r[1]=i+1;r[2]=frames[i].tag;r[3]=frames[i].owner;r[4]=frames[i].primitive;r[14]=frames[i].sp==0xffffffffu;
  if(frames[i].caller>=0x400000&&frames[i].caller<0x5c0000)r[5]=frames[i].caller-0x400000;
  for(u32 j=0;j<MAX_SURFACES;++j)if(surfaces[j].object&&surfaces[j].object==frames[i].ecx){r[6]=surfaces[j].id;break;}
  if(frames[i].record&&readable((u32)frames[i].record,96)){const u32* record=(const u32*)frames[i].record;r[7]=record[2];r[8]=record[3];r[9]=record[4];r[13]=1;}
  r[10]=frames[i].ecx==source->object;r[11]=r[8]&&r[8]==source->id;r[12]=r[9]&&r[9]==source->id;
  if(!write(file,r,64))break;
 }
 CloseHandle(file);
}
