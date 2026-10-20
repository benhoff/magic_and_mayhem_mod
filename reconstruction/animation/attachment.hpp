#pragma once
#include "no_cd.hpp"
#include <functional>
#include <cstdint>
namespace mnm::reconstruction {
// No-CD 51ff00 mode-1 entry: effect configuration entry 36, base + facing.
struct ModeOneSelection {std::uint32_t assetIndex=0,sequence=0;};
ModeOneSelection modeOneSelection(std::uint32_t base,std::uint32_t assetIndex,std::uint32_t facing);
// Selected draw/update gates. Health is tested for nonzero, not positivity.
bool modeOneVisible(std::int32_t health,std::uint32_t mode);
bool modeOneTicks(std::uint32_t mode);
using ModeOneSequenceSource = std::function<std::vector<assets::AnimationRecord>(std::uint32_t asset,std::uint32_t sequence)>;
// Owned selected 0->1 / 1->0 lifecycle. Configuration and asset resolution are
// supplied values/services; no global game addresses or widget dependencies.
class ModeOneAttachment final {
public:
    bool change(std::uint32_t requestedMode,std::uint32_t base,std::uint32_t assetIndex,
                std::uint32_t facing,const ModeOneSequenceSource&);
    std::int32_t tick();
    std::uint32_t mode() const{return mode_;}
    const std::optional<ModeOneSelection>& selection() const{return selection_;}
    const NoCdAnimationPlayer* controller() const{return player_?&*player_:nullptr;}
    bool visible(std::int32_t health) const{
        return modeOneVisible(health,mode_) && player_ && bool(player_->displayedRecord());
    }
private:
    std::uint32_t mode_=0;
    std::optional<ModeOneSelection> selection_;
    std::optional<NoCdAnimationPlayer> player_;
};
}
