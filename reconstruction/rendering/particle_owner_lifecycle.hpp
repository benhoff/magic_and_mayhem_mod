#pragma once
#include "particle_motion.hpp"

namespace mnm::reconstruction {
struct OriginalCreatureSlot {
    std::array<unsigned char,0xe4b> bytes{};
    std::uint32_t get(unsigned at)const{std::uint32_t v;std::memcpy(&v,bytes.data()+at,4);return v;}
    void put(unsigned at,std::uint32_t v){std::memcpy(bytes.data()+at,&v,4);}
};
// Original 5060a0 replaces owned subobjects without relocating the creature.
// External raw references to this slot are neither enumerated nor invalidated.
template<class Provider>
void resetOriginalParticleOwner(OriginalCreatureSlot& slot,std::uint32_t index,Provider& provider){
    if(const auto callback=slot.get(0x708)){
        provider.write(callback,0,0x5c5c40);provider.release(callback);slot.put(0x708,0);
    }
    if(const auto list=slot.get(0x8f9)){
        const auto head=provider.read(list,4);
        auto node=provider.read(head,0);
        while(node!=head){
            const auto next=provider.read(node,0),previous=provider.read(node,4);
            provider.write(previous,0,next);provider.write(next,4,previous);
            provider.release(node);provider.write(list,8,provider.read(list,8)-1);node=next;
        }
        provider.release(head);provider.write(list,4,0);provider.write(list,8,0);
        provider.release(list);slot.put(0x8f9,0);
    }
    slot.bytes.fill(0);
    const auto callback=provider.allocate(16);
    if(callback){
        provider.write(callback,4,0x51e890);provider.write(callback,8,0);
        provider.write(callback,12,provider.ownerAddress());provider.write(callback,0,0x5c7264);
    }
    slot.put(0x708,callback);slot.put(0xcf3,callback);
    const auto list=provider.allocate(12);
    if(!list)throw std::invalid_argument("Original null owner list would fault");
    provider.byte(list,0,static_cast<unsigned char>(index));
    const auto head=provider.allocate(48);
    if(!head)throw std::invalid_argument("Original null owner sentinel would fault");
    provider.write(head,0,head);provider.write(head,4,head);
    provider.write(list,4,head);provider.write(list,8,0);slot.put(0x8f9,list);
    slot.put(0,index);slot.put(0x10c,5);
}
} // namespace mnm::reconstruction
