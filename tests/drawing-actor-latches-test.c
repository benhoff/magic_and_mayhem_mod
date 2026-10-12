#include "../runtime/shadow/win32_min.h"
API u32 WIN GetTickCount(void);
API u32 WIN GetCurrentThreadId(void);
API void* WIN GetProcAddress(HANDLE,const char*);
API void WIN Sleep(u32);
static u32 menu_screen=2;
static u32 get(const void* p){const u8* b=p;return b[0]|((u32)b[1]<<8)|((u32)b[2]<<16)|((u32)b[3]<<24);}
static void put(void* p,u32 v){u8* b=p;b[0]=v;b[1]=v>>8;b[2]=v>>16;b[3]=v>>24;}
static void copy(void* a,const void* b,u32 n){u8* x=a;const u8* y=b;while(n--)*x++=*y++;}
static int equal(const void* a,const void* b,u32 n){const u8* x=a;const u8* y=b;while(n--)if(*x++!=*y++)return 0;return 1;}
static int readable(const void* p,u32 n){u32 m[7];return VirtualQuery(p,m,28)==28&&m[4]==0x1000&&!(m[5]&0x101)&&(u32)p+n>=(u32)p&&(u32)p+n<=m[0]+m[3];}
static u8* owned;
static u8 before[65536];
#define MNM_LATCH_POOL ((u32)owned+0x100)
#define MNM_LATCH_HIT ((u32)owned+0x108)
#define MNM_LATCH_COUNT ((u32)owned+0x10c)
#define MNM_LATCH_MODE ((u32)owned+0x110)
#include "../runtime/menu/drawing_actor_latches.h"
static void require(int ok,u32 code){if(!ok)ExitProcess(code);}
static void sample(u32 kind,u32 call,u32 phase){copy(before,owned,65536);latch_sample(kind,call,phase);require(equal(before,owned,65536),210);}
void start(void){
 owned=VirtualAlloc(0,65536,0x3000,4);require(owned!=0,201);
 u32 base=(u32)owned+0x1000,actor=base+0xe4b;
 put((void*)MNM_LATCH_POOL,base);put((void*)(MNM_LATCH_POOL+4),3);
 for(u32 i=0;i<3;++i){put((void*)(base+i*0xe4b),i);put((void*)(base+i*0xe4b+4),1);}
 put((void*)(actor+0x68),1);put((void*)(actor+0xa8),27);put((void*)(actor+0xe4),100);put((void*)MNM_LATCH_COUNT,1);put((void*)MNM_LATCH_MODE,1);
 require(latch_open(),202);sample(1,1,1);
 require(latch_count==1&&latch_rows[0][10]==3&&latch_rows[0][11]==1&&latch_rows[0][24]==2&&latch_rows[0][57]==2&&latch_rows[0][58]==27,203);
 put((void*)MNM_LATCH_HIT,base+2*0xe4b);sample(2,1,2);
 require(latch_rows[1][15]==3&&latch_rows[1][57]==2&&latch_rows[1][24]==2,204);
 sample(0,0,0);u32 at=latch_count;sample(0,0,0);require(latch_count==at,205);
 put((void*)(actor+0x68),0);put((void*)(actor+0x70),1);sample(1,2,3);
 require(latch_rows[at][11]==0&&latch_rows[at][12]==1&&latch_rows[at][32]==2,206);
 put((void*)actor,99);sample(2,2,4);require(!(latch_rows[at+1][7]&1)&&!latch_rows[at+1][57],207);
 put((void*)actor,1);put((void*)(MNM_LATCH_POOL+4),257);sample(1,3,5);require(!(latch_rows[at+2][7]&1),208);
 put((void*)(MNM_LATCH_POOL+4),3);put((void*)MNM_LATCH_POOL,0xfffffff0u);sample(2,3,6);require(!(latch_rows[at+3][7]&1),209);
 latch_close();u32 count=latch_count;sample(0,0,0);require(latch_count==count,211);
 latch_file=(HANDLE)1;latch_count=2048;sample(1,4,7);require(latch_count==2048&&latch_dropped==1,212);
 latch_owner^=0x80000000;sample(1,4,8);require(latch_foreign==1&&latch_dropped==1,213);
 latch_file=0;ExitProcess(0);
}
