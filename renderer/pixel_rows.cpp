#include "pixel_rows.hpp"
#include <algorithm>
#include <stdexcept>

namespace mnm::render {
void validateSurfaceFormat(PixelFormat f){
    const bool indexed=f.bits==8 && f.masks==std::array<std::uint32_t,3>{};
    const bool word=f.bits==16 && (f.masks==std::array<std::uint32_t,3>{0xf800,0x7e0,31} ||
                                 f.masks==std::array<std::uint32_t,3>{0x7c00,0x3e0,31});
    const bool rgb=(f.bits==24 || f.bits==32) && f.masks==std::array<std::uint32_t,3>{0xff0000,0xff00,0xff};
    if(!indexed && !word && !rgb)throw std::runtime_error("Unsupported owned native pixel format");
}
namespace {
std::size_t validateRows(const PixelRows& r,PixelFormat f){
    validateSurfaceFormat(f);
    if(r.width<1 || r.height<1 || r.width>2048 || r.height>2048 ||
       r.bytes.size()>64*1024*1024 || r.firstRow>r.bytes.size())
        throw std::runtime_error("Invalid owned pixel row storage");
    // Widen before negation/multiplication: INT_MIN and SIZE_MAX are refusals,
    // never unsigned wraps or unbounded pointer arithmetic.
    const auto pitch=std::int64_t(r.pitch),magnitude=pitch<0?-pitch:pitch;
    const auto active=std::int64_t(r.width)*(f.bits/8);
    if(magnitude<active || magnitude>32768)
        throw std::runtime_error("Pixel pitch outside owned row bounds");
    const auto last=std::int64_t(r.firstRow)+std::int64_t(r.height-1)*pitch;
    const auto low=std::min(std::int64_t(r.firstRow),last),high=std::max(std::int64_t(r.firstRow),last);
    if(low<0 || high>std::int64_t(r.bytes.size())-active)
        throw std::runtime_error("Pixel rows escape owned byte storage");
    return f.bits/8;
}
std::size_t at(const PixelRows& r,int y){return std::size_t(std::int64_t(r.firstRow)+std::int64_t(y)*r.pitch);}
}
Image unpackPixelRows(const PixelRows& r,PixelFormat f){
    const auto size=validateRows(r,f);Image image{r.width,r.height,{}};
    image.pixels.reserve(std::size_t(r.width)*r.height);
    for(int y=0;y<r.height;++y)for(int x=0;x<r.width;++x){
        const auto offset=at(r,y)+std::size_t(x)*size;std::uint32_t value=0;
        for(unsigned b=0;b<size;++b)value|=std::uint32_t(r.bytes[offset+b])<<(b*8);
        image.pixels.push_back(value);
    }
    return image;
}
void packPixelRows(const Image& image,PixelFormat f,PixelRows& r){
    const auto size=validateRows(r,f);
    if(image.width!=r.width || image.height!=r.height || image.pixels.size()!=std::size_t(r.width)*r.height)
        throw std::runtime_error("Pixel output dimensions differ from row layout");
    const auto maximum=f.bits==32?UINT32_MAX:(std::uint32_t{1}<<f.bits)-1;
    for(auto pixel:image.pixels)if(pixel>maximum)throw std::runtime_error("Native pixel exceeds output width");
    for(int y=0;y<r.height;++y)for(int x=0;x<r.width;++x){
        const auto offset=at(r,y)+std::size_t(x)*size;
        const auto value=image.pixels[std::size_t(y)*r.width+x];
        for(unsigned b=0;b<size;++b)r.bytes[offset+b]=std::uint8_t(value>>(b*8));
    }
}
}
