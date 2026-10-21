extern "C" {
#include "../shadow/win32_min.h"
}
#include "../../reconstruction/rendering/world_cell_producer.hpp"
#include "original_bytes.h"
extern "C" void mnm_world_cell_entry();
extern "C" int _fltused=0;
static int span(u32 p,u32 n,int writing){
    if(!p || n>~0u-p)return 0;
    const u32 end=p+n;
    while(p<end){
        u32 region[7];
        if(VirtualQuery((void*)p,region,28)!=28 || region[4]!=0x1000 ||
            (region[5]&0x101) || !(region[5]&(writing?0xcc:0xee)))return 0;
        const u32 next=region[0]+region[3];if(next<=p)return 0;p=next<end?next:end;
    }
    return 1;
}
struct RuntimeWorldCellBackend {
    u32 temporary[12];
    [[noreturn]] void fault(const char*){ExitProcess(0x57435001);__builtin_unreachable();}
    u32 r(u32 p){
        if(!span(p,4,0))fault("World cell read");
        const u8* a=(const u8*)p;return a[0]|u32(a[1])<<8|u32(a[2])<<16|u32(a[3])<<24;
    }
    u16 r16(u32 p){if(!span(p,2,0))fault("World cell word read");return ((u8*)p)[0]|u16(((u8*)p)[1])<<8;}
    u8 r8(u32 p){if(!span(p,1,0))fault("World cell byte read");return *(u8*)p;}
    void w(u32 p,u32 v){if(!span(p,4,1))fault("World cell write");for(u32 k=0;k<4;++k)((u8*)p)[k]=u8(v>>(8*k));}
    void w16(u32 p,u32 v){if(!span(p,2,1))fault("World cell word write");((u8*)p)[0]=u8(v);((u8*)p)[1]=u8(v>>8);}
    void w8(u32 p,u32 v){if(!span(p,1,1))fault("World cell byte write");*(u8*)p=u8(v);}
    u32 scratch(){return (u32)temporary;}
    double real(u32 p){
        const u32 words[]={r(p),r(p+4)};double value;__builtin_memcpy(&value,words,8);return value;
    }
    double distance(int x,int y){return ((double(*)(double,double))0x59d60c)(double(x),double(y));}
    u32 hit(u32 entry,u32 sprite,u32 x,u32 y,u32 point){
        using F=u32(__attribute__((fastcall)) *)(u32,u32,u32,u32);
        return ((F)entry)(sprite,x,y,point);
    }
    void relocate(u32 actor,u32 x,u32 y,u32 z){
        using F=void(__attribute__((fastcall)) *)(u32,u32,u32,u32);
        ((F)0x4eadf0)(actor,x,y,z);
    }
    template<class... A>u32 call(u32 entry,u32 receiver,A... args){
        using F=u32(__attribute__((thiscall)) *)(u32,A...);return ((F)entry)(receiver,args...);
    }
};
extern "C" void mnm_world_cell_dispatch(u32* regs){
    const u32 error=GetLastError();RuntimeWorldCellBackend backend{};
    mnm::reconstruction::WorldCellProducer<RuntimeWorldCellBackend>(backend).produce(regs[6]);
    SetLastError(error);
}
extern "C" int mnm_world_cell_install(){
    if(!span(0x4f8960,sizeof(mnm_world_cell_original),0))return 0;
    for(u32 k=0;k<sizeof(mnm_world_cell_original);++k)
        if(((u8*)0x4f8960)[k]!=mnm_world_cell_original[k])return 0;
    u32 protection;
    if(!VirtualProtect((void*)0x4f8960,8,0x40,&protection))return 0;
    const u32 relative=(u32)&mnm_world_cell_entry-0x4f8960-5;
    ((u8*)0x4f8960)[0]=0xe9;
    for(u32 k=0;k<4;++k)((u8*)0x4f8960)[1+k]=u8(relative>>(8*k));
    for(u32 k=5;k<8;++k)((u8*)0x4f8960)[k]=0x90;
    u32 ignored;
    if(!VirtualProtect((void*)0x4f8960,8,protection,&ignored) ||
        !FlushInstructionCache(GetCurrentProcess(),(void*)0x4f8960,8))ExitProcess(0x57435002);
    return 1;
}
extern "C" __declspec(dllexport) void WorldCellProducerAnchor(){}
#ifndef MNM_WORLD_CELL_TEST
extern "C" int WIN DllMain(void*,u32 reason,void*){
    if(reason!=1)return 1;const u32 error=GetLastError();
    const int ok=(u32)GetModuleHandleA(0)==0x400000 && mnm_world_cell_install();
    SetLastError(error);return ok;
}
#endif
