#pragma once
#include "terrain_constraints.hpp"
namespace mnm::reconstruction {
struct RegionPlacedBlock { unsigned descriptor=0, rotation=0; };
struct RegionPlacementState {
    RegionSelectionGrid grid;
    RegionDescriptorBank bank;
    // Both indexed column*rows + row; empty cells have no assignment.
    std::vector<std::optional<RegionPlacedBlock>> assignments;
    // Original grid reset leaves this field untouched; owned initial policy is zero.
    // Placement writes source Y, while cell removal writes ffffffff.
    std::vector<std::uint32_t> storedSourceRows;
    std::vector<std::vector<RegionCandidate>> candidates;
    std::optional<RegionAdmissionCarry> carry;
};
RegionPlacementState makeRegionPlacementState(unsigned columns,unsigned rows,RegionDescriptorBank);
// Recovered unchecked placement walk, with native extent/occupied/connector
// refusals. Caller performs admission first. Changes commit only on success.
void placeRegionSection(RegionPlacementState&,unsigned descriptor,unsigned rotation,unsigned column,unsigned row);
void removeRegionBlock(RegionPlacementState&,unsigned column,unsigned row);
struct RegionSolveResult {
    RegionPlacementState state;
    bool complete=false;
    unsigned backtracks=0, multiBlockBacktracks=0;
};
// One recovered generation attempt after Specific placement; owns input/result.
// No CFG loading, Specific fallback, retry/seed advancement or scene assembly.
RegionSolveResult solveRegionPlacement(RegionPlacementState,std::uint32_t seed);
}
