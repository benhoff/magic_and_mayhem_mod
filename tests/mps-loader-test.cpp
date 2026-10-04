#include "mps.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
namespace {
void require(bool ok) {if(!ok) throw std::runtime_error("MPS assertion failed");}
void put(std::vector<std::uint8_t>& b,std::size_t p,std::uint32_t v) {
    for(std::size_t i=0;i<4;++i) b[p+i]=static_cast<std::uint8_t>(v>>(8*i));
}
std::vector<std::uint8_t> fixture() {
    std::vector<std::uint8_t> b(96,0);
    put(b,0,0x0053504d);put(b,4,48);put(b,8,1);put(b,12,2);
    const std::array<std::uint32_t,20> words={32,27,4,5,2,4,0,0xffffffffU,0x80000000U,0x7fffffffU,
                                            9,8,7,0xfffffff9U,11,12,13,14,15,16};
    for(std::size_t i=0;i<words.size();++i) put(b,16+i*4,words[i]);
    return b;
}
void error(const std::vector<std::uint8_t>& b,MpsErrorCode expected,const MpsLimits& limits={}) {
    const auto result=decodeMps(b,limits);require(std::holds_alternative<MpsError>(result));
    require(std::get<MpsError>(result).code==expected);
}
}
int main() try {
    const auto good=fixture();auto bytes=good;
    auto result=decodeMps(bytes);require(std::holds_alternative<MpsAsset>(result));
    auto asset=std::get<MpsAsset>(result);bytes.clear();
    require(asset.headerSizeWord==48 && asset.sourceBytes==96 && asset.version==1 && asset.placements.size()==2);
    const auto& item=asset.placements[0];
    require(item.position==std::array<std::int32_t,3>{32,27,4});
    require(item.kind==MpsKind::artifact);
    require(item.parameters==std::array<std::int32_t,6>{2,4,0,-1,(-2147483647-1),2147483647});
    require(static_cast<std::int32_t>(asset.placements[1].kind)==-7);
    require(std::string(mpsKindName(asset.placements[1].kind))=="unknown");
    const std::array<std::string,6> names={"undefined","friendly_wizard","enemy_wizard","multiplayer_wizard","creature","artifact"};
    for(std::size_t i=0;i<names.size();++i) require(mpsKindName(static_cast<MpsKind>(i))==names[i]);
    bytes=good;put(bytes,0,0);error(bytes,MpsErrorCode::invalidFormat);
    bytes=good;put(bytes,8,2);error(bytes,MpsErrorCode::unsupportedVersion);
    bytes=good;put(bytes,4,0xffffffffU);require(std::get<MpsAsset>(decodeMps(bytes)).headerSizeWord==0xffffffffU);
    bytes=good;put(bytes,12,1);error(bytes,MpsErrorCode::malformedData);
    bytes=good;put(bytes,12,3);error(bytes,MpsErrorCode::malformedData);
    bytes=good;put(bytes,12,0xffffffffU);error(bytes,MpsErrorCode::limitExceeded);
    bytes=good;bytes.push_back(0);error(bytes,MpsErrorCode::malformedData);
    for(std::size_t n=0;n<good.size();++n) {
        bytes.assign(good.begin(),good.begin()+static_cast<std::ptrdiff_t>(n));
        require(std::holds_alternative<MpsError>(decodeMps(bytes)));
    }
    bytes.assign(good.begin(),good.begin()+16);put(bytes,12,0);put(bytes,4,16);
    require(std::get<MpsAsset>(decodeMps(bytes)).placements.empty());
    MpsLimits limits;limits.inputBytes=95;error(good,MpsErrorCode::limitExceeded,limits);
    limits={};limits.records=1;error(good,MpsErrorCode::limitExceeded,limits);
    limits={};limits.decodedBytes=2*sizeof(MpsPlacement)-1;error(good,MpsErrorCode::limitExceeded,limits);
    limits={};limits.decodedBytes=2*sizeof(MpsPlacement);require(std::holds_alternative<MpsAsset>(decodeMps(good,limits)));
    limits={};limits.records=0;limits.decodedBytes=0;require(std::holds_alternative<MpsAsset>(decodeMps(bytes,limits)));
    // Huge count with huge caller budgets still fails extent validation before allocation.
    bytes=good;put(bytes,12,0xffffffffU);limits={};limits.records=0xffffffffU;limits.decodedBytes=0xffffffffffffffffULL;
    error(bytes,MpsErrorCode::malformedData,limits);
    std::cout<<"MPS records, signed words, unknown kinds, size metadata, ownership, truncation and limits passed\n";
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
