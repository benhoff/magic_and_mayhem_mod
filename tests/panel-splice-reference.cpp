#define main sprite_reference_unused_main
#define u32 reference_word
#include "sprite-binary-reference.cpp"
#undef u32
#undef main
#include "../runtime/canvas-writers/panel_route.h"
extern "C" {
#include "../runtime/shadow/win32_min.h"
}
#include <array>
#include <sstream>
#include <algorithm>
#include <unistd.h>

extern "C" {
alignas(16) unsigned char panel_host_fx[512], panel_seed_fx[512], panel_after_fx[512];
std::uint32_t panel_saved_sp, panel_seed_flags, panel_after_flags;
std::uint32_t panel_kind, panel_function, panel_object, panel_args[4];
std::uint32_t panel_gpr[8], panel_unlock_gpr[3], panel_unlock_args[2], panel_unlocks;
void mnm_panel_probe();
void mnm_panel_test_unlock();
int __attribute__((stdcall)) PanelWritersInstallForTest();
static unsigned lastError = 1234, threadId = 1;
static const char *selection = "1";
struct Region {std::uintptr_t begin, end; unsigned protect;};
static Region region(const void *p) {
    std::ifstream maps("/proc/self/maps"); std::string line;
    const auto at = reinterpret_cast<std::uintptr_t>(p);
    while (std::getline(maps,line)) {
        std::istringstream row(line); std::string range, permissions;
        row >> range >> permissions; const auto dash=range.find('-');
        const auto begin=std::stoul(range.substr(0,dash),nullptr,16), end=std::stoul(range.substr(dash+1),nullptr,16);
        if (at>=begin && at<end) return {begin,end,permissions[0]!='r'?1u:permissions[1]=='w'?4u:2u};
    }
    return {};
}
u32 WIN VirtualQuery(const void *p,void *output,u32 size) {
    if(size!=28)return 0;const auto v=region(p);if(!v.end)return 0;
    auto *m=static_cast<u32 *>(output);std::fill_n(m,7,0);
    m[0]=v.begin;m[3]=v.end-v.begin;m[4]=0x1000;m[5]=v.protect;lastError=999;return 28;
}
u32 WIN GetLastError(){return lastError;}
void WIN SetLastError(u32 v){lastError=v;}
u32 WIN GetCurrentThreadId(){lastError=998;return threadId;}
void *WIN GetModuleHandleA(const char *){return reinterpret_cast<void *>(0x400000);}
static int WIN panelCurrentHistory(const void*,u32,u32,u32,u32,u32){return 0;}
static void WIN panelCountProducer(u32,u32,u32){}
void* WIN GetProcAddress(void*,const char* name){if(!std::strcmp(name,"NativeDrawCpuHistoryV1"))return reinterpret_cast<void*>(panelCurrentHistory);if(!std::strcmp(name,"NativeDrawProducerCountV1"))return reinterpret_cast<void*>(panelCountProducer);throw std::runtime_error("Unexpected panel helper");}
u32 WIN GetEnvironmentVariableA(const char *,char *out,u32 size) {
    if(!selection)return 0;const auto n=std::strlen(selection);if(n>=size)return n+1;
    std::memcpy(out,selection,n+1);return n;
}
void *WIN VirtualAlloc(void *,u32 n,u32,u32){auto *p=mmap(nullptr,n,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);return p==MAP_FAILED?nullptr:p;}
int WIN VirtualProtect(void *p,u32 n,u32 protection,u32 *old) {
    *old=region(p).protect;const auto begin=reinterpret_cast<std::uintptr_t>(p)&~4095u;
    const auto end=(reinterpret_cast<std::uintptr_t>(p)+n+4095u)&~4095u;
    return mprotect(reinterpret_cast<void *>(begin),end-begin,
        protection==0x40?PROT_READ|PROT_WRITE|PROT_EXEC:protection==0x20?PROT_READ|PROT_EXEC:protection==4?PROT_READ|PROT_WRITE:PROT_READ)==0;
}
int WIN FlushInstructionCache(HANDLE,const void *,u32){return 1;}
HANDLE WIN GetCurrentProcess(){return nullptr;}
void WIN ExitProcess(u32 code){_exit(code);}
}
struct State {
    std::array<std::uint32_t,8> gpr;
    std::array<std::uint32_t,3> unlockGpr;
    std::array<std::uint32_t,2> args;
    std::array<unsigned char,512> fx;
    unsigned flags,unlocks,error;
};
static State call() {
    panel_unlocks=0;lastError=1234;mnm_panel_probe();State s{};
    std::copy_n(panel_gpr,8,s.gpr.begin());std::copy_n(panel_unlock_gpr,3,s.unlockGpr.begin());
    std::copy_n(panel_unlock_args,2,s.args.begin());std::copy_n(panel_after_fx,512,s.fx.begin());
    s.flags=panel_after_flags;s.unlocks=panel_unlocks;s.error=lastError;return s;
}
int main(int argc,char **argv) try {
    if(argc!=3)throw std::runtime_error("Expected pinned original PE and JSON result");
    map_image(read(argv[1]));
    const unsigned addresses[3]={0x58de5a,0x58d92c,0x58e32c},lengths[3]={10,8,10},functions[3]={0x58ddf0,0x58d8c0,0x58e2c0};
    unsigned char original[3][10]={},patched[3][10]={};
    for(unsigned k=0;k<3;++k)std::memcpy(original[k],reinterpret_cast<void *>(addresses[k]),lengths[k]);
    selection="11";if(PanelWritersInstallForTest())throw std::runtime_error("Invalid mode admitted");
    selection="1";
    auto *bad=reinterpret_cast<unsigned char *>(addresses[2]);const auto saved=bad[0];bad[0]^=1;
    if(PanelWritersInstallForTest())throw std::runtime_error("Invalid last signature admitted");
    for(unsigned k=0;k<2;++k)if(std::memcmp(original[k],reinterpret_cast<void *>(addresses[k]),lengths[k]))throw std::runtime_error("Early mutation on late signature refusal");
    bad[0]=saved;
    if(!PanelWritersInstallForTest()||!PanelWritersInstallForTest())throw std::runtime_error("Install/idempotence failed");
    if(mprotect(reinterpret_cast<void *>(0x58d000),8192,PROT_READ|PROT_WRITE|PROT_EXEC))throw std::runtime_error("Fixture patch permission");
    for(unsigned k=0;k<3;++k)std::memcpy(patched[k],reinterpret_cast<void *>(addresses[k]),lengths[k]);
    auto *lock=reinterpret_cast<unsigned char *>(0x58b660);
    if(std::memcmp(lock,"\x83\xec\x08\x33\xc0",5))throw std::runtime_error("Lock anchor");
    lock[0]=0xb8;lock[5]=0xc3;
    std::uint32_t table[33]={},iface[1]={},surface[12]={};
    table[32]=reinterpret_cast<std::uintptr_t>(mnm_panel_test_unlock);
    iface[0]=reinterpret_cast<std::uintptr_t>(table);surface[1]=reinterpret_cast<std::uintptr_t>(iface);
    panel_object=reinterpret_cast<std::uintptr_t>(surface);
    asm volatile("fxsave %0":"=m"(panel_host_fx));
    asm volatile("fninit; fld1; fldpi; fxsave %0":"=m"(panel_seed_fx));
    asm volatile("fxrstor %0"::"m"(panel_host_fx));
    for(unsigned i=160;i<288;++i)panel_seed_fx[i]=static_cast<unsigned char>(i*37);
    unsigned cases=0,refusals=0;std::uint64_t compared=0;
    for(unsigned format=0;format<2;++format)for(unsigned kind=0;kind<3;++kind)
    for(unsigned stride:{16u,19u}) {
        surface[4]=16;surface[5]=13;surface[6]=stride*2;
        global(0x6e1f88,format);
        *reinterpret_cast<std::uint16_t *>(0x6e1f7c)=format?0x7c00:0xf800;
        *reinterpret_cast<std::uint16_t *>(0x6e1f7e)=format?0x3e0:0x7e0;
        *reinterpret_cast<std::uint16_t *>(0x6e1f80)=31;
        panel_kind=kind;panel_function=functions[kind];
        const std::vector<std::array<int,4>> rectangles={{0,0,0,0},{0,0,1,1},{3,4,4,5},{1,1,12,9},{0,0,14,11},
            kind==2?std::array<int,4>{0,0,16,13}:std::array<int,4>{0,0,14,11}};
        for(const auto &rect:rectangles)for(unsigned percent:{0u,1u,40u,99u,100u,101u,1000u})
        for(unsigned booleans=0;booleans<4;++booleans)for(unsigned origin:{0u,7u}) {
            if(kind!=0 && (percent || booleans || origin))continue;
            surface[8]=origin;surface[9]=origin?3:0;
            auto wrapperRect=rect;if(kind==0){wrapperRect[0]+=surface[8];wrapperRect[2]+=surface[8];wrapperRect[1]+=surface[9];wrapperRect[3]+=surface[9];}
            panel_args[0]=reinterpret_cast<std::uintptr_t>(wrapperRect.data());panel_args[1]=percent;
            panel_args[2]=booleans&1;panel_args[3]=booleans>>1;
            for(unsigned pattern=0;pattern<8;++pattern)for(unsigned df=0;df<2;++df) {
                std::vector<std::uint16_t> seed(stride*13+64,0xb37d);
                for(unsigned i=0;i<stride*13;++i)seed[i+32]=pattern<6?std::array<unsigned,6>{0,65535,0xf800,0x7e0,31,0x39e7}[pattern]:(i*1273+cases*271)&65535;
                auto plane=seed;const auto pointer=std::uint32_t(reinterpret_cast<std::uintptr_t>(plane.data()+32));
                std::memcpy(lock+1,&pointer,4);panel_seed_flags=0x202u|(df?0x400u:0)|0x895u;
                std::memcpy(reinterpret_cast<void *>(addresses[kind]),original[kind],lengths[kind]);
                const auto baseline=call();const auto expected=plane;
                plane=seed;std::memcpy(reinterpret_cast<void *>(addresses[kind]),patched[kind],lengths[kind]);
                const auto count=mnm_panel_native[kind];const auto native=call();
                if(plane!=expected||baseline.gpr!=native.gpr||baseline.fx!=native.fx||baseline.args!=native.args||
                   baseline.unlockGpr!=native.unlockGpr||((baseline.flags^native.flags)&0xcd5u)||
                   baseline.unlocks!=1||native.unlocks!=1||native.error!=1234||mnm_panel_native[kind]!=count+1)
                    throw std::runtime_error("Panel splice mismatch kind="+std::to_string(kind)+" case="+std::to_string(cases));
                for(unsigned i=0;i<32;++i)if(plane[i]!=0xb37d||plane[stride*13+32+i]!=0xb37d)throw std::runtime_error("Plane guards");
                ++cases;compared+=stride*13;
                /* Different producer thread must forward once, with original
                 * wrapper output, return and Lock/Unlock balance intact. */
                if(pattern==0 && !df){threadId=2;plane=seed;const auto forwards=mnm_panel_forwarded[kind];
                    const auto fallback=call();threadId=1;
                    if(plane!=expected||fallback.gpr!=baseline.gpr||fallback.fx!=baseline.fx||
                       fallback.args!=baseline.args||fallback.unlocks!=1||fallback.error!=1234||
                       mnm_panel_forwarded[kind]!=forwards+1||mnm_panel_native[kind]!=count+1)
                        throw std::runtime_error("Original-once thread refusal");
                    ++refusals;
                }
            }
        }
    }
    std::ofstream out(argv[2]);out<<"{\"success\":true,\"cases\":"<<cases<<",\"physical_words\":"<<compared
        <<",\"original_once_refusals\":"<<refusals<<",\"late_signature_atomic\":true,\"formats\":2,\"gpr_fx_flags_unlock_error_equal\":true}\n";
    if(!out)throw std::runtime_error("Output write failed");return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
