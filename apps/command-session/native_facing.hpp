#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
namespace mnm::command_session {
struct NativeFacing { unsigned direction=0,animation=0; };
// Consume already wrapped map-axis deltas. The provider owns coordinate wrapping
// and target lifetime. Wide arithmetic also admits the full signed input range.
inline std::optional<NativeFacing> nativeFacingFromDelta(std::int32_t x,std::int32_t y,
                                                       unsigned cameraTurn,bool reversed){
 if(!x && !y)return std::nullopt;
 const auto sign=[](std::int32_t n){return n<0?-1:n>0?1:0;};
 auto dx=sign(x),dy=sign(y);
 const auto ax=x<0?-std::int64_t(x):std::int64_t(x);
 const auto ay=y<0?-std::int64_t(y):std::int64_t(y);
 if(dx && dy && 2*std::max(ax,ay)>3*std::min(ax,ay)){
  if(ax>ay)dy=0;else dx=0;
 }
 constexpr unsigned table[]={7,0,1,6,0,2,5,4,3};
 const auto direction=table[dx+3*dy+4];
 return NativeFacing{direction,(direction-2u*cameraTurn-(reversed?4u:0u))&7u};
}
}
