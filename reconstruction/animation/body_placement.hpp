#pragma once
#include "no_cd.hpp"
#include "placement.hpp"
#include <functional>

namespace mnm::reconstruction {
using BodySequenceSource = std::function<std::vector<assets::AnimationRecord>(std::uint32_t)>;
struct BodyPlacement {
    std::uint32_t sprite;
    AnimationOffset offset;
    std::optional<std::uint32_t> startedSequence;
};
// Selected 507190 body descriptor, including the null-display 288+facing start.
// spriteLastIndex is the original-style inclusive bound supplied by the owned
// sprite binding. The result is a frame index, never an original SPR pointer.
BodyPlacement bodyPlacement(NoCdAnimationPlayer&, std::uint32_t facing,
    std::uint32_t spriteLastIndex, const BodySequenceSource&,
    std::uint32_t tileSizeXY=1, std::uint32_t view=0);
}
