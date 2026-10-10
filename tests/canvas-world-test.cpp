#include "../compat/legacy/canvas_world.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <QFile>
#include <QCryptographicHash>
#include <QOpenGLContext>
#include <iostream>
using namespace mnm;
using namespace resource_test;
int main(int argc,char **argv)try{
  QGuiApplication app(argc,argv);QTemporaryDir temporary;require(temporary.isValid(),"Fixture root");
  const auto root=std::filesystem::path(temporary.path().toStdString());populate(root);
  QFile file(QString::fromStdString((root/"body.spr").string()));require(file.open(QIODevice::ReadOnly),"SPR fixture");const auto raw=file.readAll();
  auto get=[&](int at){const auto *p=reinterpret_cast<const unsigned char*>(raw.data()+at);return p[0]|unsigned(p[1])<<8|unsigned(p[2])<<16|unsigned(p[3])<<24;};
  const auto table=24+get(16)*768,base=table+get(12)*4,at=base+get(table),size=get(at);
  legacy::CanvasProducer raster{};auto &r=raster.fields;
  r[2]=9;r[3]=7;r[5]=5;r[6]=3;r[8]=2;r[12]=5;r[13]=3;r[15]=get(16)?1:8;r[17]=get(16)!=0;r[19]=size;r[20]=get(16)?512:0;r[18]=size+r[20];
  raster.payload.assign(raw.begin()+at,raw.begin()+at+size);std::fill(raster.payload.begin()+28,raster.payload.begin()+32,0);
  if(r[17])for(unsigned i=0;i<256;++i){raster.payload.push_back(0);raster.payload.push_back(0xfc);}
  legacy::CanvasProducerReplay producer([](const auto&)->std::vector<std::uint8_t>{throw std::runtime_error("Unexpected file source");});
  legacy::CanvasProducer create{};create.fields[2]=1;create.fields[3]=7;create.fields[5]=5;create.fields[6]=3;producer.apply(create);
  rejected([&]{producer.read(7);});
  legacy::CanvasProducer fill{};fill.fields[2]=5;fill.fields[3]=7;fill.fields[12]=5;fill.fields[13]=3;fill.fields[14]=0x1234;producer.apply(fill);
  render::GlBlitter renderer;
  {
    legacy::CanvasWorld world(renderer,store(root));
    legacy::CanvasProducer enter{};enter.fields[2]=11;enter.fields[3]=7;enter.fields[5]=5;enter.fields[6]=3;enter.fields[14]=1;
    world.begin(enter,producer.read(7));rejected([&]{world.begin(enter,producer.read(7));});
    producer.apply(raster);world.append(raster);const auto previous=QOpenGLContext::currentContext();
    auto native=world.complete(producer.read(7));producer.commitNativeWorld(7,native);
    require(QOpenGLContext::currentContext()==previous,"World completion changed caller GL context");
    require(world.profile().cacheUploads==1&&world.profile().cacheSurfaces==2&&world.profile().visualChecks==1,"Cold World atlas/binding admission differs");
    require(world.profile().identityHashes==1&&world.profile().identityFrames==1,"Cold World identity was not retained");
    require(native.pixels[0]==0x1234&&native.pixels[7]==(r[17]?0xfc00u:0xf800u),"Native startup contents or World pixel lost");
    auto leave=enter;leave.fields[2]=12;world.end(leave);
    // Intervening native HUD update must enter the next retained World frame.
    fill.fields[12]=1;fill.fields[13]=1;fill.fields[14]=0x4321;producer.apply(fill);
    enter.fields[14]=2;world.begin(enter,producer.read(7));producer.apply(raster);world.append(raster);
    native=world.complete(producer.read(7));require(native.pixels[0]==0x4321,"Intervening HUD producer was reset");
    require(world.profile().cacheUploads==0&&world.profile().visualChecks==0&&world.profile().visualReuses==1,"Warm World repeated upload or visual verification");
    require(world.profile().identityHashes==0&&world.profile().identityHits==1,"Warm World rehashed encoded input");
    leave.fields[14]=2;world.end(leave);require(world.completedQueues()==2&&world.draws()==2,"Native queue accounting");
    enter.fields[14]=4;rejected([&]{world.begin(enter,producer.read(7));});enter.fields[14]=3;enter.fields[3]=8;rejected([&]{world.begin(enter,producer.read(7));});
    auto malformed=raster;malformed.fields[19]++;rejected([&]{legacy::producerWorldDraw(malformed);});
    malformed=raster;malformed.fields[15]=11;rejected([&]{legacy::producerWorldDraw(malformed);});
    malformed=raster;malformed.payload.resize(39);malformed.fields[19]=39;rejected([&]{legacy::producerWorldDraw(malformed);});
    malformed=raster;malformed.fields[19]=1048577;rejected([&]{legacy::producerWorldDraw(malformed);});
    malformed=raster;malformed.payload[0]^=1;rejected([&]{legacy::producerWorldDraw(malformed);});
    malformed=raster;malformed.payload[28]=1;rejected([&]{legacy::producerWorldDraw(malformed);});
  }
  {
    legacy::SnapshotFrame frame{false,QByteArray(reinterpret_cast<const char*>(raster.payload.data()),int(r[19]))};
    QByteArray borrowedBytes(frame.encoded.constData(),frame.encoded.size());
    legacy::SnapshotFrame borrowed{false,QByteArray::fromRawData(borrowedBytes.constData(),borrowedBytes.size())};
    legacy::WorldIdentityCache cache({2,512});const auto first=cache.identity(borrowed);
    // Modify the external fromRawData buffer in place: cached keys must own it.
    borrowedBytes.data()[54]^=1;const auto changed=cache.identity(borrowed);
    require(changed.digest()!=first.digest(),"Changed borrowed source reused an old identity");
    require(cache.identity(frame).digest()==first.digest()&&cache.stats().hashes==2&&cache.stats().hits==1,"Cache borrowed caller storage");
    auto tagged=frame;tagged.indexed=true;require(cache.identity(tagged).digest()!=first.digest(),"Indexed tag omitted from identity key");
    require(cache.stats().frames==2&&cache.stats().evictions==1,"Frame count bound did not evict");
    require(cache.identity(frame).digest()==first.digest(),"Retained token changed after eviction");
    auto malformed=frame;malformed.encoded[0]^=1;
    const auto before=cache.stats();rejected([&]{cache.identity(malformed);});
    require(cache.stats().hashes==before.hashes&&cache.stats().frames==before.frames,"Malformed frame entered identity cache");
    rejected([&]{legacy::WorldIdentityCache invalid({0,512});});
    rejected([&]{legacy::WorldIdentityCache invalid({4097,512});});
    const auto capacity=std::size_t(QByteArray(frame.encoded.constData(),frame.encoded.size()).capacity());
    legacy::WorldIdentityCache byteBound({4,capacity*2});byteBound.identity(frame);byteBound.identity(tagged);
    auto third=frame;third.encoded[54]^=2;byteBound.identity(third);
    require(byteBound.stats().frames==2&&byteBound.stats().bytes<=capacity*2&&byteBound.stats().evictions==1,"Owned byte capacity bound did not evict");
    legacy::WorldIdentityCache bypass({1,capacity-1});bypass.identity(frame);bypass.identity(frame);
    require(bypass.stats().frames==0&&bypass.stats().hashes==2&&bypass.stats().bypasses==2,"Oversized identity was cached or refused");
  }
  {
    const auto input=store(root);assets::ResourceManager manager(input);
    legacy::SnapshotResources bindings(input,manager);const assets::ResourceId id{assets::ResourceKind::ui,"revision/body"};
    bindings.add(id,"body.spr",QCryptographicHash::hash(raw,QCryptographicHash::Sha256).toHex());
    legacy::SnapshotFrame frame{bool(r[17]),{reinterpret_cast<const char*>(raster.payload.data()),int(r[19])}};
    legacy::WorldIdentityCache identities;const auto identity=identities.identity(frame);
    bindings.resolve(frame);bindings.resolve(identity);
    require(bindings.stats().visualChecks==1&&bindings.stats().visualReuses==1,"Resident visual identity was rehashed");
    manager.unload(id);bindings.resolve(identity);
    require(bindings.stats().visualChecks==2,"Reload did not recheck visual identity");
    manager.unload(id);write(root,"body.spr",sprite(false,0x07e0));
    rejected([&]{bindings.resolve(identity);});rejected([&]{bindings.resolve(identity);});
    require(bindings.stats().visualChecks==4,"Rejected revision entered visual cache");
    manager.unload(id);write(root,"body.spr",Bytes(raw.begin(),raw.end()));bindings.resolve(identity);
    require(bindings.stats().visualChecks==5,"Restored revision was not checked");
    manager.unload(id);manager.adopt(assets::prepareResource(manager.request(id)));bindings.resolve(identity);
    require(bindings.stats().visualChecks==6,"Prepared resource adoption skipped token verification");
    const auto indexed=sprite(true);auto other=indexed;other[27]=0;other[29]=255;
    write(root,"indexed.spr",indexed);write(root,"other.spr",other);
    for(const auto& item:std::vector<std::pair<std::string,Bytes>>{{"indexed.spr",indexed},{"other.spr",other}}){
      const QByteArray data(reinterpret_cast<const char*>(item.second.data()),int(item.second.size()));
      bindings.add({assets::ResourceKind::ui,item.first=="indexed.spr"?"indexed":"other"},item.first,QCryptographicHash::hash(data,QCryptographicHash::Sha256).toHex());
    }
    legacy::SnapshotFrame ambiguous{true,QByteArray(reinterpret_cast<const char*>(indexed.data()+804),53)};
    const auto indexedIdentity=identities.identity(ambiguous);
    bindings.resolve(indexedIdentity,true);bindings.resolve(indexedIdentity,true);
    rejected([&]{bindings.resolve(indexedIdentity,false);}); // cached identity cannot waive palette ambiguity
  }
  {
    const auto input=store(root);assets::ResourceManager manager(input);legacy::WorldResources resources(input,manager);
    const auto spriteDraw=legacy::producerWorldDraw(raster);auto primitive=spriteDraw;primitive.frame={};
    primitive.additive=render::AdditiveRectangle{1,1,{1,2,3}};
    legacy::WorldFrame frame{1,5,3,5,{spriteDraw,primitive,spriteDraw}};
    const auto cold=resources.display(frame);const auto warm=resources.display(frame);
    require(cold.size()==3&&warm.size()==3&&warm[1].additive&&warm[0].resource==warm[2].resource,"Mixed primitive/sprite order changed");
    require(resources.identityStats().hashes==1&&resources.identityStats().hits==3,"Repeated World frames were not deduplicated");
    auto unmapped=spriteDraw;unmapped.frame.encoded[54]^=4;frame.draws={unmapped};
    rejected([&]{resources.display(frame);});rejected([&]{resources.display(frame);});
    frame.draws={spriteDraw};require(resources.display(frame).size()==1,"Unmapped identity poisoned valid World binding");
  }
  {
    const auto input=store(root);assets::ResourceManager manager(input);legacy::SnapshotResources bindings(input,manager);
    const assets::ResourceId id{assets::ResourceKind::ui,"prepared/body"};
    manager.bind(id,{assets::ResourceImageFormat::sprite,"body.spr",{},{},{}});
    bindings.addPrepared(assets::prepareResource(manager.request(id)),QCryptographicHash::hash(raw,QCryptographicHash::Sha256).toHex());
    require(manager.stats().loads==1&&manager.stats().residentResources==1,"Pinned indexing did not retain its decoded resource");
    legacy::SnapshotFrame frame{bool(r[17]),{reinterpret_cast<const char*>(raster.payload.data()),int(r[19])}};
    bindings.resolve(frame);bindings.resolve(frame);
    require(manager.stats().loads==1&&bindings.stats().visualChecks==1&&bindings.stats().visualReuses==1,"First resolve repeated pinned decode or omitted revision verification");
    manager.unload(id);write(root,"body.spr",sprite(false,0x07e0));
    rejected([&]{bindings.resolve(frame);});
    manager.unload(id);write(root,"body.spr",Bytes(raw.begin(),raw.end()));bindings.resolve(frame);
    require(bindings.stats().visualChecks==3,"Reload did not recheck prepared indexing");
    const assets::ResourceId late{assets::ResourceKind::ui,"prepared/late"};
    manager.bind(late,{assets::ResourceImageFormat::sprite,"body.spr",{},{},{}});
    bindings.addPrepared(assets::prepareResource(manager.request(late)),QCryptographicHash::hash(raw,QCryptographicHash::Sha256).toHex());
    manager.unload(late);write(root,"body.spr",sprite(false,0x07e0));
    rejected([&]{bindings.resolve(frame);}); // The unrequested alias must re-pin.
    manager.unload(late);write(root,"body.spr",Bytes(raw.begin(),raw.end()));bindings.resolve(frame);
    const auto indexed=sprite(true);auto other=indexed;other[27]=0;other[29]=255;
    write(root,"prepared-indexed.spr",indexed);write(root,"prepared-other.spr",other);
    for(const auto& item:std::vector<std::pair<std::string,Bytes>>{{"prepared-indexed.spr",indexed},{"prepared-other.spr",other}}){
      const QByteArray data(reinterpret_cast<const char*>(item.second.data()),int(item.second.size()));
      const assets::ResourceId candidate{assets::ResourceKind::ui,item.first=="prepared-indexed.spr"?"prepared/indexed":"prepared/other"};
      manager.bind(candidate,{assets::ResourceImageFormat::sprite,item.first,{},{},{}});
      bindings.addPrepared(assets::prepareResource(manager.request(candidate)),QCryptographicHash::hash(data,QCryptographicHash::Sha256).toHex());
    }
    legacy::SnapshotFrame indexedFrame{true,QByteArray(reinterpret_cast<const char*>(indexed.data()+804),53)};
    rejected([&]{bindings.resolve(indexedFrame);});bindings.resolve(indexedFrame,true);
  }
  {
    assets::ResourceManager resources(store(root));const render::Image background{5,3,std::vector<std::uint32_t>(15,0x1234)};
    render::SceneRenderer scene(renderer,resources,background);scene.adoptNativeCanvas(background);
    auto invalid=background;invalid.pixels[0]=65536;rejected([&]{scene.adoptNativeCanvas(invalid);});require(scene.read().pixels==background.pixels,"Malformed handoff mutated native canvas");
    scene.beginFrame({},render::SceneStart::retainedCanvas);rejected([&]{scene.adoptNativeCanvas(background);});require(scene.drawNext(1),"Empty native frame completion");
  }
  require(renderer.stats().surfaces==0,"World handoff leaked GPU surfaces");
  std::cout<<"Native producer/World handoff, intervening HUD retention, strict admission and cleanup pass\n";return 0;
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
