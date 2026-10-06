#pragma once
#include "effect_transition.hpp"

namespace mnm::reconstruction {
// Whole 0048a950 helper. Start/end are raw fine-unit words, not inferred from
// record parameters. Cell storage is read-only; original pointers become ordinals.
// Writes startupUnits (+1de), distinct from motion.previousUnits (+1ea).
// Record, movement, startupUnits and all input storage must be disjoint.
// Invalid owned dimensions/start coordinates throw before any mutation.
void initializeEffectMotionRecord(EffectPlacementRecord& record,
    EffectTransitionState& movement,std::array<std::uint32_t,3>& startupUnits,
    const std::array<std::uint32_t,3>& from,const std::array<std::uint32_t,3>& to,
    unsigned width,unsigned height,unsigned layers,const std::vector<EffectCell>& cells);
}
