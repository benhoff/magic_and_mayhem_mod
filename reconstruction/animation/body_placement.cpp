#include "body_placement.hpp"
#include <stdexcept>
#include <utility>

namespace mnm::reconstruction {
namespace {
BodyPlacement descriptor(const NoCdAnimationPlayer& player, std::uint32_t last,
        std::uint32_t tileSizeXY, std::uint32_t view) {
    const auto record=player.displayedRecord();
    const auto sprite=player.sprite();
    if(!record || !sprite)throw std::runtime_error("Body fallback has no displayed sprite");
    return {*sprite<=last?*sprite:0, spriteOffset(*record,tileSizeXY,view), {}};
}
}
BodyPlacement bodyPlacement(NoCdAnimationPlayer& player, std::uint32_t facing,
        std::uint32_t last, const BodySequenceSource& source,
        std::uint32_t tileSizeXY, std::uint32_t view) {
    if(player.displayedRecord())return descriptor(player,last,tileSizeXY,view);
    if(facing>7 || !source)throw std::invalid_argument("Body fallback requires an owned facing sequence");
    const auto sequence=288+facing;
    NoCdAnimationPlayer next(source(sequence));
    next.start();
    auto result=descriptor(next,last,tileSizeXY,view);
    result.startedSequence=sequence;
    player=std::move(next);
    return result;
}
}
