#include "tag.hpp"
#include <limits>
#include <new>
#include <stdexcept>

namespace mnm::assets {
namespace {
[[noreturn]] void fail(TagErrorCode code,std::size_t offset,const char* detail) {
    throw TagError{code,offset,detail,std::nullopt};
}
std::uint32_t word(const std::vector<std::uint8_t>& b,std::size_t p) {
    return std::uint32_t(b[p]) | (std::uint32_t(b[p+1])<<8) |
        (std::uint32_t(b[p+2])<<16) | (std::uint32_t(b[p+3])<<24);
}

}
std::string tagEntryName(const TagEntry& entry) {
    std::string name;
    for(auto byte:entry.nameBytes) {
        if(byte==0) break;
        name.push_back(static_cast<char>(byte));
    }
    return name;
}
TagResult decodeTag(const std::vector<std::uint8_t>& bytes,const TagLimits& limits) try {
    if(bytes.size()>limits.inputBytes) fail(TagErrorCode::limitExceeded,0,"TAG input limit exceeded");
    if(bytes.size()%12!=0) fail(TagErrorCode::malformedData,bytes.size()-bytes.size()%12,"TAG length is not a multiple of 12");
    const auto count=bytes.size()/12;
    if(count>limits.records || std::uint64_t(count)>limits.decodedBytes/sizeof(TagEntry))
        fail(TagErrorCode::limitExceeded,0,"TAG record or decoded allocation limit exceeded");
    TagAsset asset;asset.sourceBytes=bytes.size();
    if(count>asset.entries.max_size()) fail(TagErrorCode::limitExceeded,0,"TAG records cannot fit in memory");
    asset.entries.resize(count);
    for(std::size_t i=0;i<count;++i) {
        auto& entry=asset.entries[i];const auto offset=i*12;
        for(std::size_t j=0;j<8;++j) entry.nameBytes[j]=bytes[offset+j];
        entry.occurrence=word(bytes,offset+8);
    }
    return asset;
} catch(const TagError& error) {return error;}
  catch(const std::bad_alloc&) {return TagError{TagErrorCode::limitExceeded,0,"TAG allocation failed",std::nullopt};}
  catch(const std::length_error&) {return TagError{TagErrorCode::limitExceeded,0,"TAG allocation too large",std::nullopt};}
TagResult loadTag(AssetFile& file,const TagLimits& limits) {
    if(limits.inputBytes>std::uint64_t(std::numeric_limits<std::int64_t>::max()))
        return TagError{TagErrorCode::invalidArgument,0,"TAG input limit exceeds file API range",std::nullopt};
    auto bytes=readWhole(file,static_cast<std::int64_t>(limits.inputBytes));
    if(const auto* error=std::get_if<Error>(&bytes)) return TagError{TagErrorCode::assetInput,0,error->detail,*error};
    return decodeTag(std::get<std::vector<std::uint8_t>>(bytes),limits);
}
}
