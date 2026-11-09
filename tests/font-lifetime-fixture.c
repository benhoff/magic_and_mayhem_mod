#define PALETTE_ABI
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
static int legacy_requested;
static u32 GetEnvironmentVariableA(const char* name,char* p,u32 n){(void)n;if(!strcmp(name,"MNM_FONT_LIFETIME")&&!legacy_requested){error=203;return 0;}if(!strcmp(name,"MNM_PALETTE_FULL_PREFIX")&&!prefix_requested){error=203;return 0;}if(disabled){error=203;return 0;}p[0]=!strcmp(name,"MNM_PALETTE_FULL_PREFIX")?'4':'1';p[1]=0;return 1;}
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

static u8* continuous_map=(u8*)1;
#define F(n) void font_enter_##n(void) __asm__("_font_enter_"#n);
F(0)F(1)F(2)F(3)F(4)
#undef F
void font_enter_observe(u32*,u32) __asm__("_font_enter_observe");
void font_leave_observe(u32*) __asm__("_font_leave_observe");
extern u32 font_trampolines[5] __asm__("_font_trampolines");
void font_leave(void) __asm__("_font_leave");
#include "../protocols/include/mnm/world_continuous_checksum.h"
static void fdc_source_context(u32* h){h[24]=7;h[26]=3;}
#include "../runtime/scene/font_observation.h"
extern void font_load_probe(u32*,u32,u32,u32,u32);
extern void font_destroy_probe(u32*,u32);
static void check_probe(u32 address,u32 object,u32 name,u32 expected,int destroy){
 u32 out[9]={0};long double value=3.25L,result;u8 vector[16],after[16];for(u32 i=0;i<16;++i)vector[i]=i+1;
 unsigned short before,now;u32 mx_before,mx_after;SetLastError(123);
 __asm__ volatile("fnstcw %0;fldt %2;movdqu %3,%%xmm0;stmxcsr %1":"=m"(before),"=m"(mx_before):"m"(value),"m"(vector):"memory");
 if(destroy)font_destroy_probe(out,address);else font_load_probe(out,address,object,name,1);
 __asm__ volatile("fnstcw %0;fstpt %2;movdqu %%xmm0,%3;stmxcsr %1":"=m"(now),"=m"(mx_after),"=m"(result),"=m"(after)::"memory");
 if(out[0]!=0x71717171||out[1]!=0x61616161||out[4]!=0x51515151||out[5]!=0x42424242||out[6]!=(destroy?0x40404040:object)||out[7]!=expected||out[3]!=out[2]-4||!(out[8]&0x400)||before!=now||mx_before!=mx_after||memcmp(&value,&result,10)||memcmp(vector,after,16)||GetLastError()!=123)exit(20);
}
static u8* new_font(void){
 u8* raw=allocation(871);put(raw+4,871);put(raw+8,3);put(raw+12,1);put(raw+16,1);put(raw+20,1);put(raw+32,1);put(raw+36,1);
 put(raw+816,0);u8* frame=raw+820;put(frame,51);put(frame+4,1);put(frame+8,1);put(frame+28,0x11223344);put(frame+40,48);put(frame+44,50);frame[49]=1;frame[50]=3;return raw;
}
static void store(u8** p,u32 object,u32 value){*(*p)++=0xc7;*(*p)++=0x05;put(*p,object);*p+=4;put(*p,value);*p+=4;}
static void load_body(u32 raw){
 u8* p=(u8*)0x4a5ae6;*p++=0x5d;*p++=0x5b;
 const u32 offsets[]={0,8,12,16,20,0x220,0x224,0x230},values[]={raw,raw+808,raw+816,raw+820,0,1,20,2};
 for(u32 i=0;i<8;++i){*p++=0xc7;*p++=0x81;put(p,offsets[i]);p+=4;put(p,values[i]);p+=4;}
 *p++=0xb8;put(p,1);p+=4;*p++=0xc2;*p++=8;*p++=0;
}
int main(int argc,char** argv){
 if(argc!=3)return 2;destination=argv[1];disabled=!strcmp(argv[2],"disabled");
 void* globals=mmap((void*)0x400000,0x380000,7,MAP_ANONYMOUS|MAP_PRIVATE|MAP_FIXED_NOREPLACE,-1,0);if(globals==MAP_FAILED)return 3;registered(globals,0x380000);
 memcpy((void*)0x4a5ae0,"\x8b\x44\x24\x04\x53\x55",6);
 const u32 destructors[]={0x4be8c0,0x4be960,0x4bea00,0x4beaa0};
 for(u32 i=0;i<4;++i){u8* p=(u8*)destructors[i];*p++=0xa1;put(p,fdc_objects[i]);p+=4;const u32 offsets[]={0,8,12,16,20};for(u32 j=0;j<5;++j)store(&p,fdc_objects[i]+offsets[j],0);*p++=0xb8;put(p,0x12343210);p+=4;*p++=0xc3;}
 if(!strcmp(argv[2],"bad-signature")){*(u8*)0x4be8c0^=1;return fdc_install()?8:0;}
 if(!strcmp(argv[2],"not-resident")){continuous_map=0;return fdc_install()?8:0;}
 if(!strcmp(argv[2],"conflicting-observers")){legacy_requested=1;const u32 sites[]={0x4a5ae0,0x4be8c0,0x4be960,0x4bea00,0x4beaa0};u8 before[5][6];for(u32 i=0;i<5;++i)memcpy(before[i],(void*)sites[i],6);if(fdc_install()||fdc_file)return 11;for(u32 i=0;i<5;++i)if(memcmp(before[i],(void*)sites[i],6))return 12;return 0;}
 if(!fdc_install())return 9;if(disabled)return fdc_file?10:0;
 u8* name=allocation(256);strcpy((char*)name,"sprites\\body640.sft");
 unsigned short saved;__asm__ volatile("fnstcw %0":"=m"(saved));
 for(u32 precision=0;precision<2;++precision){
  unsigned short cw=precision?0x27f:0x37f;__asm__ volatile("fldcw %0"::"m"(cw));
  for(u32 role=0;role<4;++role){
   const char* names[]={"sprites\\heading text 640.sft","sprites\\body text 640.sft","sprites\\toolTip text.sft","sprites\\yellowtext.sft"};strcpy((char*)name,names[role]);
   for(u32 gen=0;gen<2;++gen){u8* raw=new_font();load_body((u32)raw);check_probe(0x4a5ae0,fdc_objects[role],(u32)name,1,0);
    u8 body[323]={0};copy(body,raw+820,51);put(body+28,0);u32 fields[24]={0};fields[1]=++sequence;fields[2]=8;fields[3]=7;fields[8]=5;fields[9]=6;fields[15]=31;fields[16]=63;fields[17]=31;fields[18]=323;fields[19]=51;fields[20]=272;fields[22]=0x4a5cbd;
    fdc_glyph(fields,body,sizeof(body),(u32)raw+820);
    if(!strcmp(argv[2],"foreign"))tid=99;
   }
   check_probe(destructors[role],0,0,0x12343210,1);
  }
 }
 __asm__ volatile("fldcw %0"::"m"(saved));fdc_close();if(fdc_loss)return !strcmp(argv[2],"foreign")?0:21;puts("24 production font hook ABI cases passed");return 0;
}
