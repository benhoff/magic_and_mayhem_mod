/* Opt-in Region mouse callback observation; original callback always runs once. */
typedef struct {u32 words[10];} RegionMouseRecord;
typedef u32 (THIS *RegionMouseFn)(void*,u32,RegionMouseRecord);
static RegionMouseFn region_input_original;
static HANDLE region_input_file;
static u32 region_input_rows[8192][64];
static u32 region_input_count,region_input_calls,region_input_dropped,region_input_thread,region_input_foreign,region_input_pending,region_input_phase;
static u32 region_input_pool_base,region_input_pool_id;
static void region_input_fp_save(void* out){__asm__ volatile("fxsave (%0)"::"r"(out):"memory");}
static void region_input_fp_restore(void* in){__asm__ volatile("fxrstor (%0)"::"r"(in):"memory");}
#ifndef MNM_REGION_INPUT_VTABLE
#define MNM_REGION_INPUT_VTABLE 0x5c6a54
#define MNM_REGION_INPUT_CALLBACK 0x4fdad0
#endif
#ifndef MNM_REGION_INPUT_ACTOR
#define MNM_REGION_INPUT_ACTOR 0x656600
#define MNM_REGION_INPUT_INDEX 0x657d40
#define MNM_REGION_INPUT_CLASS 0x6a5134
#define MNM_REGION_INPUT_POOL 0x6def58
#define MNM_REGION_INPUT_CAMERA 0x689930
#define MNM_REGION_INPUT_MODE 0x6903e8
#define MNM_REGION_INPUT_CURSOR 0x5e04c8
#define MNM_REGION_INPUT_RING 0x641fbc
#define MNM_REGION_INPUT_LATCH_COUNT 0x6e984c
#endif
#include "drawing_region_tracked_actor.h"
static void region_input_snapshot(u32* row,void* object,u32 mask,RegionMouseRecord mouse,u32 result,u32 error){
 row[3]=++region_input_phase;row[4]=GetTickCount();row[5]=GetCurrentThreadId();row[6]=mask;row[18]=result;row[19]=error;
 copy(row+8,mouse.words,40);
 if(object==(void*)MNM_REGION_INPUT_CAMERA)row[7]|=1;
 if(readable(object,12)){row[7]|=2;row[20]=get((u8*)object+4);row[21]=get((u8*)object+8);}
 if(readable((void*)MNM_REGION_INPUT_CLASS,4)&&readable((void*)MNM_REGION_INPUT_INDEX,4)&&readable((void*)MNM_REGION_INPUT_ACTOR,4)){
  row[7]|=4;row[22]=get((void*)MNM_REGION_INPUT_CLASS);row[23]=get((void*)MNM_REGION_INPUT_INDEX);
  u32 actor=get((void*)MNM_REGION_INPUT_ACTOR);row[24]=actor?2:0;
  if(actor&&readable((void*)MNM_REGION_INPUT_POOL,8)){
   u32 base=get((void*)MNM_REGION_INPUT_POOL),capacity=get((void*)(MNM_REGION_INPUT_POOL+4));
   if(base&&capacity&&capacity<=256&&actor>=base&&(actor-base)%0xe4b==0){
    u32 slot=(actor-base)/0xe4b;
    if(slot<capacity&&readable((void*)actor,0x178)&&get((void*)actor)==slot){
     if(base!=region_input_pool_base){region_input_pool_base=base;++region_input_pool_id;}
     row[24]=1;row[25]=region_input_pool_id;row[26]=capacity;row[27]=slot+1;
     row[28]=get((void*)(actor+0xa8));row[29]=get((void*)(actor+0x174));row[30]=get((void*)(actor+4));row[31]=get((void*)(actor+0xe4));
     row[42]=get((void*)(actor+0x68));row[43]=get((void*)(actor+0x70));
    }
   }
  }
 }
 if(readable((void*)MNM_REGION_INPUT_CAMERA,0x51)){
  row[7]|=8;row[32]=get((void*)(MNM_REGION_INPUT_CAMERA+0x31));
  for(u32 i=0;i<6;++i)row[33+i]=get((void*)(MNM_REGION_INPUT_CAMERA+0x39+i*4));
 }
 if(readable((void*)MNM_REGION_INPUT_MODE,4)){row[7]|=16;row[39]=get((void*)MNM_REGION_INPUT_MODE);}
 if(readable((void*)MNM_REGION_INPUT_CURSOR,4)){row[7]|=32;row[40]=get((void*)MNM_REGION_INPUT_CURSOR);}
 if(readable((void*)MNM_REGION_INPUT_RING,4)){row[7]|=64;row[41]=get((void*)MNM_REGION_INPUT_RING);}
 if(readable((void*)MNM_REGION_INPUT_LATCH_COUNT,4)){row[7]|=128;row[44]=get((void*)MNM_REGION_INPUT_LATCH_COUNT);}
 region_input_track_snapshot(row);
}
static u32 THIS region_input_callback(void* object,u32 mask,RegionMouseRecord mouse){
 u8 fp[512] __attribute__((aligned(16)));u32 error=GetLastError();region_input_fp_save(fp);
 u32 at=0xffffffff,thread=GetCurrentThreadId();
 if(region_input_file){
  u32 owner=region_input_thread;
  if(!owner){__sync_bool_compare_and_swap(&region_input_thread,0,thread);owner=region_input_thread;}
  if(thread!=owner)__sync_fetch_and_add(&region_input_foreign,1);
  else if(region_input_count>8190)++region_input_dropped;
  else{
   at=region_input_count;region_input_count+=2;++region_input_calls;++region_input_pending;
   u32* begin=region_input_rows[at];u32* end=region_input_rows[at+1];
   begin[0]=at+1;end[0]=at+2;begin[1]=end[1]=region_input_calls;begin[2]=1;end[2]=2;
   region_input_snapshot(begin,object,mask,mouse,0,error);
  }
 }
 region_input_fp_restore(fp);SetLastError(error);
 u32 result=region_input_original(object,mask,mouse);error=GetLastError();region_input_fp_save(fp);
 if(at!=0xffffffff){region_input_snapshot(region_input_rows[at+1],object,mask,mouse,result,error);--region_input_pending;}
 region_input_fp_restore(fp);SetLastError(error);return result;
}
static int region_input_open(void){
 char name[1024];u32 n=GetEnvironmentVariableA("MNM_DRAW_REGION_INPUT_FILE",name,sizeof(name));
 if(!n)return GetLastError()==203;
 if(n>=sizeof(name)||region_input_file)return 0;
 if(!region_input_track_configure())return 0;
 const u8 bytes[]={0x81,0xec,0x30,0x01,0,0,0x53,0x55,0x56,0x8b,0xe9};
 if(!readable((void*)MNM_REGION_INPUT_VTABLE,4)||get((void*)MNM_REGION_INPUT_VTABLE)!=MNM_REGION_INPUT_CALLBACK||!readable((void*)MNM_REGION_INPUT_CALLBACK,sizeof(bytes))||!equal((void*)MNM_REGION_INPUT_CALLBACK,bytes,sizeof(bytes)))return 0;
 HANDLE file=CreateFileA(name,0x40000000,3,0,1,0x80,0);if(file==(HANDLE)-1)return 0;
 u32 old,restore;if(!VirtualProtect((void*)MNM_REGION_INPUT_VTABLE,4,0x40,&old)){CloseHandle(file);return 0;}
 region_input_original=(RegionMouseFn)MNM_REGION_INPUT_CALLBACK;put((void*)MNM_REGION_INPUT_VTABLE,(u32)&region_input_callback);
 VirtualProtect((void*)MNM_REGION_INPUT_VTABLE,4,old,&restore);region_input_file=file;return 1;
}
static void region_input_close(void){
 if(!region_input_file)return;
 u32 h[16]={0};copy(h,"MNMDRE01",8);h[2]=region_input_track_enabled?3:2;h[3]=256;h[4]=0x40209ca7;h[5]=8192;
 h[6]=region_input_count;h[7]=region_input_dropped;h[8]=region_input_thread;h[9]=region_input_foreign;h[10]=region_input_pending;h[11]=region_input_calls;h[12]=region_input_phase;
 u32 wrote=0;if(!WriteFile(region_input_file,h,64,&wrote,0)||wrote!=64)ExitProcess(99);
 u32 size=region_input_count*256,at=0;while(at<size){wrote=0;if(!WriteFile(region_input_file,(u8*)region_input_rows+at,size-at,&wrote,0)||!wrote||wrote>size-at)ExitProcess(99);at+=wrote;}
 if(!CloseHandle(region_input_file))ExitProcess(99);region_input_file=0;
}
