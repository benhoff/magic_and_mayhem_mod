#include "animation.hpp"
#include "sprite_loader.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::assets;
using Bytes=std::vector<std::uint8_t>;
void check(bool ok) {if(!ok) throw std::runtime_error("Legacy asset check failed");}
void put(Bytes& b,std::size_t at,std::uint32_t v) {for(unsigned i=0;i<4;++i) b.at(at+i)=std::uint8_t(v>>(i*8));}
Bytes ani(unsigned version) {
    const auto size=version==3?28U:36U;Bytes b(52+size*2);
    put(b,0,0x00494e41);put(b,4,b.size());put(b,8,2);put(b,12,version);put(b,20,2);put(b,48,2);
    for(unsigned i=0;i<size/4;++i) put(b,52+i*4,0x80000000U+i);
    put(b,52,0);put(b,52+size,6);return b;
}
Bytes spr() {
    Bytes b(20+768+8+51+32);put(b,0,0x00525053);put(b,4,b.size());put(b,8,2);put(b,12,2);put(b,16,1);
    b[20]=255;b[787]=77;put(b,792,51);
    const unsigned at=796;put(b,at,51);put(b,at+4,3);put(b,at+8,1);put(b,at+12,0xffffffff);put(b,at+20,0x12345678);put(b,at+28,0x87654321);
    put(b,at+32,40);put(b,at+36,43);b[at+40]=0;b[at+41]=2;b[at+42]=1;b[at+43]=0;b[at+44]=255;
    put(b,at+51,32);put(b,at+51+28,0xffffffff);return b;
}
int main() try {
    for(unsigned version:{3U,4U}) {
        auto b=ani(version);auto result=decodeAnimation(b);check(std::holds_alternative<Animation>(result));auto a=std::get<Animation>(result);b.assign(b.size(),0);
        check(a.version==version && a.records.size()==2 && a.records[0].metadata[0]==0x80000002);
        for(unsigned i=(version==3?5:7);i<9;++i) check(a.records[0].metadata[i]==0);
        b=ani(version);b.pop_back();check(std::holds_alternative<AnimationError>(decodeAnimation(b)));
        b=ani(version);put(b,48,1);check(std::holds_alternative<AnimationError>(decodeAnimation(b)));
    }
    auto b=spr();auto result=decodeSprite(b);check(std::holds_alternative<Sprite>(result));auto s=std::get<Sprite>(result);b.assign(b.size(),0);
    check(s.version==2 && s.headerFlags==0 && s.frames.size()==2 && s.palettes[0][0].red==255 && s.palettes[0][255].blue==77);
    check(s.frames[0].legacyPaletteWord==0x87654321 && s.frames[0].paletteIndex==0 && s.frames[0].originX==-1);
    check(s.frames[0].opaqueMask==Bytes({1,1,0}) && std::get<Bytes>(s.frames[0].pixels)==Bytes({0,255,0}));
    check(s.frames[1].empty() && s.frames[1].legacyPaletteWord==0xffffffff && s.frames[0].auxiliaryData[0].empty());
    b=spr();put(b,792,0);check(std::get<Sprite>(decodeSprite(b)).frames[1].width==3);
    b=spr();put(b,16,2);check(std::holds_alternative<SpriteError>(decodeSprite(b)));
    b=spr();put(b,796+36,39);check(std::holds_alternative<SpriteError>(decodeSprite(b)));
    b=spr();b[796+41]=4;check(std::holds_alternative<SpriteError>(decodeSprite(b)));
    b=spr();put(b,792,1);check(std::holds_alternative<SpriteError>(decodeSprite(b)));
    SpriteLimits limits;limits.scannedBytes=50;check(std::get<SpriteError>(decodeSprite(spr(),limits)).code==SpriteErrorCode::limitExceeded);
    limits={};limits.decodedBytes=1;check(std::get<SpriteError>(decodeSprite(spr(),limits)).code==SpriteErrorCode::limitExceeded);
    std::cout<<"Legacy ANI expansion and SPR ownership, metadata, rows and bounds passed\n";return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
