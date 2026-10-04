#pragma once
#include "animation.hpp"
namespace mnm::reconstruction {
struct AnimationOffset {std::int32_t x=0,y=0;};
enum class AttachmentPoint {first,second};
// Selected No-CD 507190/507250/5072c0/507330 field and adjustment contract.
// 'view' is the raw original global value; no compass/camera meaning inferred.
AnimationOffset spriteOffset(const assets::AnimationRecord&,std::uint32_t tileSizeXY=1,std::uint32_t view=0);
AnimationOffset attachmentOffset(const assets::AnimationRecord&,AttachmentPoint,std::uint32_t tileSizeXY=1,std::uint32_t view=0);
}
