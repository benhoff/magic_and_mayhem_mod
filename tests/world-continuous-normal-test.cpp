#include "../compat/legacy/world_rolling_session.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <QFile>
#include <iostream>
using namespace mnm;
using namespace resource_test;
int main(int argc, char **argv) try {
  QGuiApplication app(argc, argv);
  QTemporaryDir temporary;
  require(temporary.isValid(), "Fixture root");
  const auto root=std::filesystem::path(temporary.path().toStdString());
  populate(root);
  QFile file(QString::fromStdString((root/"body.spr").string()));
  require(file.open(QIODevice::ReadOnly), "SPR fixture");
  const auto raw=file.readAll();
  auto get=[&](int at){const auto* p=reinterpret_cast<const unsigned char*>(raw.data()+at);return p[0]|unsigned(p[1])<<8|unsigned(p[2])<<16|unsigned(p[3])<<24;};
  const auto table=24+get(16)*768, base=table+get(12)*4, at=base+get(table), size=get(at);
  legacy::CanvasProducer raster{};
  auto &r=raster.fields;
  r[2]=9;r[3]=7;r[5]=5;r[6]=3;r[8]=2;r[12]=5;r[13]=3;r[15]=get(16)?1:8;r[17]=get(16)!=0;r[19]=size;r[20]=get(16)?512:0;r[18]=size+r[20];
  raster.payload.assign(raw.begin()+at,raw.begin()+at+size);
  std::fill(raster.payload.begin()+28,raster.payload.begin()+32,0);
  if(r[17])for(unsigned i=0;i<256;++i){raster.payload.push_back(0);raster.payload.push_back(0xfc);}
  legacy::CanvasProducer create{};create.fields[2]=1;create.fields[3]=7;create.fields[5]=5;create.fields[6]=3;
  legacy::CanvasProducer fill{};fill.fields[2]=5;fill.fields[3]=7;fill.fields[12]=5;fill.fields[13]=3;fill.fields[14]=0x1234;
  legacy::CanvasProducer entry{};entry.fields[2]=11;entry.fields[3]=7;entry.fields[5]=5;entry.fields[6]=3;entry.fields[14]=1;
  auto checkpoint=entry;checkpoint.fields[2]=10;checkpoint.fields[15]=4;
  auto returned=entry;returned.fields[2]=12;
  render::GlBlitter renderer;
  {
    legacy::WorldRollingSession verify(renderer,store(root),true), normal(renderer,store(root),false);
    for(unsigned q=1;q<=512;++q){
      std::vector<legacy::CanvasProducer> ops;
      if(q==1){ops.push_back(create);ops.push_back(fill);}
      else{auto hud=fill;hud.fields[12]=1;hud.fields[13]=1;hud.fields[14]=q;ops.push_back(hud);}
      ops.push_back(entry);ops.push_back(raster);ops.push_back(checkpoint);ops.push_back(returned);
      auto bytes=legacy::encodeRollingQueue(ops);
      auto owned=legacy::decodeRollingQueue(bytes);
      const auto expected=verify.consume(q,bytes);
      const auto actual=normal.consumeOwned(q,owned);
      require(actual.width==expected.width&&actual.height==expected.height&&actual.pixels==expected.pixels,"Normal/verification output differs");
      require(actual.pixels[0]==(q==1?0x1234u:q),"Retained HUD history differs");
      require(normal.presentGpu().valid()&&verify.presentGpu().valid(),"Completed GPU frame missing");
    }
    require(normal.completed()==512&&verify.completed()==512,"Completion accounting differs");
    require(normal.producerCacheStats().decodes==0&&verify.producerCacheStats().decodes>0,"Normal path performed CPU reference raster decoding");
    auto malformed=legacy::decodeRollingQueue(legacy::encodeRollingQueue({entry,raster,checkpoint,returned}));
    malformed.operations.back().fields[14]=2;
    rejected([&]{normal.consumeOwned(513,malformed);});
    require(normal.failed()&&normal.completed()==512,"Malformed owned stream was accepted or resumed");
  }
  require(renderer.stats().surfaces==0,"GPU resources retained after teardown");
  std::cout<<"512 identical synthetic owned queues: normal GPU versus CPU-verified output, retained HUD history, direct GPU leases, zero normal CPU raster decode, malformed-stream refusal and zero teardown surfaces\n";
  return 0;
} catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
