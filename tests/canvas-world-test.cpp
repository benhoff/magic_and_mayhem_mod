#include "../compat/legacy/canvas_world.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <QFile>
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
    producer.apply(raster);world.append(raster);auto native=world.complete(producer.read(7));producer.commitNativeWorld(7,native);
    require(native.pixels[0]==0x1234&&native.pixels[7]==(r[17]?0xfc00u:0xf800u),"Native startup contents or World pixel lost");
    auto leave=enter;leave.fields[2]=12;world.end(leave);
    // Intervening native HUD update must enter the next retained World frame.
    fill.fields[12]=1;fill.fields[13]=1;fill.fields[14]=0x4321;producer.apply(fill);
    enter.fields[14]=2;world.begin(enter,producer.read(7));producer.apply(raster);world.append(raster);
    native=world.complete(producer.read(7));require(native.pixels[0]==0x4321,"Intervening HUD producer was reset");
    leave.fields[14]=2;world.end(leave);require(world.completedQueues()==2&&world.draws()==2,"Native queue accounting");
    enter.fields[14]=4;rejected([&]{world.begin(enter,producer.read(7));});enter.fields[14]=3;enter.fields[3]=8;rejected([&]{world.begin(enter,producer.read(7));});
    auto malformed=raster;malformed.fields[19]++;rejected([&]{legacy::producerWorldDraw(malformed);});
    malformed=raster;malformed.fields[15]=11;rejected([&]{legacy::producerWorldDraw(malformed);});
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
