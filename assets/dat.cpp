#include "dat.hpp"
#include <limits>
#include <new>
#include <stdexcept>
namespace mnm::assets {
namespace {
struct Reader {
    const std::vector<std::uint8_t>& bytes; const DatLimits& limits; std::size_t at=0; std::uint64_t storage=0;
    [[noreturn]] void fail(DatErrorCode code,const char* text) const { throw DatError{code,at,text,std::nullopt}; }
    void require(std::uint64_t count) const { if(count>bytes.size()-at) fail(DatErrorCode::malformedData,"Truncated DAT record"); }
    void charge(std::uint64_t count) { if(count>limits.decodedBytes-storage) fail(DatErrorCode::limitExceeded,"DAT decoded storage exceeds limit"); storage+=count; }
    std::uint32_t word() { require(4); auto v=std::uint32_t(bytes[at])|std::uint32_t(bytes[at+1])<<8|std::uint32_t(bytes[at+2])<<16|std::uint32_t(bytes[at+3])<<24; at+=4; return v; }
    void check(std::uint64_t count,std::uint64_t limit) const { if(count>limit) fail(DatErrorCode::limitExceeded,"DAT count exceeds limit"); }
};
template<class T,class Decode> std::variant<T,DatError> decode(const std::vector<std::uint8_t>& bytes,const DatLimits& limits,Decode action) try {
    Reader r{bytes,limits}; r.check(bytes.size(),limits.inputBytes); T asset; asset.sourceBytes=bytes.size(); action(r,asset); return asset;
} catch(const DatError& e) {return e;}
  catch(const std::bad_alloc&) {return DatError{DatErrorCode::limitExceeded,0,"DAT allocation failed",std::nullopt};}
  catch(const std::length_error&) {return DatError{DatErrorCode::limitExceeded,0,"DAT allocation too large",std::nullopt};}
template<class Decode> auto load(AssetFile& file,const DatLimits& limits,Decode action) -> decltype(action(std::vector<std::uint8_t>{},limits)) {
    if(limits.inputBytes>std::uint64_t(std::numeric_limits<std::int64_t>::max())) return DatError{DatErrorCode::invalidArgument,0,"DAT input limit exceeds file API range",std::nullopt};
    auto result=readWhole(file,static_cast<std::int64_t>(limits.inputBytes));
    if(auto* e=std::get_if<Error>(&result)) return DatError{e->code==ErrorCode::limitExceeded?DatErrorCode::limitExceeded:DatErrorCode::assetInput,0,e->detail,*e};
    return action(std::get<std::vector<std::uint8_t>>(result),limits);
}
}
BrainDatResult decodeBrainDat(const std::vector<std::uint8_t>& bytes,const DatLimits& limits) {
    return decode<BrainDat>(bytes,limits,[](Reader& r,BrainDat& a){
        while(r.at<r.bytes.size()) {
            r.check(a.models.size()+1,r.limits.records); r.require(12); r.charge(sizeof(BrainModel));
            BrainModel m; m.key=r.word(); m.dimension=r.word(); auto count=r.word(); r.check(count,r.limits.layers);
            r.require(std::uint64_t(count)*12); r.charge(std::uint64_t(count)*sizeof(DatLayer)); m.layers.resize(count);
            for(auto& layer:m.layers) {
                layer.scalarBits=r.word(); auto n=r.word(); r.check(n,r.limits.nodes);
                auto square=std::uint64_t(n)*n;
                r.check(square,std::numeric_limits<std::uint64_t>::max()/4);
                r.check(square,std::numeric_limits<std::size_t>::max());
                const auto nodeBytes=std::uint64_t(n)*48, matrixBytes=square*4;
                r.require(nodeBytes+4);
                if(matrixBytes>r.bytes.size()-r.at-nodeBytes-4) r.fail(DatErrorCode::malformedData,"Truncated DAT matrix");
                r.charge(nodeBytes); r.charge(matrixBytes);
                layer.nodes.resize(n); for(auto& node:layer.nodes) for(auto& value:node) value=r.word();
                layer.tailBits=r.word(); layer.matrix.resize(static_cast<std::size_t>(square)); for(auto& value:layer.matrix) value=r.word();
            }
            a.models.push_back(std::move(m));
        }
    });
}
ExperienceDatResult decodeExperienceDat(const std::vector<std::uint8_t>& bytes,const DatLimits& limits) {
    return decode<ExperienceDat>(bytes,limits,[](Reader& r,ExperienceDat& a){
        while(r.at<r.bytes.size()) {
            r.check(a.samples.size()+1,r.limits.records); r.require(16); r.charge(sizeof(ExperienceSample));
            ExperienceSample s; s.key=r.word(); auto n=r.word(); r.check(n,r.limits.values); r.require(std::uint64_t(n)*4+8); r.charge(std::uint64_t(n)*4);
            s.values.resize(n); for(auto& value:s.values) value=r.word(); for(auto& value:s.parameters) value=r.word();
            a.samples.push_back(std::move(s));
        }
    });
}
BrainDatResult loadBrainDat(AssetFile& file,const DatLimits& limits) {return load(file,limits,decodeBrainDat);}
ExperienceDatResult loadExperienceDat(AssetFile& file,const DatLimits& limits) {return load(file,limits,decodeExperienceDat);}
}
