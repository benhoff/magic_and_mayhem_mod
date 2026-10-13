#include "../runtime/shadow/win32_min.h"
API void* WIN GetProcAddress(void*,const char*);
API u32 WIN GetCurrentThreadId(void);
API u32 WIN GetTickCount(void);
API void WIN InitializeCriticalSection(void*);
API void WIN EnterCriticalSection(void*);
API void WIN LeaveCriticalSection(void*);
API i32 WIN VirtualFree(void*,u32,u32);
API HANDLE WIN CreateThread(void*,u32,u32(WIN*)(void*),void*,u32,u32*);
API u32 WIN WaitForSingleObject(HANDLE,u32);
struct Table {void** vt;void* original[36];u32 kind;};
struct Context {u32 r[32],args[6],observed;};
typedef u32(WIN *ProducerPoint)(void*,void*,u32*,u32);
static ProducerPoint producer_point;
static u32 physical_enabled=1,producer_points_enabled=1,journal_lock[6],pending;
static char directory[512];
static u32 mode,desc_calls,lock_calls,unlock_calls,getter_calls,application_calls;
static int forced;
static u16 pixels[40][48];
void* memset(void* p,int value,u32 n){u8* b=p;while(n--)*b++=(u8)value;return p;}
static void copy(void* d,const void* s,u32 n){u8* a=d;const u8* b=s;while(n--)*a++=*b++;}
static void path(char* out,const char* suffix){u32 i=0;while(directory[i]){out[i]=directory[i];++i;}out[i++]='\\';while(*suffix)out[i++]=*suffix++;out[i]=0;}
static int write(HANDLE f,const void* p,u32 n){u32 count;return WriteFile(f,p,n,&count,0)&&count==n;}
static int readable(const void* p,u32 n){u32 at=(u32)p,end=at+n;if(!at||end<at)return 0;while(at<end){u32 m[7];if(VirtualQuery((void*)at,m,28)!=28||m[4]!=0x1000||(m[5]&0x101))return 0;u32 next=m[0]+m[3];if(next<=at)return 0;at=next;}return 1;}
static void snapshot_fp(void* p){__asm__ volatile("fxsave (%0)"::"r"(p):"memory");}
static void restore_fp(void* p){__asm__ volatile("fxrstor (%0)"::"r"(p):"memory");}
static void require(int yes,u32 code){if(!yes)ExitProcess(code);}
static void* test_alloc(void* p,u32 n,u32 type,u32 protect){return mode==7?0:VirtualAlloc(p,n,type,protect);}
#define VirtualAlloc test_alloc
#include "../runtime/drawing/aux_save_readback.h"
#undef VirtualAlloc
static u32 WIN point(void* dst,void* src,u32* p,u32 n){
 (void)dst;(void)src;require(n==128,20);++getter_calls;memset(p,0,128);
 for(u32 i=0;i<2;++i){u32* q=p+i*16;q[0]=64;q[1]=1;q[2]=19;q[3]=100+getter_calls;q[4]=mode==10?29:33;q[6]=i?1:5;q[7]=i?800:40;q[8]=i?600:40;q[10]=1;q[12]=1;q[14]=GetCurrentThreadId();}
 if(mode==9&&getter_calls==2)p[10]=2;SetLastError(0xdddd);return 1;
}
static u32 WIN reentrant(void* p){(void)p;EnterCriticalSection(journal_lock);LeaveCriticalSection(journal_lock);return 0;}
static void corrupt_state(void){u16 cw=0x037f;__asm__ volatile("fldcw %0; fnclex"::"m"(cw));SetLastError(0xdead);}
static i32 WIN desc(void* p,u32* d){
 struct Table* t=p;++desc_calls;require(d[0]==(t->kind>=14?124u:108u),21);
 if(forced){EnterCriticalSection(journal_lock);aux_save_close(1);LeaveCriticalSection(journal_lock);}
 HANDLE th=CreateThread(0,0,reentrant,0,0,0);require(th&&WaitForSingleObject(th,5000)==0,22);CloseHandle(th);
 d[2]=40;d[3]=40;d[4]=96;d[9]=(u32)pixels;d[21]=mode==2?32:16;d[22]=0xf800;d[23]=0x7e0;d[24]=31;d[26]=0;
 corrupt_state();return mode==1?(i32)0x80004005:0;
}
static i32 WIN lock(void* p,void* rect,u32* d,u32 flags,HANDLE event){
 struct Table* t=p;++lock_calls;require(!rect&&!event&&flags==0x4810&&d[0]==(t->kind>=14?124u:108u),23);
 d[2]=40;d[3]=mode==4?39:40;d[4]=mode==5?79:96;d[9]=mode==6?1:(u32)pixels;d[21]=16;d[22]=0xf800;d[23]=0x7e0;d[24]=31;
 corrupt_state();return mode==3?(i32)0x887601ae:0;
}
static i32 WIN unlock(void* p,void* arg){struct Table* t=p;++unlock_calls;require(t->kind>=14?!arg:arg==(void*)(mode==6?1:(u32)pixels),24);corrupt_state();return mode==8?(i32)0x80004005:0;}
static void run(struct Context* c,struct Table* t){
 ++application_calls;desc_calls=lock_calls=unlock_calls=getter_calls=0;
 u16 cw=0x0b7f;__asm__ volatile("fldcw %0"::"m"(cw));SetLastError(0x1234);
 pending=1;aux_save_capture(c,t,t);pending=0;
 u16 after;__asm__ volatile("fnstcw %0":"=m"(after));require(GetLastError()==0x1234&&after==cw,25);
}
void start(void){
 require(GetEnvironmentVariableA("MNM_AUX_TEST_DIR",directory,sizeof(directory))>0,26);
 char f[2];forced=GetEnvironmentVariableA("MNM_AUX_TEST_FORCED",f,2)!=0;
 InitializeCriticalSection(journal_lock);producer_point=point;
 for(u32 y=0;y<40;++y)for(u32 x=0;x<48;++x)pixels[y][x]=(u16)(17+(y*40+x)*37);
 struct Table t={0};t.kind=11;t.original[22]=(void*)&desc;t.original[25]=(void*)&lock;t.original[32]=(void*)&unlock;
 struct Context c={0};c.observed=1;c.r[1]=1;c.r[2]=7;c.r[3]=5;c.r[4]=1;c.r[6]=GetCurrentThreadId();c.r[9]=0x1234;c.r[21]=16;c.r[24]=440;c.r[25]=310;c.r[26]=480;c.r[27]=350;c.r[29]=1;c.args[2]=1;
 /* Default-off: even a valid sample never calls the getter or driver. */
 run(&c,&t);require(!desc_calls&&!lock_calls&&!unlock_calls&&!getter_calls,27);
 require(aux_save_open(),28);
 if(forced){run(&c,&t);require(!aux_save_recording()&&!lock_calls&&!unlock_calls,29);ExitProcess(0);}
 for(mode=0;mode<=10;++mode){++c.r[29];run(&c,&t);
  require(desc_calls==(mode==10?0u:1u),30+mode);
  require(lock_calls==((mode==1||mode==2||mode==10)?0u:1u),45+mode);
  require(unlock_calls==((mode==1||mode==2||mode==3||mode==10)?0u:1u),60+mode);
 }
 mode=0;t.kind=14;++c.r[29];run(&c,&t);t.kind=17;++c.r[29];run(&c,&t);
 u32 before=aux_save_attempts;c.r[8]=0x80004005;run(&c,&t);require(!desc_calls&&!getter_calls&&aux_save_attempts==before,80);c.r[8]=0;
 c.r[21]=17;run(&c,&t);require(!desc_calls&&!getter_calls,81);c.r[21]=16;
 t.kind=99;run(&c,&t);require(!desc_calls&&!getter_calls,82);t.kind=11;
 c.r[26]=479;run(&c,&t);require(!desc_calls&&!getter_calls,83);c.r[26]=480;
 /* Bound without fabricating counters: fill actual remaining probe records. */
 while(aux_save_attempts<256){++c.r[29];run(&c,&t);}u32 images=aux_save_images;
 run(&c,&t);require(!desc_calls&&!lock_calls&&!unlock_calls&&aux_save_images==images&&aux_save_budget_skips==1,84);
 EnterCriticalSection(journal_lock);aux_save_close(2);LeaveCriticalSection(journal_lock);
 run(&c,&t);require(!desc_calls&&!getter_calls,85);require(application_calls>256,86);ExitProcess(0);
}
