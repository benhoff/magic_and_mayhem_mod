#include "fp.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
namespace {
void require(bool ok) {if(!ok) throw std::runtime_error("FP assertion failed");}
void put(std::vector<std::uint8_t>& b,std::size_t p,std::uint32_t v) {
    for(std::size_t i=0;i<4;++i) b[p+i]=static_cast<std::uint8_t>(v>>(8*i));
}
void error(const std::vector<std::uint8_t>& b,FpErrorCode expected,const FpLimits& limits={}) {
    auto result=decodeFp(b,limits);require(std::holds_alternative<FpError>(result));
    require(std::get<FpError>(result).code==expected);
}
}
int main() try {
    std::vector<std::uint8_t> good(140,0);
    put(good,0,0x0050462e);put(good,4,2);put(good,8,0xffffffffU);put(good,12,0x80000000U);
    put(good,16,2);put(good,20,2);put(good,24,1);put(good,56,2);
    put(good,28,0xffffffffU);put(good,60,0xfffffffeU); // Unused slots are metadata.
    put(good,84,0x7fffffffU);put(good,88,0xffffffffU);
    for(std::size_t i=0;i<3;++i) {put(good,116+i*8,static_cast<std::uint32_t>(i));put(good,120+i*8,0xfffffffdU);}
    auto bytes=good;auto asset=std::get<FpAsset>(decodeFp(bytes));bytes.clear();
    require(asset.version==2 && asset.pathCount==2 && asset.sourceBytes==140 && asset.points.size()==3);
    require(asset.headerPoint.x==-1 && asset.headerPoint.y==(-2147483647-1));
    require(asset.pointCounts[2]==0xffffffffU && asset.pointOffsets[2]==0xfffffffeU);
    require(asset.flagPositions[0].x==2147483647 && asset.flagPositions[0].y==-1);
    require(asset.points[2].x==2 && asset.points[2].y==-3);
    bytes=good;put(bytes,0,0);error(bytes,FpErrorCode::invalidFormat);
    bytes=good;put(bytes,4,1);error(bytes,FpErrorCode::unsupportedVersion);
    bytes=good;put(bytes,16,9);error(bytes,FpErrorCode::malformedData);
    bytes=good;put(bytes,20,3);error(bytes,FpErrorCode::malformedData);
    bytes=good;put(bytes,52,2);error(bytes,FpErrorCode::malformedData);
    bytes=good;put(bytes,56,0xffffffffU);error(bytes,FpErrorCode::malformedData);
    // Preserve valid noncanonical/overlapping ranges instead of reconstructing offsets.
    bytes=good;put(bytes,52,1);put(bytes,56,0);asset=std::get<FpAsset>(decodeFp(bytes));
    require(asset.pointOffsets[0]==1 && asset.pointOffsets[1]==0);
    bytes=good;bytes.push_back(0);error(bytes,FpErrorCode::malformedData);
    for(std::size_t n=0;n<good.size();++n) {
        bytes.assign(good.begin(),good.begin()+static_cast<std::ptrdiff_t>(n));
        require(std::holds_alternative<FpError>(decodeFp(bytes)));
    }
    bytes=good;bytes.resize(116);put(bytes,16,0);require(std::get<FpAsset>(decodeFp(bytes)).points.empty());
    FpLimits limits;limits.inputBytes=139;error(good,FpErrorCode::limitExceeded,limits);
    limits={};limits.points=2;error(good,FpErrorCode::limitExceeded,limits);
    limits={};limits.decodedBytes=3*sizeof(FpPoint)-1;error(good,FpErrorCode::limitExceeded,limits);
    limits={};limits.decodedBytes=3*sizeof(FpPoint);require(std::holds_alternative<FpAsset>(decodeFp(good,limits)));
    limits={};limits.points=0;limits.decodedBytes=0;require(std::get<FpAsset>(decodeFp(bytes,limits)).points.empty());
    // A zero-length active path may refer to the end of the point array.
    bytes=good;put(bytes,16,3);put(bytes,28,0);put(bytes,60,3);require(std::holds_alternative<FpAsset>(decodeFp(bytes)));
    bytes=good;put(bytes,20,0xffffffffU);put(bytes,24,0);limits={};limits.points=0xffffffffU;limits.decodedBytes=0xffffffffffffffffULL;
    error(bytes,FpErrorCode::malformedData,limits);
    std::cout<<"FP metadata, signed points, owned storage, active ranges, truncation and limits passed\n";
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
