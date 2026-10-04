#include "sft.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
namespace {
void require(bool value) {if(!value) throw std::runtime_error("SFT assertion failed");}
void put(std::vector<std::uint8_t>& b,std::size_t at,std::uint32_t v) {
    for(unsigned i=0;i<4;++i) b.at(at+i)=static_cast<std::uint8_t>(v>>(i*8));
}
std::vector<std::uint8_t> fixture(bool direct=false,bool alias=false) {
    const std::size_t count=alias ? 2 : 1,table=40+(direct ? 0 : 768)+16,base=table+4*count;
    std::vector<std::uint8_t> b(base+56,0);
    put(b,0,0x00544653);put(b,4,b.size());put(b,8,3);put(b,12,count);put(b,16,2);
    put(b,20,0xffffffffU);put(b,24,0x80000000U);put(b,28,0x12345678);put(b,32,direct ? 0 : 1);put(b,36,1);
    const auto metrics=table-16;put(b,metrics,0xffffffffU);put(b,metrics+4,0x7fffffffU);put(b,metrics+8,7);put(b,metrics+12,8);
    put(b,base,56);put(b,base+4,3);put(b,base+8,1);put(b,base+12,0xfffffffeU);put(b,base+16,2);
    b[base+20]='!';b[base+22]=0xff;put(b,base+28,direct ? 0xffffffffU : 0);
    put(b,base+40,48);put(b,base+44,50);b[base+48]=0;b[base+49]=3;
    b[base+50]=0;b[base+51]=127;b[base+52]=255;
    if(direct) {b[base+51]=0;b[base+52]=0;b[base+53]=0xf8;b[base+54]=0xff;b[base+55]=0xff;}
    return b;
}
void rejects(std::vector<std::uint8_t> b,SpriteErrorCode code,const SftLimits& limits={}) {
    const auto r=decodeSft(b,limits);require(std::holds_alternative<SftError>(r));require(std::get<SftError>(r).code==code);
}
}
int main() try {
    auto bytes=fixture();const auto good=bytes;auto font=std::get<SftFont>(decodeSft(bytes));bytes.clear();
    require(font.rowCount==2 && font.metricGlyphCount==1 && font.ascent==-1 && font.descent==(-2147483647-1));
    require(font.rowMetrics[0].leading==-1 && font.rowMetrics[0].trailing==2147483647);
    require(font.headerWord28==0x12345678 && font.glyphs.sourceBytes==good.size());
    const auto& f=font.glyphs.frames[0];require(f.sourceOffset==828 && f.originX==-2 && f.originY==2);
    require(f.name[2]==255 && f.opaqueMask==std::vector<std::uint8_t>({1,1,1}));
    require(std::get<std::vector<std::uint8_t>>(f.pixels)==std::vector<std::uint8_t>({0,127,255}));
    require(sftGlyphIndex(font,33)==0 && !sftGlyphIndex(font,32) && !sftGlyphIndex(font,34));
    font.glyphs.frames.resize(223);require(sftGlyphIndex(font,255)==222);
    const auto direct=std::get<SftFont>(decodeSft(fixture(true)));
    require(std::get<std::vector<std::uint16_t>>(direct.glyphs.frames[0].pixels)==std::vector<std::uint16_t>({0,0xf800,0xffff}));
    const auto alias=std::get<SftFont>(decodeSft(fixture(false,true)));require(alias.glyphs.frames.size()==2);
    bytes=good;put(bytes,0,0);rejects(bytes,SpriteErrorCode::invalidFormat);
    bytes=good;put(bytes,8,2);rejects(bytes,SpriteErrorCode::unsupportedVersion);
    bytes=good;put(bytes,4,0);rejects(bytes,SpriteErrorCode::malformedData);
    bytes=good;put(bytes,12,0xffffffffU);rejects(bytes,SpriteErrorCode::limitExceeded);
    bytes=good;put(bytes,16,0xffffffffU);rejects(bytes,SpriteErrorCode::limitExceeded);
    bytes=good;put(bytes,36,0xffffffffU);rejects(bytes,SpriteErrorCode::limitExceeded);
    bytes=good;put(bytes,824,0xffffffffU);rejects(bytes,SpriteErrorCode::malformedData);
    bytes=good;bytes[876]=4;rejects(bytes,SpriteErrorCode::malformedData);
    for(std::size_t n=0;n<good.size();++n) {
        bytes.assign(good.begin(),good.begin()+static_cast<std::ptrdiff_t>(n));require(std::holds_alternative<SftError>(decodeSft(bytes)));
    }
    SftLimits limits;limits.metricsBytes=15;rejects(good,SpriteErrorCode::limitExceeded,limits);
    limits={};limits.glyphs.decodedBytes=21;rejects(good,SpriteErrorCode::limitExceeded,limits);
    limits={};limits.glyphs.decodedBytes=22;require(std::holds_alternative<SftFont>(decodeSft(good,limits)));
    limits={};limits.glyphs.inputBytes=good.size()-1;rejects(good,SpriteErrorCode::limitExceeded,limits);
    std::vector<std::uint8_t> empty(40,0);put(empty,0,0x00544653);put(empty,4,40);put(empty,8,3);
    require(std::get<SftFont>(decodeSft(empty)).glyphs.frames.empty());
    std::cout<<"SFT metrics, indexed/RGB565 glyphs, byte mapping, aliases, ownership and limits passed\n";
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
