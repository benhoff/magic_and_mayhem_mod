#pragma once
#include "surface_wrappers.hpp"
#include <cstdint>
#include <stdexcept>

namespace mnm::reconstruction {
// Original 58c4a0/58c8a0 argument lifetime across retries. Initial translation
// and destination extent are cached; draw API/WAIT and source RECT are reread.
// Key preparation uses separate scratch storage; key results do not change
// these cached rectangle coordinates. This models forwarding, not rasterization.
class SurfaceCopyRetryGeometry final {
public:
    SurfaceCopyRetryGeometry(SurfaceCopyWrapper kind, SurfaceRectangle32 source,
                             std::int32_t x, std::int32_t y, SurfaceWindowContext context)
        :kind_(kind),x_(x),y_(y) {
        if(kind!=SurfaceCopyWrapper::opaqueRecovering && kind!=SurfaceCopyWrapper::keyedRecovering)
            throw std::invalid_argument("Unsupported original copy retry geometry");
        context.destinationIsPrimary=context.destinationIsPrimary && !context.fast;
        context.fast=false;
        SurfaceRectangle32 unused;
        destination_=planSurfaceWrapperCopy(kind,source,x,y,unused,context).destination;
    }
    SurfaceWrapperCopyPlan draw(SurfaceRectangle32 source, bool fast, bool noWait) const {
        SurfaceRectangle32 unused;
        auto plan=planSurfaceWrapperCopy(kind_,source,x_,y_,unused,{fast,noWait,false,0,0});
        if(!fast) plan.destination=destination_;
        return plan;
    }
private:
    SurfaceCopyWrapper kind_;
    std::int32_t x_,y_;
    SurfaceRectangle32 destination_;
};
}
