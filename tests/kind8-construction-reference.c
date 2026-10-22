/* Original known constructors and actual nondeleting virtual destructors.
 * World/RNG/format inputs are owned fixtures; no original scheduler/allocator,
 * terrain lookup, raster, live object or arbitrary-class equivalence claim. */
#include "original-pe32-image.h"
static const u32 constructors[6]={0x545dc0,0x546590,0x546230,0x548560,0x548650,0x548ac0};
static const u32 classes[6]={0x5c73f0,0x5c7420,0x5c7408,0x5c7494,0x5c74b0,0x5c74cc};
static const i32 positions[6]={-129,-1,0,95,128,257};
static const i32 velocities[3]={-7,0,9};
static void set(u32 address,u32 value){*(u32*)address=value;}
struct Result {u32 id,kind,format; i32 x,y,z,dx,dy,dz;u32 constructor_same,destructor_same,destructor_entry;
    u8 constructed[128],destroyed[128];};
static struct Result result;
void start(void){
    original_map();
    HANDLE f=CreateFileA("kind8-construction.bin",0x40000000,0,0,1,0,0);
    original_require(f!=(HANDLE)-1,113);
    set(0x5eb480,128);set(0x5eb484,96);set(0x5eb488,256);
    set(0x5e1464,19);set(0x5e1428,17);set(0x5e142c,3);set(0x5e1430,0);
    u32 ordinal=0;
    for(u32 kind=0;kind<6;++kind)for(u32 format=0;format<2;++format)
    for(u32 xi=0;xi<6;++xi)for(u32 yi=0;yi<6;++yi)
    for(u32 zi=0;zi<2;++zi)for(u32 vi=0;vi<3;++vi){
        u8 object[128];for(u32 i=0;i<128;++i)object[i]=0xa5;
        /* A fully zero-owned lagged-XOR state produces a declared zero stream;
           it is not a recovery of the game's strategic RNG seeding. */
        for(u32 i=0;i<128;++i)((u32*)0x6574c8)[i]=0;
        set(0x6e1f88,format);
        result.id=ordinal++;result.kind=kind;result.format=format;
        result.x=positions[xi];result.y=positions[yi];result.z=zi?512:256;
        result.dx=velocities[vi];result.dy=result.dx+1;result.dz=result.dx-1;
        typedef void*(__attribute__((thiscall))*Ctor6)(void*,i32,i32,i32,i32,i32,i32);
        typedef void*(__attribute__((thiscall))*Ctor3)(void*,i32,i32,i32);
        void* returned=kind==2?
            ((Ctor3)constructors[kind])(object,result.x,result.y,result.z):
            ((Ctor6)constructors[kind])(object,result.x,result.y,result.z,result.dx,result.dy,result.dz);
        result.constructor_same=returned==object;
        original_require(result.constructor_same&&*(u32*)object==classes[kind],114);
        original_copy(result.constructed,object,128);
        void** table=*(void***)object;result.destructor_entry=(u32)table[0];
        typedef void*(__attribute__((thiscall))*Dtor)(void*,u32);
        result.destructor_same=((Dtor)table[0])(object,0)==object;
        original_require(result.destructor_same&&*(u32*)object==0x5c73a8,115);
        original_copy(result.destroyed,object,128);
        u32 written=0;original_require(WriteFile(f,&result,sizeof(result),&written,0)&&written==sizeof(result),116);
    }
    original_require(ordinal==2592,117);CloseHandle(f);ExitProcess(0);
}
