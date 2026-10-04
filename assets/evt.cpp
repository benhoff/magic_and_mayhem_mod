#include "evt.hpp"
#include <limits>
#include <new>
#include <stdexcept>

namespace mnm::assets {
namespace {
[[noreturn]] void fail(EvtErrorCode code,std::size_t offset,const char* detail) {
    throw EvtError{code,offset,detail,std::nullopt};
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
std::string evtAreaName(const EvtArea& area) {
    std::string name;
    for(auto byte:area.nameBytes) {
        if(byte==0) break;
        name.push_back(static_cast<char>(byte));
    }
    return name;
}
EvtResult decodeEvt(const std::vector<std::uint8_t>& bytes,const EvtLimits& limits) try {
    if(bytes.size()>limits.inputBytes) fail(EvtErrorCode::limitExceeded,0,"EVT input limit exceeded");
    if(bytes.size()<16) fail(EvtErrorCode::malformedData,bytes.size(),"Truncated EVT header");
    if(word(bytes,0)!=0x00545645) fail(EvtErrorCode::invalidFormat,0,"Invalid EVT signature");
    if(word(bytes,8)!=1) fail(EvtErrorCode::unsupportedVersion,8,"Only EVT version 1 is supported");
    const auto count=word(bytes,12);
    if(count>limits.records || std::uint64_t(count)>limits.decodedBytes/sizeof(EvtArea))
        fail(EvtErrorCode::limitExceeded,12,"EVT record or decoded allocation limit exceeded");
    const std::uint64_t extent=16+std::uint64_t(count)*72;
    if(extent!=bytes.size()) fail(EvtErrorCode::malformedData,12,"EVT count disagrees with 72-byte record extent");
    EvtAsset asset;asset.headerSizeWord=word(bytes,4);asset.version=word(bytes,8);asset.sourceBytes=bytes.size();
    if(count>asset.areas.max_size()) fail(EvtErrorCode::limitExceeded,12,"EVT records cannot fit in memory");
    asset.areas.resize(count);
    for(std::size_t i=0;i<count;++i) {
        auto& item=asset.areas[i];const auto offset=16+i*72;
        for(std::size_t j=0;j<3;++j) {
            item.first[j]=signedWord(bytes,offset+j*4);
            item.second[j]=signedWord(bytes,offset+12+j*4);
        }
        for(std::size_t j=0;j<48;++j) item.nameBytes[j]=bytes[offset+24+j];
    }
    return asset;
} catch(const EvtError& error) {return error;}
  catch(const std::bad_alloc&) {return EvtError{EvtErrorCode::limitExceeded,0,"EVT allocation failed",std::nullopt};}
  catch(const std::length_error&) {return EvtError{EvtErrorCode::limitExceeded,0,"EVT allocation too large",std::nullopt};}
EvtResult loadEvt(AssetFile& file,const EvtLimits& limits) {
    if(limits.inputBytes>std::uint64_t(std::numeric_limits<std::int64_t>::max()))
        return EvtError{EvtErrorCode::invalidArgument,0,"EVT input limit exceeds file API range",std::nullopt};
    auto bytes=readWhole(file,static_cast<std::int64_t>(limits.inputBytes));
    if(const auto* error=std::get_if<Error>(&bytes)) return EvtError{EvtErrorCode::assetInput,0,error->detail,*error};
    return decodeEvt(std::get<std::vector<std::uint8_t>>(bytes),limits);
}
}
