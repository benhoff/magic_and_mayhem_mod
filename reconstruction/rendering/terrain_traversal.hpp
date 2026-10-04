#pragma once
#include "terrain_submission.hpp"
#include <functional>
namespace mnm::reconstruction {
// Decoded annotations of the pinned camera receiver; never a wire layout.
struct TerrainCamera {
    std::uint32_t view=0,span=20,diagonal=10,cutLevel=1,mode=0;
    std::int32_t column=0,row=0;
    std::int32_t f11=256,f15=0,f41=20,f45=0,f49=0,f4d=0,f51=0,f55=0;
    std::int32_t wrapColumnPriority=0,wrapRowPriority=0;
    std::uint32_t specialLayer=0xffffffffu;
};
struct TerrainViewport {std::int32_t left=0,top=0,right=512,bottom=256;};
struct TerrainVisit {
    std::uint32_t column=0,row=0,layer=0;
    std::int32_t anchorX=0,anchorY=0,priority=0;
    std::uint32_t cell=0; // Selected physical grid index may differ at recovered boundary carries.
};
// Selected four-orientation traversal. Callback addresses the physical cell;
// visit coordinates retain the original depth/light annotations and order.
std::vector<TerrainVisit> traverseTerrain(std::uint32_t width,std::uint32_t height,
    std::uint32_t layers,const TerrainCamera&,TerrainViewport,
    const std::function<std::array<std::uint16_t,2>(std::uint32_t,std::uint32_t,std::uint32_t)>& flags);
}
