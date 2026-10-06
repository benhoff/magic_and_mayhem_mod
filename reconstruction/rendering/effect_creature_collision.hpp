#pragma once
#include "effect_transition.hpp"
#include "creature_occupancy.hpp"
namespace mnm::reconstruction {
inline constexpr unsigned NoCreature = 0xffffffffu;
struct EffectCollisionCreature {
    std::uint32_t id=0,status=0;
    std::array<std::uint32_t,3> units{};
    CreatureOccupancy footprint;
};
struct EffectCreatureWorld {
    // One original cell+4 WORD occupant; NoEffect and out-of-catalog ordinals are empty.
    std::vector<std::uint16_t> cells;
    std::vector<EffectCollisionCreature> creatures;
};
struct EffectCreatureTransitionState {
    EffectTransitionState movement;
    // Owned ordinal replacing original +198 pointer. Retained when kind 68 skips scanning.
    unsigned candidate=NoCreature;
};
// Selected effect types admit kinds 0/34/68; kind 34 admits the creator.
// This does not initialize creator ownership or broaden empty-world admission.
unsigned transitionEffectCreatureWorld(EffectPlacementPool&,unsigned slot,
    EffectCreatureTransitionState&,unsigned width,unsigned height,unsigned layers,
    const EffectCreatureWorld&,const std::vector<EffectCleanupColumn>& columns={},
    bool updateMembership=true,const EffectTerrainOccupancy& occupancy={});
}
