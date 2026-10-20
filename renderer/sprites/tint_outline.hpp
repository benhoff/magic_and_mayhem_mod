#pragma once
#include "sprite_loader.hpp"
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <vector>
namespace mnm::render {
struct TintWordSpan {std::int64_t first,last;}; // linear half-open WORD offsets
// Unclipped horizontal tint writes pairs(start-1,start), and for runs longer
// than one pixel,(end-2,end-1). Those pairs may cross a clip or scanline edge.
// Return the complete owned write set before any renderer mutation.
inline std::vector<TintWordSpan> tintOutlineSpans(const assets::SpriteFrame& f,
        std::int64_t left,std::int64_t top,int clipTop,int clipBottom,int width,int height){
    if(width<1||height<1||width>2048||height>2048||clipTop<0||clipBottom>height||clipTop>=clipBottom)
        throw std::invalid_argument("Tint canvas/clip outside owned bounds");
    const auto extent=std::int64_t(width)*height;std::vector<TintWordSpan> result;
    auto append=[&](std::int64_t first){
        if(first<0||first>extent-2)throw std::invalid_argument("Tint pair outside owned canvas");
        result.push_back({first,first+2});
    };
    for(const auto& run:f.opaqueRuns){
        if(run.row>=f.height||run.left>=run.right||run.right>f.width)
            throw std::invalid_argument("Tint run outside frame");
        const auto y=top+run.row;if(y<clipTop||y>=clipBottom)continue;
        const auto base=y*width+left;append(base+run.left-1);
        if(run.right-run.left>1)append(base+run.right-2);
    }
    return result;
}
}
