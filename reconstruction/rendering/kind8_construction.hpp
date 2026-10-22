#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <stdexcept>

namespace mnm::reconstruction {
enum class Kind8Constructor { inherited, additive, conditional, colour, terrainColour, flag };
using Kind8ObjectBytes = std::array<std::uint8_t,128>;
struct Kind8ConstructorInput { std::int32_t x,y,z,dx,dy,dz; };
struct Kind8ConstructorWorld {
    std::int32_t width{128}, height{96}, depth{256};
    bool rgb555{}, zeroRandomStream{true};
    std::uint32_t phase{19}, colourPhase{17}, colourDimension{3};
};
inline constexpr std::array<std::uint32_t,6> kind8ConstructorClasses{
    0x5c73f0,0x5c7420,0x5c7408,0x5c7494,0x5c74b0,0x5c74cc};
inline void kind8Write(Kind8ObjectBytes& bytes,unsigned at,std::uint32_t v,unsigned count=4) {
    for(unsigned i=0;i<count;++i)bytes.at(at+i)=std::uint8_t(v>>(8*i));
}
// Numeric class identities are build tags, never dereferenced original pointers.
// Zero-stream RNG and above-terrain/no-plane construction are explicit domains;
// original game RNG seeding, terrain caches, allocation and deleting destruction
// remain separate. Unwritten caller bytes are preserved, never assumed zero.
inline void constructKind8(Kind8Constructor type,Kind8ObjectBytes& bytes,
        const Kind8ConstructorInput& in,const Kind8ConstructorWorld& world) {
    const auto index=unsigned(type);
    if(index>=kind8ConstructorClasses.size()||world.width<=0||world.height<=0||
            world.depth<=0||world.depth>0x3fffffff||!world.zeroRandomStream||
            (type==Kind8Constructor::terrainColour&&in.z<world.depth))
        throw std::invalid_argument("Unrecovered kind8 constructor domain");
    auto put=[&](unsigned at,std::uint32_t v){kind8Write(bytes,at,v);};
    auto wrap=[](std::int32_t value,std::int32_t extent) {
        const auto n=std::int64_t(value)%extent;
        return std::uint32_t(n<0?n+extent:n);
    };
    const auto x=wrap(in.x,world.width),y=wrap(in.y,world.height);
    const auto z=std::uint32_t(std::clamp(in.z,0,2*world.depth));
    put(4,x);put(8,y);put(12,z);
    put(0x10,in.dx);put(0x14,in.dy);put(0x18,in.dz);
    const auto green=world.rgb555?16u:32u;
    if(type==Kind8Constructor::inherited||type==Kind8Constructor::conditional) {
        put(0x1c,128);put(0x20,128);put(0x24,32);put(0x28,100);put(0x2c,5);
        put(0x30,0);put(0x34,2);put(0x38,2);put(0x3c,16);put(0x40,green);put(0x44,4);
        if(type==Kind8Constructor::conditional) {
            put(0x10,0);put(0x14,0);put(0x18,16);
            for(unsigned at:{0x54u,0x58u,0x5cu,0x60u})put(at,0);
        }
    } else if(type==Kind8Constructor::additive) {
        put(0x1c,128);put(0x20,128);put(0x24,32);put(0x28,75);put(0x2c,world.phase);
        put(0x30,2);put(0x34,2);put(0x38,16);put(0x3c,green);put(0x40,4);
        kind8Write(bytes,0x44,0,1);
    } else if(type==Kind8Constructor::colour||type==Kind8Constructor::terrainColour) {
        put(0x1c,2);put(0x20,2);put(0x24,2);put(0x2e,0);
        kind8Write(bytes,0x28,24,2);kind8Write(bytes,0x2a,0,2);kind8Write(bytes,0x2c,0,2);
        if(type==Kind8Constructor::terrainColour) {
            put(0x32,x>>5);put(0x36,y>>5);put(0x3a,z>>4);
            put(0x1c,0);put(0x56,world.colourPhase);
            const auto d=std::clamp(world.colourDimension,1u,16u);put(0x20,d);put(0x24,d);
        }
    } else {
        put(0x10,10);put(0x14,20);put(0x18,0xfffffffeu);
        put(0x1c,in.dx);put(0x20,in.dy);put(0x24,in.dz);put(0x28,3);
        put(0x2c,224);put(0x30,32);put(0x34,0);put(0x38,64);put(0x3c,0);
        kind8Write(bytes,0x40,1,1);
    }
    put(0,kind8ConstructorClasses[index]);
}
inline void destroyKind8NonDeleting(Kind8Constructor type,Kind8ObjectBytes& bytes) {
    if(unsigned(type)>=kind8ConstructorClasses.size())
        throw std::invalid_argument("Unknown kind8 destruction class");
    if((type==Kind8Constructor::colour||type==Kind8Constructor::terrainColour)&&
            std::any_of(bytes.begin()+0x2e,bytes.begin()+0x32,[](auto v){return v!=0;}))
        throw std::invalid_argument("Owned colour-plane destruction is outside this domain");
    kind8Write(bytes,0,0x5c73a8);
}
}
