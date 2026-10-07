#include "scene_snapshot.hpp"
#include "resource-fixtures.hpp"
#include "../protocols/include/mnm/scene_snapshot_v1.h"
#include <QGuiApplication>
#include <QCryptographicHash>
#include <iostream>
using namespace mnm;
using namespace resource_test;
static QByteArray bytes(const Bytes& b){return {reinterpret_cast<const char*>(b.data()),qsizetype(b.size())};}
static void put(QByteArray& b,std::size_t at,std::uint32_t v){for(unsigned i=0;i<4;++i)b[qsizetype(at+i)]=char(v>>(8*i));}
static QByteArray snapshot(const Bytes& sprite){
    auto frame=bytes(sprite).mid(36,55);put(frame,28,0);
    QByteArray b(64+3*32+16,0);b.replace(qsizetype(0),qsizetype(8),"MNMSCNE1",qsizetype(8));
    put(b,8,1);put(b,12,64);put(b,20,1);put(b,24,3);put(b,28,1);put(b,32,2);put(b,40,0x40209ca7);put(b,48,64);put(b,52,160);put(b,56,5);put(b,60,3);
    for(unsigned i=0;i<3;++i){const auto at=64+i*32;put(b,at,i==2?0:1);put(b,at+4,7);put(b,at+8,i==0?2:3);put(b,at+12,0);put(b,at+16,i==2?0xfffffffe:0);put(b,at+24,0x8ad08ad0);put(b,at+28,i==2?1:0);}
    put(b,160,1);put(b,164,55);b.append(frame);put(b,16,b.size());return b;
}
int main(int argc,char** argv)try{
    QGuiApplication app(argc,argv);QTemporaryDir dir;require(dir.isValid(),"Root unavailable");const auto root=std::filesystem::path(dir.path().toStdString());
    const auto sprBytes=sprite();write(root,"body.spr",sprBytes);auto storeRoot=store(root);assets::ResourceManager manager(storeRoot);legacy::SnapshotResources map(storeRoot,manager);
    const assets::ResourceId body{assets::ResourceKind::ui,"observed/body"};const auto pin=QCryptographicHash::hash(bytes(sprBytes),QCryptographicHash::Sha256).toHex();
    rejected([&]{map.add(body,"body.spr",QByteArray(64,'0'));});require(manager.stats().bindings==0,"Bad pin published binding");
    map.add(body,"body.spr",pin);require(manager.stats().residentResources==0,"Mapping retained decoded resources");
    auto wire=snapshot(sprBytes);const auto s=legacy::decodeSceneSnapshot(wire);auto display=legacy::snapshotDisplay(s,map);
    require(display.draws.size()==2&&display.hidden==1&&display.complete()&&display.draws[0].resource==body,"Display identity/order/hidden differs");
    // Snapshot and identity bytes are owned, independent of producer storage.
    wire.fill('x');require(s.frames[0].encoded.size()==55&&map.resolve(s.frames[0]).frame==0,"Snapshot frame identity borrowed storage");
    render::GlBlitter renderer;render::Image background{5,3,std::vector<std::uint32_t>(15,0x1234)};
    {render::SceneRenderer scene(renderer,manager,background);scene.draw(display.draws);auto expected=background;expected.pixels[6]=0;expected.pixels[7]=0;expected.pixels[8]=0xf800;
        require(scene.read().pixels==expected.pixels,"Rebound snapshot complete pixels differ");
        const auto before=renderer.stats();scene.draw(display.draws);const auto after=renderer.stats();require(before.uploads==after.uploads&&before.nativeReadbacks==after.nativeReadbacks,"Snapshot resident draw uploads/readbacks");}
    require(renderer.stats().surfaces==0,"Snapshot replay leaked surfaces");
    manager.unload(body);write(root,"body.spr",sprite(false,0x07e0));rejected([&]{map.resolve(s.frames[0]);});manager.unload(body);write(root,"body.spr",sprBytes);
    auto invalid=s;invalid.records[0].kind=31;rejected([&]{legacy::snapshotDisplay(invalid,map);});auto partial=legacy::snapshotDisplay(invalid,map,true);require(partial.unsupported==1&&partial.draws.size()==1&&!partial.complete(),"Partial mode did not retain gap");
    invalid=s;invalid.frames[0].encoded[54]^=1;rejected([&]{legacy::snapshotDisplay(invalid,map);});require(legacy::snapshotDisplay(invalid,map,true).unmapped==2,"Unmapped frame silently substituted");
    // Different palettes with otherwise identical encoded frame bytes are refused.
    const auto indexed=sprite(true);write(root,"indexed.spr",indexed);auto other=indexed;other[24+3]=0;other[24+5]=255;write(root,"other.spr",other);
    for(const auto& entry:std::vector<std::pair<std::string,Bytes>>{{"indexed.spr",indexed},{"other.spr",other}})
        map.add({assets::ResourceKind::ui,entry.first=="indexed.spr"?"indexed":"other"},entry.first,QCryptographicHash::hash(bytes(entry.second),QCryptographicHash::Sha256).toHex());
    legacy::SnapshotFrame indexedFrame{true,bytes(indexed).mid(804,53)};rejected([&]{map.resolve(indexedFrame);});
    // Exact envelope/token/normalization validation; no truncation accepted.
    const auto valid=snapshot(sprBytes);
    for(qsizetype n=0;n<valid.size();++n)rejected([&]{legacy::decodeSceneSnapshot(valid.first(n));});
    for(const auto& mutation:std::vector<std::pair<unsigned,unsigned>>{{8,2},{12,60},{20,0},{24,12321},{28,4097},{32,4},{36,2},{40,0},{44,1},{48,68},{52,164},{56,2049},{64,2},{92,4},{128,1},{144,0},{160,2},{164,39},{168,2},{172,1},{204,1}}){
        auto b=valid;put(b,mutation.first,mutation.second);rejected([&]{legacy::decodeSceneSnapshot(b);});}
    {auto b=valid;put(b,92,3);rejected([&]{legacy::decodeSceneSnapshot(b);});}
    auto trailing=valid;trailing.append('\0');put(trailing,16,trailing.size());rejected([&]{legacy::decodeSceneSnapshot(trailing);});
    std::cout<<"Owned snapshot parsing/rebinding, complete native pixels, identity ambiguity, strict/partial gap policy and malformed wire checks pass\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
