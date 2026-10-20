#pragma once
#include "sprite_loader.hpp"
#include "../../assets/sprite_run_clipping.hpp"
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <vector>
namespace mnm::render {
enum class RowEffectKind {placeSource,sampleDestination};
struct SpriteRowEffect {
    RowEffectKind kind=RowEffectKind::placeSource;
    std::vector<std::int32_t> offsets;
    bool wholeRunClip=false;
};
struct RowEffectSpan {
    int sourceRow,sourceFirst,count;
    std::int64_t destination,sample;
};
// Return a complete plan before mutation. Source coordinates stay independent
// of signed sprite origins; destination/sample addresses are owned WORD indices.
inline std::vector<RowEffectSpan> planRowEffect(const assets::SpriteFrame& f,
        const SpriteRowEffect& effect,std::int64_t left,std::int64_t top,
        int clipLeft,int clipTop,int clipRight,int clipBottom,int width,int height){
    if(width<1||width>2048||height<1||height>2048||clipLeft<0||clipTop<0||
       clipRight>width||clipBottom>height||clipLeft>=clipRight||clipTop>=clipBottom||
       f.width>2048||f.height>2048||effect.offsets.size()!=f.height||
       f.opaqueMask.size()!=std::size_t(f.width)*f.height||
       (effect.kind!=RowEffectKind::placeSource&&effect.kind!=RowEffectKind::sampleDestination)||
       std::any_of(effect.offsets.begin(),effect.offsets.end(),[](auto n){return n<0||n>32;}))
        throw std::invalid_argument("Unsupported owned row-effect inputs");
    std::uint64_t covered=0,previous=0;
    for(const auto& run:f.opaqueRuns){
        const auto start=std::uint64_t(run.row)*f.width+run.left;
        if(run.row>=f.height||run.left>=run.right||run.right>f.width||start<previous)
            throw std::invalid_argument("Malformed owned row-effect run history");
        for(auto x=run.left;x<run.right;++x)if(f.opaqueMask[std::size_t(run.row)*f.width+x]!=1)
            throw std::invalid_argument("Row-effect run disagrees with coverage");
        covered+=run.right-run.left;previous=std::uint64_t(run.row)*f.width+run.right;
    }
    if(std::any_of(f.opaqueMask.begin(),f.opaqueMask.end(),[](auto n){return n>1;})||
       covered!=std::uint64_t(std::count(f.opaqueMask.begin(),f.opaqueMask.end(),std::uint8_t(1))))
        throw std::invalid_argument("Incomplete owned row-effect run history");
    const auto extent=std::int64_t(width)*height;
    if(left>=clipRight||left<=std::int64_t(clipLeft)-f.width||top>=clipBottom||top<=std::int64_t(clipTop)-f.height)return {};
    const bool clipped=left<clipLeft||left+f.width>clipRight;
    auto runs=effect.wholeRunClip&&clipped?assets::clipWholeOpaqueRuns(f,std::int64_t(clipLeft)-left,std::int64_t(clipRight)-left):f.opaqueRuns;
    if(runs.empty()&&!f.opaqueRuns.empty())return {};
    if(f.opaqueRuns.empty()&&std::any_of(f.opaqueMask.begin(),f.opaqueMask.end(),[](auto n){return n!=0;}))
        throw std::invalid_argument("Row effect lacks encoded run history");
    std::vector<RowEffectSpan> result;
    for(const auto& run:runs){
        if(run.row>=f.height||run.left>=run.right||run.right>f.width)
            throw std::invalid_argument("Row effect run outside frame");
        const auto y=top+run.row;if(y<clipTop||y>=clipBottom)continue;
        const auto first=std::max<std::int64_t>(run.left,std::int64_t(clipLeft)-left);
        const auto last=std::min<std::int64_t>(run.right,std::int64_t(clipRight)-left);
        if(first>=last)continue;
        const auto at=y*width+left+first;const auto shift=effect.offsets[run.row];
        const auto destination=at+(effect.kind==RowEffectKind::placeSource?shift:0);
        const auto sample=at+(effect.kind==RowEffectKind::sampleDestination?shift:0);
        const auto count=last-first;
        if(destination<0||destination>extent-count||sample<0||sample>extent-count)
            throw std::invalid_argument("Covered row-effect range outside owned canvas");
        result.push_back({int(run.row),int(first),int(count),destination,sample});
    }
    return result;
}
}
