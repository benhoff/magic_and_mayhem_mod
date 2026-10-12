#include "realm_selection.hpp"

namespace mnm::recovered::campaign {
RegionSelectionResult regionSelection(const RegionSelectionInput& in) {
    RegionSelectionResult out;
    out.eligible = in.region != 0 && (in.alternateMode || in.playerOwns || in.adjacentOwned);
    out.releaseLatched = in.releaseLatched;
    if (!out.eligible || in.tutorialActive) return out;
    out.hoverCue = true;
    if (in.auxiliaryPending) {
        out.releaseLatched = false;
    } else if (in.buttonDown) {
        out.releaseLatched = true;
    } else if (in.releaseLatched) {
        out.releaseLatched = false;
        out.selectionCue = true;
        if (in.alternateMode) {
            out.effect = in.eligibleOccupants < 4 ? RegionSelectionEffect::Destination
                                                : RegionSelectionEffect::CapacityRefusal;
        } else {
            out.effect = in.playerOwns ? RegionSelectionEffect::Destination
                                      : RegionSelectionEffect::RegionEntry;
        }
    }
    return out;
}
}
