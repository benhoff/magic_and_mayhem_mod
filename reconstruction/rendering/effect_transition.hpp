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
// authored trajectory and the existing projection bounds. Changed blocked cells
// commit the original partial updates and return 1; ordinary completion returns 3.
// A blocked result remains active but detached; its lifecycle belongs to the caller.
// Unsupported inputs throw atomically; supported early termination commits together.
unsigned transitionEffectEmptyWorld(EffectPlacementPool&,unsigned slot,EffectTransitionState&,
                                    unsigned width,unsigned height,unsigned layers,
                                    const std::vector<EffectCleanupColumn>& columns={});
}
