#pragma once
#include "animation.hpp"

namespace mnm::reconstruction {
struct AnimationState {
    std::size_t pc=0;
    std::optional<std::size_t> displayedRecord;
    bool active=false;
    std::uint32_t delay=0,elapsed=0,repeats=0,breakFlag=0;
};
// Selected forward contract of No-CD 464cb0/464ec0/465000. No wall-clock rate.
// Owns a sequence; bounded dispatch replaces unsafe original control loops.
class NoCdAnimationPlayer final {
public:
    explicit NoCdAnimationPlayer(std::vector<assets::AnimationRecord> sequence);
    void start();
    // Original 464e20: retain relative record positions and timing. Native
    // policy rejects inactive/unstarted or incompatible targets atomically.
    void switchSequence(std::vector<assets::AnimationRecord> sequence);
    std::int32_t tick(); // raw event argument, 0 normally, 1 when stopped
    void requestBreak(){state_.breakFlag=1;}
    const AnimationState& state() const{return state_;}
    std::optional<std::uint32_t> sprite() const;
private:
    std::int32_t dispatch(bool initial);
    std::vector<assets::AnimationRecord> records_;
    AnimationState state_;
};
}
