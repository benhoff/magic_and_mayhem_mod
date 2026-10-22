#include "../reconstruction/rendering/kind8_construction.hpp"
#include <fstream>
#include <iostream>

using namespace mnm::reconstruction;
struct Result {
    std::uint32_t id,kind,format;
    std::int32_t x,y,z,dx,dy,dz;
    std::uint32_t constructorSame,destructorSame,destructorEntry;
    Kind8ObjectBytes constructed,destroyed;
};
static_assert(sizeof(Result)==304);
int main(int argc,char** argv) try {
    if(argc!=2)throw std::invalid_argument("Expected new native output file");
    std::ofstream out(argv[1],std::ios::binary);
    const std::uint32_t destructors[]{0x545e60,0x546630,0x546270,0x548600,0x5487f0,0x548b90};
    const int positions[]{-129,-1,0,95,128,257},velocities[]{-7,0,9};
    unsigned id=0;
    for(unsigned kind=0;kind<6;++kind)for(unsigned format=0;format<2;++format)
    for(int x:positions)for(int y:positions)for(int z:{256,512})for(int dx:velocities) {
        Result r{};r.id=id++;r.kind=kind;r.format=format;
        r.x=x;r.y=y;r.z=z;r.dx=dx;r.dy=dx+1;r.dz=dx-1;
        auto bytes=Kind8ObjectBytes{};bytes.fill(0xa5);
        auto type=Kind8Constructor(kind);Kind8ConstructorWorld world;world.rgb555=bool(format);
        constructKind8(type,bytes,{x,y,z,r.dx,r.dy,r.dz},world);r.constructed=bytes;
        destroyKind8NonDeleting(type,bytes);r.destroyed=bytes;
        r.constructorSame=r.destructorSame=1;r.destructorEntry=destructors[kind];
        out.write(reinterpret_cast<const char*>(&r),sizeof(r));
    }
    if(id!=2592||!out)throw std::runtime_error("Incomplete native construction matrix");
    out.close();
    unsigned refusals=0;
    for(unsigned trial=0;trial<5;++trial) {
        Kind8ObjectBytes bytes;bytes.fill(0xa5);const auto before=bytes;
        Kind8ConstructorWorld world;auto type=Kind8Constructor::inherited;
        if(trial==0)type=Kind8Constructor(99);
        if(trial==1)world.width=0;
        if(trial==2)world.zeroRandomStream=false;
        if(trial==3)type=Kind8Constructor::terrainColour;
        try {
            if(trial==4)destroyKind8NonDeleting(Kind8Constructor::colour,bytes);
            else constructKind8(type,bytes,{0,0,0,0,0,0},world);
        } catch(const std::invalid_argument&) {
            if(bytes!=before)throw std::runtime_error("Unsupported constructor mutated bytes");
            ++refusals;
        }
    }
    if(refusals!=5)throw std::runtime_error("Unsupported construction admitted");
    std::cout<<"2592 constructor/destructor cells and5 atomic refusals pass\n";
} catch(const std::exception& e) { std::cerr<<e.what()<<'\n';return 1; }
