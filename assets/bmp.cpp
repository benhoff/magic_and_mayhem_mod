#include "bmp.hpp"
#include <array>
#include <limits>
#include <new>
#include <stdexcept>

namespace mnm::assets {
namespace {
[[noreturn]] void fail(BmpErrorCode code,std::size_t offset,const char* detail) {
    throw BmpError{code,offset,detail,std::nullopt};
}
void malformed(std::size_t at,const char* detail) {fail(BmpErrorCode::malformedData,at,detail);}
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
struct Channel {
    std::uint32_t mask=0,maximum=0;unsigned shift=0;
    std::uint8_t read(std::uint32_t value) const {
        return static_cast<std::uint8_t>(std::uint64_t((value&mask)>>shift)*255/maximum);
    }
};
Channel channel(std::uint32_t mask,unsigned bits,std::size_t at) {
    if(!mask || (bits==16 && mask>0xffff))malformed(at,"BMP mask is empty or exceeds pixel depth");
    Channel c;c.mask=mask;
    while(!(mask&1)){mask>>=1;++c.shift;}
    if((mask&(mask+1))!=0)malformed(at,"BMP mask is not contiguous");
    c.maximum=mask;return c;
}
}
BmpResult decodeBmp(const std::vector<std::uint8_t>& b,const BmpLimits& limits) try {
    if(b.size()>limits.inputBytes)fail(BmpErrorCode::limitExceeded,0,"BMP input limit exceeded");
    if(b.size()<18)malformed(b.size(),"Truncated BMP file/header size");
    if(b[0]!='B'||b[1]!='M')fail(BmpErrorCode::invalidFormat,0,"Invalid BMP signature");
    const auto header=dword(b,14);const bool core=header==12;
    if(!core && header!=40)fail(BmpErrorCode::unsupportedEncoding,14,"Expected core or information BMP header");
    const std::size_t end=14+header;
    if(b.size()<end)malformed(b.size(),"Truncated BMP header");
    if(word(b,6)||word(b,8))malformed(6,"Nonzero BMP reserved fields");
    if(word(b,core?22:26)!=1)malformed(core?22:26,"BMP must have one plane");
    const unsigned bits=word(b,core?24:28),compression=core?0:dword(b,30);
    const bool indexed=bits==1||bits==4||bits==8;
    const bool rle=(bits==8&&compression==1)||(bits==4&&compression==2);
    const bool masks=(bits==16||bits==32)&&compression==3;
    if((core && !(indexed||bits==24)) ||
       (!core && !((compression==0&&(indexed||bits==16||bits==24||bits==32))||rle||masks)))
        fail(BmpErrorCode::unsupportedEncoding,core?24:28,"Unsupported BMP pixel encoding");
    const auto width=core?std::int32_t(word(b,18)):signedDword(b,18);
    const auto height=core?std::int32_t(word(b,20)):signedDword(b,22);
    if(width<=0||height==0||height==std::numeric_limits<std::int32_t>::min())malformed(18,"Invalid BMP dimensions");
    if(rle&&height<0)malformed(22,"Compressed BMP cannot be top-down");
    BmpImage image;
    image.width=static_cast<std::uint32_t>(width);image.sourceTopDown=height<0;
    image.height=static_cast<std::uint32_t>(height<0?-height:height);
    const std::uint64_t pixels=std::uint64_t(image.width)*image.height;
    if(image.width>limits.width||image.height>limits.height||pixels>limits.pixels||pixels>limits.decodedBytes/3)
        fail(BmpErrorCode::limitExceeded,18,"BMP dimensions or decoded size limit exceeded");
    const std::uint64_t stride=((std::uint64_t(image.width)*bits+31)/32)*4;
    if(stride>std::numeric_limits<std::uint32_t>::max())fail(BmpErrorCode::limitExceeded,18,"BMP stride exceeds API range");
    image.pixelOffset=dword(b,10);image.rowStride=static_cast<std::uint32_t>(stride);
    if(!core){
        image.declaredImageBytes=dword(b,34);image.colorsUsed=dword(b,46);image.colorsImportant=dword(b,50);
        image.horizontalPixelsPerMeter=signedDword(b,38);image.verticalPixelsPerMeter=signedDword(b,42);
    }
    const unsigned paletteCount=indexed?(image.colorsUsed?image.colorsUsed:1U<<bits):image.colorsUsed;
    if(indexed&&paletteCount>(1U<<bits))malformed(46,"BMP palette exceeds indexed pixel range");
    const std::uint64_t paletteAt=end+(masks?12:0);
    const std::uint64_t headerEnd=paletteAt+std::uint64_t(paletteCount)*(core?3:4);
    const std::uint64_t declared=dword(b,2);
    const std::uint64_t encoded=rle?image.declaredImageBytes:stride*image.height;
    if(image.pixelOffset<headerEnd)malformed(10,"BMP pixels overlap header, masks or color table");
    if(declared>b.size()||image.pixelOffset>declared||encoded>declared-image.pixelOffset)
        malformed(2,"BMP declared file extent does not contain pixels");
    if(rle&&!encoded)malformed(34,"Compressed BMP requires an image extent");
    if(!rle&&image.declaredImageBytes&&image.declaredImageBytes!=encoded)
        malformed(34,"BMP declared image size disagrees with row extent");
    std::array<Channel,3> channels{};
    if(bits==16||bits==32){
        const std::array<std::uint32_t,3> defaults=bits==16?
            std::array<std::uint32_t,3>{0x7c00,0x3e0,0x1f}:std::array<std::uint32_t,3>{0xff0000,0xff00,0xff};
        for(unsigned k=0;k<3;++k)channels[k]=channel(masks?dword(b,end+k*4):defaults[k],bits,end+k*4);
        if((channels[0].mask&channels[1].mask)||(channels[0].mask&channels[2].mask)||(channels[1].mask&channels[2].mask))
            malformed(end,"BMP color masks overlap");
    }
    std::array<std::array<std::uint8_t,3>,256> palette{};
    if(indexed)for(unsigned i=0;i<paletteCount;++i){
        const auto p=static_cast<std::size_t>(paletteAt+i*(core?3:4));palette[i]={b[p+2],b[p+1],b[p]};
    }
    image.sourceBytes=b.size();
    if(pixels>image.rgb.max_size()/3)fail(BmpErrorCode::limitExceeded,18,"BMP output cannot fit in memory");
    image.rgb.resize(static_cast<std::size_t>(pixels*3));
    const auto setIndex=[&](unsigned x,unsigned y,unsigned index,std::size_t at){
        if(index>=paletteCount)malformed(at,"BMP pixel references an absent palette entry");
        const auto to=(std::size_t(image.height-1-y)*image.width+x)*3;
        for(unsigned k=0;k<3;++k)image.rgb[to+k]=palette[index][k];
    };
    if(rle){
        // Explicit native policy: skipped RLE cells have palette index zero.
        for(std::size_t i=0;i<image.rgb.size();++i)image.rgb[i]=palette[0][i%3];
        std::size_t p=image.pixelOffset;const auto stop=p+static_cast<std::size_t>(encoded);
        unsigned x=0,y=0;bool complete=false;
        const auto need=[&](std::size_t n){if(n>stop-p)malformed(p,"Truncated BMP RLE command");};
        while(p<stop){
            need(2);const auto at=p;const unsigned count=b[p++],value=b[p++];
            if(count){
                if(y>=image.height||count>image.width-x)malformed(at,"BMP RLE run exceeds row");
                for(unsigned i=0;i<count;++i)setIndex(x++,y,bits==8?value:(value>>(i%2?0:4))&15,at);
            }else if(value==0){
                if(y>=image.height)malformed(at,"BMP RLE line exceeds image");
                x=0;++y;
            }else if(value==1){complete=true;break;}
            else if(value==2){
                need(2);const unsigned dx=b[p++],dy=b[p++];
                if(y>=image.height||dx>image.width-x||dy>=image.height-y)malformed(at,"BMP RLE delta exceeds image");
                x+=dx;y+=dy;
            }else{
                const unsigned bytes=bits==8?value:(value+1)/2,padded=(bytes+1)&~1U;
                need(padded);
                if(y>=image.height||value>image.width-x)malformed(at,"BMP RLE literal exceeds row");
                for(unsigned i=0;i<value;++i)setIndex(x++,y,bits==8?b[p+i]:(b[p+i/2]>>(i%2?0:4))&15,p+(bits==8?i:i/2));
                p+=padded;
            }
        }
        if(!complete)malformed(p,"BMP RLE end marker is missing");
    }else for(std::uint32_t y=0;y<image.height;++y){
        const auto sourceY=image.sourceTopDown?y:image.height-1-y;
        const auto source=std::uint64_t(image.pixelOffset)+std::uint64_t(sourceY)*stride;
        for(std::uint32_t x=0;x<image.width;++x){
            const auto from=static_cast<std::size_t>(source+std::uint64_t(x)*bits/8);
            const auto to=(std::size_t(y)*image.width+x)*3;
            if(indexed){
                const unsigned index=bits==8?b[from]:bits==4?(b[from]>>(x%2?0:4))&15:(b[from]>>(7-x%8))&1;
                if(index>=paletteCount)malformed(from,"BMP pixel references an absent palette entry");
                for(unsigned k=0;k<3;++k)image.rgb[to+k]=palette[index][k];
            }else if(bits==24){image.rgb[to]=b[from+2];image.rgb[to+1]=b[from+1];image.rgb[to+2]=b[from];}
            else{
                const auto pixel=bits==16?std::uint32_t(word(b,from)):dword(b,from);
                for(unsigned k=0;k<3;++k)image.rgb[to+k]=channels[k].read(pixel);
            }
        }
    }
    return image;
} catch(const BmpError& error){return error;}
  catch(const std::bad_alloc&){return BmpError{BmpErrorCode::limitExceeded,0,"BMP allocation failed",std::nullopt};}
  catch(const std::length_error&){return BmpError{BmpErrorCode::limitExceeded,0,"BMP allocation too large",std::nullopt};}
BmpResult loadBmp(AssetFile& file,const BmpLimits& limits){
    if(limits.inputBytes>std::uint64_t(std::numeric_limits<std::int64_t>::max()))
        return BmpError{BmpErrorCode::invalidArgument,0,"BMP input limit exceeds file API range",std::nullopt};
    auto bytes=readWhole(file,static_cast<std::int64_t>(limits.inputBytes));
    if(const auto* error=std::get_if<Error>(&bytes))return BmpError{BmpErrorCode::assetInput,0,error->detail,*error};
    return decodeBmp(std::get<std::vector<std::uint8_t>>(bytes),limits);
}
}
