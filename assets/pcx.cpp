#include "pcx.hpp"
#include <algorithm>
#include <limits>
#include <new>
#include <stdexcept>

namespace mnm::assets {
namespace {
[[noreturn]] void fail(PcxErrorCode code,std::size_t offset,const char* detail) {
    throw PcxError{code,offset,detail,std::nullopt};
}
std::uint16_t word(const std::vector<std::uint8_t>& b,std::size_t p) {
    return std::uint16_t(b[p]) | (std::uint16_t(b[p+1])<<8);
}
}
PcxResult decodePcx(const std::vector<std::uint8_t>& b,const PcxLimits& limits) try {
    if(b.size()>limits.inputBytes) fail(PcxErrorCode::limitExceeded,0,"PCX input limit exceeded");
    if(b.size()<128) fail(PcxErrorCode::malformedData,b.size(),"Truncated PCX header");
    if(b[0]!=10) fail(PcxErrorCode::invalidFormat,0,"Invalid PCX manufacturer");
    if(b[1]!=5 || b[2]!=1 || b[3]!=8 || b[65]!=1)
        fail(PcxErrorCode::unsupportedEncoding,1,"Expected version 5 RLE 8-bit single-plane PCX");
    PcxImage image;
    image.originX=word(b,4);image.originY=word(b,6);
    if(word(b,8)<image.originX || word(b,10)<image.originY)
        fail(PcxErrorCode::malformedData,4,"Reversed PCX bounds");
    image.width=std::uint32_t(word(b,8))-image.originX+1;
    image.height=std::uint32_t(word(b,10))-image.originY+1;
    const std::uint64_t pixels=std::uint64_t(image.width)*image.height;
    if(image.width>limits.width || image.height>limits.height || pixels>limits.pixels)
        fail(PcxErrorCode::limitExceeded,4,"PCX dimensions or pixel limit exceeded");
    image.horizontalDpi=word(b,12);image.verticalDpi=word(b,14);
    image.bytesPerLine=word(b,66);image.paletteInfo=word(b,68);image.sourceBytes=b.size();
    if(image.bytesPerLine<image.width || (image.bytesPerLine&1))
        fail(PcxErrorCode::malformedData,66,"PCX row stride must be even and cover width");
    if(b.size()<128+769) fail(PcxErrorCode::malformedData,b.size(),"Truncated PCX palette");
    const std::size_t end=b.size()-769;
    if(b[end]!=12) fail(PcxErrorCode::malformedData,end,"Missing terminal PCX palette marker");
    for(std::size_t i=0;i<256;++i)
        std::copy_n(b.begin()+static_cast<std::ptrdiff_t>(end+1+3*i),3,image.palette[i].begin());
    if(pixels>image.indices.max_size()) fail(PcxErrorCode::limitExceeded,4,"PCX output cannot fit in memory");
    image.indices.resize(static_cast<std::size_t>(pixels));
    std::size_t pos=128;
    for(std::uint32_t y=0;y<image.height;++y) {
        std::uint32_t x=0;
        while(x<image.bytesPerLine) {
            if(pos>=end) fail(PcxErrorCode::malformedData,pos,"Truncated PCX scanline");
            const std::size_t packet=pos;
            std::uint8_t value=b[pos++];std::uint32_t count=1;
            if((value&0xc0)==0xc0) {
                count=value&63;
                if(!count) fail(PcxErrorCode::malformedData,packet,"Zero-length PCX run");
                if(pos>=end) fail(PcxErrorCode::malformedData,pos,"Truncated PCX run value");
                value=b[pos++];
            }
            if(count>image.bytesPerLine-x) fail(PcxErrorCode::malformedData,packet,"PCX run crosses scanline");
            if(x<image.width) {
                const auto visible=std::min(count,image.width-x);
                std::fill_n(image.indices.begin()+static_cast<std::ptrdiff_t>(std::uint64_t(y)*image.width+x),visible,value);
            }
            x+=count;
        }
    }
    if(pos!=end) fail(PcxErrorCode::malformedData,pos,"Unexpected data between PCX rows and palette");
    return image;
} catch(const PcxError& error) {return error;}
  catch(const std::bad_alloc&) {return PcxError{PcxErrorCode::limitExceeded,0,"PCX allocation failed",std::nullopt};}
  catch(const std::length_error&) {return PcxError{PcxErrorCode::limitExceeded,0,"PCX allocation too large",std::nullopt};}
PcxResult loadPcx(AssetFile& file,const PcxLimits& limits) {
    if(limits.inputBytes>std::uint64_t(std::numeric_limits<std::int64_t>::max()))
        return PcxError{PcxErrorCode::invalidArgument,0,"PCX input limit exceeds file API range",std::nullopt};
    auto bytes=readWhole(file,static_cast<std::int64_t>(limits.inputBytes));
    if(const auto* error=std::get_if<Error>(&bytes))
        return PcxError{PcxErrorCode::assetInput,0,error->detail,*error};
    return decodePcx(std::get<std::vector<std::uint8_t>>(bytes),limits);
}
}
