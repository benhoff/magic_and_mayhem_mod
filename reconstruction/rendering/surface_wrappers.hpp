#pragma once
#include <cstdint>

namespace mnm::reconstruction {
struct SurfaceRectangle32 {
    std::int32_t left=0, top=0, right=0, bottom=0;
};
enum class SurfaceCopyWrapper {
    opaqueGuarded, opaqueRecovering, keyedRecovering, keyedRectangle, keyedExplicit
};
// Supplied host context replaces original globals and ClientToScreen. A failed
// coordinate query supplies zero offsets. This structure owns no window pointer.
struct SurfaceWindowContext {
    bool fast=false, noWait=false, destinationIsPrimary=false;
    std::int32_t clientX=0, clientY=0;
};
struct SurfaceWrapperCopyPlan {
    bool called=false, fast=false, windowCall=false;
    std::uint32_t flags=0;
    SurfaceRectangle32 source, destination;
};
// Successful-call argument construction only. No clipping or driver admission.
// Only keyedExplicit mutates the caller's destination rectangle. All coordinate
// arithmetic wraps to signed 32 bits without host signed-overflow assumptions.
SurfaceWrapperCopyPlan planSurfaceWrapperCopy(
    SurfaceCopyWrapper, SurfaceRectangle32 source, std::int32_t x, std::int32_t y,
    SurfaceRectangle32& destination, SurfaceWindowContext,
    bool sourceAvailable=true, bool destinationAvailable=true);
}
