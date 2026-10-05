#pragma once
#include "effect_projection.hpp"
#include "effect_cleanup.hpp"

namespace mnm::reconstruction {
// Four owned catalog entries, each an 8x8x4 MSB-first occupancy bitmap.
using EffectTerrainOccupancy=std::array<std::array<std::uint8_t,32>,4>;
struct EffectTransitionState {
    EffectProjectionState motion;
    std::array<unsigned,3> previousPosition{};
    unsigned terrain=1;
};
// Complete 004883f0 selected no-creature movement with optional membership updates.
// Caller supplies no creatures, catalog ordinals 0..3 and owned terrain bits,
// authored trajectory and the existing projection bounds. Changed blocked cells
// commit the original partial updates and return 1. Height exits commit raw
// trajectory parameters/counter while retaining the last valid fine/cell state,
// and return 2. Ordinary completion returns 3.
// With updates disabled, chains and cell flags stay unchanged, including on return 1.
// With updates enabled, return 1 remains active but detached; caller owns lifecycle.
// Unsupported inputs throw atomically; supported early termination commits together.
unsigned transitionEffectEmptyWorld(EffectPlacementPool&,unsigned slot,EffectTransitionState&,
                                    unsigned width,unsigned height,unsigned layers,
                                    const std::vector<EffectCleanupColumn>& columns={},
                                    bool updateMembership=true,
                                    const EffectTerrainOccupancy& occupancy={});
}
