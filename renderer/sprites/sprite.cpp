#include "sprite.hpp"
#include <limits>
#include <algorithm>
#include <stdexcept>

namespace mnm::render {
PreparedSpriteFrame prepareSpriteFrame(const assets::Sprite& sprite,std::size_t index,const SpriteColourTable* colours,const std::function<void()>& checkpoint,bool retainIndices){
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
        if(i%std::max(1u,f.width)==0 && checkpoint)checkpoint();
        if(f.opaqueMask[i]>1)throw std::runtime_error("Sprite mask must contain zero or one");
        mask.pixels[i]=f.opaqueMask[i];
        if(indices){const auto c=sprite.palettes[*f.paletteIndex][(*indices)[i]];
            pixels.pixels[i]=retainIndices?(*indices)[i]:colours?(*colours)[(*indices)[i]]:(std::uint32_t(c.red>>3)<<11)|(std::uint32_t(c.green>>2)<<5)|(c.blue>>3);
        }else pixels.pixels[i]=(*words)[i];
    }
    // Keep encoded run endpoints separate from adjacent opaque pixels. A zero
    // transparent skip can separate two opaque runs with distinct endpoints.
    for(const auto& run:f.opaqueRuns){
        if(run.row>=f.height||run.left>=run.right||run.right>f.width)
            throw std::runtime_error("Sprite opaque run outside frame");
        const auto base=std::size_t(run.row)*f.width;
        if(!f.opaqueMask[base+run.left]||!f.opaqueMask[base+run.right-1])
            throw std::runtime_error("Sprite run endpoint is transparent");
        mask.pixels[base+run.left]=mask.pixels[base+run.right-1]=3;
    }
    PreparedSpriteFrame result{std::move(pixels),std::move(mask),f.originX,f.originY,{}};
    if(retainIndices){
        if(!indices)throw std::invalid_argument("Indexed preparation requires indices");
        result.palette.emplace();
        for(unsigned i=0;i<256;++i){const auto c=sprite.palettes[*f.paletteIndex][i];
            (*result.palette)[i]=(std::uint16_t(c.red>>3)<<11)|(std::uint16_t(c.green>>2)<<5)|(c.blue>>3);}
    }
    return result;
}
UploadedSpriteFrame::UploadedSpriteFrame(GlBlitter& renderer,const assets::Sprite& sprite,std::size_t index,const SpriteColourTable* colours)
    :UploadedSpriteFrame(renderer,prepareSpriteFrame(sprite,index,colours)){
    auto bytes=std::numeric_limits<std::size_t>::max();while(!advanceUpload(bytes)){}
}
UploadedSpriteFrame::UploadedSpriteFrame(GlBlitter& renderer,PreparedSpriteFrame prepared):renderer_(renderer){
    if(prepared.palette)throw std::invalid_argument("Indexed preparation requires atlas upload");
    const auto& p=prepared.pixels;const auto& m=prepared.mask;
    if(p.width<0||p.height<0||p.width>2048||p.height>2048||(p.width==0)!=(p.height==0)||
       p.width!=m.width||p.height!=m.height||p.pixels.size()!=std::size_t(p.width)*p.height||m.pixels.size()!=p.pixels.size())
        throw std::runtime_error("Invalid prepared sprite planes");
    width_=p.width;height_=p.height;originX_=prepared.originX;originY_=prepared.originY;
    if(!width_)return;
    pixels_=renderer_.allocate(width_,height_,spriteFormat);
    try{mask_=renderer_.allocate(width_,height_,{8,{}});}catch(...){renderer_.destroy(pixels_);pixels_=0;throw;}
    pending_=std::move(prepared);
}
bool UploadedSpriteFrame::advanceUpload(std::size_t& availableBytes){
    if(!pending_)return true;
    const auto rowBytes=std::size_t(width_)*sizeof(std::uint32_t);
    const auto rows=int(std::min(availableBytes/rowBytes,std::size_t(height_-uploadRow_)));
    if(!rows)return false;
    if(uploadPlane_){const auto begin=pending_->mask.pixels.begin()+std::size_t(uploadRow_)*width_;
        if(std::any_of(begin,begin+std::size_t(rows)*width_,[](auto value){return value>3||value==2;}))throw std::runtime_error("Sprite coverage must contain zero, one or three");}
    renderer_.updateRows(uploadPlane_?mask_:pixels_,uploadPlane_?pending_->mask:pending_->pixels,uploadRow_,rows);
    availableBytes-=std::size_t(rows)*rowBytes;uploadRow_+=rows;
    if(uploadRow_==height_){uploadRow_=0;++uploadPlane_;}
    if(uploadPlane_==2)pending_.reset();
    return ready();
}
UploadedSpriteFrame::~UploadedSpriteFrame(){
    // Wrong-thread use and destroying the renderer first violate this API's lifetime contract.
    if(mask_)renderer_.destroy(mask_);
    if(pixels_)renderer_.destroy(pixels_);
}
void UploadedSpriteFrame::draw(SurfaceId destination,int anchorX,int anchorY,const SpriteComposite& composite){
    if(!ready())throw std::runtime_error("Sprite upload is incomplete");
    if(empty())return;
    const auto x=std::int64_t(anchorX)-originX_,y=std::int64_t(anchorY)-originY_;
    if(x<0 || y<0 || x>std::numeric_limits<int>::max() || y>std::numeric_limits<int>::max())
        throw std::runtime_error("Sprite placement outside renderer coordinates");
    renderer_.composite(pixels_,destination,{0,0,width_,height_},int(x),int(y),mask_,composite);
}
void UploadedSpriteFrame::drawClipped(SurfaceId destination,int anchorX,int anchorY,Rect viewport,const SpriteComposite& composite){
    if(viewport.left<0 || viewport.top<0 || viewport.right<=viewport.left || viewport.bottom<=viewport.top)
        throw std::invalid_argument("Invalid sprite clipping viewport");
    if(!ready())throw std::runtime_error("Sprite upload is incomplete");
    if(empty())return;
    const auto x=std::int64_t(anchorX)-originX_,y=std::int64_t(anchorY)-originY_;
    const auto left=std::max<std::int64_t>(x,viewport.left),top=std::max<std::int64_t>(y,viewport.top);
    const auto right=std::min<std::int64_t>(x+width_,viewport.right),bottom=std::min<std::int64_t>(y+height_,viewport.bottom);
    if(left>=right || top>=bottom)return;
    renderer_.composite(pixels_,destination,{int(left-x),int(top-y),int(right-x),int(bottom-y)},int(left),int(top),mask_,composite);
}

void UploadedSpriteFrame::drawRegion(SurfaceId destination,Rect source,int x,int y,const SpriteComposite& composite){
    if(!ready())throw std::runtime_error("Sprite upload is incomplete");
    if(source.left<0||source.top<0||source.right<=source.left||source.bottom<=source.top||source.right>width_||source.bottom>height_)
        throw std::invalid_argument("Sprite region outside source");
    renderer_.composite(pixels_,destination,source,x,y,mask_,composite);
}

}
