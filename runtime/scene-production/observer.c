/* Native diagnostic only; no pacing changes, state restoration or default use. */
#include "../shadow/win32_min.h"
API u32 WIN GetCurrentThreadId(void);
API int WIN QueryPerformanceCounter(void*);
API int WIN QueryPerformanceFrequency(void*);
#define THIS __attribute__((thiscall))
#define CASE_MAX 16u
#define ROW_WORDS 40u
#define DIFF_MAX (CASE_MAX*REGION_MAX*16u)
static u32 get(u32 at){const u8* p=(void*)at;return p[0]|(u32)p[1]<<8|(u32)p[2]<<16|(u32)p[3]<<24;}
static void put(void* out,u32 v){u8* p=out;p[0]=v;p[1]=v>>8;p[2]=v>>16;p[3]=v>>24;}
static void copy(void* out,const void* in,u32 n){u8* d=out;const u8* s=in;while(n--)*d++=*s++;}
static int same(const void* a,const void* b,u32 n){const u8* x=a;const u8* y=b;while(n--)if(*x++!=*y++)return 0;return 1;}
static int readable(u32 at,u32 n){
 if(!at||n>0xffffffffu-at)return 0;u32 end=at+n;
 while(at<end){u32 m[7];if(VirtualQuery((void*)at,m,28)!=28||m[4]!=0x1000||(m[5]&0x101)||!(m[5]&0xee)||m[0]+m[3]<=at)return 0;at=m[0]+m[3];}
 return 1;
}
#include "state_audit.h"
static struct SceneState before,after,first_before,first_after,bad_before,bad_after;
static u32 have_first,have_bad;
static u32 rows[CASE_MAX][ROW_WORDS],diffs[DIFF_MAX][8],diff_count;
static u32 count,primary_calls,queue_calls,mode,enabled,owner,foreign,failed,refused;
static u32 history_forward,history_skipped;
static u32 frequency[2];static HANDLE file;
#define SCENE_BYTES (12320u*36u)
static u8 primary_scene[SCENE_BYTES],extra_scene[SCENE_BYTES];
static u32 primary_scene_bytes,extra_scene_bytes,scene_capture,scene_phase,scene_bad;
static unsigned long long consumer_begin,consumer_ticks;static u32 consumer_active,consumer_calls;
u32 production_trampoline,production_slice_stack;
extern void production_enter(void),production_leave(void),production_draw_end(void),production_queue(void);
extern void production_history(void),production_queue_end_first(void),production_queue_end_second(void);
extern void production_repeat_full(u32,void*),production_repeat_slice(u32,void*);
struct SceneFrame {u32 sp,caller,object,selected,queues,histories,skipped;};
static struct SceneFrame frames[8];
static void finish(void);
static int owned(void){
 u32 tid=GetCurrentThreadId();__sync_bool_compare_and_swap(&owner,0,tid);
 if(tid==__atomic_load_n(&owner,__ATOMIC_ACQUIRE))return 1;
 __sync_fetch_and_add(&foreign,1);return 0;
}
void production_queue_seen(u32* r){
 u32 error=GetLastError();if(enabled&&owned()){
  ++queue_calls;
  if(scene_capture){
   u32 queue=r[6];
   if(!readable(queue,24)){++scene_bad;}
   else{
    u32 base=get(queue),n=get(queue+8),capacity=get(queue+12);
    if(!n||n>12320||n>capacity||!readable(base,n*36))++scene_bad;
    else if(!scene_phase){primary_scene_bytes=n*36;copy(primary_scene,(void*)base,n*36);}
    else{extra_scene_bytes=n*36;copy(extra_scene,(void*)base,n*36);}
   }
   if(scene_phase){QueryPerformanceCounter(&consumer_begin);consumer_active=1;}
  }
 }SetLastError(error);
}
void production_queue_done(void){
 u32 error=GetLastError();
 if(consumer_active&&enabled&&owned()){unsigned long long end=0;QueryPerformanceCounter(&end);consumer_ticks+=end-consumer_begin;++consumer_calls;consumer_active=0;}
 SetLastError(error);
}
void production_history_gate(u32* r){
 u32 error=GetLastError();r[3]=0; /* PUSHAD's ignored saved ESP is decision storage. */
 if(enabled&&owned()){
  if((mode==3||mode==4)&&scene_capture&&scene_phase&&r[9]==0x4fa4f4){
   u32 base=get(0x6def58),capacity=get(0x6def5c),object=r[6];
   if(object>=base&&(object-base)%0xe4b==0&&(object-base)/0xe4b<capacity){r[3]=1;++history_skipped;}
   else{++refused;++history_forward;}
  }else ++history_forward;
 }SetLastError(error);
}
void production_before(u32* r){
 u32 error=GetLastError();if(!enabled||!owned()){SetLastError(error);return;}
 u32* args=r+9;u32 sp=(u32)args+4;++primary_calls;
 for(u32 i=0;i<8;++i)if(!frames[i].sp){
  struct SceneFrame* f=frames+i;f->sp=sp;f->caller=args[0];f->object=r[6];f->queues=queue_calls;f->histories=history_forward;f->skipped=history_skipped;
  int caller=f->caller==0x46bbd8;
#ifdef MNM_SCENE_PRODUCTION_SELFTEST
  caller=1;
#endif
  f->selected=caller&&r[6]==0x689930&&count<CASE_MAX&&(!failed||mode==0)&&!refused&&!foreign&&
      primary_calls>=512&&primary_calls%128==0&&get(0x6cbb78+0xfd5a)==0&&get(0x6f34e0)==0x6cbb78;
  if(f->selected&&mode==0&&!state_capture(&before,f->object)){f->selected=0;++refused;}
  if(f->selected){scene_capture=1;scene_phase=0;scene_bad=primary_scene_bytes=extra_scene_bytes=0;}
  args[0]=(u32)production_leave;SetLastError(error);return;
 }
 ++refused;SetLastError(error);
}
static void retain(struct SceneState* a,struct SceneState* b){
 copy(a->regions,before.regions,before.count*sizeof(*before.regions));a->count=before.count;a->bytes=before.bytes;copy(a->data,before.data,before.bytes);
 copy(b->regions,after.regions,after.count*sizeof(*after.regions));b->count=after.count;b->bytes=after.bytes;copy(b->data,after.data,after.bytes);
}
static void compare(u32* row){
 if(!state_shape(&before,&after)){row[7]=2;++refused;return;}
 for(u32 i=0;i<before.count;++i){
  struct StateRegion* region=before.regions+i;u32 changes=0,saved=0;
  for(u32 j=0;j<region->bytes;++j){u8 a=before.data[region->offset+j],b=after.data[region->offset+j];if(a==b)continue;
   ++changes;
   if(saved<16&&diff_count<DIFF_MAX){u32* d=diffs[diff_count++];d[0]=count+1;d[1]=region->kind;d[2]=region->base;d[3]=j;d[4]=a;d[5]=b;d[6]=region->kind>=10;d[7]=region->bytes;++saved;}
   if(region->kind>=10&&!row[25]){row[25]=region->kind;row[26]=j;row[27]=a;row[28]=b;}
  }
  if(region->kind>=10&&changes){++row[10];row[11]+=changes;}
  else if(region->kind==1)row[12]=changes;
  else if(region->kind==2)row[13]=changes;
 }
 if(row[11]){row[7]=1;++failed;}
 if(!have_first){retain(&first_before,&first_after);have_first=1;}
 if(row[7]&&!have_bad){retain(&bad_before,&bad_after);have_bad=1;}
}
static void production_record(struct SceneFrame f,void* fx,u32 error){
  u32* row=rows[count];row[0]=count+1;row[1]=mode;row[2]=get(0x6c4830);row[3]=get(0x6cbb78+0xfd62);
  row[4]=f.object;row[5]=primary_calls;row[6]=queue_calls;row[18]=get(0x6c4830);row[20]=get(0x6cbb78+0xfd62);
  u32 history_base=history_forward;row[34]=history_base-f.histories;
  unsigned long long begin=0,end=0;
  if(foreign||(mode&&(get(f.object+0x231)||get(f.object+0x22d)!=get(f.object+0x229)||!state_capture(&before,f.object)))){row[7]=2;++refused;}
  else{
   row[8]=before.bytes;
   if(mode){
    scene_phase=1;consumer_ticks=0;consumer_calls=consumer_active=0;
    QueryPerformanceCounter(&begin);SetLastError(error);
    if(mode==1)production_repeat_full(f.object,fx);else production_repeat_slice(f.object,fx);
    production_slice_stack=0;QueryPerformanceCounter(&end);
   }
   if(!state_capture(&after,f.object)){row[7]=2;++refused;}
   else{row[9]=after.bytes;compare(row);}
  }
  row[37]=(u32)consumer_ticks;row[38]=(u32)(consumer_ticks>>32);row[39]=consumer_calls;
  row[14]=(u32)(end-begin);row[15]=(u32)((end-begin)>>32);
  row[16]=queue_calls-row[6];row[17]=queue_calls-f.queues;
  row[19]=get(0x6c4830);row[21]=get(0x6cbb78+0xfd62);row[29]=foreign;
  row[35]=history_skipped-f.skipped;row[36]=history_forward-history_base;
  row[30]=primary_scene_bytes/36;row[31]=extra_scene_bytes/36;row[33]=scene_bad;
  if(mode&&primary_scene_bytes==extra_scene_bytes){for(u32 k=0;k<primary_scene_bytes;++k)row[32]+=primary_scene[k]!=extra_scene[k];}
  else if(mode)row[32]=0xffffffffu;
  scene_capture=0;
  if(scene_bad||(mode&&(!primary_scene_bytes||!extra_scene_bytes))){row[7]=2;++refused;}
  if(mode&&row[16]!=1){row[7]=2;++refused;}
  if(mode&&row[32]){row[7]=2;++refused;}
  ++count;
  if(count==CASE_MAX||(mode&&(failed||refused)))finish();
}
void production_after(u32* r,void* fx){
 u32 error=GetLastError(),sp=(u32)r+40;
 for(u32 i=0;i<8;++i)if(frames[i].sp==sp){
  struct SceneFrame f=frames[i];frames[i].sp=0;r[9]=f.caller;
  if(f.selected&&enabled&&count<CASE_MAX&&mode!=4)production_record(f,fx,error);
  SetLastError(error);return;
 }
 ++refused;SetLastError(error);
}
void production_middle(u32* r,void* fx){
 u32 error=GetLastError();
 if(enabled&&mode==4&&owned()){
  /* Original four-register prologue remains on the stack at the cleanup marker. */
  u32 return_sp=(u32)r+56;
  for(u32 i=0;i<8;++i)if(frames[i].sp==return_sp){
   if(frames[i].selected&&count<CASE_MAX){
    struct SceneFrame f=frames[i];frames[i].selected=0;
    production_record(f,fx,error);
   }
   SetLastError(error);return;
  }
  ++refused;
 }
 SetLastError(error);
}

static int guards(void){
 static const u8 entry[5]={0x53,0x55,0x56,0x8b,0xf1},start[5]={0x8b,0x46,0x25,0x8b,0xce};
 static const u8 cleanup[20]={0x33,0xdb,0x8b,0x96,0x29,0x02,0,0,0x89,0x9e,0x31,0x02,0,0,0x89,0x96,0x2d,0x02,0,0};
 static const u8 end[7]={0xa0,0x0c,0x14,0x5e,0,0x84,0xc0};
 static const u8 q1[5]={0xe8,0xf0,0x2d,0,0},q2[5]={0xe8,0x47,0x2d,0,0};
 static const u8 history[5]={0xe8,0x6c,0x55,0x02,0};
 static const u8 qend1[5]={0xb9,0xe0,0x49,0x6a,0},qend2[5]={0xa1,0x2c,0x48,0x6c,0};
 return (u32)GetModuleHandleA(0)==0x400000&&readable(0x4fd280,0x500)&&readable(0x4fa4ef,5)&&
   same((void*)0x4fd280,entry,5)&&same((void*)0x4fd315,start,5)&&same((void*)0x4fd5ad,cleanup,20)&&same((void*)0x4fd5c1,end,7)&&
   same((void*)0x4fd4ab,q1,5)&&same((void*)0x4fd554,q2,5)&&same((void*)0x4fa4ef,history,5)&&same((void*)0x4fd4b0,qend1,5)&&same((void*)0x4fd559,qend2,5);
}
static int patch(u32 at,u32 target,u32 n,u8 opcode){
 u32 old,restore;if(!VirtualProtect((void*)at,n,0x40,&old))return 0;
 *(u8*)at=opcode;put((void*)(at+1),target-at-5);for(u32 i=5;i<n;++i)*(u8*)(at+i)=0x90;
 return FlushInstructionCache(GetCurrentProcess(),(void*)at,n)&&VirtualProtect((void*)at,n,old,&restore);
}
static int install(u32 candidate){
 if(candidate>4||production_trampoline||!guards())return 0;
 u32 a,b,c,d;__asm__ volatile("cpuid":"=a"(a),"=b"(b),"=c"(c),"=d"(d):"a"(1));if(!(d&(1u<<24)))return 0;
 u8* t=VirtualAlloc(0,10,0x3000,0x40);if(!t)return 0;
 copy(t,(void*)0x4fd280,5);t[5]=0xe9;put(t+6,0x4fd285-(u32)t-10);
 if(!FlushInstructionCache(GetCurrentProcess(),t,10)||!QueryPerformanceFrequency(frequency))return 0;
 production_trampoline=(u32)t;mode=candidate;
 if(!patch(0x4fd280,(u32)production_enter,5,0xe9)||!patch(0x4fd5c1,(u32)production_draw_end,7,0xe9)||
    !patch(0x4fd4ab,(u32)production_queue,5,0xe8)||!patch(0x4fd554,(u32)production_queue,5,0xe8)||
    !patch(0x4fa4ef,(u32)production_history,5,0xe8)||!patch(0x4fd4b0,(u32)production_queue_end_first,5,0xe9)||!patch(0x4fd559,(u32)production_queue_end_second,5,0xe9))return 0;
 enabled=1;return 1;
}
static int write(const void* p,u32 n){u32 done=0;while(done<n){u32 wrote=0;if(!WriteFile(file,(const u8*)p+done,n-done,&wrote,0)||!wrote||wrote>n-done)return 0;done+=wrote;}return 1;}
static int save_state(u32 tag,const struct SceneState* s){
 u32 h[4]={tag,s->count,s->bytes,0};return write(h,sizeof(h))&&write(s->regions,s->count*sizeof(*s->regions))&&write(s->data,s->bytes);
}
static void finish(void){
 if(!file)return;enabled=0;u32 h[16]={0};copy(h,"MNMSPR01",8);h[2]=5;h[3]=ROW_WORDS*4;h[4]=count;h[5]=mode;h[6]=failed;h[7]=refused;h[8]=foreign;h[9]=owner;h[10]=frequency[0];h[11]=frequency[1];h[12]=diff_count;h[13]=queue_calls;h[14]=primary_calls;h[15]=2*have_first+2*have_bad;
 if(write(h,sizeof(h))&&write(rows,count*sizeof(*rows))&&write(diffs,diff_count*sizeof(*diffs))){
  if(have_first){save_state(1,&first_before);save_state(2,&first_after);}
  if(have_bad){save_state(3,&bad_before);save_state(4,&bad_after);}
 }
 CloseHandle(file);file=0;
}
__declspec(dllexport) void SceneProductionAnchor(void){}
#ifdef MNM_SCENE_PRODUCTION_SELFTEST
__declspec(dllexport) u32 WIN SceneProductionInstallForTest(u32 candidate){u32 e=GetLastError(),ok=install(candidate);primary_calls=511;SetLastError(e);return ok;}
__declspec(dllexport) u32 WIN SceneProductionStatusForTest(u32 index){return index==0?count:index==1?failed:index==2?refused:index==3?queue_calls:index==4?rows[0][11]:index==5?rows[0][16]:index==6?history_skipped:index==8?rows[0][39]:history_forward;}
#endif
int WIN DllMain(void* dll,u32 reason,void* reserved){
 (void)dll;(void)reserved;u32 error=GetLastError();int ok=1;
 if(reason==1){
  char path[2048],option[2];u32 n=GetEnvironmentVariableA("MNM_SCENE_PRODUCTION_FILE",path,sizeof(path));
  if(n){
   u32 m=GetEnvironmentVariableA("MNM_SCENE_PRODUCTION_MODE",option,2);
   if(n>=sizeof(path)||m!=1||option[0]<'0'||option[0]>'4')ok=0;
   else{file=CreateFileA(path,0x40000000,0,0,1,0x80,0);if(file==(HANDLE)-1){file=0;ok=0;}else ok=install(option[0]-'0');}
  }
 }else if(reason==0)finish();SetLastError(error);return ok;
}
