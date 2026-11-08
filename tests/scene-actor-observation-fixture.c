/* Execute the production encoder on owned 32-bit memory, with passive API stubs. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
typedef uint32_t u32;typedef uint8_t u8;typedef FILE* HANDLE;
#define WIN
struct Surface {u32 id,width,height;};
static struct Surface surface={9,800,600};
static struct Surface* bound(void){return &surface;}
static u8* continuous_map=(u8*)1;
static u32 continuous_resident=1,continuous_public_bind=1,continuous_generation_low=1,continuous_generation_high;
static u32 continuous_nonce_low=123,continuous_nonce_high=456,sequence=21;
static const char* destination;static int disabled;static u32 tid=67,journal=4;
static u32 get(const void* p){u32 v;memcpy(&v,p,4);return v;}
static void put(void* p,u32 v){memcpy(p,&v,4);}
static void copy(void* d,const void* s,u32 n){memcpy(d,s,n);}
static void zero(void* p,u32 n){memset(p,0,n);}
static struct {uintptr_t p;u32 n;} regions[64];static unsigned regions_count;
static void registered(void* p,u32 n){regions[regions_count].p=(uintptr_t)p;regions[regions_count++].n=n;}
static void* allocation(u32 n){void* p=calloc(1,n);if(!p)exit(4);registered(p,n);return p;}
static int readable(u32 p,u32 n){if(!p||p+n<p)return 0;for(unsigned i=0;i<regions_count;++i)if(p>=regions[i].p&&(uint64_t)p+n<=regions[i].p+regions[i].n)return 1;return 0;}
static u32 GetEnvironmentVariableA(const char* name,char* p,u32 n){(void)name;if(disabled)return 0;if(n<2)return 2;p[0]='1';p[1]=0;return 1;}
static HANDLE CreateFileA(const char* n,u32 access,u32 shared,void* sec,u32 creation,u32 flags,void* template){(void)n;(void)access;(void)shared;(void)sec;(void)creation;(void)flags;(void)template;HANDLE f=fopen(destination,"wb");return f?f:(HANDLE)-1;}
static int write(HANDLE f,const void* p,u32 n){return fwrite(p,1,n,f)==n;}
static int CloseHandle(HANDLE f){return fclose(f)==0;}
static void path(char* out,const char* p,u32 n,const char* s){(void)p;(void)n;(void)s;strcpy(out,"fixture");}
static u32 GetCurrentThreadId(void){return tid;}
static u32 GetTickCount(void){static u32 t=100;return t+=20;}
static u32 gameplay(u32 owner,u32* p,u32 n){if(owner!=67||n!=16)return 0;p[0]=1;p[1]=16;p[2]=0x40209ca7;p[3]=journal;return 1;}
static void* GetModuleHandleA(const char* n){return strcmp(n,"MnmMenu.dll")==0?(void*)1:0;}
static void* GetProcAddress(void* p,const char* n){return p&&strcmp(n,"MenuGameplayPointV1")==0?(void*)&gameplay:0;}
#include "../protocols/include/mnm/world_continuous_checksum.h"
#include "../runtime/scene/scene_actor_observation.h"
static void asset(u8* actor,u32 disp,u32 asset_at,u32 argument,u32 bound,u8* frame){
 u32* record=allocation(44);record[1]=argument;record[2]=4;record[3]=(u32)-5;record[7]=12;record[8]=34;
 u8* a=allocation(0x114);u32* spr=allocation(16);u32* h=allocation(16);h[3]=bound;u32* table=allocation((bound+1)*4);
 for(u32 i=0;i<=bound;++i)table[i]=32;
 spr[0]=(u32)(uintptr_t)h;spr[2]=(u32)(uintptr_t)table;spr[3]=(u32)(uintptr_t)(frame-32);put(a+0x110,(u32)(uintptr_t)spr);
 put(actor+disp,(u32)(uintptr_t)record);put(actor+asset_at,(u32)(uintptr_t)a);
}
int main(int argc,char** argv){
 if(argc!=3)return 2;
 destination=argv[1];disabled=strcmp(argv[2],"disabled")==0;
 void* globals=mmap((void*)0x600000,0x200000,PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_PRIVATE|MAP_FIXED_NOREPLACE,-1,0);
 if(globals==MAP_FAILED)return 3;
 registered(globals,0x200000);
 u8* actor=allocation(0xe4b);put(actor+4,1);put(actor+0xe4,200);put(actor+0xa8,0);put(actor+0x174,0);put(actor+0x7cd,1);
 u32* type=allocation(12);type[2]=2;put(actor+0xac,(u32)(uintptr_t)type);
 put((void*)0x6def58,(u32)(uintptr_t)actor);put((void*)0x6def5c,1);put((void*)0x6e9850,3);
 u8* body=(u8*)allocation(96)+32;put(body,64);put(body+4,2);put(body+8,2);put(body+28,0x12345678);
 u8* child=(u8*)allocation(96)+32;memcpy(child,body,64);child[40]=23;
 u8* equipment=(u8*)allocation(96)+32;memcpy(equipment,body,64);equipment[40]=31;
 asset(actor,0xb4,0xcc,2,2,body);asset(actor,0x7d9,0x7f1,99,1,child);
 asset(actor,0x130,0x148,0,0,equipment);put(actor+0x124,1);
 put(actor+0x891,get(actor+0x130));put(actor+0x8a9,get(actor+0x148));put(actor+0x8bd,1);
 put((void*)0x6df150,get(actor+0x130));put((void*)0x6df168,get(actor+0x148));put(actor+0x8d1,1);
 if(strcmp(argv[2],"zero-health")==0)put(actor+0xe4,0);
 if(strcmp(argv[2],"bad-base")==0)put((void*)(get((void*)(get(actor+0xcc)+0x110))+12),0xfffffff0);
 u32* queue=allocation(24);u32* rows=allocation(36*4);queue[0]=(u32)(uintptr_t)rows;queue[2]=queue[3]=4;queue[5]=3;
 rows[1]=(u32)(uintptr_t)body;rows[2]=100;rows[3]=90;
 rows[10]=(u32)(uintptr_t)child;rows[11]=106;rows[12]=132;
 rows[19]=(u32)(uintptr_t)body;rows[20]=100;rows[21]=90;rows[24]=9;
 rows[28]=(u32)(uintptr_t)equipment;rows[29]=112;rows[30]=124;
 sao_capture((u32)(uintptr_t)queue);
 continuous_generation_low=2;sequence=22;journal=7;put(actor+0xe4,190);sao_capture((u32)(uintptr_t)queue);
 if(strcmp(argv[2],"foreign")==0){tid=99;continuous_generation_low=3;sao_capture((u32)(uintptr_t)queue);}
 sao_close();return 0;
}
