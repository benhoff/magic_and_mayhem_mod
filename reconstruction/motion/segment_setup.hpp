#pragma once
#include "creature_motion.hpp"
#include "route_scalar.hpp"
namespace mnm::reconstruction {
// Selected ordinary 005070e0 snap; caller resolves zero/nonzero definition.
// Generator type +8 == 2 uses a separate height helper and is excluded.
int ordinary_creature_height(int layer,int terrainHeight);
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
    int gridZ=1,destinationTerrainHeight=0;
};
struct SegmentSetup {
    MotionState state;
    std::uint32_t rate=0,duration=0;
    int sampleBank=0,animation=0;
    bool carried=false,resetRate=false;
    int heightOrigin=0,heightDelta=0;
};
// Ordinary forward category-zero/four setup with admitted/legal target and disabled
// environment/attachment branches. Scalar computation runs once before the
// continuity test and a second time on the reset path.
SegmentSetup initialize_creature_segment(const SegmentPrevious&,const SegmentRequest&,const CreatureScalarState&);
}
