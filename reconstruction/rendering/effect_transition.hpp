#pragma once
#include "effect_projection.hpp"
#include "effect_cleanup.hpp"

namespace mnm::reconstruction {
struct EffectTransitionState {
    EffectProjectionState motion;
    std::array<unsigned,3> previousPosition{};
    unsigned terrain=1;
};
// Complete 004883f0 selected empty-world movement with membership updates on.
// Caller supplies empty terrain bits/no creatures, catalog ordinals 0..3,
// unblocked touched cells, authored trajectory and the existing projection bounds.
// The pool and state commit together only after all iterations succeed.
unsigned transitionEffectEmptyWorld(EffectPlacementPool&,unsigned slot,EffectTransitionState&,
                                    unsigned width,unsigned height,unsigned layers,
                                    const std::vector<EffectCleanupColumn>& columns={});
}
