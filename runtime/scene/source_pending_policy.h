/* Opt-in diagnostic admission under enforced read-only source pages.
 * Different canvas IDs do NOT establish disjoint storage. Aliased writes still
 * fault and reject the entire run. Only the pinned tags32/39 copy shapes is allowed;
 * no create/release/Lock/primitive/unknown or duplicate selected frame passes. */
static void source_pending_admission(struct Surface*,const u32*);
static int source_pending_copy_enabled(void){
 char flag[2];return GetEnvironmentVariableA("MNM_DRAW_SOURCE_PENDING_COPY",flag,2)==1&&flag[0]=='1';
}
static int source_lease_protected(const struct Surface* source){
 if(!source_lease_active||!source_lease_base||!source_lease_extent||source_lease_extent>~source_lease_base||source_lease_owner!=GetCurrentThreadId()||source_lease_canvas!=source->id||source_lease_object!=source->object||source_lease_pixels!=source->pixels)return 0;
 for(u32 at=source_lease_base;at<source_lease_base+source_lease_extent;){u32 m[7];if(VirtualQuery((void*)at,m,28)!=28||m[4]!=0x1000||m[5]!=2||m[0]+m[3]<=at)return 0;at=m[0]+m[3];}
 return 1;
}
/* protection=1 means either an already verified lease or acquisition followed
 * immediately by checked VirtualProtect under the same recorder scope. */
static u32 source_pending_check(const struct Surface* source,int protection){
 u32 selected=0;
 for(u32 i=0;i<MAX_FRAMES;++i)if(frames[i].sp){
  if(frames[i].sp==0xffffffffu||frames[i].primitive)return 1;
  if(frames[i].tag==7&&frames[i].owner==GetCurrentThreadId()&&frames[i].ecx==source->object){++selected;continue;}
  if(!protection||!source_pending_copy_enabled()||(frames[i].tag!=32&&frames[i].tag!=39)||!frames[i].owner||frames[i].owner==GetCurrentThreadId()||frames[i].ecx==source->object||!frames[i].record||!readable((u32)frames[i].record,96))return 1;
  const u32* r=(const u32*)frames[i].record;
  if(r[0]!=96||r[2]!=6||!r[3]||!r[4]||r[3]==source->id||r[4]==source->id)return 1;
  struct Surface *src=0,*dst=0;
  for(u32 j=0;j<MAX_SURFACES;++j)if(surfaces[j].object){if(surfaces[j].id==r[4])src=surfaces+j;if(surfaces[j].id==r[3])dst=surfaces+j;}
  if(!src||!dst||src->object!=frames[i].ecx||src->object==source->object||dst->object==source->object)return 1;
 }
 return selected==1?0:2;
}
