/* Isolated No-CD diagnostic. No pacing/tick/state mutation; memory-only events
 * while gameplay runs. Scripted import staging verifies the complete source PE. */
#include "../shadow/win32_min.h"
API u32 WIN GetCurrentThreadId(void);
API int WIN QueryPerformanceCounter(void*);
API int WIN QueryPerformanceFrequency(void*);
#define LIMIT 262144u
#define WORDS 24u
static u32 rows[LIMIT][WORDS],count,dropped,owner,foreign,enabled;
static HANDLE file;
static u32 frequency[2];
static u32 get(u32 at){const u8* p=(void*)at;return p[0]|(u32)p[1]<<8|(u32)p[2]<<16|(u32)p[3]<<24;}
static void put(void* out,u32 v){u8* p=out;p[0]=v;p[1]=v>>8;p[2]=v>>16;p[3]=v>>24;}
static int same(u32 at,const u8* bytes,u32 n){while(n--)if(*(u8*)at++!=*bytes++)return 0;return 1;}
extern void cadence_pacer(void),cadence_sequence(void),cadence_tick(void);
extern void cadence_animation(void),cadence_scheduler(void),cadence_maintenance(void),cadence_queue(void);
struct Patch {u32 site,length;void(*entry)(void);u8 bytes[6];};
static struct Patch patches[]={
 {0x557403,5,cadence_pacer,{0xe8,0x38,0xca,0xf8,0xff}},
 {0x46b45f,6,cadence_sequence,{0x89,0xbe,0x62,0xfd,0,0}},
 {0x46b496,5,cadence_tick,{0xa3,0x30,0x48,0x6c,0}},
 {0x46b735,5,cadence_maintenance,{0xe8,0x16,0xe7,9,0}},
 {0x46b79f,5,cadence_scheduler,{0xe8,0xdc,0xef,0x0b,0}},
 {0x46b74f,5,cadence_animation,{0xe8,0x6c,0x97,0xff,0xff}},
 {0x46b759,5,cadence_animation,{0xe8,0x62,0x97,0xff,0xff}},
 {0x46b763,5,cadence_animation,{0xe8,0x58,0x97,0xff,0xff}},
 {0x46b76d,5,cadence_animation,{0xe8,0x4e,0x97,0xff,0xff}},
 {0x46b777,5,cadence_animation,{0xe8,0x44,0x97,0xff,0xff}},
 {0x46b781,5,cadence_animation,{0xe8,0x3a,0x97,0xff,0xff}},
 {0x46b78b,5,cadence_animation,{0xe8,0x30,0x97,0xff,0xff}},
 {0x46b795,5,cadence_animation,{0xe8,0x26,0x97,0xff,0xff}},
 {0x4fd4ab,5,cadence_queue,{0xe8,0xf0,0x2d,0,0}},
 {0x4fd554,5,cadence_queue,{0xe8,0x47,0x2d,0,0}}
};
void cadence_record(u32* registers,u32 event){
 if(!enabled)return;
 u32 error=GetLastError(),tid=GetCurrentThreadId();
 __sync_bool_compare_and_swap(&owner,0,tid);
 if(tid!=__atomic_load_n(&owner,__ATOMIC_ACQUIRE)){__sync_fetch_and_add(&foreign,1);SetLastError(error);return;}
 if(count==LIMIT){++dropped;SetLastError(error);return;}
 u32* r=rows[count];r[0]=count+1;r[1]=event;r[2]=tid;
 if(!QueryPerformanceCounter(r+3)){++dropped;SetLastError(error);return;}
 r[5]=get(0x6c4830);
 r[6]=get(0x6cbb78+0xfd62);r[7]=get(0x6dbe95);r[8]=get(0x6dbe99);
 r[9]=get(0x6e1f00);r[10]=*(u8*)(0x6cbb78+0xfd5a);r[11]=get(0x6f34e0);
 r[12]=registers[6];r[13]=event==2?0x46b45f:event==3?0x46b496:registers[9];r[14]=registers[7];
 if(event==5||event==15){ /* eight static global ANI controllers, never retained pointers */
  u32 caller=registers[9],p=0;
  if(caller>=0x46b754&&caller<=0x46b79a&&(caller-0x46b754)%10==0)
   p=0x6deffc+(caller-0x46b754)/10*48;
  if(p&&(event==15||registers[6]==p))
   for(u32 i=0;i<9;++i)r[15+i]=get(p+i*4);
 }
 if(event==6||event==16){r[15]=get(0x6df188);r[16]=get(0x6def58+0x258);r[17]=get(0x6def58+0x25c);r[18]=get(0x6def58+0x264);}
 ++count;SetLastError(error);
}
static int install(void){
 char path[2048];u32 n=GetEnvironmentVariableA("MNM_ENGINE_CADENCE_FILE",path,sizeof(path));
 if(!n)return 1;
 if(n>=sizeof(path)||(u32)GetModuleHandleA(0)!=0x400000)return 0;
 /* Pinned .text/.data and CPUID FXSR guards precede every mutation. */
 u32 a,b,c,d;__asm__ volatile("cpuid":"=a"(a),"=b"(b),"=c"(c),"=d"(d):"a"(1));
 if(!(d&(1u<<24)))return 0;
 for(u32 i=0;i<sizeof(patches)/sizeof(*patches);++i)
  if(!same(patches[i].site,patches[i].bytes,patches[i].length))return 0;
 if(!QueryPerformanceFrequency(frequency)||(!frequency[0]&&!frequency[1]))return 0;
 file=CreateFileA(path,0x40000000,0,0,1,0x80,0);if(file==(HANDLE)-1){file=0;return 0;}
 for(u32 i=0;i<sizeof(patches)/sizeof(*patches);++i){
  struct Patch* p=patches+i;u32 old,restore;
  if(!VirtualProtect((void*)p->site,p->length,0x40,&old))return 0;
  u8 bytes[6]={p->entry==cadence_sequence||p->entry==cadence_tick?0xe9:0xe8,0,0,0,0,0x90};
  put(bytes+1,(u32)p->entry-p->site-5);
  for(u32 j=0;j<p->length;++j)*(u8*)(p->site+j)=bytes[j];
  if(!FlushInstructionCache(GetCurrentProcess(),(void*)p->site,p->length)||
     !VirtualProtect((void*)p->site,p->length,old,&restore))return 0;
 }
 enabled=1;return 1;
}
static void finish(void){
 if(!file)return;enabled=0;
 u32 header[16]={0};const u8 magic[8]={'M','N','M','C','A','D','0','1'};
 for(u32 i=0;i<8;++i)((u8*)header)[i]=magic[i];
 header[2]=1;header[3]=WORDS*4;header[4]=frequency[0];header[5]=frequency[1];
 header[6]=count;header[7]=dropped;header[8]=foreign;header[9]=owner;header[10]=LIMIT;
 u32 wrote=0;
 if(WriteFile(file,header,sizeof(header),&wrote,0)&&wrote==sizeof(header)){
  u32 n=count*sizeof(*rows),done=0;
  while(done<n){wrote=0;if(!WriteFile(file,(u8*)rows+done,n-done,&wrote,0)||!wrote||wrote>n-done)break;done+=wrote;}
 }
 CloseHandle(file);file=0;
}
__declspec(dllexport) void EngineCadenceAnchor(void){}
int WIN DllMain(void* dll,u32 reason,void* reserved){
 (void)dll;(void)reserved;u32 error=GetLastError();int ok=1;
 if(reason==1)ok=install();else if(reason==0)finish();SetLastError(error);return ok;
}
