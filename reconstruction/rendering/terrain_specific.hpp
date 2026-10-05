#pragma once
#include "terrain_solver.hpp"
namespace mnm::reconstruction {
struct RegionSpecificRequest {
    std::int32_t column=-1, row=-1, rotation=-1;
};
enum class RegionSpecificNotice {
    RequestedRotationRejected, FixedLocationRejected,
    RequestedLocationsEmpty, AllLocationsEmpty
};
struct RegionSpecificDiagnostic {
    unsigned descriptor=0;
    RegionSpecificNotice notice=RegionSpecificNotice::FixedLocationRejected;
};
struct RegionSpecificResult {
    RegionPlacementState state;
    // One request per descriptor, including ignored ordinary descriptors.
    std::vector<RegionSpecificRequest> requests;
    // Last wildcard search, retained even after placement; row-first compact table.
    std::vector<RegionLocation> locations;
    std::vector<RegionSpecificDiagnostic> diagnostics;
};
// Recovered two-pass Specific placement. Owns all input/output; notices replace
// blocking configuration dialogs. Does not run the solver or advance the seed.
RegionSpecificResult placeRegionSpecifics(RegionPlacementState,
    std::vector<RegionSpecificRequest>, std::uint32_t seed);
}
