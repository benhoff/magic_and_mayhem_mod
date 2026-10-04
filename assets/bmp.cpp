#include "bmp.hpp"
#include <limits>
#include <new>
#include <stdexcept>

namespace mnm::assets {
namespace {
[[noreturn]] void fail(BmpErrorCode code,std::size_t offset,const char* detail) {
    throw BmpError{code,offset,detail,std::nullopt};
}
std::uint16_t word(const std::vector<std::uint8_t>& b,std::size_t p) {
    return std::uint16_t(b[p]) | (std::uint16_t(b[p+1])<<8);
}
std::uint32_t dword(const std::vector<std::uint8_t>& b,std::size_t p) {
    return std::uint32_t(word(b,p)) | (std::uint32_t(word(b,p+2))<<16);
}
std::int32_t signedDword(const std::vector<std::uint8_t>& b,std::size_t p) {
    const auto raw=dword(b,p);
    const std::int64_t value=raw<0x80000000U ? std::int64_t(raw) : std::int64_t(raw)-0x100000000LL;
    return static_cast<std::int32_t>(value);
}
}
BmpResult decodeBmp(const std::vector<std::uint8_t>& b,const BmpLimits& limits) try {
    if(b.size()>limits.inputBytes) fail(BmpErrorCode::limitExceeded,0,"BMP input limit exceeded");
    if(b.size()<18) fail(BmpErrorCode::malformedData,b.size(),"Truncated BMP file/header size");
    if(b[0]!='B' || b[1]!='M') fail(BmpErrorCode::invalidFormat,0,"Invalid BMP signature");
    if(dword(b,14)!=40) fail(BmpErrorCode::unsupportedEncoding,14,"Expected 40-byte BITMAPINFOHEADER");
    if(b.size()<54) fail(BmpErrorCode::malformedData,b.size(),"Truncated BMP information header");
    if(word(b,6) || word(b,8)) fail(BmpErrorCode::malformedData,6,"Nonzero BMP reserved fields");
    if(word(b,26)!=1) fail(BmpErrorCode::malformedData,26,"BMP must have one plane");
    if(word(b,28)!=24 || dword(b,30)!=0)
        fail(BmpErrorCode::unsupportedEncoding,28,"Expected uncompressed 24-bit BMP");
    const auto width=signedDword(b,18), height=signedDword(b,22);
    if(width<=0 || height==0 || height==std::numeric_limits<std::int32_t>::min())
        fail(BmpErrorCode::malformedData,18,"Invalid BMP dimensions");
    BmpImage image;
    image.width=static_cast<std::uint32_t>(width);
    image.sourceTopDown=height<0;
    image.height=static_cast<std::uint32_t>(height<0 ? -height : height);
    const std::uint64_t pixels=std::uint64_t(image.width)*image.height;
    if(image.width>limits.width || image.height>limits.height || pixels>limits.pixels || pixels>limits.decodedBytes/3)
        fail(BmpErrorCode::limitExceeded,18,"BMP dimensions or decoded size limit exceeded");
    const std::uint64_t stride=(std::uint64_t(image.width)*3+3)&~std::uint64_t(3);
    if(stride>std::numeric_limits<std::uint32_t>::max()) fail(BmpErrorCode::limitExceeded,18,"BMP stride exceeds API range");
    const std::uint64_t encoded=stride*image.height;
    image.pixelOffset=dword(b,10);image.rowStride=static_cast<std::uint32_t>(stride);
    image.declaredImageBytes=dword(b,34);image.colorsUsed=dword(b,46);image.colorsImportant=dword(b,50);
    const std::uint64_t headerEnd=54+std::uint64_t(image.colorsUsed)*4;
    const std::uint64_t declared=dword(b,2);
    if(image.pixelOffset<headerEnd) fail(BmpErrorCode::malformedData,10,"BMP pixels overlap header or optional color table");
    if(declared>b.size() || image.pixelOffset>declared || encoded>declared-image.pixelOffset)
        fail(BmpErrorCode::malformedData,2,"BMP declared file extent does not contain pixels");
    if(image.declaredImageBytes && image.declaredImageBytes!=encoded)
        fail(BmpErrorCode::malformedData,34,"BMP declared image size disagrees with row extent");
    image.horizontalPixelsPerMeter=signedDword(b,38);image.verticalPixelsPerMeter=signedDword(b,42);
    image.sourceBytes=b.size();
    if(pixels>image.rgb.max_size()/3) fail(BmpErrorCode::limitExceeded,18,"BMP output cannot fit in memory");
    image.rgb.resize(static_cast<std::size_t>(pixels*3));
    for(std::uint32_t y=0;y<image.height;++y) {
        const auto sourceY=image.sourceTopDown ? y : image.height-1-y;
        const auto source=std::uint64_t(image.pixelOffset)+std::uint64_t(sourceY)*stride;
        const auto target=std::uint64_t(y)*image.width*3;
        for(std::uint32_t x=0;x<image.width;++x) {
            const auto from=static_cast<std::size_t>(source+std::uint64_t(x)*3);
            const auto to=static_cast<std::size_t>(target+std::uint64_t(x)*3);
            image.rgb[to]=b[from+2];image.rgb[to+1]=b[from+1];image.rgb[to+2]=b[from];
        }
    }
    return image;
} catch(const BmpError& error) {return error;}
  catch(const std::bad_alloc&) {return BmpError{BmpErrorCode::limitExceeded,0,"BMP allocation failed",std::nullopt};}
  catch(const std::length_error&) {return BmpError{BmpErrorCode::limitExceeded,0,"BMP allocation too large",std::nullopt};}
BmpResult loadBmp(AssetFile& file,const BmpLimits& limits) {
    if(limits.inputBytes>std::uint64_t(std::numeric_limits<std::int64_t>::max()))
        return BmpError{BmpErrorCode::invalidArgument,0,"BMP input limit exceeds file API range",std::nullopt};
    auto bytes=readWhole(file,static_cast<std::int64_t>(limits.inputBytes));
    if(const auto* error=std::get_if<Error>(&bytes))
        return BmpError{BmpErrorCode::assetInput,0,error->detail,*error};
    return decodeBmp(std::get<std::vector<std::uint8_t>>(bytes),limits);
}
}
