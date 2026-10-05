#pragma once
#include "creature_motion.hpp"
#include "route_scalar.hpp"
namespace mnm::reconstruction {
struct SegmentPrevious {
    MotionState state;
    std::uint32_t rate=0;
    int action=0,direction=0,vertical=0,category=0;
    bool preserveCandidate=false;
};
struct SegmentRequest {
    Coordinates delta;
    int direction=0,vertical=0,category=0;
    int gridX=0,gridY=0,heightOrigin=0;
};
struct SegmentSetup {
    MotionState state;
    std::uint32_t rate=0,duration=0;
    int sampleBank=0,animation=0;
    bool carried=false,resetRate=false;
};
// Ordinary forward category-zero setup with admitted/legal target and disabled
// environment/attachment branches. Scalar computation runs once before the
// continuity test and a second time on the reset path.
SegmentSetup initialize_creature_segment(const SegmentPrevious&,const SegmentRequest&,const CreatureScalarState&);
}
