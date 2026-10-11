// Independent status sink: API side effects and exact byte records are observed.
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
static u32 last_error,opens,closes,writes,module_checks,position;
static bool enabled=true,loaded=false;
static unsigned fault;
static std::array<u32,24> sink;
static u32 GetLastError(){return last_error;}
static void SetLastError(u32 value){last_error=value;}
static void ExitProcess(u32 value){throw Termination{value};}
static void* GetModuleHandleA(const char* name){assert(!std::strcmp(name,"ddraw.dll"));++module_checks;last_error=111;return loaded?reinterpret_cast<void*>(2):nullptr;}
u32 GetEnvironmentVariableA(const char*,char* out,u32 size){last_error=112;if(fault==1)return size;if(!enabled){*out=0;return 0;}std::strcpy(out,"status.bin");return 10;}
static HANDLE CreateFileA(const char* path,u32 access,u32 sharing,void*,u32 creation,u32 flags,HANDLE){assert(!std::strcmp(path,"status.bin")&&access==0x40000000&&sharing==3&&creation==2&&flags==0x80);++opens;last_error=113;return fault==2?reinterpret_cast<void*>(-1):reinterpret_cast<void*>(1);}
u32 SetFilePointer(HANDLE file,i32 offset,i32* high,u32 origin){assert(file==reinterpret_cast<void*>(1)&&offset==0&&!high&&!origin);last_error=114;position=0;return fault==3?~0u:0;}
static int WriteFile(HANDLE file,const void* data,u32 size,u32* written,void* async){assert(file==reinterpret_cast<void*>(1)&&size==96&&position==0&&!async);last_error=115;++writes;std::memcpy(sink.data(),data,96);position=size;*written=fault==5?95:96;return fault!=4;}
static int CloseHandle(HANDLE file){assert(file==reinterpret_cast<void*>(1));++closes;last_error=116;return 1;}
#include "../runtime/native-draw/status.h"
static void reset(){nd_objects=nd_surfaces=nd_creates=nd_attempts=nd_enumerations=nd_last_create=0;ns_bytes=ns_dcs=nc_objects=ns_creates=ns_locks=ns_copies=ns_flips=0;nd_status_phase=nd_status_exports=nd_peak_surfaces=nd_peak_bytes=nd_peak_dcs=nd_peak_clippers=0;nd_status_file=nullptr;nd_status_busy=0;std::memset(nd_status_path,0,sizeof nd_status_path);last_error=opens=closes=writes=module_checks=position=0;enabled=true;loaded=false;fault=0;sink.fill(0);}
static void expect_exit(u32 code){try{nd_status_start();assert(false);}catch(Termination x){assert(x.code==code);}}
int main(){
 reset();nd_status_start();assert(opens==1&&closes==0&&writes==1);
 const std::array<u32,24> initial={0x444d4e4d,0x31305752,2,96,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0};assert(sink==initial);
 // 1024 independent producer snapshots: one session open, no close or growth.
 for(u32 i=1;i<=1024;++i){nd_objects=2;nd_surfaces=i%7;ns_bytes=(i%7)*64;ns_dcs=i%3;nc_objects=i%4;nd_creates=nd_attempts=2;ns_creates=7;ns_locks=i;ns_copies=i*2;ns_flips=i/3;nd_enumerations=1;last_error=0x12345678;nd_status();assert(last_error==0x12345678);assert(opens==1&&closes==0&&position==96&&writes==i+1&&module_checks==i+1);
  const std::array<u32,24> expected={0x444d4e4d,0x31305752,2,96,0,2,i%7,i%3,i%4,2,7,i,i*2,i/3,i+1,i>=6?6:i,i>=6?384:i*64,i>=2?2:i,i>=3?3:i,0,2,1,0,0};assert(sink==expected);
 }
 nd_status_busy=1;auto old=sink;last_error=0x89abcdef;nd_status();assert(sink==old&&last_error==0x89abcdef&&module_checks==1026&&writes==1025);nd_status_busy=0;
 nd_status_finish();assert(closes==1&&!nd_status_file);nd_status_finish();assert(closes==1);
 reset();enabled=false;nd_status_start();last_error=42;nd_status();assert(last_error==42&&opens==0&&closes==0&&writes==0&&module_checks==2);
 for(unsigned f=1;f<=5;++f){reset();fault=f;expect_exit(f==1?102:100);}
 reset();loaded=true;expect_exit(101);assert(sink[4]==1);
 reset();nd_status_start();nd_status_busy=1;loaded=true;try{nd_status();assert(false);}catch(Termination x){assert(x.code==101);}assert(writes==1);
 std::cout<<"status sink: 1024 complete overwrites, every-call module checks, LastError, collision and terminal failures PASS\n";
}
