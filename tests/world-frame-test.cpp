#include "world_frame.hpp"
#include "resource-fixtures.hpp"
#include "../protocols/include/mnm/world_frame_v1.h"
#include <QGuiApplication>
#include <QCryptographicHash>
#include <iostream>
using namespace mnm;
using namespace resource_test;
static QByteArray bytes(const Bytes& b){return {reinterpret_cast<const char*>(b.data()),qsizetype(b.size())};}
static void put(QByteArray& b,qsizetype at,std::uint32_t v){for(unsigned i=0;i<4;++i)b[at+i]=char(v>>(i*8));}
static QByteArray wire(const Bytes& sprite){
    auto encoded=bytes(sprite).mid(804,53);put(encoded,28,0);
    QByteArray b(80+64,0);b.replace(qsizetype(0),qsizetype(8),"MNMWRLD1",qsizetype(8));put(b,8,1);put(b,12,80);put(b,20,1);put(b,24,5);put(b,28,3);put(b,32,8);put(b,36,1);put(b,44,MNM_WORLD_BUILD);
    put(b,80,64+53+512);put(b,88,2);put(b,104,5);put(b,108,3);put(b,112,53);put(b,116,1);put(b,120,512);
    b.append(encoded);for(unsigned i=0;i<256;++i){b.append(char(i));b.append(char(i>>8));}put(b,16,b.size());return b;
}
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);QTemporaryDir directory;require(directory.isValid(),"Fixture root unavailable");const auto root=std::filesystem::path(directory.path().toStdString());
    const auto indexed=sprite(true);write(root,"body.spr",indexed);auto different=indexed;different[27]=0;different[29]=255;write(root,"other.spr",different);
    auto input=store(root);assets::ResourceManager resources(input);legacy::SnapshotResources bindings(input,resources);
    for(const auto& item:std::vector<std::pair<std::string,Bytes>>{{"body.spr",indexed},{"other.spr",different}})bindings.add({assets::ResourceKind::ui,item.first=="body.spr"?"body":"other"},item.first,QCryptographicHash::hash(bytes(item.second),QCryptographicHash::Sha256).toHex());
    const auto valid=wire(indexed);auto frame=legacy::decodeWorldFrame(valid);const auto draws=legacy::worldDisplay(frame,bindings);
    require(draws.size()==1&&draws[0].colours&&(*draws[0].colours)[1]==1,"Owned actual palette binding differs");
    rejected([&]{bindings.resolve(frame.draws[0].frame);});
    render::GlBlitter renderer;render::Image background{5,3,std::vector<std::uint32_t>(15,0xffff)};
    {render::SceneRenderer scene(renderer,resources,background);scene.draw(draws);auto expected=background;expected.pixels[6]=0;expected.pixels[7]=1;require(scene.read().pixels==expected.pixels,"World opaque index zero/palette override differs");
        auto clipped=draws;clipped[0].viewport=render::Rect{2,0,5,3};scene.draw(clipped);expected.pixels[6]=0xffff;require(scene.read().pixels==expected.pixels,"Per-record clip ignored");
        const auto previous=scene.read().pixels;clipped[0].composite.mode=render::CompositeMode::displace;clipped[0].composite.rowOffsets[0]=-1;rejected([&]{scene.draw(clipped);});require(scene.read().pixels==previous,"Late unsafe displacement admission changed completed frame");
        clipped=draws;clipped[0].viewport=render::Rect{-1,0,5,3};rejected([&]{scene.draw(clipped);});require(scene.read().pixels==previous,"Invalid clip changed completed frame");
    }
    require(renderer.stats().surfaces==0,"World surfaces leaked");
    for(qsizetype n=0;n<valid.size();++n)rejected([&]{legacy::decodeWorldFrame(valid.first(n));});
    for(const auto& mutation:std::vector<std::pair<unsigned,unsigned>>{{8,2},{12,64},{20,0},{24,2049},{28,0},{32,4},{36,32769},{40,1},{44,0},{72,1},{80,1},{84,6},{96,6},{104,6},{112,39},{116,2},{120,64},{132,12},{140,1},{172,1}}){auto b=valid;put(b,mutation.first,mutation.second);rejected([&]{legacy::decodeWorldFrame(b);});}
    auto trailing=valid;trailing.append('\0');put(trailing,16,trailing.size());rejected([&]{legacy::decodeWorldFrame(trailing);});
    frame.draws[0].frame.encoded[52]^=1;rejected([&]{legacy::worldDisplay(frame,bindings);});
    std::cout<<"World wire admission, actual palette aliases, opaque zero, per-draw clip, atomic refusal and lifetime pass\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
