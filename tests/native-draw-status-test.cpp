// Independent mapped sink: admitted drawing calls may only touch owned memory.
#include <array>
#include <cassert>
#include <cstring>
#include <iostream>
using u32=unsigned;using i32=int;using HANDLE=void*;
#define API
#define WIN
static u32 nd_objects,nd_surfaces,nd_creates,nd_attempts,nd_enumerations,nd_last_create;
static u32 ns_bytes,ns_dcs,nc_objects,ns_creates,ns_locks,ns_copies,ns_flips;
static volatile u32 nd_status_phase;
struct Termination{u32 code;};
static u32 last_error,opens,closes,mappings,views,flushes,unmaps,module_checks;
static bool enabled=true,loaded=false;
static unsigned fault;
static std::array<u32,26> guarded;
static u32* sink(){return guarded.data()+1;}
static u32 GetLastError(){return last_error;}
static void SetLastError(u32 value){last_error=value;}
static void ExitProcess(u32 value){throw Termination{value};}
static void* GetModuleHandleA(const char* name){assert(!std::strcmp(name,"ddraw.dll"));++module_checks;last_error=111;return loaded?reinterpret_cast<void*>(2):nullptr;}
u32 GetEnvironmentVariableA(const char*,char* out,u32 size){last_error=112;if(fault==1)return size;if(!enabled){*out=0;return 0;}std::strcpy(out,"status.bin");return 10;}
static HANDLE CreateFileA(const char* path,u32 access,u32 sharing,void*,u32 creation,u32 flags,HANDLE){assert(!std::strcmp(path,"status.bin")&&access==0xc0000000&&sharing==3&&creation==2&&flags==0x80);++opens;last_error=113;return fault==2?reinterpret_cast<void*>(-1):reinterpret_cast<void*>(1);}
HANDLE CreateFileMappingA(HANDLE file,void* security,u32 protection,u32 high,u32 low,const char* name){assert(file==reinterpret_cast<void*>(1)&&!security&&protection==4&&!high&&low==96&&!name);++mappings;last_error=114;return fault==3?nullptr:reinterpret_cast<void*>(3);}
void* MapViewOfFile(HANDLE mapping,u32 access,u32 high,u32 low,u32 bytes){assert(mapping==reinterpret_cast<void*>(3)&&access==2&&!high&&!low&&bytes==96);++views;last_error=115;return fault==4?nullptr:sink();}
i32 FlushViewOfFile(const void* view,u32 bytes){assert(view==sink()&&bytes==96);++flushes;last_error=116;return fault!=5;}
i32 UnmapViewOfFile(const void* view){assert(view==sink());++unmaps;last_error=117;return fault!=6;}
static int CloseHandle(HANDLE file){assert(file==reinterpret_cast<void*>(1)||file==reinterpret_cast<void*>(3));++closes;last_error=118;return 1;}
// WriteFile and SetFilePointer deliberately have no callable fixture definitions.
#include "../runtime/native-draw/status.h"
static void reset(){nd_objects=nd_surfaces=nd_creates=nd_attempts=nd_enumerations=nd_last_create=0;ns_bytes=ns_dcs=nc_objects=ns_creates=ns_locks=ns_copies=ns_flips=0;nd_status_phase=nd_status_exports=nd_peak_surfaces=nd_peak_bytes=nd_peak_dcs=nd_peak_clippers=0;nd_status_file=nd_status_mapping=nullptr;nd_status_view=nullptr;nd_status_busy=0;std::memset(nd_status_path,0,sizeof nd_status_path);last_error=opens=closes=mappings=views=flushes=unmaps=module_checks=0;enabled=true;loaded=false;fault=0;guarded.fill(0);guarded.front()=guarded.back()=0xa5a5a5a5;}
static void check(const std::array<u32,24>& expected){assert(std::memcmp(sink(),expected.data(),96)==0);assert(guarded.front()==0xa5a5a5a5&&guarded.back()==0xa5a5a5a5);}
int main(){
 reset();nd_status_start();assert(opens==1&&mappings==1&&views==1&&closes==0&&flushes==0);
 const std::array<u32,24> initial={0x444d4e4d,0x31305752,2,96,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0};check(initial);
 for(u32 i=1;i<=1024;++i){nd_objects=2;nd_surfaces=i%7;ns_bytes=(i%7)*64;ns_dcs=i%3;nc_objects=i%4;nd_creates=nd_attempts=2;ns_creates=7;ns_locks=i;ns_copies=i*2;ns_flips=i/3;nd_enumerations=1;last_error=0x12345678;nd_status();assert(last_error==0x12345678);assert(opens==1&&mappings==1&&views==1&&closes==0&&flushes==0&&unmaps==0&&module_checks==i+1);
  const std::array<u32,24> expected={0x444d4e4d,0x31305752,2,96,0,2,i%7,i%3,i%4,2,7,i,i*2,i/3,i+1,i>=6?6:i,i>=6?384:i*64,i>=2?2:i,i>=3?3:i,0,2,1,0,0};check(expected);
 }
 nd_status_busy=1;auto old=guarded;last_error=0x89abcdef;nd_status();assert(guarded==old&&last_error==0x89abcdef&&module_checks==1026&&nd_status_exports==1025);nd_status_busy=0;
 nd_status_finish();assert(flushes==1&&unmaps==1&&closes==2&&!nd_status_file&&!nd_status_mapping&&!nd_status_view);nd_status_finish();assert(flushes==1&&unmaps==1&&closes==2);
 reset();enabled=false;nd_status_start();last_error=42;nd_status();nd_status_finish();assert(last_error==42&&opens==0&&mappings==0&&views==0&&closes==0&&flushes==0&&unmaps==0&&module_checks==2);
 for(unsigned f=1;f<=6;++f){reset();fault=f;try{nd_status_start();nd_status_finish();assert(false);}catch(Termination x){assert(x.code==(f==1?102u:100u));}}
 reset();loaded=true;try{nd_status_start();assert(false);}catch(Termination x){assert(x.code==101);}assert(sink()[4]==1);
 reset();nd_status_start();nd_status_busy=1;loaded=true;try{nd_status();assert(false);}catch(Termination x){assert(x.code==101);}assert(nd_status_exports==1);
 std::cout<<"mapped status: 1024 complete bounded memory snapshots, zero drawing file IO, every-call module checks, LastError, collision and terminal failures PASS\n";
}
