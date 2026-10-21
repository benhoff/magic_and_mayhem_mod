#include "../reconstruction/rendering/actor_lifecycle.hpp"
#define main unused_sprite_reference
#include "sprite-binary-reference.cpp"
#undef main
#include "original-linux-seh.hpp"
#include "../runtime/actor-lifecycle/original_bytes.h"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <map>

using U=unsigned;
extern "C" int mnm_actor_install();
static U protectCalls,protectFailure,queryFailure,lastError=0x12345678;
extern "C" U __attribute__((stdcall)) GetLastError(){return lastError;}
extern "C" void __attribute__((stdcall)) SetLastError(U value){lastError=value;}
extern "C" U __attribute__((stdcall)) VirtualQuery(const void* p,void* out,U size){
    if(U(p)==queryFailure)return 0;
    U* fields=static_cast<U*>(out);for(U i=0;i<7;++i)fields[i]=0;
    fields[0]=U(p)&~4095u;fields[3]=4096;fields[4]=0x1000;fields[5]=4;return size;
}
extern "C" int __attribute__((stdcall)) VirtualProtect(void*,U,U,U* old){*old=0x20;return ++protectCalls!=protectFailure;}
extern "C" void* __attribute__((stdcall)) GetCurrentProcess(){return reinterpret_cast<void*>(~0u);}
extern "C" int __attribute__((stdcall)) FlushInstructionCache(void*,const void*,U){return 1;}
extern "C" [[noreturn]] void __attribute__((stdcall)) ExitProcess(U code){std::cerr<<"Actor adapter exit "<<std::hex<<code<<"\n";_exit(82);}
static constexpr U arenaBase=0x20000000,manager=0x6def58,stride=0xe4b;
static void emit(const void* p,U size){std::cout.write(static_cast<const char*>(p),size);}
static void word(U value){emit(&value,4);}
struct Backend {
    struct Block{U address,size;bool alive;};
    std::vector<Block> blocks;
    U cursor{},calls{},failure{},actorBase{},scenario{};
    bool trace{},native{};
    U r(U p){U v;std::memcpy(&v,reinterpret_cast<void*>(p),4);return v;}
    void w(U p,U value){std::memcpy(reinterpret_cast<void*>(p),&value,4);}
    unsigned char r8(U p){return *reinterpret_cast<unsigned char*>(p);}
    void w8(U p,unsigned char value){*reinterpret_cast<unsigned char*>(p)=value;}
    [[noreturn]] void fault(const char* reason){throw std::runtime_error(reason);}
    void loadError(){word(0xeeee0001);}
    void state(){
        const U globals[]={0x6df180,0x6e9848,0x6e984c,0x6e9850,0x6903e8,
            0x6e9854,0x644524,0x6e007c,0x6a46bc,0x6c502f};
        for(U g:globals)word(r(g));
        word(r8(0x690618));word(r8(0x644568));
        emit(reinterpret_cast<void*>(manager),0x268);
        emit(reinterpret_cast<void*>(0x6e9218),1008);
        emit(reinterpret_cast<void*>(0x6de0e8),27*36);
        word(U(blocks.size()));
        for(auto block:blocks){word(block.address);word(block.size);word(block.alive);if(block.alive)emit(reinterpret_cast<void*>(block.address),block.size);}
    }
    void checkpoint(U kind){word(0xcafe0000|kind);state();}
    U allocate(U size){
        ++calls;if(calls==failure){word(0xa1100000);word(size);word(0);return 0;}
        U p=arenaBase+cursor;cursor+=(size+15)&~15u;
        if(cursor>0xf0000)fault("Fixture allocation budget");
        std::memset(reinterpret_cast<void*>(p),0xa5,size);blocks.push_back({p,size,true});
        word(0xa1100000);word(size);word(p);return p;
    }
    void free(U p){
        word(0xfee00000);word(p);if(!p)return;
        for(auto& block:blocks)if(block.address==p && block.alive){
            emit(reinterpret_cast<void*>(p),block.size);block.alive=false;return;
        }
        fault("Free without owned allocation");
    }
    void arrayFree(U p){free(p);}
    void seedList(U actor){
        const U list=r(actor+0x8f9),head=r(list+4);
        U previous=head;
        for(U i=0;i<2;++i){
            const U node=allocate(48);w(node,head);w(node+4,previous);
            w(previous,node);w(head+4,node);w(node+8,i?1:2);
            w(node+40,i?0:0x20106000);previous=node;
        }
        w(list+8,2);
    }
    U scratch(){return 0x20100000;}
    U service(U entry,U receiver,U n,const U* args){
        word(entry);word(receiver);word(n);
        for(U i=0;i<n;++i)word(entry==0x520790?0:args[i]);
        if(trace)state();
        switch(entry){
        case 0x5057b0:return 7;
        case 0x5055a0:return (scenario&1)?0:1;
        case 0x5075f0:return r(r(receiver+0xac)+24);
        case 0x466870:return (scenario&2)?actorBase:0;
        case 0x414890:return scenario&4;
        case 0x463cb0:return receiver+4;
        case 0x474b40:return 33;
        case 0x49bcf0:return receiver+16;
        case 0x49c530:return scenario&1;
        case 0x4943f0:w(receiver+4,0);return 0;
        case 0x58f750:w(receiver+0x1c,allocate(32));w(receiver+0x20,8);return 0;
        case 0x57d0d0:free(args[0]);return 0;
        default:return 0;
        }
    }
    template<class... A> U call(U entry,U receiver,A... args){
        const U values[]={U(args)...,0};
        switch(entry){
#define ACTOR_SERVICE(address,argc) case 0x##address:if(sizeof...(args)!=argc)fault("Wrong external service arity");return service(entry,receiver,sizeof...(args),values);
#include "actor-lifecycle-services.inc"
#undef ACTOR_SERVICE
        case 0x52cbe0:
            if(sizeof...(args)!=1)fault("Callback arity");
            w(receiver,0x5c5c40);free(receiver);return receiver;
        default:fault("Unknown external service");
        }
    }
    template<class... A>U stdcall(U entry,A... args){
        const U values[]={U(args)...};
        word(entry);word(sizeof...(args));for(U v:values)word(v);
        if(entry==0x20110000){
            if constexpr(sizeof...(args)==2){w(values[1],scenario&1);return 0;}
            else fault("Status call arity");
        }
        return scenario&2;
    }
};
static Backend* active;
extern "C" __attribute__((force_align_arg_pointer)) U actor_service_dispatch(U* regs,U entry,U count){
    try{return active->service(entry,regs[6],count,regs+9);}catch(...){_exit(80);}
}
extern "C" __attribute__((force_align_arg_pointer)) void* actor_alloc(U n){return reinterpret_cast<void*>(active->allocate(n));}
extern "C" __attribute__((force_align_arg_pointer)) void actor_free(void* p){active->free(U(p));}
extern "C" __attribute__((stdcall,force_align_arg_pointer)) U actor_status(U object,U out){
    // Normalize the original global status scratch to the native scratch token.
    const U result=active->stdcall(0x20110000,object,active->scratch());active->w(out,active->r(active->scratch()));return result;
}
extern "C" __attribute__((stdcall,force_align_arg_pointer)) U actor_dispose(U object){return active->stdcall(0x20110010,object);}
extern "C" __attribute__((stdcall,force_align_arg_pointer)) U actor_stop(U object,U flags){return active->stdcall(0x20110020,object,flags);}
static void patch(U address,void* destination){
    const U relative=U(destination)-address-5;
    *reinterpret_cast<unsigned char*>(address)=0xe9;
    std::memcpy(reinterpret_cast<void*>(address+1),&relative,4);
}
#define ACTOR_SERVICE(address,argc) extern "C" void actor_service_##address();
#include "actor-lifecycle-services.inc"
#undef ACTOR_SERVICE
template<class... A>static U original(U address,U receiver,A... args){
    using F=U(__attribute__((thiscall)) *)(U,A...);
    return reinterpret_cast<F>(address)(receiver,args...);
}
int main(int argc,char** argv)try{
    if(argc!=3)throw std::runtime_error("Expected original image and original/native mode");
    const std::string mode=argv[2];
    const bool native=mode=="native";
    map_image(read(argv[1]));OriginalLinuxSeh seh;
    if(mmap(reinterpret_cast<void*>(arenaBase),0x200000,PROT_READ|PROT_WRITE|PROT_EXEC,
            MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE,-1,0)==MAP_FAILED)throw std::runtime_error("Arena map");
    if(mode=="guards") {
        U mutations=0;
        for(const auto& guard:mnm_actor_original)for(U i=0;i<guard.size;++i){
            auto* byte=reinterpret_cast<unsigned char*>(guard.address+i);*byte^=1;
            if(mnm_actor_install())throw std::runtime_error("Admitted changed actor body");
            *byte^=1;++mutations;
        }
        for(U failure=1;failure<=13;++failure){protectCalls=0;protectFailure=failure;
            if(mnm_actor_install())throw std::runtime_error("Admitted protection refusal");
            for(const auto& guard:mnm_actor_original)if(std::memcmp(reinterpret_cast<void*>(guard.address),guard.bytes,guard.size))throw std::runtime_error("Partial adapter installation");
        }
        protectFailure=0;
        for(const auto& guard:mnm_actor_original){queryFailure=guard.address;
            if(mnm_actor_install())throw std::runtime_error("Admitted ownership refusal");
        }
        queryFailure=0;
        if(!mnm_actor_install())throw std::runtime_error("Adapter install failed");
        std::cout<<"{\"success\":true,\"body_mutations\":"<<mutations<<",\"protection_refusals\":13,\"ownership_refusals\":13}\n";return 0;
    }
    if(mode=="runtime" && !mnm_actor_install())throw std::runtime_error("Runtime installation");
    patch(0x597850,reinterpret_cast<void*>(&actor_alloc));patch(0x597870,reinterpret_cast<void*>(&actor_free));patch(0x5979b0,reinterpret_cast<void*>(&actor_free));
#define ACTOR_SERVICE(address,argc) patch(0x##address,reinterpret_cast<void*>(&actor_service_##address));
#include "actor-lifecycle-services.inc"
#undef ACTOR_SERVICE
    patch(0x20110000,reinterpret_cast<void*>(&actor_status));patch(0x20110010,reinterpret_cast<void*>(&actor_dispose));patch(0x20110020,reinterpret_cast<void*>(&actor_stop));
    if(native){
        const std::pair<U,U> ranges[]={{0x505db0,0x50621c},{0x506290,0x5070a8},
            {0x51f5e0,0x51fa5c},{0x52a5c0,0x52a661},{0x4bfef0,0x4c0077},{0x4c00d0,0x4c0191},{0x473100,0x4731ff},{0x52baf0,0x52bc30}};
        for(auto [begin,end]:ranges)std::memset(reinterpret_cast<void*>(begin),0xcc,end-begin);
    }
    U cases=0;
    for(U scenario=0;scenario<16;++scenario)for(U type:{0u,1u,13u,24u,25u,26u})for(U quiet:{0u,1u}){
        Backend backend;active=&backend;backend.scenario=scenario;backend.native=native;
        mnm::reconstruction::ActorLifecycle<Backend> life(backend);
        std::memset(reinterpret_cast<void*>(0x640000),0,0xc0000);
        std::memset(reinterpret_cast<void*>(manager),0xa5,0x268);
        backend.w(manager+0x228,0);
        backend.w(0x644520,(scenario&8)?1:0);backend.w(0x6c5494,17);backend.w(0x6c5498,19);
        backend.w(0x6c4830,101);backend.w(0x6f437c,0x6f0000);
        backend.w(0x6f0018,1);backend.w(0x6f001c,1);backend.w(0x689920,scenario%3);
        backend.w8(0x6dbe93,scenario&1);backend.w(0x6e1f0c,scenario&2);
        backend.w(0x6e9850,2);backend.w8(0x690618,0x77);backend.w8(0x644568,0x77);
        backend.w(0x643a80,1);backend.w(0x643a80+12,0);backend.w(0x643bd4,1);backend.w(0x643bd4+12,~0u);
        const U definition=0x6a5f80+type*0x5c9;
        backend.w(definition,type);backend.w(definition+8,scenario%3);backend.w(definition+12,7);
        backend.w(definition+20,22);backend.w(definition+24,120);backend.w(definition+40,19);
        backend.w(definition+0x3c,9);backend.w(definition+0x58,32);backend.w(definition+0x74,(scenario&8)?7:~0u);
        backend.w(definition+0x7c,~1u);backend.w(definition+0x8c,4);
        word(cases++);
        if(native)life.constructPool(manager,4);else original(0x4bfef0,manager,4u);
        backend.actorBase=backend.r(manager);backend.checkpoint(1);
        auto invoke=[&](U entry,U actor,auto... args){
            if(!native)return original(entry,actor,args...);
            switch(entry){
            case 0x506290:{const U a[]={U(args)...,0};life.activate(actor,a[0],a[1],static_cast<unsigned char>(a[2]));break;}
            case 0x51f5e0:{const U a[]={U(args)...,0};life.cleanup(actor,static_cast<unsigned char>(a[0]));break;}
            case 0x506d30:life.release(actor);break;
            case 0x52baf0:life.releasePresentation(actor);break;
            case 0x473100:life.prepare(actor);break;
            case 0x4c00d0:life.destroyPool(actor);break;
            default:throw std::runtime_error("Fixture dispatch");
            }
            return 0u;
        };
        const U first=native?life.firstFree():original(0x52a640,manager);
        if(first!=backend.actorBase)throw std::runtime_error("Free slot order");
        if((native?life.firstFree():original(0x52a640,manager))!=first)throw std::runtime_error("Free slot was reserved");
        backend.w(first+8,~1u);backend.w(first+12,21);backend.w(first+16,3);
        backend.trace=true;
        invoke(0x506290,first,type,0u,scenario&1);backend.checkpoint(2);
        const U second=native?life.firstFree():original(0x52a640,manager);
        backend.seedList(second);
        backend.w(second+8,3);backend.w(second+12,4);backend.w(second+16,5);
        invoke(0x506290,second,type,1u,1u);backend.checkpoint(3);
        // Populate actual owned type resources and raw particle-like aliases.
        const U resources=0x6de0e8+type*36;
        if(type!=1 && type!=13){for(U at:{0u,16u,20u,24u,28u,32u})backend.w(resources+at,backend.allocate(16));}
        backend.w(first+0x618,1);backend.w(first+0x977,2);backend.w(first+0x9b7,8);backend.w(first+0x9bb,9);backend.w(first+0x9bf,10);
        backend.w(first+0x68,1);backend.w(0x6e984c,1);backend.w(first+0x114,17);
        backend.w(first+0x120,0x180);backend.w(first+0x794,0x20101000);backend.w(first+0x805,0x20101010);
        backend.w(0x20101000,0x20102000);backend.w(0x20101010,0x20102000);backend.w(0x20102000,0x20103000);
        backend.w(0x20103024,0x20110000);backend.w(0x20103048,0x20110010);backend.w(0x20103034,0x20110020);
        backend.w(first+0x967,0x20104000);backend.w(first+0x664,1);backend.w(first+0x5fc,10);backend.w(first+0x64c,1);
        backend.w(first+0x124,2);backend.w(first+0x7cd,(scenario&1)?2:3);backend.w(first+0x7d1,second);
        backend.w(first+0x678,0x20105000);backend.w(first+0x67c,0x20105004);
        backend.w(first+0x798,1);backend.w(first+0x6e8,second);
        invoke(0x51f5e0,first,quiet);backend.checkpoint(4);
        // Restore the peer after scripted secondary release to exercise cascade.
        backend.w(second+4,1);backend.w(0x6df180,2);backend.w(first+0x7c,0);
        invoke(0x506d30,first);backend.checkpoint(5);
        invoke(0x52baf0,manager);backend.checkpoint(6);
        invoke(0x473100,manager);backend.checkpoint(7);
        const U reused=native?life.firstFree():original(0x52a640,manager);
        if(reused!=first)throw std::runtime_error("Stable storage reuse");
        invoke(0x506290,reused,type,1u,1u);backend.checkpoint(8);
        backend.seedList(reused);
        invoke(0x4c00d0,manager);backend.checkpoint(9);
        // Ordinary types retain their shared resources for the external owner.
        U retained=0;
        for(auto block:backend.blocks)if(block.alive){
            if(block.size!=32)throw std::runtime_error("Unaccounted retained allocation");
            ++retained;
        }
        // 5060a0 clears the histogram pointer without invoking 58f6f0.
        // Resetting an activated pool preserves this original retention defect.
        if(retained!=2)throw std::runtime_error("Unexpected reset histogram retention");
        seh.check();
        if(lastError!=0x12345678)throw std::runtime_error("Runtime LastError preservation");
    }
    std::cerr<<"actor lifecycle cases "<<cases<<"\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<"\n";return 1;}
