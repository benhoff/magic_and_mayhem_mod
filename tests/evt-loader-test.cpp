#include "evt.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
namespace {
void require(bool ok) {if(!ok) throw std::runtime_error("EVT assertion failed");}
void put(std::vector<std::uint8_t>& b,std::size_t p,std::uint32_t v) {
    for(std::size_t i=0;i<4;++i) b[p+i]=static_cast<std::uint8_t>(v>>(8*i));
}
std::vector<std::uint8_t> fixture() {
    std::vector<std::uint8_t> b(160,0);
    put(b,0,0x00545645);put(b,4,48);put(b,8,1);put(b,12,2);
    const std::array<std::uint32_t,6> words={32,27,4,9,0x80000000U,0x7fffffffU};
    for(std::size_t i=0;i<words.size();++i) put(b,16+i*4,words[i]);
    b[40]='A';b[42]=0xff; // Prefix A, NUL, retained nonzero tail.
    for(std::size_t i=112;i<160;++i) b[i]=0xe9; // Full unterminated name.
    return b;
}
void error(const std::vector<std::uint8_t>& b,EvtErrorCode expected,const EvtLimits& limits={}) {
    const auto result=decodeEvt(b,limits);require(std::holds_alternative<EvtError>(result));
    require(std::get<EvtError>(result).code==expected);
}
}
int main() try {
    const auto good=fixture();auto bytes=good;
    auto result=decodeEvt(bytes);require(std::holds_alternative<EvtAsset>(result));
    auto asset=std::get<EvtAsset>(result);bytes.clear();
    require(asset.headerSizeWord==48 && asset.sourceBytes==160 && asset.version==1 && asset.areas.size()==2);
    const auto& item=asset.areas[0];
    require(item.first==std::array<std::int32_t,3>{32,27,4});
    require(item.second==std::array<std::int32_t,3>{9,(-2147483647-1),2147483647});
    require(evtAreaName(item)=="A" && item.nameBytes[2]==0xff);
    require(evtAreaName(asset.areas[1])==std::string(48,static_cast<char>(0xe9)));
    EvtArea empty;require(evtAreaName(empty).empty());
    bytes=good;put(bytes,0,0);error(bytes,EvtErrorCode::invalidFormat);
    bytes=good;put(bytes,8,2);error(bytes,EvtErrorCode::unsupportedVersion);
    bytes=good;put(bytes,4,0xffffffffU);require(std::get<EvtAsset>(decodeEvt(bytes)).headerSizeWord==0xffffffffU);
    bytes=good;put(bytes,12,1);error(bytes,EvtErrorCode::malformedData);
    bytes=good;put(bytes,12,3);error(bytes,EvtErrorCode::malformedData);
    bytes=good;put(bytes,12,0xffffffffU);error(bytes,EvtErrorCode::limitExceeded);
    bytes=good;bytes.push_back(0);error(bytes,EvtErrorCode::malformedData);
    for(std::size_t n=0;n<good.size();++n) {
        bytes.assign(good.begin(),good.begin()+static_cast<std::ptrdiff_t>(n));
        require(std::holds_alternative<EvtError>(decodeEvt(bytes)));
    }
    bytes.assign(good.begin(),good.begin()+16);put(bytes,12,0);put(bytes,4,16);
    require(std::get<EvtAsset>(decodeEvt(bytes)).areas.empty());
    EvtLimits limits;limits.inputBytes=159;error(good,EvtErrorCode::limitExceeded,limits);
    limits={};limits.records=1;error(good,EvtErrorCode::limitExceeded,limits);
    limits={};limits.decodedBytes=2*sizeof(EvtArea)-1;error(good,EvtErrorCode::limitExceeded,limits);
    limits={};limits.decodedBytes=2*sizeof(EvtArea);require(std::holds_alternative<EvtAsset>(decodeEvt(good,limits)));
    limits={};limits.records=0;limits.decodedBytes=0;require(std::holds_alternative<EvtAsset>(decodeEvt(bytes,limits)));
    // Huge count with huge caller budgets still fails extent validation before allocation.
    bytes=good;put(bytes,12,0xffffffffU);limits={};limits.records=0xffffffffU;limits.decodedBytes=0xffffffffffffffffULL;
    error(bytes,EvtErrorCode::malformedData,limits);
    std::cout<<"EVT endpoints, bounded raw names, size metadata, ownership, truncation and limits passed\n";
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
