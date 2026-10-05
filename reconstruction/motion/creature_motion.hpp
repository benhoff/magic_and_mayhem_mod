#pragma once
#include <array>
#include <cstdint>
#include <functional>

namespace mnm::reconstruction {
// Bounded forward-animation slice of No-CD 5104b0. External animation events
// default to a supplied 12-sample cycle, or use caller-staged ANI callbacks.
// Completion is returned
// before behavior/combat/environment callbacks. No original host pointers.
struct MotionState {
    std::int32_t accumulator=0,progress=0,travelX=0,travelY=0;
    std::int32_t fineX=0,fineY=0,fineZ=0,residualX=0,residualY=0;
    std::uint32_t frame=0,animationFrame=0,initialFrame=0;
    std::int32_t initialResidualX=0,initialResidualY=0;
};
struct MotionInputs {
    std::int32_t rate=0,duration=0,gridX=0,gridY=0,heightOrigin=0,heightDelta=0;
    std::int32_t direction=0;
    bool vertical=false,force32=false,separateCursor=false;
    std::array<std::int32_t,48> samples{};
};
// Nonnegative bounded rates only; excludes reverse-animation/type-specific
// callback chains. Completion discards unused iterations of this invocation.
struct MotionAnimation {
    std::function<std::int32_t()> tick;
    std::function<void()> restart;
};
// Callbacks must mutate only caller-staged owned state; unsupported events throw.
bool advance_creature_motion(MotionState&,const MotionInputs&,const MotionAnimation* = nullptr);
}
