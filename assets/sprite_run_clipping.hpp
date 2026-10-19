#pragma once
#include "sprite_loader.hpp"
#include <algorithm>
#include <stdexcept>

namespace mnm::assets {
// Horizontal whole-run clipping policy. Call only when the frame itself crosses
// a horizontal boundary. A run already inside the left edge must end strictly
// before the effective right edge (at most frame width); a run crossing the left
// edge may end at that edge. Preserve separate adjacent opaque runs.
inline std::vector<SpriteOpaqueRun> clipWholeOpaqueRuns(const SpriteFrame& f,
                                                       std::int64_t left,std::int64_t right){
  if(f.opaqueMask.size()!=std::size_t(f.width)*f.height || left>=right)
    throw std::invalid_argument("Invalid whole-run clip geometry");
  left=std::max<std::int64_t>(0,left);right=std::min<std::int64_t>(f.width,right);
  std::vector<SpriteOpaqueRun> result;
  std::uint64_t covered=0,previous=0;
  for(const auto& run:f.opaqueRuns){
    const auto start=std::uint64_t(run.row)*f.width+run.left;
    if(run.row>=f.height || run.left>=run.right || run.right>f.width || start<previous)
      throw std::invalid_argument("Invalid owned opaque run");
    for(auto x=run.left;x<run.right;++x)if(f.opaqueMask[std::size_t(run.row)*f.width+x]!=1)
      throw std::invalid_argument("Opaque run disagrees with coverage");
    previous=std::uint64_t(run.row)*f.width+run.right;covered+=run.right-run.left;
    if(run.left<left){
      if(run.right>left && run.right<=right)result.push_back({run.row,std::uint32_t(left),run.right});
    }else if(run.right<right)result.push_back(run);
  }
  if(covered!=std::uint64_t(std::count(f.opaqueMask.begin(),f.opaqueMask.end(),std::uint8_t(1))))
    throw std::invalid_argument("Missing encoded opaque run history");
  return result;
}
}
