/* Actual provider lifetime, not a simulated AddRef counter. The caller drains
 * every public interface credit while opaque pins and Lock/DC writers survive. */
#include "../runtime/shadow/win32_min.h"
API void* WIN LoadLibraryA(const char*);
API void* WIN GetProcAddress(void*,const char*);
API HANDLE WIN CreateThread(void*,u32,void*,void*,u32,u32*);
API HANDLE WIN CreateEventA(void*,i32,i32,const char*);
API u32 WIN SetEvent(HANDLE);
API u32 WIN WaitForSingleObject(HANDLE,u32);
API u32 WIN GetCurrentThreadId(void);
API u32 WIN SetPixel(void*,i32,i32,u32);
typedef i32(WIN* Create)(void*,void**,void*);
typedef i32(WIN* Query)(void*,const u8*,void**);
typedef u32(WIN* Ref)(void*);
typedef i32(WIN* Lock)(void*,const i32*,u32*,u32,HANDLE);
typedef i32(WIN* Unlock)(void*,void*);
typedef i32(WIN* Fast)(void*,u32,u32,void*,const i32*,u32);
typedef i32(WIN* Pin)(void*,u32*);
typedef i32(WIN* Unpin)(u32);
typedef i32(WIN* Live)(void*);
typedef u32(WIN* Count)(void);
static Create create;static Pin pin;static Unpin unpin;static Live live;static Count surfaces,devices,dcs;
static void* device;static u32 checks,planes,cases;static HANDLE start_event;
static const u8 ids[4][16]={
 {0x81,0xdb,0x14,0x6c,0x33,0xa7,0xce,0x11,0xa5,0x21,0,0x20,0xaf,0x0b,0xe5,0x60},
 {0x85,0x58,0x80,0x57,0xec,0x6e,0xcf,0x11,0x94,0x41,0xa8,0x23,0x03,0xc1,0x0e,0x27},
 {0x30,0x86,0x2b,0x0b,0x35,0xad,0xd0,0x11,0x8e,0xa6,0,0x60,0x97,0x97,0xea,0x5b},
 {0x00,0x4e,0x04,0xda,0xb2,0x69,0xd0,0x11,0xa1,0xd5,0,0xaa,0,0xb8,0xdf,0xbb}};
static void** vt(void* p){return *(void***)p;}
static void need(int value,u32 code){if(!value)ExitProcess(code);__atomic_add_fetch(&checks,1,__ATOMIC_RELAXED);}
static void* surface(void** aliases){
 u32 d[27]={108,0x1007,9,13};d[18]=32;d[19]=0x40;d[21]=16;d[22]=0xf800;d[23]=0x7e0;d[24]=31;d[26]=0x840;
 void* p=0;need(!((i32(WIN*)(void*,u32*,void**,void*))vt(device)[6])(device,d,&p,0),10);
 if(aliases)for(u32 k=0;k<4;++k){aliases[k]=0;need(!((Query)vt(p)[0])(p,ids[k],aliases+k),11);}
 return p;
}
static void release_all(void* base,void** aliases){
 for(u32 i=0;i<4;++i){void* p=aliases[i];u32 n=0;while(((Ref)vt(p)[2])(p)){if(++n==1000)ExitProcess(12);}}
 // aliases[0] includes the constructor's Surface1 credit, as well as its query.
 need(((Ref)vt(base)[2])(base)==0,13);
}
static u16 pattern(u32 x,u32 y,u32 seed){return (u16)(seed+x*719+y*257);}
static void plane(void* p,u32 kind,u32 seed,u32 write){
 u32 d[31]={0};d[0]=kind==3?124:108;
 need(!((Lock)vt(p)[25])(p,0,d,1,0),14);
 need(d[2]==9 && d[3]==13 && d[21]==16 && d[22]==0xf800 && d[23]==0x7e0 && d[24]==31,15);
 for(u32 y=0;y<9;++y)for(u32 x=0;x<13;++x){u16* at=(u16*)(d[9]+y*d[4])+x;if(write)*at=pattern(x,y,seed);else need(*at==pattern(x,y,seed),16);}
 need(!((Unlock)vt(p)[32])(p,kind==3?0:(void*)d[9]),17);
 if(!write)__atomic_add_fetch(&planes,1,__ATOMIC_RELAXED);
}
static void pinned_pairs(void){
 for(u32 a=1;a<=4;++a)for(u32 b=1;b<=4;++b){
  void *sa[4],*da[4];void* s=surface(sa);void* d=surface(da);u32 sp=0,dp=0;
  plane(sa[a-1],a,0x2300+a*31+b,1);need(!pin(sa[a-1],&sp)&&!pin(da[b-1],&dp)&&sp&&dp&&sp!=dp,18);
  release_all(s,sa);release_all(d,da);need(live(s)==1 && live(d)==1,19);
  // New resources never replace the queued old generation's pixels.
  void* fresh=surface(0);plane(fresh,1,0x7300,1);
  i32 r[4]={0,0,13,9};need(!((Fast)vt(da[b-1])[7])(da[b-1],0,0,sa[a-1],r,16),20);plane(da[b-1],b,0x2300+a*31+b,0);
  need(!unpin(sp)&&live(s)==0,21);need(unpin(sp)<0 && live(d)==1,22);
  u32 out[31]={108};need(((Lock)vt(s)[25])(s,0,out,1,0)<0,23);
  need(((Fast)vt(da[b-1])[7])(da[b-1],0,0,sa[a-1],r,16)<0,24);
  need(!unpin(dp)&&live(d)==0,25);need(((Ref)vt(fresh)[2])(fresh)==0,26);need(surfaces()==0,27);++cases;
 }
}
static void writer_lifetime(void){
 for(u32 kind=1;kind<=4;++kind)for(u32 dc_mode=0;dc_mode<2;++dc_mode){
  void* aliases[4];void* p=surface(aliases);u32 token=0;need(!pin(p,&token),30);
  if(!dc_mode){u32 d[31]={0};d[0]=kind==3?124:108;need(!((Lock)vt(aliases[kind-1])[25])(aliases[kind-1],0,d,1,0),31);
   release_all(p,aliases);need(!unpin(token)&&live(p)==1,32);
   for(u32 y=0;y<9;++y)for(u32 x=0;x<13;++x)((u16*)(d[9]+y*d[4]))[x]=pattern(x,y,0x3300);
   need(!((Unlock)vt(aliases[kind-1])[32])(aliases[kind-1],kind==3?0:(void*)d[9])&&live(p)==0,33);
  }else {void* dc=0;need(!((i32(WIN*)(void*,void**))vt(aliases[kind-1])[17])(aliases[kind-1],&dc)&&dc,34);
   release_all(p,aliases);need(!unpin(token)&&live(p)==1 && dcs()==1,35);need(SetPixel(dc,1,1,0xff)!=0xffffffffu,36);
   need(!((i32(WIN*)(void*,void*))vt(aliases[kind-1])[26])(aliases[kind-1],dc)&&live(p)==0 && !dcs(),37);
  }
  need(!surfaces(),38);++cases;
 }
}
struct Worker {void* source;u32 seed;};
static u32 WIN worker(void* raw){
 struct Worker* w=raw;void* d=surface(0);i32 r[4]={0,0,13,9};
 need(WaitForSingleObject(start_event,10000)==0,40);
 for(u32 i=0;i<400;++i){need(!((Fast)vt(d)[7])(d,0,0,w->source,r,16),41);plane(d,1,w->seed,0);}
 need(((Ref)vt(d)[2])(d)==0,42);return 0;
}
static void concurrent_lifetime(void){
 void* aliases[4];void* p=surface(aliases);plane(p,1,0x5300,1);u32 token=0;need(!pin(p,&token),43);
 start_event=CreateEventA(0,1,0,0);need(start_event!=0,44);HANDLE threads[4];struct Worker args[4];
 for(u32 i=0;i<4;++i){args[i]=(struct Worker){aliases[i],0x5300};threads[i]=CreateThread(0,0,(void*)&worker,args+i,0,0);need(threads[i]!=0,45);}
 need(SetEvent(start_event),46);release_all(p,aliases);
 for(u32 i=0;i<100;++i){void* fresh=surface(0);plane(fresh,1,0x6300+i,1);need(((Ref)vt(fresh)[2])(fresh)==0,47);}
 for(u32 i=0;i<4;++i){need(WaitForSingleObject(threads[i],30000)==0,48);CloseHandle(threads[i]);}CloseHandle(start_event);
 need(!unpin(token)&&live(p)==0 && !surfaces(),49);++cases;
}
static void capacity_and_identity(void){
 void* p=surface(0);u32 pins[512],invalid=0x76543210;
 for(u32 i=0;i<512;++i)need(!pin(p,pins+i)&&pins[i]&&(i==0||pins[i]>pins[i-1]),50);
 need(pin(p,&invalid)<0 && invalid==0x76543210,51);need(((Ref)vt(p)[2])(p)==0 && live(p)==1,52);
 for(u32 i=0;i<512;++i)need(!unpin(pins[i]),53);
 need(live(p)==0 && !surfaces(),54);++cases;
}
void start(void){
 void* module=LoadLibraryA("MnmDraw.dll");need(module!=0 && !GetModuleHandleA("ddraw.dll"),2);
 create=(Create)GetProcAddress(module,"NativeDrawCreate");pin=(Pin)GetProcAddress(module,"NativeDrawPinV1");unpin=(Unpin)GetProcAddress(module,"NativeDrawUnpinV1");live=(Live)GetProcAddress(module,"NativeDrawBackingLiveV1");surfaces=(Count)GetProcAddress(module,"NativeDrawSurfaces");devices=(Count)GetProcAddress(module,"NativeDrawObjects");dcs=(Count)GetProcAddress(module,"NativeDrawDCs");
 need(create&&pin&&unpin&&live&&surfaces&&devices&&dcs,3);need(!create(0,&device,0),4);
 pinned_pairs();writer_lifetime();concurrent_lifetime();capacity_and_identity();
 need(((Ref)vt(device)[2])(device)==0 && !surfaces() && !devices() && !dcs(),5);
 u32 result[8]={1,checks,planes,cases,1600,16,8,512},done=0;HANDLE f=CreateFileA("provider-lifetime.bin",0x40000000,1,0,1,0x80,0);
 need(f!=(HANDLE)-1 && WriteFile(f,result,sizeof(result),&done,0) && done==sizeof(result),6);CloseHandle(f);ExitProcess(0);
}
