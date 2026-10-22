#pragma once
#include "particle_motion.hpp"

namespace mnm::reconstruction {
struct ParticleTerrainCell {
    std::uint16_t block{};
    std::array<unsigned char,10> bytes{};
};
struct ParticleTerrainFacts {
    // Two explicitly retained generations of a 4x3x4 fine-cell grid. Shade
    // uses32-unit Z cells, occupancy uses16-unit Z cells on the same strides.
    std::array<std::array<ParticleTerrainCell,48>,2> cells{};
    std::array<std::array<std::int8_t,48>,2> shades{};
    std::array<std::array<unsigned char,356>,3> descriptors{};
    unsigned generation{};
    const ParticleTerrainCell& cell(std::uint32_t token)const{
        if(!token||token>96)throw std::invalid_argument("Retired or unknown particle terrain cell");
        return cells[(token-1)/48][(token-1)%48];
    }
};
struct ParticleTerrainValidity {bool descriptorDefined{true};};
inline bool updateTerrainParticle(ParticleObject& object,ParticleWorld world,
        const ParticleTerrainFacts& terrain,std::int32_t duration,bool rgb555,
        ParticleTerrainValidity& validity){
    if(world.width!=128||world.height!=96||world.depth!=64||terrain.generation>1)
        throw std::invalid_argument("Owned terrain grid extent");
    const std::array<std::int32_t,3> old{object.get(4),object.get(8),object.get(12)};
    const auto moving=updateParticleMotion(object,world);object.put(0x18,particleSub(object.get(0x18),1));
    const auto z=object.get(12);if(!moving||z<1)return false;
    if(z>=world.depth)return true;
    const auto x=std::uint32_t(object.get(4)),y=std::uint32_t(object.get(8));
    const std::array<std::uint32_t,3> cell{x>>5,y>>5,std::uint32_t(z)>>4};
    const bool changed=std::uint32_t(object.get(0x32))!=cell[0]||std::uint32_t(object.get(0x36))!=cell[1]||std::uint32_t(object.get(0x3a))!=cell[2];
    const auto index=cell[2]*12+cell[1]*4+cell[0];
    const auto token=changed?terrain.generation*48+index+1:std::uint32_t(object.get(0x3e));
    const auto& current=terrain.cell(token);
    const auto shade=changed?std::int32_t(terrain.shades[terrain.generation][(std::uint32_t(z)>>5)*12+cell[1]*4+cell[0]]):object.get(0x46);
    bool collision=false;
    if(current.block){
        if(current.block>=terrain.descriptors.size())throw std::invalid_argument("Unknown particle terrain descriptor");
        const auto sx=(x-cell[0]*32)>>2,sy=(y-cell[1]*32)>>2,sz=(std::uint32_t(z)-cell[2]*16)>>2;
        collision=(terrain.descriptors[current.block][sy+sz*8]&(0x80u>>sx))!=0;
    }else collision=(current.bytes[9]&0x40)!=0;
    if(collision){
        for(unsigned axis=0;axis<3;++axis){object.put(4+axis*4,old[axis]);object.put(16+axis*4,0);}
        const auto remaining=particleSub(object.get(0x56),1);object.put(0x56,remaining);
        object.put(0x1c,std::min(3,particleSub(4,particleDivide(particleMul(remaining,4),duration))));return remaining>0;
    }
    if(changed){
        if(object.get(0x46)!=shade){
            object.put(0x46,shade);const auto factor=particleAdd(shade,127);
            for(unsigned channel=0;channel<3;++channel){
                const auto value=std::clamp(particleDivide(particleMul(object.get(0x4a+channel*4),factor),127),0,255);
                const auto quantized=std::uint16_t(value/(channel==1?(rgb555?8:4):8));
                std::memcpy(object.bytes.data()+0x28+channel*2,&quantized,2);
            }
        }
        // 548860 never initializes its descriptor scratch when block==0.
        // Preserve that uncertainty explicitly; do not invent a null pointer.
        validity.descriptorDefined=current.block!=0;
        if(validity.descriptorDefined)object.put(0x42,0x2000+current.block);
        object.put(0x3e,token);for(unsigned axis=0;axis<3;++axis)object.put(0x32+axis*4,cell[axis]);
    }return true;
}
}
