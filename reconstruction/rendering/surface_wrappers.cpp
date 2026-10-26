#include "surface_wrappers.hpp"
#include <stdexcept>

namespace mnm::reconstruction {
namespace {
std::int32_t signedWord(std::uint32_t value) {
    return static_cast<std::int32_t>(value<0x80000000u ? std::int64_t(value)
                                                           : std::int64_t(value)-0x100000000LL);
}
std::int32_t add(std::int32_t a, std::int32_t b) {
    return signedWord(std::uint32_t(a)+std::uint32_t(b));
}
std::int32_t end(std::int32_t start, std::int32_t low, std::int32_t high) {
    return signedWord(std::uint32_t(start)+std::uint32_t(high)-std::uint32_t(low));
}
}
SurfaceWrapperCopyPlan planSurfaceWrapperCopy(
    SurfaceCopyWrapper kind, SurfaceRectangle32 source, std::int32_t x, std::int32_t y,
    SurfaceRectangle32& destination, SurfaceWindowContext context,
    bool sourceAvailable, bool destinationAvailable) {
    const bool guarded=kind==SurfaceCopyWrapper::opaqueGuarded ||
                       kind==SurfaceCopyWrapper::opaqueRecovering;
    const bool explicitRect=kind==SurfaceCopyWrapper::keyedExplicit;
    const bool keyed=kind==SurfaceCopyWrapper::keyedRecovering ||
                     kind==SurfaceCopyWrapper::keyedRectangle || explicitRect;
    if(!guarded && !keyed) throw std::invalid_argument("Unknown surface copy wrapper");
    if(!sourceAvailable || !destinationAvailable) {
        if(guarded) return {};
        throw std::invalid_argument("Keyed wrapper requires both owned surfaces");
    }
    SurfaceWrapperCopyPlan out;
    out.called=true;
    out.fast=context.fast && !explicitRect;
    out.windowCall=!context.fast && context.destinationIsPrimary;
    out.source=source;
    const auto dx=out.windowCall ? context.clientX : 0;
    const auto dy=out.windowCall ? context.clientY : 0;
    if(explicitRect) {
        destination={add(destination.left,dx), add(destination.top,dy),
                     add(destination.right,dx), add(destination.bottom,dy)};
        out.destination=destination;
    } else if(out.fast) {
        out.destination={x,y,0,0};
    } else {
        out.destination={add(x,dx), add(y,dy),
                         add(end(x,source.left,source.right),dx),
                         add(end(y,source.top,source.bottom),dy)};
    }
    out.flags=keyed ? (out.fast ? 1u : 0x8000u) : 0u;
    if(!context.noWait || explicitRect) out.flags|=out.fast ? 0x10u : 0x01000000u;
    return out;
}
}
