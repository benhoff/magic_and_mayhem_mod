#pragma once
#include "kind8_random.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace mnm::reconstruction {
inline std::int32_t particleSigned(std::uint32_t value){return value<=0x7fffffffu?std::int32_t(value):-1-std::int32_t(0xffffffffu-value);}
inline std::int32_t particleAdd(std::int32_t a,std::int32_t b){return particleSigned(std::uint32_t(a)+std::uint32_t(b));}
inline std::int32_t particleSub(std::int32_t a,std::int32_t b){return particleSigned(std::uint32_t(a)-std::uint32_t(b));}
inline std::int32_t particleMul(std::int32_t a,std::int32_t b){return particleSigned(std::uint32_t(a)*std::uint32_t(b));}
inline std::int32_t particleDivide(std::int32_t a,std::int32_t b){
    if(!b||(a==INT32_MIN&&b==-1))throw std::invalid_argument("Original particle integer division fault");
    return a/b;
}
struct ParticleObject {
    std::array<unsigned char,160> bytes{};
    std::int32_t get(unsigned at)const{if(at>bytes.size()-4)throw std::invalid_argument("Particle field extent");std::uint32_t value;std::memcpy(&value,bytes.data()+at,4);return particleSigned(value);}
    void put(unsigned at,std::int32_t value){if(at>bytes.size()-4)throw std::invalid_argument("Particle field extent");std::memcpy(bytes.data()+at,&value,4);}
};
struct ParticleWorld {std::int32_t width{},height{},depth{};};
struct ParticleOwnerFacts {
    bool present{},active{};
    std::array<std::int32_t,3> position{};
    std::int32_t horizontalOffset{},verticalOffset{};
    std::uint32_t keepAlive{};
};
inline void requireParticleWorld(ParticleWorld w){if(w.width<=0||w.height<=0||w.depth<0||w.depth>INT32_MAX/2)throw std::invalid_argument("Original particle world requires positive XY extents and bounded depth");}
inline std::int32_t wrapParticle(std::int32_t value,std::int32_t size){const auto remainder=value%size;return remainder<0?remainder+size:remainder;}
inline void constructParticleMotion(ParticleObject& object,ParticleWorld world,
        const std::array<std::int32_t,3>& position,const std::array<std::int32_t,3>& velocity,bool moving){
    requireParticleWorld(world);object.put(0,moving?0x5c73c0:0x5c73a8);
    object.put(4,wrapParticle(position[0],world.width));object.put(8,wrapParticle(position[1],world.height));object.put(12,std::clamp(position[2],0,world.depth*2));
    if(moving)for(unsigned axis=0;axis<3;++axis)object.put(16+axis*4,velocity[axis]);
}
inline bool updateParticleMotion(ParticleObject& object,ParticleWorld world){
    requireParticleWorld(world);
    object.put(4,wrapParticle(particleAdd(object.get(4),object.get(16)),world.width));
    object.put(8,wrapParticle(particleAdd(object.get(8),object.get(20)),world.height));
    const auto dz=object.get(24);const auto z=std::max(particleAdd(object.get(12),dz),0);object.put(12,z);
    return (z<=world.depth*2||dz<0)&&(z!=0||dz>=0);
}
inline void attachParticleAxes(ParticleObject& object,ParticleWorld world,const ParticleOwnerFacts& owner,
        unsigned offsets,unsigned flags,bool useOffsets){
    if(!owner.present||!owner.active)return;
    for(unsigned axis=0;axis<3;++axis){
        if(flags&&!object.get(flags+axis*4))continue;
        const auto offset=axis==2?owner.verticalOffset:owner.horizontalOffset;
        auto value=particleAdd(owner.position[axis],offset);
        if(useOffsets)value=particleAdd(value,object.get(offsets+axis*4));
        if(axis<2)value=wrapParticle(value,axis?world.height:world.width);
        object.put(4+axis*4,value);
    }
}
inline bool updateFadingParticle(ParticleObject& object,ParticleWorld world,bool rgb555,
        const ParticleOwnerFacts* owner=nullptr){
    requireParticleWorld(world);
    if(owner)attachParticleAxes(object,world,*owner,0x48,0x54,true);
    if(!updateParticleMotion(object,world))return false;
    auto delay=object.get(0x30),level=object.get(0x28);
    if(delay<1){level=particleSub(level,object.get(0x2c));object.put(0x28,level);}
    else object.put(0x30,delay-1);
    if(level<=0)return false;
    for(unsigned channel=0;channel<3;++channel){
        const auto value=std::clamp(particleDivide(particleMul(object.get(0x1c+channel*4),level),100),0,255);
        object.put(0x3c+channel*4,value/(channel==1?(rgb555?8:4):8));
    }return true;
}
inline bool updateAttachedParticle(ParticleObject& object,ParticleWorld world,const ParticleOwnerFacts& owner){
    requireParticleWorld(world);attachParticleAxes(object,world,owner,0x18,0,true);
    const auto remaining=particleSub(object.get(0x14),1);object.put(0x14,remaining);return remaining>0;
}
struct ParticleSeekConfig {std::int32_t jitter{},nearDistance{},maximumVelocity{};};
inline std::uint32_t updateSeekingParticle(ParticleObject& object,ParticleWorld world,
        const ParticleOwnerFacts& owner,ParticleSeekConfig config,Kind8Random& random){
    if(!updateParticleMotion(object,world))return 0;
    if(!owner.present)throw std::invalid_argument("Original seeking particle dereferences its owner");
    if(config.jitter<0||config.jitter==INT32_MAX||config.maximumVelocity<0)throw std::invalid_argument("Particle steering configuration domain");
    std::array<std::int32_t,3> target{},distance{},direction{},velocity{},old{};
    for(unsigned axis=0;axis<3;++axis){
        target[axis]=particleAdd(owner.position[axis],axis==2?owner.verticalOffset:owner.horizontalOffset);
        const auto jitter=std::int32_t((random.next()&0x7fffffffu)%std::uint32_t(config.jitter+1))-config.jitter/2;
        velocity[axis]=particleAdd(object.get(16+axis*4),jitter);object.put(16+axis*4,velocity[axis]);old[axis]=object.get(4+axis*4);
    }
    const auto absolute=[](std::int32_t value){return value<0?particleSub(0,value):value;};
    for(unsigned axis=0;axis<2;++axis){
        const auto direct=absolute(particleAdd(particleSub(old[axis],target[axis]),velocity[axis]));
        const auto wrapped=particleSub(axis?world.height:world.width,direct);
        direction[axis]=direct<wrapped?1:-1;distance[axis]=direct<wrapped?direct:wrapped;
    }
    const auto nextZ=particleAdd(old[2],velocity[2]);direction[2]=nextZ<target[2]?1:-1;
    // The branch difference remains a wrapped signed value for the later
    // denominator/product. Only the proximity tests take its absolute value.
    distance[2]=nextZ<target[2]?particleSub(target[2],nextZ):particleSub(nextZ,target[2]);
    const auto sum=particleAdd(particleAdd(absolute(distance[0]),absolute(distance[1])),absolute(distance[2]));
    if(sum>3){
        const auto total=particleAdd(particleAdd(distance[0],distance[2]),distance[1]);
        for(unsigned axis=0;axis<3;++axis){
            if(absolute(distance[axis])<config.nearDistance)direction[axis]=-direction[axis];
            const auto delta=particleDivide(particleMul(particleMul(config.maximumVelocity,distance[axis]),direction[axis]),total<2?1:total);
            // The original Z branch compares target X with old Z. Preserve
            // this recovered behavior rather than silently correcting it.
            const bool subtract=(axis==2?target[0]:target[axis])<old[axis];
            object.put(16+axis*4,std::clamp(subtract?particleSub(velocity[axis],delta):particleAdd(velocity[axis],delta),-config.maximumVelocity,config.maximumVelocity));
        }
    }return owner.keepAlive;
}
}
