#pragma once
#include "terrain_selection.hpp"
namespace mnm::reconstruction {
struct RegionBlockDescriptor {
    SingleRegionDescriptor selection;
    unsigned sourceX=0, sourceY=0, columns=1, rows=1, layers=1;
};
struct RegionConnectorState {
    // Original descriptor builder's four independent +4 counters, in host order.
    std::array<std::int32_t,4> next{{1000,1001,1002,1003}};
};
std::vector<RegionBlockDescriptor> expandRegionDescriptors(const assets::MapAsset&,
    unsigned section, bool specific, std::int32_t maximum, RegionConnectorState&);
struct RegionDescriptorBank {
    std::int32_t internalThreshold=1000;
    std::vector<RegionBlockDescriptor> blocks;
};
std::optional<unsigned> findRegionConnector(const RegionDescriptorBank&,
    std::int32_t label, std::int32_t placed, unsigned current);
struct RegionAdmissionCarry {
    unsigned section=0, descriptor=0, rotation=0;
    RegionEdges edges{};
};
struct RegionAdmission { bool admitted=false; RegionAdmissionCarry carry; };
// The input grid/counts remain unchanged; carry records the original scratch result.
RegionAdmission admitRegionSection(const RegionSelectionGrid&, const RegionDescriptorBank&,
    unsigned descriptor, unsigned column, unsigned row, unsigned rotation);
std::vector<RegionCandidate> regionCandidates(const RegionSelectionGrid&,
    const RegionDescriptorBank&, unsigned column, unsigned row, bool special);
std::vector<RegionLocation> regionLocations(const RegionSelectionGrid&,
    const RegionDescriptorBank&, unsigned descriptor, int rotation);
// Owned result; -1 removes the tried descriptor regardless of rotation.
// At most 49 compact entries: reserve the original fiftieth termination slot.
struct RegionPruningResult {
    std::vector<RegionCandidate> candidates;
    std::optional<RegionAdmissionCarry> carry; // No processed descriptor leaves scratch unchanged.
};
RegionPruningResult pruneRegionCandidateDetails(const RegionDescriptorBank&,
    const std::vector<RegionCandidate>&, unsigned triedDescriptor, int rotation);
std::vector<RegionCandidate> pruneRegionCandidates(const RegionDescriptorBank&,
    const std::vector<RegionCandidate>&, unsigned triedDescriptor, int rotation);
}
