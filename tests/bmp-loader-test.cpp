#include "bmp.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
namespace {
void require(bool ok) {if(!ok) throw std::runtime_error("BMP assertion failed");}
void put(std::vector<std::uint8_t>& b,std::size_t p,std::uint32_t value) {
    for(std::size_t i=0;i<4;++i) b[p+i]=static_cast<std::uint8_t>(value>>(8*i));
}
std::vector<std::uint8_t> fixture(bool topDown=false) {
    std::vector<std::uint8_t> b(58,0);
    b[0]='B';b[1]='M';put(b,2,74);put(b,10,58);put(b,14,40);
    put(b,18,2);put(b,22,topDown ? 0xfffffffeU : 2U);b[26]=1;b[28]=24;
    put(b,34,16);put(b,38,0xffffffffU);put(b,42,72);
    const std::vector<std::uint8_t> upper={3,2,1,6,5,4,99,98},lower={9,8,7,12,11,10,97,96};
    const auto& first=topDown ? upper : lower;const auto& second=topDown ? lower : upper;
    b.insert(b.end(),first.begin(),first.end());b.insert(b.end(),second.begin(),second.end());
    return b;
}
void error(const std::vector<std::uint8_t>& b,BmpErrorCode code,const BmpLimits& limits={}) {
    const auto result=decodeBmp(b,limits);require(std::holds_alternative<BmpError>(result));
    require(std::get<BmpError>(result).code==code);
}
}
int main() try {
    const auto good=fixture();
    for(bool topDown:{false,true}) {
        auto bytes=fixture(topDown);const auto decoded=decodeBmp(bytes);
        require(std::holds_alternative<BmpImage>(decoded));
        auto image=std::get<BmpImage>(decoded);bytes.clear();
        require(image.rgb==std::vector<std::uint8_t>({1,2,3,4,5,6,7,8,9,10,11,12}));
        require(image.width==2 && image.height==2 && image.sourceTopDown==topDown);
        require(image.rowStride==8 && image.pixelOffset==58 && image.sourceBytes==74);
        require(image.horizontalPixelsPerMeter==-1 && image.verticalPixelsPerMeter==72);
    }
    auto b=good;b[0]=0;error(b,BmpErrorCode::invalidFormat);
    for(auto header:{12U,108U,124U,0xffffffffU}) {b=good;put(b,14,header);error(b,BmpErrorCode::unsupportedEncoding);}
    for(auto offset:{6,8,26}) {b=good;b[std::size_t(offset)]=2;error(b,BmpErrorCode::malformedData);}
    for(auto bits:{1,8,16,32}) {b=good;b[28]=static_cast<std::uint8_t>(bits);error(b,BmpErrorCode::unsupportedEncoding);}
    b=good;put(b,30,1);error(b,BmpErrorCode::unsupportedEncoding);
    for(auto width:{0U,0xffffffffU,0x80000000U}) {b=good;put(b,18,width);error(b,BmpErrorCode::malformedData);}
    for(auto height:{0U,0x80000000U}) {b=good;put(b,22,height);error(b,BmpErrorCode::malformedData);}
    for(auto offset:{0U,53U,74U,0xffffffffU}) {b=good;put(b,10,offset);error(b,BmpErrorCode::malformedData);}
    for(auto size:{0U,73U,75U,0xffffffffU}) {b=good;put(b,2,size);error(b,BmpErrorCode::malformedData);}
    b=good;put(b,34,15);error(b,BmpErrorCode::malformedData);
    b=good;put(b,34,0);require(std::holds_alternative<BmpImage>(decodeBmp(b)));
    b=good;put(b,46,1);require(std::get<BmpImage>(decodeBmp(b)).colorsUsed==1);
    b=good;put(b,46,2);error(b,BmpErrorCode::malformedData);
    b=good;put(b,46,0xffffffffU);error(b,BmpErrorCode::malformedData);
    b=good;b.push_back(99);require(std::get<BmpImage>(decodeBmp(b)).sourceBytes==75);
    for(std::size_t n=0;n<good.size();++n) {
        b.assign(good.begin(),good.begin()+static_cast<std::ptrdiff_t>(n));
        require(std::holds_alternative<BmpError>(decodeBmp(b)));
    }
    BmpLimits limits;limits.inputBytes=73;error(good,BmpErrorCode::limitExceeded,limits);
    limits={};limits.width=1;error(good,BmpErrorCode::limitExceeded,limits);
    limits={};limits.height=1;error(good,BmpErrorCode::limitExceeded,limits);
    limits={};limits.pixels=3;error(good,BmpErrorCode::limitExceeded,limits);
    limits={};limits.decodedBytes=11;error(good,BmpErrorCode::limitExceeded,limits);
    limits={};limits.decodedBytes=12;require(std::holds_alternative<BmpImage>(decodeBmp(good,limits)));
    b=good;put(b,18,0x7fffffffU);error(b,BmpErrorCode::limitExceeded);
    b=good;put(b,22,0x7fffffffU);error(b,BmpErrorCode::limitExceeded);
    // Oversized caller budgets still cannot bypass file extent validation.
    limits={};limits.width=0xffffffffU;limits.height=0xffffffffU;
    limits.pixels=0xffffffffffffffffULL;limits.decodedBytes=0xffffffffffffffffULL;
    b=good;put(b,18,0x7fffffffU);error(b,BmpErrorCode::limitExceeded,limits);
    b=good;put(b,18,1000000);put(b,22,1000000);error(b,BmpErrorCode::malformedData,limits);
    std::cout<<"BMP orientation, BGR, padding, ownership, extents, truncation and limits passed\n";
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
