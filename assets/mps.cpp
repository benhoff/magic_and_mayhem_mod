#include "mps.hpp"
#include <limits>
#include <new>
#include <stdexcept>

namespace mnm::assets {
namespace {
[[noreturn]] void fail(MpsErrorCode code,std::size_t offset,const char* detail) {
    throw MpsError{code,offset,detail,std::nullopt};
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
const char* mpsKindName(MpsKind kind) {
    switch(kind) {
    case MpsKind::undefined:return "undefined";
    case MpsKind::friendlyWizard:return "friendly_wizard";
    case MpsKind::enemyWizard:return "enemy_wizard";
    case MpsKind::multiplayerWizard:return "multiplayer_wizard";
    case MpsKind::creature:return "creature";
    case MpsKind::artifact:return "artifact";
    }
    return "unknown";
}
MpsResult decodeMps(const std::vector<std::uint8_t>& bytes,const MpsLimits& limits) try {
    if(bytes.size()>limits.inputBytes) fail(MpsErrorCode::limitExceeded,0,"MPS input limit exceeded");
    if(bytes.size()<16) fail(MpsErrorCode::malformedData,bytes.size(),"Truncated MPS header");
    if(word(bytes,0)!=0x0053504d) fail(MpsErrorCode::invalidFormat,0,"Invalid MPS signature");
    if(word(bytes,8)!=1) fail(MpsErrorCode::unsupportedVersion,8,"Only MPS version 1 is supported");
    const auto count=word(bytes,12);
    if(count>limits.records || std::uint64_t(count)>limits.decodedBytes/sizeof(MpsPlacement))
        fail(MpsErrorCode::limitExceeded,12,"MPS record or decoded allocation limit exceeded");
    const std::uint64_t extent=16+std::uint64_t(count)*40;
    if(extent!=bytes.size()) fail(MpsErrorCode::malformedData,12,"MPS count disagrees with 40-byte record extent");
    MpsAsset asset;asset.headerSizeWord=word(bytes,4);asset.version=word(bytes,8);asset.sourceBytes=bytes.size();
    if(count>asset.placements.max_size()) fail(MpsErrorCode::limitExceeded,12,"MPS records cannot fit in memory");
    asset.placements.resize(count);
    for(std::size_t i=0;i<count;++i) {
        auto& item=asset.placements[i];const auto offset=16+i*40;
        for(std::size_t j=0;j<3;++j) item.position[j]=signedWord(bytes,offset+j*4);
        item.kind=static_cast<MpsKind>(signedWord(bytes,offset+12));
        for(std::size_t j=0;j<6;++j) item.parameters[j]=signedWord(bytes,offset+16+j*4);
    }
    return asset;
} catch(const MpsError& error) {return error;}
  catch(const std::bad_alloc&) {return MpsError{MpsErrorCode::limitExceeded,0,"MPS allocation failed",std::nullopt};}
  catch(const std::length_error&) {return MpsError{MpsErrorCode::limitExceeded,0,"MPS allocation too large",std::nullopt};}
MpsResult loadMps(AssetFile& file,const MpsLimits& limits) {
    if(limits.inputBytes>std::uint64_t(std::numeric_limits<std::int64_t>::max()))
        return MpsError{MpsErrorCode::invalidArgument,0,"MPS input limit exceeds file API range",std::nullopt};
    auto bytes=readWhole(file,static_cast<std::int64_t>(limits.inputBytes));
    if(const auto* error=std::get_if<Error>(&bytes)) return MpsError{MpsErrorCode::assetInput,0,error->detail,*error};
    return decodeMps(std::get<std::vector<std::uint8_t>>(bytes),limits);
}
}
