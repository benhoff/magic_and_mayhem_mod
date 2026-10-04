#include "nod.hpp"
#include <algorithm>
#include <limits>
#include <new>
#include <stdexcept>
namespace mnm::assets {
namespace {
[[noreturn]] void fail(NodErrorCode code,std::size_t offset,const char* detail) {
    throw NodError{code,offset,detail,std::nullopt};
}
std::uint32_t word(const std::vector<std::uint8_t>& bytes,std::size_t at) {
    return std::uint32_t(bytes[at]) | std::uint32_t(bytes[at+1])<<8 |
        std::uint32_t(bytes[at+2])<<16 | std::uint32_t(bytes[at+3])<<24;
}
std::int32_t signedWord(const std::vector<std::uint8_t>& bytes,std::size_t at) {
    const auto value=word(bytes,at);
    return static_cast<std::int32_t>(value<0x80000000U ? std::int64_t(value) : std::int64_t(value)-0x100000000LL);
}
}
NodResult decodeNod(const std::vector<std::uint8_t>& bytes,const NodLimits& limits) try {
    if(bytes.size()>limits.inputBytes) fail(NodErrorCode::limitExceeded,0,"NOD input byte limit exceeded");
    if(bytes.size()<16) fail(NodErrorCode::malformedData,bytes.size(),"Truncated NOD header");
    if(word(bytes,0)!=0x00444f4e) fail(NodErrorCode::invalidFormat,0,"Invalid NOD signature");
    if(word(bytes,8)!=1) fail(NodErrorCode::unsupportedVersion,8,"Only NOD version 1 is supported");
    if(word(bytes,4)!=bytes.size()) fail(NodErrorCode::malformedData,4,"NOD declared size disagrees with input");
    const auto count=word(bytes,12);
    if(count>limits.nodes || count>limits.decodedBytes/sizeof(NodNode))
        fail(NodErrorCode::limitExceeded,12,"NOD node count or decoded storage exceeds limit");
    if(24+std::uint64_t(count)*494!=bytes.size())
        fail(NodErrorCode::malformedData,12,"NOD node count disagrees with records and trailer");
    NodAsset asset;asset.version=1;asset.sourceBytes=bytes.size();
    if(count>asset.nodes.max_size()) fail(NodErrorCode::limitExceeded,12,"NOD nodes cannot fit in memory");
    asset.nodes.resize(count);
    for(std::size_t i=0;i<count;++i) {
        const auto at=16+i*494;auto& node=asset.nodes[i];
        node.state=word(bytes,at);node.word4=word(bytes,at+4);
        for(std::size_t j=0;j<3;++j) node.position[j]=signedWord(bytes,at+8+j*4);
        for(std::size_t j=0;j<18;++j) {
            const auto edge=at+20+j*25;auto& connection=node.connections[j];
            connection.value=word(bytes,edge);connection.target=signedWord(bytes,edge+4);
            std::copy_n(bytes.begin()+static_cast<std::ptrdiff_t>(edge+8),17,connection.metadata.begin());
        }
        for(std::size_t j=0;j<6;++j) node.tailWords[j]=word(bytes,at+470+j*4);
    }
    asset.trailerWords={word(bytes,bytes.size()-8),word(bytes,bytes.size()-4)};
    return asset;
} catch(const NodError& error) {return error;}
  catch(const std::bad_alloc&) {return NodError{NodErrorCode::limitExceeded,0,"NOD allocation failed",std::nullopt};}
  catch(const std::length_error&) {return NodError{NodErrorCode::limitExceeded,0,"NOD allocation too large",std::nullopt};}
NodResult loadNod(AssetFile& file,const NodLimits& limits) {
    if(limits.inputBytes>std::uint64_t(std::numeric_limits<std::int64_t>::max()))
        return NodError{NodErrorCode::invalidArgument,0,"NOD input limit exceeds file API range",std::nullopt};
    auto bytes=readWhole(file,static_cast<std::int64_t>(limits.inputBytes));
    if(const auto* error=std::get_if<Error>(&bytes))
        return NodError{error->code==ErrorCode::limitExceeded ? NodErrorCode::limitExceeded : NodErrorCode::assetInput,0,error->detail,*error};
    return decodeNod(std::get<std::vector<std::uint8_t>>(bytes),limits);
}
}
