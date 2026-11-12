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
API int WIN QueryPerformanceCounter(void*);
API int WIN QueryPerformanceFrequency(void*);
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
__attribute__((used,aligned(16))) static u8 guard_saved[512],guard_before[512],guard_after[512];
__attribute__((used,aligned(16))) static u32 guard_seed[4]={0x12345678,0xabcdef01,0x87654321,0x10fedcba};
__attribute__((used)) static Count guard_begin;
__attribute__((used)) static void(WIN*guard_end)(void);
__attribute__((used)) static u32 guard_mxcsr=0x5f80;
__attribute__((naked)) static void guard_state_calls(void){__asm__ volatile(
 "pushal;fxsave _guard_saved;fninit;fld1;fldz;ldmxcsr _guard_mxcsr;"
 "movdqu _guard_seed,%xmm0;movdqu _guard_seed,%xmm1;movdqu _guard_seed,%xmm2;movdqu _guard_seed,%xmm3;"
 "movdqu _guard_seed,%xmm4;movdqu _guard_seed,%xmm5;movdqu _guard_seed,%xmm6;movdqu _guard_seed,%xmm7;"
 "fxsave _guard_before;calll *_guard_begin;calll *_guard_begin;calll *_guard_end;calll *_guard_end;"
 "fxsave _guard_after;fxrstor _guard_saved;popal;retl;");}
static void guard_state(void* module){
 guard_begin=(Count)GetProcAddress(module,"NativeDrawGuardBeginV1");guard_end=(void(WIN*)(void))GetProcAddress(module,"NativeDrawGuardEndV1");
 need(guard_begin && guard_end,90);SetLastError(0x13579bdf);guard_state_calls();need(GetLastError()==0x13579bdf,91);
 for(u32 i=0;i<288;++i){
  if(i==5 || i==14 || i==15 || i==22 || i==23)continue;
  if(i>=32 && i<160){u32 reg=(i-32)/16,byte=(i-32)%16,top=(guard_before[3]>>3)&7;
   if(byte>=10 || !(guard_before[4]&(1u<<((reg+top)&7))))continue;}
  need(guard_after[i]==guard_before[i],92);
 }
 ++cases;
}
static void nested_retirement(void* module){
 Count begin=(Count)GetProcAddress(module,"NativeDrawGuardBeginV1");
 void(WIN*end)(void)=(void(WIN*)(void))GetProcAddress(module,"NativeDrawGuardEndV1");
 need(begin&&end,60);
 void* p=surface(0);need(begin()&&begin(),61);need(!((Ref)vt(p)[2])(p)&&live(p)==1,62);
 end();need(live(p)==1,63);end();need(live(p)==0&&!surfaces(),64);++cases;
 /* A pending candidate can gain a public credit before the outer exit. */
 p=surface(0);need(begin(),65);need(!((Ref)vt(p)[2])(p),66);need(((Ref)vt(p)[1])(p)==1,67);
 end();need(live(p)==1,68);need(!((Ref)vt(p)[2])(p)&&live(p)==0&&!surfaces(),69);++cases;
}
static void attached_retirement(void){
 need(!((i32(WIN*)(void*,void*,u32))vt(device)[20])(device,0,8),70);
 for(u32 held=0;held<3;++held){
  u32 d[31]={108,0x21};d[5]=1;d[26]=0x218;void *front=0,*back=0,*dc=0;
  need(!((i32(WIN*)(void*,u32*,void**,void*))vt(device)[6])(device,d,&front,0),71);
  u32 caps[4]={4};need(!((i32(WIN*)(void*,u32*,void**))vt(front)[12])(front,caps,&back),72);
  if(!held){
   /* Drop the external back credit; destroying front must drain its child. */
   need(((Ref)vt(back)[2])(back)==1,73);need(!((Ref)vt(front)[2])(front),74);
   need(live(front)==0&&live(back)==0&&!surfaces(),75);
  }else{
   if(held==1){d[0]=108;need(!((Lock)vt(front)[25])(front,0,d,1,0),76);}
   else need(!((i32(WIN*)(void*,void**))vt(front)[17])(front,&dc)&&dc,77);
   need(!((Ref)vt(front)[2])(front)&&live(front)==1,78);
   need(!((i32(WIN*)(void*,void*,u32))vt(front)[11])(front,0,1),79);
   if(held==1){
    /* The Lock moves to back, making the zero-credit front eligible. */
    need(live(front)==0&&live(back)==1&&surfaces()==1,80);
    need(!((Unlock)vt(back)[32])(back,(void*)d[9]),81);
   }else{
    /* The logical DC still owns front. Restore its physical lease first. */
    need(live(front)==1&&surfaces()==2,82);
    need(((i32(WIN*)(void*,void*))vt(front)[26])(front,dc)<0,83);
    need(((i32(WIN*)(void*,void*,u32))vt(front)[11])(front,0,1)<0,84);
    need(!((i32(WIN*)(void*,void*))vt(front)[26])(front,dc),85);
    need(live(front)==0&&live(back)==1&&!dcs(),86);
   }
   need(!((Ref)vt(back)[2])(back)&&live(back)==0&&!surfaces(),87);
  }
  ++cases;
 }
}
static void scan_cost(void){
 /* Keep the queried object at the list head: this isolates guard exit from
  * the separate alias lookup. No timing threshold is a correctness claim. */
 u32 result[26]={1,20000,64,512};need(QueryPerformanceFrequency(result+4),90);
 for(u32 size=0;size<2;++size){
  void* objects[512];u32 count=size?512:64;
  for(u32 i=0;i<count;++i)objects[i]=surface(0);
  void* target=objects[count-1];u32 caps[4];
  for(u32 repeat=0;repeat<5;++repeat){
   unsigned long long start=0,stop=0;need(QueryPerformanceCounter(&start),91);
   for(u32 i=0;i<20000;++i)if(((i32(WIN*)(void*,u32*))vt(target)[14])(target,caps)||caps[0]!=0x840)ExitProcess(92);
   need(QueryPerformanceCounter(&stop)&&stop>=start,93);
   unsigned long long ticks=stop-start;u32 at=6+size*10+repeat*2;result[at]=(u32)ticks;result[at+1]=(u32)(ticks>>32);
  }
  for(u32 i=0;i<count;++i)need(!((Ref)vt(objects[i])[2])(objects[i]),94);
  need(!surfaces(),95);
 }
 u32 done=0;HANDLE f=CreateFileA("provider-scan-cost.bin",0x40000000,1,0,1,0x80,0);
 need(f!=(HANDLE)-1&&WriteFile(f,result,sizeof(result),&done,0)&&done==sizeof(result),96);CloseHandle(f);
}
void start(void){
 void* module=LoadLibraryA("MnmDraw.dll");need(module!=0 && !GetModuleHandleA("ddraw.dll"),2);
 create=(Create)GetProcAddress(module,"NativeDrawCreate");pin=(Pin)GetProcAddress(module,"NativeDrawPinV1");unpin=(Unpin)GetProcAddress(module,"NativeDrawUnpinV1");live=(Live)GetProcAddress(module,"NativeDrawBackingLiveV1");surfaces=(Count)GetProcAddress(module,"NativeDrawSurfaces");devices=(Count)GetProcAddress(module,"NativeDrawObjects");dcs=(Count)GetProcAddress(module,"NativeDrawDCs");
 need(create&&pin&&unpin&&live&&surfaces&&devices&&dcs,3);need(!create(0,&device,0),4);
 pinned_pairs();writer_lifetime();concurrent_lifetime();capacity_and_identity();guard_state(module);nested_retirement(module);attached_retirement();scan_cost();
 need(((Ref)vt(device)[2])(device)==0 && !surfaces() && !devices() && !dcs(),5);
 u32 result[8]={1,checks,planes,cases,1600,16,8,512},done=0;HANDLE f=CreateFileA("provider-lifetime.bin",0x40000000,1,0,1,0x80,0);
 need(f!=(HANDLE)-1 && WriteFile(f,result,sizeof(result),&done,0) && done==sizeof(result),6);CloseHandle(f);ExitProcess(0);
}
