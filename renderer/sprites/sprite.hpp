#pragma once
#include "blit.hpp"
#include "sprite_loader.hpp"

namespace mnm::render {
inline constexpr PixelFormat spriteFormat{16,{0xf800,0x07e0,0x001f}};
// An owned RGB565 upload and separate coverage texture. Indexed source colours
// use embedded RGB >> 3/2/3, without original lighting/palette-chain construction.
// Renderer must outlive this object; creation/drawing/destruction use its GUI thread.
class UploadedSpriteFrame final {
public:
    UploadedSpriteFrame(GlBlitter& renderer,const assets::Sprite& sprite,std::size_t frame);
    ~UploadedSpriteFrame();
    UploadedSpriteFrame(const UploadedSpriteFrame&)=delete;
    UploadedSpriteFrame& operator=(const UploadedSpriteFrame&)=delete;
    // Top-left = anchor - signed origin. Fully in-bounds draws only, no clipping.
    // Empty frames are no-ops and allocate no surfaces.
    void draw(SurfaceId destination,int anchorX,int anchorY);
    // Native destination clipping policy; viewport must lie within destination.
    void drawClipped(SurfaceId destination,int anchorX,int anchorY,Rect viewport);
    bool empty() const {return width_==0;}
private:
    GlBlitter& renderer_;
    SurfaceId pixels_=0,mask_=0;
    int width_=0,height_=0;
    std::int32_t originX_=0,originY_=0;
};
}
