/* Unchanged original cursor helpers, queue and copy wrapper with real providers. */
#include "graphics-lifetime-common.h"
#include "original-pe32-image.h"
API HANDLE WIN CreateMutexA(void*,int,const char*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
API int WIN ReleaseMutex(HANDLE);
static u32 waits,releases,wait_result,release_result;
static u32 WIN observed_wait(HANDLE h,u32 n){++waits;gl_require(n==0xffffffffu,201);return wait_result=WaitForSingleObject(h,n);}
static int WIN observed_release(HANDLE h){++releases;return release_result=(u32)ReleaseMutex(h);}
#define CURSOR_ROWS 78u
#define CURSOR_WORDS (16u+2u*117u)
static u32 results[CURSOR_ROWS][CURSOR_WORDS],full_queue[128*11];
static u32 get(u32 at){return *(u32*)at;}
static void put(u32 at,u32 v){*(u32*)at=v;}
void start(void){
 original_map();u8 restore[302],invalidate[11],enqueue[138],drain[227],opaque[1024];
 original_copy(restore,(void*)0x4a4380,sizeof(restore));original_copy(invalidate,(void*)0x4a4370,sizeof(invalidate));
 original_copy(enqueue,(void*)0x58d5b0,sizeof(enqueue));original_copy(drain,(void*)0x58d660,sizeof(drain));original_copy(opaque,(void*)0x58c4a0,sizeof(opaque));
 put(0x5c5140,(u32)observed_wait);put(0x5c513c,(u32)observed_release);
 u32 native;void* module=gl_module(&native);GlCreate create=(GlCreate)GetProcAddress(module,"DirectDrawCreate");void* draw=0;
 gl_require(create&&!create(0,&draw,0)&&draw,202);gl_require(!((i32(WIN*)(void*,void*,u32))gl_vt(draw)[20])(draw,0,8),203);
 const u8 draw2id[16]={0xe0,0xf3,0xa6,0xb3,0x43,0x2b,0xcf,0x11,0xa2,0xde,0,0xaa,0,0xb9,0x33,0x56};
 const u8 draw4id[16]={0x9a,0x50,0x59,0x9c,0xbd,0x39,0xd1,0x11,0x8c,0x4a,0,0xc0,0x4f,0xd9,0x30,0xc5};
 put(0x6f98c4,(u32)draw);put(0x6f98c8,(u32)gl_query(draw,draw2id));put(0x6f98cc,(u32)gl_query(draw,draw4id));put(0x6f98d0,0);put(0x6f68d8,0);put(0x6e1f1c,0);put(0x6f8154,0);put(0x6f98c0,0);
 for(u32 i=0;i<128;++i)put(0x6f6950+i*4,0);
 for(u32 i=0;i<2;++i){u32 descriptor[31];gl_desc(descriptor,0);void* wrapper=(void*)(i?0x6a49e0:0x641fc8);gl_zero(wrapper,64);((void(__attribute__((thiscall))*)(void*,u32*,u32,u32))0x58ad90)(wrapper,descriptor,0,0);gl_require(get((u32)wrapper+4)&&get((u32)wrapper+8),204);}
 HANDLE mutex=CreateMutexA(0,0,0);gl_require(mutex!=0,205);u32 id=0;
 for(u32 gate=0;gate<13;++gate)for(u32 deferred=0;deferred<2;++deferred)for(u32 position=0;position<3;++position){
  u32* r=results[id];r[0]=id++;r[1]=gate;r[2]=deferred;r[3]=position;r[4]=0x1100+r[0]*3;r[5]=0x5500+r[0]*5;
  gl_pixels((void*)get(0x641fd0),1,0,r[4],1);gl_pixels((void*)get(0x6a49e8),1,0,r[5],1);
  gl_zero((void*)0x6f6b50,128*44);put(0x6f98d4,0);put(0x6f98d8,0);
  put(0x642c64,gate!=1);put(0x5e04c8,gate==2?22:3);put(0x642c54,gate!=10);put(0x642c58,(u32)mutex);put(0x6e1ee4,deferred);
  put(0x641a78,1);put(0x641a7c,1);put(0x641a80,6);put(0x641a84,4);
  if(gate>=3&&gate<=6)put(0x641a78+(gate-3)*4,0xffffffffu);
  if(gate==7)put(0x641a80,1);if(gate==8)put(0x641a84,1);
  put(0x641aa0,position==0?0:position==1?2:0xffffffffu);put(0x641aa4,position==0?0:position==1?3:0xffffffffu);
  if(gate==9)((void(*)(void))0x4a4370)();
  if(gate==12&&deferred){
   for(u32 i=0;i<128;++i){u32* q=(u32*)(0x6f6b50+i*44);q[0]=1;q[1]=1;q[2]=0x641fc8;q[3]=0x6a49e0;q[4]=2;q[5]=0;q[6]=0;q[7]=1;q[8]=1;q[9]=12;q[10]=8;}put(0x6f98d4,128);
  }
  if(gate==12&&deferred)original_copy(full_queue,(void*)0x6f6b50,sizeof(full_queue));
  waits=releases=0;wait_result=release_result=0;u32 interface=get(0x6a49e4);if(gate==11)put(0x6a49e4,0);
  ((void(*)(void))0x4a4380)();put(0x6a49e4,interface);
  if(gate==12&&deferred){for(u32 i=0;i<128*11;++i)gl_require(full_queue[i]==((u32*)0x6f6b50)[i],210);r[14]=1;}
  r[6]=waits;r[7]=releases;r[8]=wait_result;r[9]=release_result;r[10]=get(0x642c54);r[11]=get(0x6f98d4);
  gl_pixels((void*)get(0x6a49e8),1,r+16,0,0);((void(*)(void))0x58d660)();r[12]=get(0x6f98d4);r[13]=get(0x6f98d8);
  gl_pixels((void*)get(0x6a49e8),1,r+16+117,0,0);gl_require(!r[12]&&!r[13],206);
 }
 gl_require(id==CURSOR_ROWS,207);((void(*)(void))0x58acf0)();gl_require(CloseHandle(mutex),208);if(native)gl_counts(module,0,0,0,0,0);
 const u8* originals[5]={restore,invalidate,enqueue,drain,opaque};const u32 sites[5]={0x4a4380,0x4a4370,0x58d5b0,0x58d660,0x58c4a0},sizes[5]={302,11,138,227,1024};
 for(u32 n=0;n<5;++n)for(u32 i=0;i<sizes[n];++i)gl_require(originals[n][i]==((u8*)sites[n])[i],209);
 gl_save("cursor-provider.bin",results,sizeof(results));ExitProcess(0);
}
