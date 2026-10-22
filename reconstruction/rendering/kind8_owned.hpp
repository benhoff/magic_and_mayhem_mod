#pragma once
#include "kind8_construction.hpp"
#include "kind8_random.hpp"
#include <cmath>
#include <vector>

namespace mnm::reconstruction {
struct OwnedKind8 {
    Kind8ObjectBytes bytes;
    std::vector<std::uint16_t> plane;
    bool alive{true};
};
inline std::uint32_t kind8Read(const Kind8ObjectBytes& b,unsigned at,unsigned n=4) {
    std::uint32_t value=0;for(unsigned i=0;i<n;++i)value|=std::uint32_t(b.at(at+i))<<(8*i);return value;
}
inline std::vector<std::uint16_t> kind8Plane(unsigned width,unsigned height,
        std::array<unsigned,3> colour,bool rgb555) {
    if(!width||!height||width>16||height>16)throw std::invalid_argument("Unrecovered colour-plane extent");
    const float diagonal=float(std::sqrt(static_cast<long double>((width-1)*(width-1)+(height-1)*(height-1))));
    if(diagonal<=1.f)return {};
    const float high=1.3f,low=.7f,delta=float(high-low);
    std::vector<std::uint16_t> result;result.reserve(width*height*3);
    for(unsigned y=0;y<height;++y)for(unsigned x=0;x<width;++x) {
        const float squareY=float(y*y);
        const auto factor=static_cast<long double>(high)-
            std::sqrt(static_cast<long double>(x*x)+squareY)/diagonal*delta;
        for(unsigned channel=0;channel<3;++channel)
            result.push_back(std::uint16_t(std::clamp(factor*colour[channel],0.L,
                static_cast<long double>(channel==1&&!rgb555?63:31))));
    }
    return result;
}
inline OwnedKind8 constructOwnedKind8(Kind8Constructor type,const Kind8ConstructorInput& in,
        bool rgb555,std::int32_t dimension,bool planeEnabled,Kind8Random& random) {
    OwnedKind8 out;out.bytes.fill(0xa5);
    Kind8ConstructorWorld world;world.rgb555=rgb555;
    world.colourDimension=unsigned(std::clamp(dimension,1,16));
    auto safe=in;
    if(type==Kind8Constructor::terrainColour)safe.z=std::max(in.z,world.depth);
    constructKind8(type,out.bytes,safe,world);
    auto put=[&](unsigned at,std::uint32_t value){kind8Write(out.bytes,at,value);};
    if(type==Kind8Constructor::conditional) {
        auto next=[&](){return random.next()&0x7fffffffu;};
        std::int32_t x,y,z;
        if(next()&1) {x=next()%16;y=next()%(16-x);}
        else {y=next()%16;x=next()%(16-y);}
        z=16-x-y;
        if(next()&1)x=-x;
        if(next()&1)y=-y;
        if(next()&1)z=-z;
        put(0x10,x);put(0x14,y);put(0x18,z);
    }
    if(type==Kind8Constructor::flag)put(0x28,(random.next()&0x7fffffff)%6+3);
    if(type==Kind8Constructor::terrainColour) {
        const auto x=kind8Read(out.bytes,4),y=kind8Read(out.bytes,8);
        const auto z=unsigned(std::clamp(in.z,0,2*world.depth));put(12,z);put(0x3a,z>>4);
        if(z<unsigned(world.depth)) {
            const auto cell=(z>>4)*12+(y>>5)*4+(x>>5);
            const auto shadeCell=(z>>5)*12+(y>>5)*4+(x>>5);
            put(0x3e,0x10000000u+cell);put(0x42,0x20000000u+cell%3);
            const auto byte=(shadeCell*37)&255;
            put(0x46,byte<128?byte:byte-256u);
        }
        if(planeEnabled) {
            out.plane=kind8Plane(world.colourDimension,world.colourDimension,{24,0,0},rgb555);
            if(!out.plane.empty())put(0x2e,0x30000001);
        }
    }
    return out;
}
inline void setOwnedKind8Colour(OwnedKind8& object,std::array<int,3> requested,bool rgb555) {
    if(!object.alive)throw std::invalid_argument("Retired kind8 object");
    if(requested[0]==int(kind8Read(object.bytes,0x28,2))&&
       requested[1]==int(kind8Read(object.bytes,0x2a,2))&&
       requested[2]==int(kind8Read(object.bytes,0x2c,2)))return;
    std::array<unsigned,3> words;
    for(unsigned i=0;i<3;++i) {
        words[i]=unsigned(std::clamp(requested[i],0,255))/(i==1&&!rgb555?4:8);
        kind8Write(object.bytes,0x28+2*i,words[i],2);
    }
    if(!object.plane.empty()) {
        const auto values=kind8Plane(kind8Read(object.bytes,0x20),kind8Read(object.bytes,0x24),words,rgb555);
        std::copy(values.begin(),values.end(),object.plane.begin());
    }
}
// Returned records are logical free kind/phase pairs, not host addresses.
inline std::vector<std::uint32_t> retireOwnedKind8(OwnedKind8& object,bool deleting) {
    if(!object.alive)throw std::invalid_argument("Duplicate kind8 retirement");
    std::vector<std::uint32_t> frees;
    if(!object.plane.empty()) {
        frees.insert(frees.end(),{1,0});std::vector<std::uint16_t>{}.swap(object.plane);kind8Write(object.bytes,0x2e,0);
    }
    kind8Write(object.bytes,0,0x5c73a8);
    if(deleting){frees.insert(frees.end(),{2,0});object.alive=false;}
    return frees;
}
}
