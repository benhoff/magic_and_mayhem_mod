/* Opt-in native actor lifecycle. External gameplay/platform services keep their
 * existing entry points; lifecycle entries never fall back to original bodies. */
extern "C" {
#include "../shadow/win32_min.h"
}
#include "../../reconstruction/rendering/actor_lifecycle.hpp"
#include "original_bytes.h"
extern "C" {
#define MNM_ACTOR_ENTRY(name,kind,pop) void mnm_actor_##name##_entry();
#include "entries.inc"
#undef MNM_ACTOR_ENTRY
}
static int span(u32 p,u32 n,int writing) {
    if(!p || n>~0u-p)return 0;
    const u32 end=p+n;
    while(p<end) {
        u32 region[7];
        if(VirtualQuery((void*)p,region,28)!=28 || region[4]!=0x1000 ||
           (region[5]&0x101) || !(region[5]&(writing?0xcc:0xee)))return 0;
        const u32 next=region[0]+region[3];if(next<=p)return 0;
        p=next<end?next:end;
    }
    return 1;
}
struct RuntimeActorBackend {
    u32 temporary[5];
    [[noreturn]] void fault(const char*) {ExitProcess(0x41435401);__builtin_unreachable();}
    u32 r(u32 p) {
        if(!span(p,4,0))fault("Actor read");
        const u8* a=(const u8*)p;
        return a[0]|u32(a[1])<<8|u32(a[2])<<16|u32(a[3])<<24;
    }
    void w(u32 p,u32 v) {
        if(!span(p,4,1))fault("Actor write");
        for(u32 i=0;i<4;++i)((u8*)p)[i]=u8(v>>(8*i));
    }
    u8 r8(u32 p) {if(!span(p,1,0))fault("Actor byte read");return *(u8*)p;}
    void w8(u32 p,u8 v) {if(!span(p,1,1))fault("Actor byte write");*(u8*)p=v;}
    u32 allocate(u32 n) {return ((u32(*)(u32))0x597850)(n);}
    void free(u32 p) {((void(*)(u32))0x597870)(p);}
    void arrayFree(u32 p) {((void(*)(u32))0x5979b0)(p);}
    u32 scratch() {return (u32)temporary;}
    void loadError() {
        using Message=u32(WIN *)(u32,u32,u32,u32);
        using Destroy=u32(WIN *)(u32);
        ((Message)r(0x5c520c))(0,0x5da204,0x5da224,0x42030);
        ((Destroy)r(0x5c5208))(r(0x656614));
    }
    template<class... A>u32 call(u32 entry,u32 receiver,A... args) {
        using F=u32(__attribute__((thiscall)) *)(u32,A...);
        return ((F)entry)(receiver,args...);
    }
    template<class... A>u32 stdcall(u32 entry,A... args) {
        using F=u32(WIN *)(A...);return ((F)entry)(args...);
    }
};
extern "C" void mnm_actor_dispatch(u32* regs,u32 kind) {
    const u32 error=GetLastError(),p=regs[6];
    u32* args=regs+9;
    RuntimeActorBackend backend{};
    mnm::reconstruction::ActorLifecycle<RuntimeActorBackend> life(backend);
    switch(kind) {
    case 0:life.construct(p);regs[7]=p;break;
    case 1:life.destruct(p);break;
    case 2:life.reset(p,args[0]);break;
    case 3:life.baseline(p);break;
    case 4:life.activate(p,args[0],args[1],u8(args[2]));break;
    case 5:life.cleanup(p,u8(args[0]));break;
    case 6:life.release(p);break;
    case 7:life.prepare(p);break;
    case 8:regs[7]=life.constructPool(p,args[0]);break;
    case 9:life.destroyPool(p);break;
    case 10:life.resetAll(p);break;
    case 11:regs[7]=life.firstFree(p);break;
    case 12:life.releasePresentation(p);break;
    default:backend.fault("Actor dispatch");
    }
    SetLastError(error);
}
extern "C" int mnm_actor_install() {
    const u32 targets[]={
#define MNM_ACTOR_ENTRY(name,kind,pop) (u32)&mnm_actor_##name##_entry,
#include "entries.inc"
#undef MNM_ACTOR_ENTRY
    };
    static_assert(sizeof(targets)/sizeof(targets[0])==13,"Actor guards and entries");
    u32 protection[13],prepared=0;
    for(u32 i=0;i<13;++i) {
        const auto& guard=mnm_actor_original[i];
        if(!span(guard.address,guard.size,0))return 0;
        for(u32 j=0;j<guard.size;++j)if(((u8*)guard.address)[j]!=guard.bytes[j])return 0;
    }
    for(;prepared<13;++prepared) {
        if(!VirtualProtect((void*)mnm_actor_original[prepared].address,8,0x40,&protection[prepared]))break;
    }
    if(prepared!=13) {
        while(prepared) {--prepared;u32 ignored;VirtualProtect((void*)mnm_actor_original[prepared].address,8,protection[prepared],&ignored);}
        return 0;
    }
    for(u32 i=0;i<13;++i) {
        const u32 p=mnm_actor_original[i].address,relative=targets[i]-p-5;
        ((u8*)p)[0]=0xe9;
        for(u32 j=0;j<4;++j)((u8*)p)[1+j]=u8(relative>>(8*j));
        for(u32 j=5;j<8;++j)((u8*)p)[j]=0x90;
    }
    for(u32 i=0;i<13;++i) {
        u32 ignored;const u32 p=mnm_actor_original[i].address;
        if(!VirtualProtect((void*)p,8,protection[i],&ignored) || !FlushInstructionCache(GetCurrentProcess(),(void*)p,8))ExitProcess(0x41435402);
    }
    return 1;
}
extern "C" __declspec(dllexport) void ActorLifecycleAnchor() {}
#ifndef MNM_ACTOR_TEST
extern "C" int WIN DllMain(void*,u32 reason,void*) {
    if(reason!=1)return 1;
    const u32 error=GetLastError();
    const int ok=(u32)GetModuleHandleA(0)==0x400000 && mnm_actor_install();
    SetLastError(error);return ok;
}
#endif
