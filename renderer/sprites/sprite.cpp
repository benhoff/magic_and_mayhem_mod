#include "sprite.hpp"
#include <limits>
#include <algorithm>
#include <stdexcept>

namespace mnm::render {
UploadedSpriteFrame::UploadedSpriteFrame(GlBlitter& renderer,const assets::Sprite& sprite,std::size_t index,const SpriteColourTable* colours)
    :renderer_(renderer){
    if(index>=sprite.frames.size())throw std::runtime_error("Sprite frame index out of range");
    const auto& f=sprite.frames[index];
    if(f.width>2048 || f.height>2048 || (f.width==0)!=(f.height==0))
        throw std::runtime_error("Unsupported sprite dimensions");
    const auto count=std::size_t(f.width)*f.height;
    if(f.opaqueMask.size()!=count)throw std::runtime_error("Sprite mask extent differs");
    const auto* indices=std::get_if<std::vector<std::uint8_t>>(&f.pixels);
    const auto* words=std::get_if<std::vector<std::uint16_t>>(&f.pixels);
    if(sprite.storage==assets::SpriteStorage::indexed8){
        if(!indices || indices->size()!=count || !f.paletteIndex || *f.paletteIndex>=sprite.palettes.size())
            throw std::runtime_error("Invalid indexed sprite pixels/palette");
    }else if(sprite.storage==assets::SpriteStorage::rgb565){
        if(colours)throw std::invalid_argument("Palette override requires indexed sprite");
        if(!words || words->size()!=count || f.paletteIndex)throw std::runtime_error("Invalid RGB565 sprite pixels");
    }else throw std::runtime_error("Unknown sprite storage");
    Image pixels{int(f.width),int(f.height),std::vector<std::uint32_t>(count)};
    Image mask{int(f.width),int(f.height),std::vector<std::uint32_t>(count)};
    for(std::size_t i=0;i<count;++i){
        if(f.opaqueMask[i]>1)throw std::runtime_error("Sprite mask must contain zero or one");
        mask.pixels[i]=f.opaqueMask[i];
        if(indices){const auto c=sprite.palettes[*f.paletteIndex][(*indices)[i]];
            pixels.pixels[i]=colours?(*colours)[(*indices)[i]]:(std::uint32_t(c.red>>3)<<11)|(std::uint32_t(c.green>>2)<<5)|(c.blue>>3);
        }else pixels.pixels[i]=(*words)[i];
    }
    width_=int(f.width);height_=int(f.height);originX_=f.originX;originY_=f.originY;
    if(!count)return;
    pixels_=renderer_.create(pixels,spriteFormat);
    try{mask_=renderer_.create(mask,{8,{}});}catch(...){renderer_.destroy(pixels_);pixels_=0;throw;}
}
UploadedSpriteFrame::~UploadedSpriteFrame(){
    // Wrong-thread use and destroying the renderer first violate this API's lifetime contract.
    if(mask_)renderer_.destroy(mask_);
    if(pixels_)renderer_.destroy(pixels_);
}
void UploadedSpriteFrame::draw(SurfaceId destination,int anchorX,int anchorY,const SpriteComposite& composite){
    if(empty())return;
    const auto x=std::int64_t(anchorX)-originX_,y=std::int64_t(anchorY)-originY_;
    if(x<0 || y<0 || x>std::numeric_limits<int>::max() || y>std::numeric_limits<int>::max())
        throw std::runtime_error("Sprite placement outside renderer coordinates");
    renderer_.composite(pixels_,destination,{0,0,width_,height_},int(x),int(y),mask_,composite);
}
void UploadedSpriteFrame::drawClipped(SurfaceId destination,int anchorX,int anchorY,Rect viewport,const SpriteComposite& composite){
    if(viewport.left<0 || viewport.top<0 || viewport.right<=viewport.left || viewport.bottom<=viewport.top)
        throw std::invalid_argument("Invalid sprite clipping viewport");
    if(empty())return;
    const auto x=std::int64_t(anchorX)-originX_,y=std::int64_t(anchorY)-originY_;
    const auto left=std::max<std::int64_t>(x,viewport.left),top=std::max<std::int64_t>(y,viewport.top);
    const auto right=std::min<std::int64_t>(x+width_,viewport.right),bottom=std::min<std::int64_t>(y+height_,viewport.bottom);
    if(left>=right || top>=bottom)return;
    renderer_.composite(pixels_,destination,{int(left-x),int(top-y),int(right-x),int(bottom-y)},int(left),int(top),mask_,composite);
}

}
