#pragma once
#include "effect_placement.hpp"
namespace mnm::reconstruction {
// Recovered six-byte 006a49c0 column entries; trailing bytes are unused here.
struct EffectCleanupColumn {
    std::uint16_t marker=0;
    std::int8_t lower=0,upper=0;
};
// Empty vector models the original null table. Dimensions are caller-owned.
bool effectCleanupEligible(unsigned cell,unsigned width,unsigned height,unsigned layers,
                           const std::vector<EffectCleanupColumn>& columns);
void cleanupEmptyEffectCell(EffectCell&,unsigned cell,unsigned width,unsigned height,unsigned layers,
                            const std::vector<EffectCleanupColumn>& columns,
                            std::uint16_t creatureHead=NoEffect,std::uint16_t otherHead=NoEffect);
}
