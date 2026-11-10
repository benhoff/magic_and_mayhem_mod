// Production metadata encoder and installer with authored PE32-sized memory.
// Stub services intentionally change LastError. No original engine executes.
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <sys/mman.h>
#define __declspec(x)
extern "C" {
#include "../runtime/scene/required_boundaries.c"
}
static std::vector<unsigned char> output;
static u32 last_error=123,tick,owner_thread=17;
static int enabled;
extern "C" {
u32 WIN GetCurrentThreadId(){return owner_thread;}
u32 WIN GetTickCount(){last_error=77;return ++tick;}
u32 WIN GetLastError(){return last_error;}
void WIN SetLastError(u32 n){last_error=n;}
u32 WIN GetEnvironmentVariableA(const char* n,char* p,u32 size){
 const char* value=!std::strcmp(n,"MNM_REQUIRED_BOUNDARIES")?(enabled?"1":nullptr):!std::strcmp(n,"MNM_SCENE_DIR")?"owned":nullptr;
 if(!value){last_error=203;return 0;}u32 len=std::strlen(value);if(len>=size)return len+1;std::memcpy(p,value,len+1);last_error=99;return len;
}
void* WIN GetModuleHandleA(const char*){return reinterpret_cast<void*>(0x400000);}
u32 WIN VirtualQuery(const void* p,void* out,u32){
 u32 at=reinterpret_cast<u32>(p);if(at<0x400000||at>=0x800000)return 0;
 u32 q[7]={0x400000,0x400000,4,0x400000,0x1000,4,0};std::memcpy(out,q,28);last_error=66;return 28;
}
void* WIN VirtualAlloc(void*,u32 n,u32,u32){auto p=mmap(nullptr,n,7,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);return p==MAP_FAILED?nullptr:p;}
int WIN VirtualProtect(void*,u32,u32,u32* old){*old=0x40;return 1;}
int WIN FlushInstructionCache(HANDLE,const void*,u32){return 1;}
HANDLE WIN GetCurrentProcess(){return reinterpret_cast<void*>(1);}
HANDLE WIN CreateFileA(const char*,u32,u32,void*,u32,u32,HANDLE){return reinterpret_cast<void*>(2);}
int WIN WriteFile(HANDLE,const void* p,u32 n,u32* wrote,void*){const auto* b=static_cast<const unsigned char*>(p);output.insert(output.end(),b,b+n);*wrote=n;last_error=55;return 1;}
int WIN CloseHandle(HANDLE){last_error=44;return 1;}
void WIN ExitProcess(u32){std::abort();}
}
static void check(bool ok,const char* reason){if(!ok)throw std::runtime_error(reason);}
static void store(u32 p,u32 v){std::memcpy(reinterpret_cast<void*>(p),&v,4);}
static void owner(u32 object,u32 table,u32 array_offset,u32 controls){
 std::memset(reinterpret_cast<void*>(object),0,0x300);store(object,table);store(object+array_offset,0x710000);
 for(u32 i=0;i<controls;++i)store(0x710000+4*i,0x720000+i*32);
}
int main(int argc,char**argv)try{
 check(argc==2,"Expected wire output");check(mmap(reinterpret_cast<void*>(0x400000),0x400000,7,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0)!=MAP_FAILED,"Private mapping failed");
 check(boundary_trace_install()&&last_error==123&&output.empty(),"Disabled observer changed state");enabled=1;
 for(const auto&a:anchors)std::memcpy(reinterpret_cast<void*>(a.address),a.bytes,a.length);
 *reinterpret_cast<u8*>(anchors[0].address)^=1;
 check(!boundary_trace_install()&&output.empty(),"Bad anchor installed hooks");
 *reinterpret_cast<u8*>(anchors[0].address)^=1;last_error=123;
 check(boundary_trace_install()&&last_error==123,"Installer lost LastError");
 for(u32 i=0;i<MNM_BOUNDARY_HOOKS;++i){const auto&a=anchors[i];const auto* p=reinterpret_cast<const u8*>(a.address);check(p[0]==0xe9&&a.address+5+get(a.address+1)==u32(a.entry),"Entry branch changed");
  const u32 t=boundary_trampolines[i];check(get(t+a.length+1)+t+a.length+5==a.address+a.length,"Continuation changed");
  if(a.relative)check(t+5+get(t+1)==a.address+5+u32(a.bytes[1]|u32(a.bytes[2])<<8|u32(a.bytes[3])<<16|u32(a.bytes[4])<<24),"Relative call relocation changed");
  else check(!std::memcmp(reinterpret_cast<void*>(t),a.bytes,a.length),"Trampoline prefix changed");}
 u32 h[128]={0};
 for(auto c:std::array<std::array<u32,4>,4>{{{{0x5c63e4,0x4b,6,0}},{{0x5c641c,0x4b,4,0}},{{0x5c6644,0x53,5,0}},{{0x5c5ebc,0x4b,21,0}}}}){
  owner(0x700000,c[0],c[1],c[2]);std::memset(h,0,sizeof h);snapshot(h,0x700000);
  check(h[56]==c[2],"Control array field/count differs");for(u32 j=0;j<c[2];++j)check(h[57+j]==0x720000+j*32,"Wrong populated control pointer");}
 owner(0x700000,0x5c63e4,0x4b,6);store(0x700047,0x730000);store(0x730000,0x5c6414);store(0x730004,0x4a7850);store(0x730008,0x700000);
 std::memset(h,0,sizeof h);snapshot(h,0x700000);check(h[80]&&h[81]==0x5c6414&&h[82]==0x4a7850&&h[83]==0x700000,"Main callback field differs");
 owner(0x701000,0x5c5ebc,0x4b,21);store(0x701047,0x731000);store(0x731000,0x732000);store(0x70104f,0x730020);store(0x730020,0x5c5eec);store(0x730024,0x4747a0);store(0x730028,0x701000);
 std::memset(h,0,sizeof h);snapshot(h,0x701000);check(h[56]==21&&h[57]==0x720000&&h[77]==0x720280,"Report labels confused with the one-button array");check(h[84]&&h[85]==0x5c5eec&&h[86]==0x4747a0&&h[87]==0x701000,"Report callback field differs");
 for(u32 tag=0;tag<23;++tag){u32 r[20]={0};r[6]=0x700000;r[9]=0x401234;last_error=123;boundary_trace_enter(r,tag);check(last_error==123,"Entry lost LastError");
  if(tag==18||tag==19){check(r[9]==0x401234,"Entry-only tag changed continuation");continue;}
  check(r[9]==u32(boundary_trace_leave),"Missing return observation");u32* returned=r+anchors[tag].pop/4;
  boundary_trace_return(returned);check(returned[9]==0x401234&&last_error==123,"Return continuation/LastError differs");}
 boundary_trace_world(0x740000,800,600,800,1);check(last_error==123,"World lost LastError");
 boundary_trace_world(0x740000,800,600,800,2);check(last_error==123&&worlds==2&&world_changes==1,"Every-World/dedup mismatch");
 boundary_trace_close();check(last_error==123&&!loss,"Close lost LastError or observations");
 std::ofstream f(argv[1],std::ios::binary);f.write(reinterpret_cast<const char*>(output.data()),output.size());check(bool(f),"Wire write failed");
 std::cout<<"{\"success\":true,\"anchors\":23,\"normal_pairs\":21,\"entry_only\":2,\"control_array_classes\":4,\"callbacks\":2,\"worlds\":2,\"world_changes\":1,\"LastError_preserved\":true}\n";
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}
