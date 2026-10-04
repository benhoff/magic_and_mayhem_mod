#include "fp.hpp"
#include <limits>
#include <new>
#include <stdexcept>

namespace mnm::assets {
namespace {
[[noreturn]] void fail(FpErrorCode code,std::size_t offset,const char* detail) {
    throw FpError{code,offset,detail,std::nullopt};
}
std::uint32_t word(const std::vector<std::uint8_t>& b,std::size_t p) {
    return std::uint32_t(b[p]) | (std::uint32_t(b[p+1])<<8) |
        (std::uint32_t(b[p+2])<<16) | (std::uint32_t(b[p+3])<<24);
}
std::int32_t signedWord(const std::vector<std::uint8_t>& b,std::size_t p) {
    const auto raw=word(b,p);
    return static_cast<std::int32_t>(raw<0x80000000U ? std::int64_t(raw) : std::int64_t(raw)-0x100000000LL);
}
}
FpResult decodeFp(const std::vector<std::uint8_t>& bytes,const FpLimits& limits) try {
    if(bytes.size()>limits.inputBytes) fail(FpErrorCode::limitExceeded,0,"FP input limit exceeded");
    if(bytes.size()<116) fail(FpErrorCode::malformedData,bytes.size(),"Truncated FP header");
    if(word(bytes,0)!=0x0050462e) fail(FpErrorCode::invalidFormat,0,"Invalid FP signature");
    if(word(bytes,4)!=2) fail(FpErrorCode::unsupportedVersion,4,"Only FP version 2 is supported");
    const auto paths=word(bytes,16);
    if(paths>8) fail(FpErrorCode::malformedData,16,"FP path count exceeds eight header slots");
    std::uint64_t count=0;
    for(std::size_t i=0;i<paths;++i) count+=word(bytes,20+i*4);
    if(count>limits.points || count>limits.decodedBytes/sizeof(FpPoint))
        fail(FpErrorCode::limitExceeded,20,"FP point or decoded allocation limit exceeded");
    if(116+count*8!=bytes.size()) fail(FpErrorCode::malformedData,20,"FP point counts disagree with payload extent");
    for(std::size_t i=0;i<paths;++i)
        if(std::uint64_t(word(bytes,52+i*4))+word(bytes,20+i*4)>count)
            fail(FpErrorCode::malformedData,52+i*4,"FP active path range exceeds point array");
    FpAsset asset;asset.version=word(bytes,4);asset.pathCount=paths;asset.sourceBytes=bytes.size();
    asset.headerPoint={signedWord(bytes,8),signedWord(bytes,12)};
    for(std::size_t i=0;i<8;++i) {
        asset.pointCounts[i]=word(bytes,20+i*4);asset.pointOffsets[i]=word(bytes,52+i*4);
    }
    for(std::size_t i=0;i<4;++i) asset.flagPositions[i]={signedWord(bytes,84+i*8),signedWord(bytes,88+i*8)};
    if(count>asset.points.max_size()) fail(FpErrorCode::limitExceeded,20,"FP points cannot fit in memory");
    asset.points.resize(static_cast<std::size_t>(count));
    for(std::size_t i=0;i<count;++i) asset.points[i]={signedWord(bytes,116+i*8),signedWord(bytes,120+i*8)};
    return asset;
} catch(const FpError& error) {return error;}
  catch(const std::bad_alloc&) {return FpError{FpErrorCode::limitExceeded,0,"FP allocation failed",std::nullopt};}
  catch(const std::length_error&) {return FpError{FpErrorCode::limitExceeded,0,"FP allocation too large",std::nullopt};}
FpResult loadFp(AssetFile& file,const FpLimits& limits) {
    if(limits.inputBytes>std::uint64_t(std::numeric_limits<std::int64_t>::max()))
        return FpError{FpErrorCode::invalidArgument,0,"FP input limit exceeds file API range",std::nullopt};
    auto bytes=readWhole(file,static_cast<std::int64_t>(limits.inputBytes));
    if(const auto* error=std::get_if<Error>(&bytes)) return FpError{FpErrorCode::assetInput,0,error->detail,*error};
    return decodeFp(std::get<std::vector<std::uint8_t>>(bytes),limits);
}
}
