/* Production observer encoder on owned PE32-sized memory; no original code. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
typedef uint32_t u32;typedef uint8_t u8;typedef FILE* HANDLE;
#define WIN
#define API
static const char* destination;static u32 error,tid=67,queue=1,sequence=20,disabled,prefix_requested;
static u8 test_fx[512]={0x7f,3,[24]=0x80,[25]=0x1f};
static u32* forced_node;
static u32 get(const void* p){u32 v;memcpy(&v,p,4);return v;}
static void put(void* p,u32 v){memcpy(p,&v,4);}
static void copy(void* d,const void* s,u32 n){memcpy(d,s,n);}
static void zero(void* d,u32 n){memset(d,0,n);}
static struct {u32 p,n;} regions[64];static u32 region_count;
static void registered(void* p,u32 n){regions[region_count++]=(typeof(regions[0])){(u32)p,n};}
static void* allocation(u32 n){void* p=calloc(1,n);if(!p)exit(4);registered(p,n);return p;}
static int readable(u32 p,u32 n){if(!p||p+n<p)return 0;for(u32 i=0;i<region_count;++i)if(p>=regions[i].p&&(uint64_t)p+n<=(uint64_t)regions[i].p+regions[i].n)return 1;return 0;}
static u32 GetLastError(void){return error;}
static void SetLastError(u32 e){error=e;}
static u32 GetCurrentThreadId(void){return tid;}
static u32 GetTickCount(void){
#ifdef PALETTE_ABI
 __asm__ volatile("fninit;pxor %%xmm0,%%xmm0":::"memory");
#endif
 return 100;
}
static u32 GetEnvironmentVariableA(const char* name,char* p,u32 n){(void)n;if(!strcmp(name,"MNM_PALETTE_FULL_PREFIX")&&!prefix_requested){error=203;return 0;}if(disabled){error=203;return 0;}p[0]=!strcmp(name,"MNM_PALETTE_FULL_PREFIX")?'4':'1';p[1]=0;return 1;}
static HANDLE CreateFileA(const char* n,u32 a,u32 s,void* sec,u32 c,u32 f,void* t){(void)a;(void)s;(void)sec;(void)c;(void)f;(void)t;char profile[1024];snprintf(profile,sizeof(profile),"%s.profiles",destination);HANDLE h=fopen(strstr(n,"palette-profiles-")?profile:destination,"wb");return h?h:(HANDLE)-1;}
static int write(HANDLE f,const void* p,u32 n){return fwrite(p,1,n,f)==n;}
static int CloseHandle(HANDLE f){return fclose(f)==0;}
static void path(char* p,const char* b,u32 i,const char* s){snprintf(p,260,"%s%u%s",b,i,s);}
static void* GetModuleHandleA(const char* n){(void)n;return(void*)1;}
static void* GetProcAddress(void* h,const char* n){return h&&!strcmp(n,"GetPrivateProfileStringA")?(void*)0x5b0000:0;}
static HANDLE producer_heap(void){return 0;}
static void* HeapAlloc(HANDLE h,u32 flags,u32 n){(void)h;(void)flags;return malloc(n);}
static int HeapFree(HANDLE h,u32 flags,void* p){(void)h;(void)flags;free(p);return 1;}
static void* VirtualAlloc(void* p,u32 n,u32 a,u32 f){(void)p;(void)a;(void)f;
#ifdef PALETTE_ABI
 void* memory=mmap(0,n,7,MAP_ANONYMOUS|MAP_PRIVATE,-1,0);return memory==MAP_FAILED?0:memory;
#else
 return malloc(n);
#endif
}
static int VirtualProtect(void* p,u32 n,u32 flags,u32* old){(void)p;(void)n;(void)flags;*old=0x40;return 1;}
static HANDLE GetCurrentProcess(void){return 0;}
static int FlushInstructionCache(HANDLE h,void* p,u32 n){(void)h;(void)p;(void)n;return 1;}
#ifdef PALETTE_ABI
#define P(n) void palette_enter_##n(void) __asm__("_palette_enter_"#n);
void palette_enter_observe(u32*,u32,const u8*) __asm__("_palette_enter_observe");
void palette_leave_observe(u32*,const u8*) __asm__("_palette_leave_observe");
extern u32 palette_trampolines[14] __asm__("_palette_trampolines");
void palette_leave(void) __asm__("_palette_leave");
#else
#define P(n) void palette_enter_##n(void){}
void palette_leave(void){}
#endif
P(0)P(1)P(2)P(3)P(4)P(5)P(6)P(7)P(8)P(9)P(10)P(11)P(12)P(13)
#undef P
#include "../protocols/include/mnm/world_continuous_checksum.h"
#include "../runtime/scene/palette_observation.h"
static __attribute__((noinline)) void profile_call(const char* key){
 u8* text[5];for(u32 i=0;i<5;++i)text[i]=allocation(256);strcpy((char*)text[0],"STONE_EFFECT");strcpy((char*)text[1],key);strcpy((char*)text[3],"Z:\\owned\\cfg\\tables.cfg");
 u32 args[6]={(u32)text[0],(u32)text[1],(u32)text[2],(u32)text[4],256,(u32)text[3]};
#ifdef PALETTE_ABI
 extern void check_profile_call(const u32*);
 check_profile_call(args);
#else
 u32 r[40]={0};r[9]=0x586800;for(u32 i=0;i<6;++i)r[10+i]=args[i];palette_enter_observe(r,11,test_fx);strcpy((char*)text[4],".25");u32* leave=r+6;leave[7]=3;palette_leave_observe(leave,test_fx);if(leave[9]!=0x586800||leave[7]!=3||GetLastError()!=123)exit(22);
#endif
}
static u32* builder(u32 tag,u32 colour){
 u32* slot=allocation(4);u8* rgb=allocation(768);for(u32 i=0;i<768;++i)rgb[i]=(u8)i;
 u32 r[24]={0};r[9]=0x57d75a;r[10]=(u32)rgb;r[6]=4;r[5]=(u32)slot;SetLastError(123);palette_enter_observe(r,tag,test_fx);
 if(GetLastError()!=123||r[9]!=(u32)palette_leave)exit(5);
 if(tag==4){profile_call("Reducer");profile_call("SpecalValue");profile_call("Darkest");}
 u32* node=forced_node?forced_node:allocation(12+(tag<8?3*512:2*1024));forced_node=0;node[0]=0;node[1]=6;node[2]=1;for(u32 i=0;i<(tag<8?3u:2u)*256;++i){if(tag<8)((uint16_t*)(node+3))[i]=colour+i;else node[3+i]=colour+i;}
 *slot=(u32)node;u32* leave=r+1;leave[7]=1;palette_leave_observe(leave,test_fx);
 if(GetLastError()!=123||leave[9]!=0x57d75a)exit(6);return node;
}
static void refresh(u32* node,int success,int mutate){
 u32* rgb=allocation(768);u32 r[48]={0};r[9]=0x57db1a;r[10]=(u32)rgb;r[6]=4;r[5]=(u32)node;
 palette_enter_observe(r,12,test_fx);if(mutate)((uint16_t*)(node+3))[256]^=1;
 u32* leave=r+7;leave[7]=(u32)success;palette_leave_observe(leave,test_fx);if(leave[9]!=0x57db1a||GetLastError()!=123)exit(26);
}
static void retire(u32* node){u32 r[24]={0};r[9]=0x57d337;r[10]=(u32)node;palette_enter_observe(r,10,test_fx);r[7]=0x12345678;palette_leave_observe(r,test_fx);if(r[9]!=0x57d337||r[7]!=0x12345678)exit(7);}
int main(int argc,char** argv){
 if(argc!=3)return 2;destination=argv[1];prefix_requested=strcmp(argv[2],"full-prefix")==0;disabled=strcmp(argv[2],"disabled")==0;
 void* globals=mmap((void*)0x580000,0x180000,3,MAP_ANONYMOUS|MAP_PRIVATE|MAP_FIXED_NOREPLACE,-1,0);if(globals==MAP_FAILED)return 3;registered(globals,0x180000);
 const u32 addresses[]={0x582440,0x582a80,0x583330,0x583bb0,0x586660,0x5870d0,0x585d80,0x587e70,0x588730,0x588bf0,0x5979b0,0,0x584430,0x589280};
 for(u32 i=0;i<14;++i)if(i!=10&&i!=11)memcpy((void*)addresses[i],"\x55\x8b\xec\x83\xe4\xf8",6);memcpy((void*)addresses[10],"\x8b\x44\x24\x04\x85\xc0",6);
 put((void*)0x5c507c,0x5b0000);
 if(strcmp(argv[2],"bad-signature")==0){put((void*)addresses[3],0);return pal_install()?8:0;}
 if(strcmp(argv[2],"bad-import")==0){put((void*)0x5c507c,0x5b0004);return pal_install()?8:0;}
 if(!pal_install())return 9;if(disabled){pal_close();return 0;}
 u32* a=builder(0,100);u32* b=builder(1,200);a[0]=(u32)b;
 u8 body[64+512]={0};put(body,64);u32 fields[24]={0};fields[3]=9;fields[8]=100;fields[9]=90;fields[14]=0;fields[15]=1;fields[19]=64;fields[22]=0x500400;
 for(u32 i=0;i<3;++i){u32* node=i==1?b:a;queue=i+1;copy(body+64,(u8*)node+12+512,512);pal_bind(0x100000,(u32)a,(u32)node,i==1?1:0,1,fields,body);pal_bind(0x100000,(u32)a,(u32)node,i==1?1:0,1,fields,body);}
 if(strcmp(argv[2],"refresh")==0||strcmp(argv[2],"refresh-failed")==0){
  for(u32 i=0;i<2;++i){int success=strcmp(argv[2],"refresh-failed")!=0;refresh(b,success,success);copy(body+64,(u8*)b+12+512,512);pal_bind(0x100000,(u32)a,(u32)b,1,1,fields,body);}
 }
 if(strcmp(argv[2],"refresh-unknown")==0)refresh(a,1,0);
 if(strcmp(argv[2],"outside-world")==0)queue=0;
 if(strcmp(argv[2],"positive-regression")==0){queue=2;copy(body+64,(u8*)b+12+512,512);pal_bind(0x100000,(u32)a,(u32)b,1,1,fields,body);}
 if(strcmp(argv[2],"foreign")==0)tid=99;
 if(strcmp(argv[2],"unknown")==0)pal_bind(0x100000,(u32)a,0x100100,0,1,fields,body);
 if(strcmp(argv[2],"changed-table")==0){body[64]^=1;pal_bind(0x100000,(u32)a,(u32)a,0,1,fields,body);}
 if(strcmp(argv[2],"shade-only")==0){for(u32 i=0;i<32;++i){u32 table=i&1;fields[16]=table?0:(u32)-127;copy(body+64,(u8*)a+12+table*512,512);pal_bind(0x100000,(u32)a,(u32)a,0,table,fields,body);}}
 if(strcmp(argv[2],"reuse")==0){retire(a);forced_node=a;u32* c=builder(0,100);copy(body+64,(u8*)c+12+512,512);pal_bind(0x100000,(u32)c,(u32)c,0,1,fields,body);retire(c);}else
 retire(a);retire(b);
 if(strcmp(argv[2],"profile")==0){u32* c=builder(4,100);retire(c);}
 if(strcmp(argv[2],"retired")==0)pal_bind(0x100000,(u32)a,(u32)a,0,1,fields,body);
 pal_close();if(get((void*)0x5c507c)!=0x5b0000)return 23;return 0;
}
