#pragma once
#include "no_cd.hpp"

namespace mnm::reconstruction {
// Original 16-byte effect animation metadata entry. Property/opaque words retain
// their raw values; this adapter does not assign rendering/gameplay semantics.
struct EffectAnimationEntry {
    std::uint32_t sequence=0,assetIndex=0,property=0,opaque=0;
};
struct EffectAnimationBinding {
    std::uint32_t animationOrdinal;
    EffectAnimationEntry entry;
    NoCdAnimationPlayer player;
};
// Metadata resolution from static 00494be3..00494c26, then selected forward
// 00464ca0/00464cb0 behavior. Copies the selected sequence; no borrowed pointers.
// Rejects invalid metadata/asset/sequence/extents and unsafe sprite records.
// Start/control-dispatch failure returns no binding and leaves inputs unchanged.
EffectAnimationBinding bindEffectAnimation(std::uint32_t animationOrdinal,
    const std::vector<EffectAnimationEntry>& entries,
    const std::vector<assets::Animation>& animations);
}
