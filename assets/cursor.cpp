#include "cursor.hpp"
#include <limits>
#include <new>
#include <stdexcept>

namespace mnm::assets {
namespace {
[[noreturn]] void fail(CursorErrorCode code,std::size_t offset,std::optional<std::uint32_t> image,std::string detail) {
    throw CursorError{code,offset,image,std::move(detail),std::nullopt};
}
struct Reader {
    const std::vector<std::uint8_t>& bytes;
    std::size_t begin=0,end=0;
    std::optional<std::uint32_t> image;
    void require(std::size_t offset,std::size_t count) const {
        if(offset<begin || offset>end || count>end-offset)
            fail(CursorErrorCode::malformedData,offset,image,"truncated or out-of-range cursor block");
    }
    std::uint8_t u8(std::size_t p) const {require(p,1);return bytes[p];}
    std::uint16_t u16(std::size_t p) const {require(p,2);return std::uint16_t(bytes[p])|(std::uint16_t(bytes[p+1])<<8);}
    std::uint32_t u32(std::size_t p) const {
        require(p,4);return std::uint32_t(bytes[p])|(std::uint32_t(bytes[p+1])<<8)|(std::uint32_t(bytes[p+2])<<16)|(std::uint32_t(bytes[p+3])<<24);
    }
};
void budget(std::uint64_t amount,std::uint64_t limit,std::uint64_t& total,std::size_t offset,std::uint32_t image) {
    if(amount>limit || total>limit-amount) fail(CursorErrorCode::limitExceeded,offset,image,"cursor aggregate limit exceeded");
    total+=amount;
}
}
CursorResult decodeCursor(const std::vector<std::uint8_t>& bytes,const CursorLimits& limits) {
    try {
        if(bytes.size()>limits.inputBytes) fail(CursorErrorCode::limitExceeded,0,std::nullopt,"cursor input limit exceeded");
        const Reader file{bytes,0,bytes.size(),std::nullopt};file.require(0,6);
        if(file.u16(0)!=0 || file.u16(2)!=2) fail(CursorErrorCode::invalidFormat,0,std::nullopt,"expected CUR reserved=0, type=2");
        const auto count=file.u16(4);
        if(!count) fail(CursorErrorCode::malformedData,4,std::nullopt,"empty cursor directory");
        if(count>limits.images) fail(CursorErrorCode::limitExceeded,4,std::nullopt,"cursor image count limit exceeded");
        const std::size_t directoryEnd=6+std::size_t(count)*16;file.require(6,std::size_t(count)*16);
        CursorAsset asset;asset.sourceBytes=bytes.size();asset.images.reserve(count);
        std::uint64_t totalPixels=0,totalDecoded=0;
        for(std::uint32_t i=0;i<count;++i) {
            const std::size_t d=6+std::size_t(i)*16;CursorImage image;
            image.width=file.u8(d)?file.u8(d):256;image.height=file.u8(d+1)?file.u8(d+1):256;
            image.directoryColorCount=file.u8(d+2);
            if(file.u8(d+3)!=0) fail(CursorErrorCode::malformedData,d+3,i,"nonzero directory reserved byte");
            if(image.width>limits.width || image.height>limits.height) fail(CursorErrorCode::limitExceeded,d,i,"cursor dimension limit exceeded");
            image.hotspotX=file.u16(d+4);image.hotspotY=file.u16(d+6);
            if(image.hotspotX>=image.width || image.hotspotY>=image.height) fail(CursorErrorCode::malformedData,d+4,i,"cursor hotspot outside image");
            image.encodedSize=file.u32(d+8);image.sourceOffset=file.u32(d+12);
            const std::size_t p=image.sourceOffset;
            if(p<directoryEnd) fail(CursorErrorCode::malformedData,d+12,i,"cursor image overlaps directory");
            const Reader payload{bytes,p,bytes.size(),i};payload.require(p,image.encodedSize);
            const Reader dib{bytes,p,p+image.encodedSize,i};dib.require(p,4);
            if(dib.u32(p)!=40) fail(CursorErrorCode::unsupportedEncoding,p,i,"only 40-byte BITMAPINFOHEADER supported");
            dib.require(p,40);
            if(dib.u32(p+4)!=image.width || dib.u32(p+8)!=std::uint32_t(image.height)*2)
                fail(CursorErrorCode::malformedData,p+4,i,"DIB and directory dimensions disagree; bottom-up doubled height required");
            if(dib.u16(p+12)!=1) fail(CursorErrorCode::malformedData,p+12,i,"DIB requires one plane");
            image.bitDepth=dib.u16(p+14);
            if((image.bitDepth!=1 && image.bitDepth!=8) || dib.u32(p+16)!=0)
                fail(CursorErrorCode::unsupportedEncoding,p+14,i,"only uncompressed 1/8-bpp indexed cursor images supported");
            const auto maximumColors=std::uint32_t(1)<<image.bitDepth;
            auto colors=dib.u32(p+32);if(!colors) colors=maximumColors;
            if(colors>maximumColors) fail(CursorErrorCode::malformedData,p+32,i,"palette exceeds bit-depth capacity");
            // Directory colour count is metadata, not a reliable palette size:
            // all installed files use zero, including their 1-bpp images.
            const std::size_t xorStride=((std::size_t(image.width)*image.bitDepth+31)/32)*4;
            const std::size_t andStride=((std::size_t(image.width)+31)/32)*4;
            const std::size_t xorBytes=xorStride*image.height,andBytes=andStride*image.height;
            const std::size_t pixelStart=p+40+std::size_t(colors)*4;
            dib.require(p+40,std::size_t(colors)*4);dib.require(pixelStart,xorBytes+andBytes);
            const auto declaredBitmapBytes=dib.u32(p+20);
            if(declaredBitmapBytes && declaredBitmapBytes!=xorBytes && declaredBitmapBytes!=xorBytes+andBytes)
                fail(CursorErrorCode::malformedData,p+20,i,"unexpected DIB image-size field");
            const std::uint64_t pixels=std::uint64_t(image.width)*image.height;
            budget(pixels,limits.pixels,totalPixels,p,i);budget(pixels*2+std::uint64_t(colors)*4,limits.decodedBytes,totalDecoded,p,i);
            image.palette.reserve(colors);
            for(std::uint32_t c=0;c<colors;++c) {
                const auto q=p+40+std::size_t(c)*4;
                image.palette.push_back({dib.u8(q+2),dib.u8(q+1),dib.u8(q),dib.u8(q+3)});
            }
            image.xorIndices.resize(static_cast<std::size_t>(pixels));image.andMask.resize(static_cast<std::size_t>(pixels));
            for(std::size_t y=0;y<image.height;++y) for(std::size_t x=0;x<image.width;++x) {
                const auto row=image.height-1-y,index=y*image.width+x;
                const std::uint32_t value=image.bitDepth==8 ? bytes[pixelStart+row*xorStride+x]
                    : (bytes[pixelStart+row*xorStride+x/8]>>(7-x%8))&1;
                if(value>=colors) fail(CursorErrorCode::malformedData,pixelStart+row*xorStride+(x*image.bitDepth)/8,i,"cursor palette index out of range");
                image.xorIndices[index]=static_cast<std::uint8_t>(value);
                image.andMask[index]=(bytes[pixelStart+xorBytes+row*andStride+x/8]>>(7-x%8))&1;
            }
            asset.images.push_back(std::move(image));
        }
        return asset;
    } catch(const CursorError& error) {return error;}
    catch(const std::bad_alloc&) {return CursorError{CursorErrorCode::limitExceeded,0,std::nullopt,"cursor allocation failed",std::nullopt};}
    catch(const std::length_error&) {return CursorError{CursorErrorCode::limitExceeded,0,std::nullopt,"cursor allocation size exceeded",std::nullopt};}
}
CursorResult loadCursor(AssetFile& file,const CursorLimits& limits) {
    if(limits.inputBytes>std::uint64_t(std::numeric_limits<std::int64_t>::max()))
        return CursorError{CursorErrorCode::invalidArgument,0,std::nullopt,"input limit exceeds AssetFile range",std::nullopt};
    auto result=readWhole(file,static_cast<std::int64_t>(limits.inputBytes));
    if(const auto* error=std::get_if<Error>(&result)) return CursorError{CursorErrorCode::assetInput,0,std::nullopt,"cursor asset read failed",*error};
    return decodeCursor(std::get<std::vector<std::uint8_t>>(result),limits);
}
}
