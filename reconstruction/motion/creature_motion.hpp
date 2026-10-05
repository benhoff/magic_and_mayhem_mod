#pragma once
#include <array>
#include <cstdint>

namespace mnm::reconstruction {
// Bounded forward-animation slice of No-CD 5104b0. External animation events
// are supplied as a 12-sample cycle (event 2 at its end); completion is returned
// before behavior/combat/environment callbacks. No original host pointers.
struct MotionState {
    std::int32_t accumulator=0,progress=0,travelX=0,travelY=0;
    std::int32_t fineX=0,fineY=0,fineZ=0,residualX=0,residualY=0;
    std::uint32_t frame=0;
};
struct MotionInputs {
    std::int32_t rate=0,duration=0,gridX=0,gridY=0,heightOrigin=0,heightDelta=0;
    std::int32_t direction=0;
    bool vertical=false,force32=false;
    std::array<std::int32_t,12> samples{};
};
// Positive bounded arithmetic only; excludes reverse-animation/type-specific
// callback chains. Completion discards unused iterations of this invocation.
bool advance_creature_motion(MotionState&,const MotionInputs&);
}
