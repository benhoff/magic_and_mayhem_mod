#pragma once
#include <cstdint>

namespace mnm::recovered::campaign {
// No-CD 40209ca7, settled Realm tick state 0, after original hit-map lookup.
// Caller supplies valid original region/ownership/adjacency/occupancy state.
// This does not authorize live commands or implement campaign progression.
struct RegionSelectionInput {
    std::uint32_t region = 0;
    bool playerOwns = false;
    bool adjacentOwned = false;
    bool alternateMode = false;
    std::uint32_t eligibleOccupants = 0;
    bool tutorialActive = false;
    bool auxiliaryPending = false;
    bool buttonDown = false;
    bool releaseLatched = false;
};
enum class RegionSelectionEffect { None, Destination, RegionEntry, CapacityRefusal };
struct RegionSelectionResult {
    bool eligible = false;
    bool releaseLatched = false;
    bool hoverCue = false;
    bool selectionCue = false;
    RegionSelectionEffect effect = RegionSelectionEffect::None;
};
RegionSelectionResult regionSelection(const RegionSelectionInput&);
}
