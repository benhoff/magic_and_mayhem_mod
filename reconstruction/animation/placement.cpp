#include "placement.hpp"
#include <cstring>
#include <stdexcept>
namespace mnm::reconstruction {
static std::int32_t signedWord(std::uint32_t word){std::int32_t result;std::memcpy(&result,&word,4);return result;}
static AnimationOffset offset(const assets::AnimationRecord& r,std::size_t at,std::uint32_t tileSizeXY,std::uint32_t view){
    if(r.opcode!=0)throw std::runtime_error("Placement requires a displayed sprite record");
    auto x=r.metadata[at],y=r.metadata[at+1];
    // Unsigned arithmetic deliberately reproduces 32-bit x86 wraparound.
    if(tileSizeXY==2){switch(view){case 1:x+=32;y-=16;break;case 2:y-=32;break;case 3:x-=32;y-=16;break;default:break;}}
    return {signedWord(x),signedWord(y)};
}
AnimationOffset spriteOffset(const assets::AnimationRecord& r,std::uint32_t tileSizeXY,std::uint32_t view){return offset(r,0,tileSizeXY,view);}
AnimationOffset attachmentOffset(const assets::AnimationRecord& r,AttachmentPoint point,std::uint32_t tileSizeXY,std::uint32_t view){
    switch(point){case AttachmentPoint::first:return offset(r,5,tileSizeXY,view);case AttachmentPoint::second:return offset(r,7,tileSizeXY,view);}
    throw std::runtime_error("Unknown attachment point");
}
}
