/* Execute the production observer over owned low pages and exact callback stubs. */
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include <sys/mman.h>
using u8=std::uint8_t;using u32=std::uint32_t;using i32=std::int32_t;using HANDLE=void*;
#define MNM_STATE_WORLD 0x6cbb78
#define THIS __attribute__((thiscall))
static u32 error=0xabc123,tick=100,thread=316;
static u32 admission_world_returns=1,menu_screen=2,menu_generation=8,menu_request=7,menu_status=1;
static unsigned jumps,protects,text_calls,health_calls,closed;
static bool enabled;
static std::vector<u8> output;
static u32 get(const void* p){u32 value;std::memcpy(&value,p,4);return value;}
static void put(void* p,u32 value){std::memcpy(p,&value,4);}
static void copy(void* d,const void* s,u32 n){std::memcpy(d,s,n);}
static int equal(const void* a,const void* b,u32 n){return !std::memcmp(a,b,n);}
static int readable(const void* p,u32 n){return p&&std::uintptr_t(p)+n>=std::uintptr_t(p);}
static u32 GetLastError(){return error;}static void SetLastError(u32 e){error=e;}
static u32 GetCurrentThreadId(){return thread;}static u32 GetTickCount(){tick+=10;return tick;}
static u32 GetEnvironmentVariableA(const char*,char* p,u32){if(!enabled)return 0;std::memcpy(p,"test",5);return 4;}
static HANDLE CreateFileA(const char*,u32,u32,void*,u32 disposition,u32,void*){if(disposition==2)output.clear();return (void*)1;}
static int WriteFile(HANDLE,const void* p,u32 n,u32* done,void*){auto at=(const u8*)p;output.insert(output.end(),at,at+n);*done=n;error=0xdead;return 1;}
static int CloseHandle(HANDLE){++closed;return 1;}
static int VirtualProtect(void*,u32,u32,u32* old){++protects;*old=4;return 1;}
static HANDLE GetCurrentProcess(){return (void*)1;}static int FlushInstructionCache(HANDLE,void*,u32){return 1;}
static void region_input_fp_save(void* p){asm volatile("fxsave (%0)"::"r"(p):"memory");}
static void region_input_fp_restore(void* p){asm volatile("fxrstor (%0)"::"r"(p):"memory");}
static u32 identity(u32 owner,u32* p,u32 n){assert(owner==thread&&n==32);u32 w[]={1,32,0x40209ca7,1,0,123,0,316};copy(p,w,32);return 1;}
static auto frame_input_identity=&identity;
static u32 THIS original_text(void* object,const char* text){assert(object==(void*)0x657fe8&&!std::strcmp(text,"Health: 200"));++text_calls;put((void*)0x658162,1);error=0x123456;return 0x345678;}
static u32 THIS original_health(void* object,u32 value){assert(object==(void*)0x680000&&value==170);++health_calls;put((u8*)object+0xc7,value);error=0x654321;return 0x876543;}
static void* trampoline(u32 site,u32 length,int relative){assert(length==6&&!relative);return site==0x4dba60?(void*)&original_text:(void*)&original_health;}
static void jump(u32 site,u32,u32 length){assert((site==0x4dba60||site==0x4d3a30)&&length==6);++jumps;}
#include "../runtime/menu/hud_acceptance.h"
#include "../runtime/render/ui_key_observation.h"
int main(int argc,char** argv){
 assert(argc==3);assert(mmap((void*)0x400000,0x300000,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0)==(void*)0x400000);
 assert(ui_acceptance_open()&&jumps==0&&!ui_acceptance_file);enabled=true;
 assert(!ui_acceptance_open()&&jumps==0);const u8 text[]={0x83,0xec,0x34,0x53,0x55,0x56},health[]={0x83,0xec,0x10,0x56,0x8b,0xf1};
 copy((void*)0x4dba60,text,6);assert(!ui_acceptance_open()&&jumps==0);copy((void*)0x4d3a30,health,6);
 assert(ui_acceptance_open()&&jumps==2&&protects==4);
 put((void*)0x6def58,0x660000);put((void*)0x6def5c,1);put((void*)0x660004,1);put((void*)0x6600e4,200);put((void*)0x6600ec,20);put((void*)0x6600e8,15*256);
 put((void*)0x6801ef,0x660000);put((void*)0x6800cb,200);
 put((void*)MNM_STATE_WORLD,0x5c5dd8);*(u8*)(MNM_STATE_WORLD+0xfd5a)=1;put((void*)(MNM_STATE_WORLD+0xfd62),42);put((void*)0x6903e8,2);
 error=0xabc123;ui_acceptance_sample(1,0,0,0x42,error);assert(error==0xabc123);
 assert(get(output.data()+16+10*4)==1&&get(output.data()+16+7*4)==42&&get(output.data()+16+18*4)==2);
 assert(ui_acceptance_text_callback((void*)0x657fe8,"Health: 200")==0x345678&&error==0x123456&&text_calls==1);
 assert(ui_acceptance_health_callback((void*)0x680000,170)==0x876543&&error==0x654321&&health_calls==1);
 ui_acceptance_close();assert(closed==1);auto* f=std::fopen(argv[1],"wb");assert(f);assert(std::fwrite(output.data(),1,output.size(),f)==output.size());assert(!std::fclose(f));
 output.clear();input_ui_acceptance(32,0x8001,2,1,0x46bf32);assert(output.empty());
 input_ui_configure();assert(!std::strcmp(input_ui_path,"test.keys"));input_ui_acceptance(39,0x8001,2,1,0x46bf32);assert(output.empty());
 input_ui_acceptance(32,0x8001,2,1,0x46bf32);input_ui_acceptance(32,0x8000,4,1,0x46bf32);assert(input_ui_count==1);
 input_ui_acceptance(32,0,6,1,0x46bf32);input_ui_acceptance(32,0x8001,8,1,0x46bf32);input_ui_acceptance(32,0,10,1,0x46bf32);assert(input_ui_count==4);
 f=std::fopen(argv[2],"wb");assert(f);assert(std::fwrite(output.data(),1,output.size(),f)==output.size());assert(!std::fclose(f));
}
