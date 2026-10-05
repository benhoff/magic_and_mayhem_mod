#pragma once
#include "terrain_specific.hpp"
namespace mnm::reconstruction {
struct RegionGenerationAttempt {
    std::uint32_t seed=0;
    bool complete=false;
    unsigned backtracks=0, multiBlockBacktracks=0;
    std::vector<RegionSpecificDiagnostic> diagnostics;
};
struct RegionGenerationResult {
    RegionPlacementState state;
    std::vector<RegionSpecificRequest> requests;
    std::vector<RegionLocation> locations;
    std::vector<RegionGenerationAttempt> attempts;
    std::uint32_t nextSeed=0;
    bool complete=false;
};
// Reset one described attempt, retaining stored source rows and descriptor data.
RegionPlacementState resetRegionGenerationAttempt(RegionPlacementState);
// Selected caller loop after CFG/catalog loading. At most ten attempts; requests
// survive resets and the seed advances by 500 even after a successful attempt.
// Owns the final state, including partial placements after exhausted attempts.
RegionGenerationResult generateRegionPlacement(RegionPlacementState,
    std::vector<RegionSpecificRequest>,std::uint32_t seed);
}
