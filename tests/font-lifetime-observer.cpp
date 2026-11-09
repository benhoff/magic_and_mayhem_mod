// Production entry/return assembly with authored callees; no game starts.
#include <array>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <sys/mman.h>
#define __declspec(x)
extern "C" {
#include "../runtime/scene/font_lifetime.c"
unsigned contract_host_sp,contract_function,contract_self,contract_argc;
unsigned contract_call_sp,contract_first_arg,contract_gpr[8],contract_flags,contract_guards[2];
void contract_original_call(unsigned,unsigned,unsigned,const unsigned*);
}
static std::vector<unsigned char> output;
static unsigned last_error=123,thread_id=17,clock_ticks;
extern "C" {
u32 WIN GetCurrentThreadId(){return thread_id;}
u32 WIN GetTickCount(){return ++clock_ticks;}
u32 WIN GetLastError(){return last_error;}
void WIN SetLastError(u32 n){last_error=n;}
u32 WIN GetEnvironmentVariableA(const char* name,char* out,u32 length){
 const char* value=!std::strcmp(name,"MNM_FONT_LIFETIME")?"1":!std::strcmp(name,"MNM_SCENE_DIR")?"owned":nullptr;
 if(!value){last_error=203;return 0;}auto n=std::strlen(value);if(n>=length)return n+1;std::memcpy(out,value,n+1);return n;
}
void* WIN GetModuleHandleA(const char*){return reinterpret_cast<void*>(0x400000);}
u32 WIN VirtualQuery(const void* p,void* out,u32){
 auto v=reinterpret_cast<unsigned>(p);if(v<0x400000||v>=0x800000)return 0;
 unsigned q[7]={0x400000,0x400000,4,0x400000,0x1000,4,0};std::memcpy(out,q,28);return 28;
}
void* WIN VirtualAlloc(void*,u32 n,u32,u32){auto p=mmap(nullptr,n,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);return p==MAP_FAILED?nullptr:p;}
int WIN VirtualProtect(void*,u32,u32,u32* prior){*prior=0x40;return 1;}
int WIN FlushInstructionCache(HANDLE,const void*,u32){return 1;}
HANDLE WIN GetCurrentProcess(){return reinterpret_cast<void*>(1);}
HANDLE WIN GetProcessHeap(){return reinterpret_cast<void*>(3);}
void* WIN HeapAlloc(HANDLE,u32,u32 n){return std::malloc(n);}
int WIN HeapFree(HANDLE,u32,void* p){std::free(p);return 1;}
HANDLE WIN CreateFileA(const char*,u32,u32,void*,u32,u32,HANDLE){return reinterpret_cast<void*>(2);}
int WIN WriteFile(HANDLE,const void* data,u32 n,u32* written,void*){const auto* p=static_cast<const unsigned char*>(data);output.insert(output.end(),p,p+n);*written=n;return 1;}
int WIN CloseHandle(HANDLE){return 1;}
void WIN ExitProcess(u32){std::abort();}
void glyph_fixture(){font_trace_glyph(0x700080,10,20,0x4a5abd,7,99);}
}
static void require(bool b,const char* reason){if(!b)throw std::runtime_error(reason);}
static void append(std::vector<unsigned char>& b,unsigned v){for(unsigned i=0;i<4;++i)b.push_back((v>>(i*8))&255);}
static void mov_mem(std::vector<unsigned char>& b,unsigned p,unsigned v){b.insert(b.end(),{0xc7,0x05});append(b,p);append(b,v);}
static void reset_font(){for(unsigned p:objects)std::memset(reinterpret_cast<void*>(p),0,0x238);}
struct Result{std::array<unsigned,8> gpr;unsigned flags;};
static Result invoke(unsigned entry,unsigned self,unsigned argc){
 unsigned args[4]={0x710000,2,3,4};
 last_error=123;
 contract_original_call(entry,self,argc,args);
 require(last_error==123,"LastError changed");
 require(contract_gpr[4]==contract_call_sp+argc*4,"RET immediate/stack changed");
 require(contract_guards[0]==0x2468ace0&&contract_guards[1]==0x13579bdf,"Stack guards changed");
 Result r{};std::memcpy(r.gpr.data(),contract_gpr,32);r.gpr[4]=0;r.flags=contract_flags&0xcd5;return r;
}
int main(int argc,char** argv)try{
 require(argc==2,"Output transcript required");
 require(mmap(reinterpret_cast<void*>(0x400000),0x400000,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0)!=MAP_FAILED,"Private image mapping failed");
 const unsigned entries[11]={0x4be870,0x4be8c0,0x4be910,0x4be960,0x4be9b0,0x4bea00,0x4bea50,0x4beaa0,0x4a5ae0,0x4a59e0,0x5575b0};
 for(unsigned i=0;i<11;++i){
  std::vector<unsigned char> b;
  if(i<8&&!(i&1)){b={0x33,0xc0,0x68};append(b,entries[i+1]);b.insert(b.end(),{0x83,0xc4,4});mov_mem(b,objects[i/2],0);}
  else if(i<8){b={0xa1};append(b,objects[i/2]);mov_mem(b,objects[i/2],0);}
  else if(i==8){b={0x8b,0x44,0x24,4,0x53};mov_mem(b,objects[0],0x700000);b.push_back(0x5b);}
  else if(i==9){b={0x53,0x8b,0x5c,0x24,8,0xe8};append(b,reinterpret_cast<unsigned>(&glyph_fixture)-(entries[i]+10));b.push_back(0x5b);}
  else{b={0xa1,0xd5,0xe6,0x6d,0};}
  b.push_back(0xb8);append(b,1);b.push_back(0xb9);append(b,0x11223344);b.push_back(0xba);append(b,0x55667788);
  b.insert(b.end(),{0x3b,0xc0});
  if(i==8||i==9)b.insert(b.end(),{0xc2,static_cast<unsigned char>(i==8?8:16),0});else b.push_back(0xc3);
  std::memcpy(reinterpret_cast<void*>(entries[i]),b.data(),b.size());
 }
 std::strcpy(reinterpret_cast<char*>(0x710000),"Fonts/owned.sft");
 put(reinterpret_cast<void*>(0x700004),256);put(reinterpret_cast<void*>(0x700080),40);
 std::array<Result,11> before;
 reset_font();for(unsigned i=0;i<11;++i){if(i==9)put(reinterpret_cast<void*>(objects[0]),0x700000);before[i]=invoke(entries[i],objects[0],i==8?2:i==9?4:0);}
 reset_font();require(font_trace_install(),"Production hook installation failed");
 for(unsigned i=0;i<11;++i){auto after=invoke(entries[i],objects[0],i==8?2:i==9?4:0);require(before[i].gpr==after.gpr&&before[i].flags==after.flags,"Entry/return ABI changed");}
 require(!loss&&resources==1&&count==23,"Incomplete production lifecycle events");
 glyph_fixture();require(!loss&&count==24,"Direct layout glyph was not bound to the live raw range");
 invoke(entries[8],objects[0],2);require(resources==2&&fonts[0].token==2,"Reload did not advance generation");
 invoke(entries[1],objects[0],0);require(!fonts[0].live,"Destructor did not retire current generation");
 font_trace_close();require(!loss&&!file,"Terminal closure failed");
 std::ofstream out(argv[1],std::ios::binary);out.write(reinterpret_cast<const char*>(output.data()),output.size());out.close();
 std::cout<<"{\"success\":true,\"assembly_entries\":11,\"reload_generations\":2,\"records\":"<<count<<",\"bytes\":"<<output.size()<<",\"original_game_runs\":0}\n";
 return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
