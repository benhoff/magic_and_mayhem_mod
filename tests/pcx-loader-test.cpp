#include "pcx.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
namespace {
void require(bool ok) {if(!ok) throw std::runtime_error("PCX assertion failed");}
std::vector<std::uint8_t> fixture() {
    std::vector<std::uint8_t> b(128,0);
    b[0]=10;b[1]=5;b[2]=1;b[3]=8;b[8]=2;b[10]=1;b[65]=1;b[66]=4;
    b.insert(b.end(),{1,2,3,99,0xc3,200,88,12});
    for(int i=0;i<768;++i) b.push_back(static_cast<std::uint8_t>(i));
    return b;
}
void error(const std::vector<std::uint8_t>& b,PcxErrorCode expected,const PcxLimits& limits={}) {
    auto result=decodePcx(b,limits);require(std::holds_alternative<PcxError>(result));
    require(std::get<PcxError>(result).code==expected);
}
}
int main() try {
    const auto good=fixture();auto result=decodePcx(good);
    require(std::holds_alternative<PcxImage>(result));
    auto image=std::get<PcxImage>(result);
    require(image.width==3 && image.height==2 && image.bytesPerLine==4);
    require(image.indices==std::vector<std::uint8_t>({1,2,3,200,200,200}));
    require(image.palette[255]==std::array<std::uint8_t,3>{253,254,255});
    auto b=good;b[0]=0;error(b,PcxErrorCode::invalidFormat);
    for(auto offset:{1,2,3,65}) {b=good;b[std::size_t(offset)]=0;error(b,PcxErrorCode::unsupportedEncoding);}
    b=good;b[4]=3;error(b,PcxErrorCode::malformedData);
    for(auto stride:{0,2,3}) {b=good;b[66]=static_cast<std::uint8_t>(stride);error(b,PcxErrorCode::malformedData);}
    b=good;b[128]=0xc0;error(b,PcxErrorCode::malformedData);
    b=good;b[128]=0xc5;error(b,PcxErrorCode::malformedData);
    b=good;b[b.size()-769]=0;error(b,PcxErrorCode::malformedData);
    b=good;b.insert(b.begin()+135,0);error(b,PcxErrorCode::malformedData);
    b=good;b.erase(b.begin()+134);b[133]=0xc1;error(b,PcxErrorCode::malformedData);
    // Every truncation must fail, including header, run, rows and palette.
    for(std::size_t n=0;n<good.size();++n) {
        b.assign(good.begin(),good.begin()+static_cast<std::ptrdiff_t>(n));
        require(std::holds_alternative<PcxError>(decodePcx(b)));
    }
    PcxLimits limits;limits.inputBytes=good.size()-1;error(good,PcxErrorCode::limitExceeded,limits);
    limits={};limits.pixels=5;error(good,PcxErrorCode::limitExceeded,limits);
    limits={};limits.width=2;error(good,PcxErrorCode::limitExceeded,limits);
    limits={};limits.height=1;error(good,PcxErrorCode::limitExceeded,limits);
    // Nonzero origins and literal values at the escape threshold.
    b=good;b[4]=7;b[8]=9;b[6]=9;b[10]=10;
    result=decodePcx(b);require(std::get<PcxImage>(result).originX==7);
    require(std::get<PcxImage>(result).originY==9);
    b=good;b[128]=191;require(std::get<PcxImage>(decodePcx(b)).indices[0]==191);
    std::cout<<"PCX fixture, padding, palette, malformed input and limit checks passed\n";
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
