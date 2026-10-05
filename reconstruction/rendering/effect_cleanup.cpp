#include "effect_cleanup.hpp"
#include <stdexcept>
namespace mnm::reconstruction {
bool effectCleanupEligible(unsigned cell,unsigned width,unsigned height,unsigned layers,
                           const std::vector<EffectCleanupColumn>& columns){
    if(!width || !height || !layers || width>128 || height>128 || layers>32 ||
       cell>=width*height*layers || (!columns.empty() && columns.size()!=width*height))
        throw std::invalid_argument("Effect cleanup dimensions/table outside owned bounds");
    if(columns.empty())return true;
    const auto& entry=columns[cell%(width*height)];
    const auto layer=int(cell/(width*height));
    return !entry.marker || (layer!=entry.lower && layer!=entry.upper);
}
void cleanupEmptyEffectCell(EffectCell& value,unsigned cell,unsigned width,unsigned height,unsigned layers,
                            const std::vector<EffectCleanupColumn>& columns,
                            std::uint16_t creatureHead,std::uint16_t otherHead){
    if(!value.terrain && value.head==NoEffect && creatureHead==NoEffect && otherHead==NoEffect &&
       !(value.flags&0x20000000u) && effectCleanupEligible(cell,width,height,layers,columns))
        value.flags|=0x80u;
}
}
