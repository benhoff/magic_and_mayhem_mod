#include "../runtime/shadow/win32_min.h"
API u32 WIN GetCurrentThreadId(void);
API u32 WIN GetTickCount(void);
API void WIN InitializeCriticalSection(void*);
API void WIN EnterCriticalSection(void*);
API void WIN LeaveCriticalSection(void*);
API void* WIN GetProcAddress(void*,const char*);
static u32 journal_lock[6],wine_submission_enabled=1,wine_lifetime_records;
static u32 wine_blit_packet_enabled=1,mode,forwarded,held,returned,context_calls,destroyed,waited;
static struct {const void* pointer;u32 command,submission,api_entry,words[25];} wine_blit_packets[2048];
void* memset(void* out,int value,unsigned int count){volatile u8* p=out;while(count--)*p++=(u8)value;return out;}
static void require(int p,u32 code){if(!p)ExitProcess(code);}
static void copy(void* dst,const void* src,u32 n){u8* a=dst;const u8* b=src;while(n--)*a++=*b++;}
static int equal(const void* a,const void* b,u32 n){const u8* x=a;const u8* y=b;while(n--)if(*x++!=*y++)return 0;return 1;}
static int readable(const void* p,u32 n){u32 m[7];return p&&VirtualQuery(p,m,28)==28&&m[4]==0x1000&&!(m[5]&0x101)&&(u32)p+n<=m[0]+m[3];}
static int write(HANDLE f,const void* p,u32 n){u32 done;return WriteFile(f,p,n,&done,0)&&done==n;}
static void path(char* out,const char* name){while((*out++=*name++));}
static void snapshot_fp(void* p){__asm__ volatile("fxsave (%0)"::"r"(p):"memory");}
static void restore_fp(void* p){__asm__ volatile("fxrstor (%0)"::"r"(p):"memory");}
static u32 wine_submission_identity(void* p,u32 k){(void)p;(void)k;return 17;}
static u32 wine_lifetime_find(void* p,u32 k){(void)p;(void)k;return 0;}
static void wine_blit_packet_worker(const void* p,u32 w,u32 phase){(void)p;(void)w;if(phase==2)held=1;else{require(held,201);held=0;++returned;}}
static void wine_worker_context_publish(const void* p,u32 w,u32 phase){(void)p;(void)w;(void)phase;}
static u32 producer_points_enabled=1;
static u32 producer_entry_seen,producer_return_seen;
static u32 WIN producer_point(void* object,u32* out,u32 size){
 require(!object&&size==64&&held&&!returned,212);u32 phase=forwarded?2:1;
 if(phase==1){require(!forwarded&&!producer_entry_seen,213);++producer_entry_seen;}
 else{require(phase==2&&forwarded==1&&!producer_return_seen,214);++producer_return_seen;}
 for(u32 i=0;i<16;++i)out[i]=0;out[0]=64;out[1]=1;out[2]=3;out[3]=forwarded;out[14]=GetCurrentThreadId();
 u16 control=0x037f;__asm__ volatile("fldcw %0"::"m"(control));SetLastError(0x4321);
 return 1;
}
#include "../runtime/drawing/wine_blit_worker.h"
#include "../runtime/drawing/wine_gpu_completion.h"
static void clobber(void){u16 control=0x037f;__asm__ volatile("fldcw %0"::"m"(control));SetLastError(0x1234);}
static void* WIN current(void){clobber();if(mode==2)return 0;++context_calls;return (void*)(mode==8&&context_calls>1?0x5678:0x1234);}
static void* WIN fence(u32 condition,u32 flags){require(held&&forwarded==1&&producer_return_seen==1&&condition==0x9117&&!flags,202);clobber();return mode==4?0:(void*)0x9876;}
static u32 WIN wait(void* sync,u32 flags,unsigned long long timeout){require(held&&sync==(void*)0x9876&&flags==1&&timeout==1000000000ULL,203);++waited;clobber();if(mode==12){EnterCriticalSection(journal_lock);wine_gpu_close(2);LeaveCriticalSection(journal_lock);}return mode==5?0x911b:mode==6?0x911d:mode==9?0x1234:mode==1?0x911a:0x911c;}
static void WIN destroy(void* sync){require(held&&sync==(void*)0x9876&&waited==1,204);++destroyed;clobber();}
static void* WIN proc(const char* n){clobber();if(mode==3)return(void*)1;if(n[2]=='F')return(void*)&fence;if(n[2]=='C')return(void*)&wait;return(void*)&destroy;}
static void __attribute__((cdecl)) original(void* ctx,const void* packet){require(ctx==(void*)0x4567&&packet==wine_blit_packets[0].pointer&&held&&GetLastError()==0xbeef,205);++forwarded;u16 control=0x077f;__asm__ volatile("fldcw %0"::"m"(control));SetLastError(0xcafe);}
static void __attribute__((cdecl)) teardown_original(void* ctx,const void* packet){
 require(ctx==(void*)0x4567&&packet==wine_blit_packets[0].pointer&&!held&&GetLastError()==0xbeef,217);
 ++forwarded;u16 control=0x087f;__asm__ volatile("fldcw %0"::"m"(control));SetLastError(0xface);
}
void start(void){
 char value[2];require(GetEnvironmentVariableA("MNM_GPU_TEST",value,2)==1,200);mode=value[0]-'0';InitializeCriticalSection(journal_lock);
 if(mode==10){require(!wine_gpu_open(),211);ExitProcess(0);}
 require(wine_gpu_open(),206);wine_gpu_current=&current;wine_gpu_proc=&proc;wine_blit_worker_original=(void*)&original;
 require(wine_blit_worker_open(),207);worker_producer_getter=&producer_point;u32 packet[25]={0};packet[0]=42;wine_blit_packets[0].pointer=packet;wine_blit_packets[0].command=7;wine_blit_packets[0].submission=8;copy(wine_blit_packets[0].words,packet,100);
 SetLastError(0xbeef);u32 stack_before,stack_after;__asm__ volatile("movl %%esp,%0":"=r"(stack_before));wine_blit_worker((void*)0x4567,packet);__asm__ volatile("movl %%esp,%0":"=r"(stack_after));
 u16 control;__asm__ volatile("fnstcw %0":"=m"(control));require(stack_before==stack_after&&control==0x077f&&GetLastError()==0xcafe&&forwarded==1&&returned==1&&!held,208);
 require(producer_entry_seen==1&&producer_return_seen==1,215);
 require(destroyed==waited&&(mode==2||mode==11||destroyed==1),209);wine_blit_worker_close(1);wine_gpu_close(1);
 require(!wine_gpu_enabled,216);wine_submission_enabled=0;wine_blit_worker_original=(void*)&teardown_original;
 u32 queries=context_calls,fences=waited;SetLastError(0xbeef);wine_blit_worker((void*)0x4567,packet);
 __asm__ volatile("fnstcw %0":"=m"(control));require(control==0x087f&&GetLastError()==0xface&&forwarded==2&&context_calls==queries&&waited==fences&&!wine_gpu_file,218);
 ExitProcess(0);
}
