#include "sprite_transforms.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>

namespace mnm::render {
namespace {
void extent(int w,int h) {
    if(w<0 || h<0 || w>2048 || h>2048 || (w==0)!=(h==0) ||
       std::uint64_t(w)*std::uint64_t(h)>1024*1024)
        throw std::length_error("Native sprite transform extent budget");
}
std::int32_t origin(std::int64_t value) {
    if(value<std::numeric_limits<std::int32_t>::min() || value>std::numeric_limits<std::int32_t>::max())
        throw std::overflow_error("Transformed sprite origin exceeds signed32-bit domain");
    return std::int32_t(value);
}
std::int64_t floorDivide(std::int64_t value,int divisor) {
    const auto quotient=value/divisor;
    return quotient-(value%divisor<0);
}
bool covered(const PreparedSpriteFrame& frame,int x,int y) {
    return x>=0 && y>=0 && x<frame.mask.width && y<frame.mask.height &&
           frame.mask.pixels[std::size_t(y)*frame.mask.width+x]!=0;
}
}
assets::Sprite transformSpriteFrame(const assets::Sprite& source,std::size_t index,
                                   const SpriteTransform& request,const SpriteColourTable* colours) {
    const auto& input=source.frames.at(index);
    // Reject before the supplier allocates either prepared plane.
    if(input.width>2048 || input.height>2048) throw std::length_error("Sprite transform input dimensions");
    extent(int(input.width),int(input.height));
    switch(request.kind) {
    case SpriteTransformKind::crop:case SpriteTransformKind::trim:
    case SpriteTransformKind::mirrorX:case SpriteTransformKind::mirrorY:
    case SpriteTransformKind::clockwise:case SpriteTransformKind::halfTurn:
    case SpriteTransformKind::counterclockwise:case SpriteTransformKind::nearestResize:
    case SpriteTransformKind::silhouette:case SpriteTransformKind::outerOutline:break;
    default:throw std::invalid_argument("Unknown native sprite transform");
    }
    auto prepared=prepareSpriteFrame(source,index,colours);
    assets::Sprite result;result.storage=assets::SpriteStorage::rgb565;
    assets::SpriteFrame output;output.name=input.name;
    output.pixels=std::vector<std::uint16_t>{};
    int width=prepared.pixels.width,height=prepared.pixels.height;
    auto ox=std::int64_t(input.originX),oy=std::int64_t(input.originY);
    auto region=request.region;
    if(request.kind==SpriteTransformKind::trim) {
        region={width,height,0,0};
        for(int y=0;y<height;++y) for(int x=0;x<width;++x) if(covered(prepared,x,y)) {
            region.left=std::min(region.left,x);region.top=std::min(region.top,y);
            region.right=std::max(region.right,x+1);region.bottom=std::max(region.bottom,y+1);
        }
        if(region.right==0) width=height=0;
    }
    if(request.kind==SpriteTransformKind::crop || (request.kind==SpriteTransformKind::trim && width)) {
        if(region.left<0 || region.top<0 || region.right<=region.left || region.bottom<=region.top ||
           region.right>width || region.bottom>height)
            throw std::out_of_range("Sprite crop outside selected frame");
        width=region.right-region.left;height=region.bottom-region.top;ox-=region.left;oy-=region.top;
    } else if(request.kind==SpriteTransformKind::nearestResize) {
        extent(request.width,request.height);
        if(!width || !request.width) throw std::invalid_argument("Resize needs nonempty source and destination");
        ox=floorDivide(ox*request.width,width);oy=floorDivide(oy*request.height,height);
        width=request.width;height=request.height;
    } else if(width) {
        switch(request.kind) {
        case SpriteTransformKind::mirrorX:ox=width-1-ox;break;
        case SpriteTransformKind::mirrorY:oy=height-1-oy;break;
        case SpriteTransformKind::clockwise: {
            const auto x=height-1-oy;oy=ox;ox=x;std::swap(width,height);break;
        }
        case SpriteTransformKind::halfTurn:ox=width-1-ox;oy=height-1-oy;break;
        case SpriteTransformKind::counterclockwise: {
            const auto y=width-1-ox;ox=oy;oy=y;std::swap(width,height);break;
        }
        case SpriteTransformKind::outerOutline:width+=2;height+=2;++ox;++oy;break;
        default:break;
        }
    }
    extent(width,height);
    output.width=std::uint32_t(width);output.height=std::uint32_t(height);
    output.originX=width?origin(ox):0;output.originY=height?origin(oy):0;
    auto& words=std::get<std::vector<std::uint16_t>>(output.pixels);
    words.resize(std::size_t(width)*height);output.opaqueMask.resize(words.size());
    for(int y=0;y<height;++y) for(int x=0;x<width;++x) {
        int sx=x,sy=y;
        switch(request.kind) {
        case SpriteTransformKind::crop:case SpriteTransformKind::trim:sx+=region.left;sy+=region.top;break;
        case SpriteTransformKind::mirrorX:sx=prepared.pixels.width-1-x;break;
        case SpriteTransformKind::mirrorY:sy=prepared.pixels.height-1-y;break;
        case SpriteTransformKind::clockwise:sx=y;sy=prepared.pixels.height-1-x;break;
        case SpriteTransformKind::halfTurn:sx=prepared.pixels.width-1-x;sy=prepared.pixels.height-1-y;break;
        case SpriteTransformKind::counterclockwise:sx=prepared.pixels.width-1-y;sy=x;break;
        case SpriteTransformKind::nearestResize:sx=x*prepared.pixels.width/width;sy=y*prepared.pixels.height/height;break;
        case SpriteTransformKind::outerOutline:--sx;--sy;break;
        default:break;
        }
        const auto at=std::size_t(y)*width+x;
        if(covered(prepared,sx,sy)) {
            output.opaqueMask[at]=1;
            words[at]=request.kind==SpriteTransformKind::silhouette?request.colour:
                std::uint16_t(prepared.pixels.pixels[std::size_t(sy)*prepared.pixels.width+sx]);
        } else if(request.kind==SpriteTransformKind::outerOutline &&
                  (covered(prepared,sx-1,sy)||covered(prepared,sx+1,sy)||
                   covered(prepared,sx,sy-1)||covered(prepared,sx,sy+1))) {
            output.opaqueMask[at]=1;words[at]=request.colour;
        }
    }
    result.frames.push_back(std::move(output));return result;
}
}
