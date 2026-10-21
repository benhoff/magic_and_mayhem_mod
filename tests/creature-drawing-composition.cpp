// Unchanged original helper execution, owned native composition, then the same
// producer-facing PE32 entry points with native installation and poisoned tails.
#include "../reconstruction/rendering/creature_drawing.h"
#include "../runtime/creature-drawing/original_bytes.h"
#define main unused_creature_sprite_main
#include "sprite-binary-reference.cpp"
#undef main
#include <array>
#include <algorithm>
#include <cstdlib>

using U=std::uint32_t;
extern "C" int mnm_creature_draw_install();
extern "C" int creature_queue_call(void*,U);
static U ptr(const void* p){return U(reinterpret_cast<std::uintptr_t>(p));}
static U lastError=0x12345678;
static U protects=0,protectFail=0,flushes=0;
static std::array<unsigned char,0x5000> arena;
static constexpr U poolOffset=0x100,rendererOffset=0x3b00,envelopeOffset=0x4000;
static void put(U p,U value){std::memcpy(arena.data()+p,&value,4);}
static bool span(U p,U n){return p>=ptr(arena.data()) && n<=arena.size() && p-ptr(arena.data())<=arena.size()-n;}
static int readOwned(void*,U p,void* target,U n){if(!span(p,n))return 0;std::memcpy(target,reinterpret_cast<void*>(p),n);return 1;}
static int writeOwned(void*,U p,const void* source,U n){if(!span(p,n))return 0;std::memcpy(reinterpret_cast<void*>(p),source,n);return 1;}
static MnmCreatureDrawMemory memory{nullptr,readOwned,writeOwned};
static void require(bool ok,const char* reason){if(!ok)throw std::runtime_error(reason);}

extern "C" {
U __attribute__((stdcall)) GetLastError(){return lastError;}
void __attribute__((stdcall)) SetLastError(U value){lastError=value;}
void* __attribute__((stdcall)) GetModuleHandleA(const char*){return reinterpret_cast<void*>(0x400000);}
void* __attribute__((stdcall)) GetCurrentProcess(){return nullptr;}
int __attribute__((stdcall)) VirtualProtect(void*,U,U,U* old){
    ++protects;lastError=0xdeadbeef;*old=0x40;return !protectFail||protects!=protectFail;
}
int __attribute__((stdcall)) FlushInstructionCache(void*,const void*,U){++flushes;return 1;}
U __attribute__((stdcall)) VirtualQuery(const void* p,void* output,U size){
    if(size!=28)return 0;
    const U address=ptr(p);
    auto* r=static_cast<U*>(output);
    U start=0,length=0;
    if(span(address,1)){start=ptr(arena.data());length=arena.size();}
    else if(address>=0x400000&&address<0x700000){start=0x400000;length=0x300000;}
    else if(U(address-ptr(&address)+0x100000u)<0x200000u){start=address&~4095u;length=4096;}
    else return 0;
    const U region[]{start,start,0x40,length,0x1000,0x40,0x20000};
    std::memcpy(r,region,28);lastError=0xfeedbeef;return 28;
}
void __attribute__((stdcall,noreturn)) ExitProcess(U code){std::cerr<<"Terminal native refusal "<<std::hex<<code<<'\n';std::abort();}
}

static void setup(U id){
    // Four packed, deliberately unaligned original creature records.
    for(U i=0;i<arena.size();++i)arena[i]=static_cast<unsigned char>((i*37+id*11)^0xa5);
    for(U i=0;i<4;++i){
        const U c=poolOffset+i*0xe4b;
        put(c+4,(id>>(i+2))&1);put(c+0xe4,(id>>(i+4))&1);
        put(c+0x6e4,i==2?0xffffffffu:id+i);
        put(c+0x68,(id>>1)&1);put(c+0x64c,(id%5==4)?0xffffffffu:(id+i)%4);
        constexpr U actions[]{0,3,9,10,11,12,0xffffffffu};
        put(c+0x5ec,actions[(id+i)%7]);
        constexpr U subtypes[]{0,1,5,6,0xffffffffu};
        put(c+0x684,subtypes[(id/7+i)%5]);
        arena[c+0x723]=static_cast<unsigned char>((id/35+i)%2);
        put(c+0xa8,(id/70+i)%2?18:17);put(c+0x108,(id/140+i)%2);
        put(c+0xac,ptr(arena.data()+0x4300+i*64));
        constexpr U flags[]{0,1,0x80000000,0xffffffffu};
        put(0x4300+i*64+0x34,flags[(id/280+i)%4]);
        // Descriptor aliasing must observe the post-shift, post-X/Y state.
        const U descriptors[]{0x4500,c+0x750,c+0x754,c+0x75c,c+0x768};
        put(c+0xb4,ptr(arena.data()+descriptors[(id+i)%5]));
    }
    put(envelopeOffset,ptr(arena.data()+poolOffset));put(envelopeOffset+0x228,id%5);
    put(rendererOffset+0x229,ptr(arena.data()+0x4800));
    put(rendererOffset+0x22d,ptr(arena.data()+0x4800)+(id%4)*36);
    put(rendererOffset+0x231,id%4);
    global(0x6def58,ptr(arena.data()+poolOffset));global(0x6def5c,4);
}

using Prepare=void(__attribute__((thiscall)) *)(void*);
using History=void(__attribute__((thiscall)) *)(void*,U,U);
struct LifecycleResult {
    std::array<std::array<unsigned char,0x5000>,6> phases;
    bool operator==(const LifecycleResult& other)const{return phases==other.phases;}
};
static LifecycleResult execute(U id,bool native){
    LifecycleResult result;
    const U address=ptr(arena.data());
    for(U i=0;i<4;++i){
        const U c=poolOffset+i*0xe4b;
        // Original outer producer's selected-creature clear precedes helper.
        put(c+0x74,0);
        if(native)require(mnm_creature_draw_prepare(memory,address+c,address+poolOffset,4),"Native preparation");
        else reinterpret_cast<Prepare>(0x51e260)(arena.data()+c);
    }
    result.phases[0]=arena;
    for(U frame=0;frame<3;++frame){
        for(U i=0;i<4;++i){
            const U c=poolOffset+i*0xe4b,x=(id^0x80000000u)+frame*19+i,y=0xffffffffu-id-frame*23-i;
            if(native)require(mnm_creature_draw_history(memory,address+c,x,y),"Native history");
            else reinterpret_cast<History>(0x51fa60)(arena.data()+c,x,y);
        }
        result.phases[frame+1]=arena;
    }
    if(native)require(mnm_scene_queue_retire(memory,address+rendererOffset),"Native retirement");
    else require(creature_queue_call(arena.data()+rendererOffset,0x4fd5ad),"Queue span register/flag ABI");
    result.phases[4]=arena;
    // In the actual producer this phase follows consumption and queue cleanup.
    if(native)require(mnm_creature_draw_finish(memory,address+envelopeOffset),"Native finish");
    else reinterpret_cast<Prepare>(0x52ab80)(arena.data()+envelopeOffset);
    result.phases[5]=arena;
    return result;
}

int main(int argc,char** argv)try{
    require(argc==2,"Expected pinned PE32 input");map_image(read(argv[1]));
    // A RET at the explicit post-cleanup external boundary ends the isolated
    // queue span. None of its twenty compared instructions are changed.
    *reinterpret_cast<unsigned char*>(0x4fd5c1)=0xc3;
    constexpr U cases=2240;
    std::vector<LifecycleResult> expected;
    expected.reserve(cases);
    for(U id=0;id<cases;++id){
        setup(id);expected.push_back(execute(id,false));
        setup(id);require(execute(id,true)==expected.back(),"Original/native lifecycle phase bytes differ");
    }
    // Original finish retains the backing pointer but rereads the high-water
    // count. Aliasing its own writes makes both orderings observable.
    for(U mode=0;mode<2;++mode){
        auto configure=[&]{
            setup(0);
            const U e=poolOffset+0x6e4-(mode?0u:0x228u);
            put(e,ptr(arena.data()+poolOffset));put(e+0x228,3);
            for(U i=0;i<4;++i)put(poolOffset+i*0xe4b+4,1);
            return e;
        };
        const U e=configure();
        reinterpret_cast<Prepare>(0x52ab80)(arena.data()+e);
        const auto original=arena;
        configure();require(mnm_creature_draw_finish(memory,ptr(arena.data()+e)),"Mutable finish refused");
        require(arena==original,"Cached backing/reloaded count differs");
    }
    // Native ownership refusals: malformed target, null definition, descriptor,
    // overflowing owner address, and inactive preservation are explicit.
    setup(0);put(poolOffset+0x5ec,3);put(poolOffset+0x68,1);put(poolOffset+0x64c,4);
    const auto before=arena;
    require(!mnm_creature_draw_prepare(memory,ptr(arena.data()+poolOffset),ptr(arena.data()+poolOffset),4),"Out-of-range target accepted");
    require(arena==before,"Refused target mutated owner");
    require(!mnm_creature_draw_history(memory,0xfffffff0u,1,2),"Wrapped history accepted");
    setup(0);put(poolOffset+0x5ec,0);arena[poolOffset+0x723]=1;
    put(poolOffset+0xa8,17);put(poolOffset+0xac,0);
    require(!mnm_creature_draw_prepare(memory,ptr(arena.data()+poolOffset),ptr(arena.data()+poolOffset),4),"Null definition accepted");
    setup(0);put(poolOffset+0xb4,0);
    require(!mnm_creature_draw_history(memory,ptr(arena.data()+poolOffset),1,2),"Null descriptor accepted");
    // Every byte of every original body participates in transactional install
    // refusal, with no earlier site changed when a later guard is wrong.
    U guardCases=0;
    for(const auto& guard:mnm_creature_original){
        for(U i=0;i<guard.size;++i){
            auto* p=reinterpret_cast<unsigned char*>(guard.address+i);*p^=1;
            require(!mnm_creature_draw_install(),"Wrong original byte accepted");*p^=1;++guardCases;
            for(const auto& other:mnm_creature_original)
                require(!std::memcmp(reinterpret_cast<void*>(other.address),other.bytes,other.size),"Partial installation after refusal");
        }
    }
    for(U failure=1;failure<=4;++failure){
        protects=0;protectFail=failure;
        require(!mnm_creature_draw_install(),"Protection refusal accepted");
        for(const auto& guard:mnm_creature_original)
            require(!std::memcmp(reinterpret_cast<void*>(guard.address),guard.bytes,guard.size),"Instruction changed on failed preflight");
    }
    protectFail=0;require(mnm_creature_draw_install(),"Native install refused");
    require(flushes==4,"Missing instruction cache publication");
    for(const auto& guard:mnm_creature_original){
        require(*reinterpret_cast<unsigned char*>(guard.address)==0xe9,"Missing native entry jump");
        std::memset(reinterpret_cast<void*>(guard.address+8),0xcc,guard.size-8);
    }
    *reinterpret_cast<unsigned char*>(0x59cb60)=0xcc;
    // Execute through the original producer-facing entries. No original body
    // can execute after its entry jump; x87 state and LastError survive.
    for(U id=0;id<cases;++id){
        setup(id);lastError=0x12345678;
        asm volatile("fninit; fld1");
        const auto actual=execute(id,false);
        double value;asm volatile("fstpl %0":"=m"(value));
        require(value==1.0,"Floating state changed");
        require(lastError==0x12345678,"LastError changed");
        require(actual==expected[id],"Runtime/original lifecycle phase bytes differ");
    }
    std::cout<<"{\"success\":true,\"cases\":"<<cases
        <<",\"frames_per_case\":3,\"guard_mutations\":"<<guardCases
        <<",\"protection_refusals\":4,\"mutable_finish_cases\":2,\"ownership_refusals\":4,\"full_owner_bytes_per_case\":"<<sizeof(LifecycleResult)
        <<",\"native_and_runtime_match_original\":true,\"original_tails_poisoned\":true}\n";
    return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
