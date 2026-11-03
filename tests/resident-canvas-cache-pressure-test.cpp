#include "../compat/legacy/world_rolling_session.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <iostream>
using namespace mnm;
using namespace resource_test;
legacy::CanvasProducer operation(unsigned kind,unsigned id=1){legacy::CanvasProducer c{};c.fields[2]=kind;c.fields[3]=id;return c;}
legacy::CanvasProducer create(unsigned id){auto c=operation(1,id);c.fields[5]=c.fields[6]=8;return c;}
legacy::CanvasProducer raster;
legacy::CanvasProducerStream frame(bool initial){
 std::vector<legacy::CanvasProducer> ops;
 if(initial){ops.push_back(create(1));auto fill=operation(5);fill.fields[12]=fill.fields[13]=8;fill.fields[14]=0x1234;ops.push_back(fill);}
 auto entry=operation(11);entry.fields[5]=entry.fields[6]=8;entry.fields[14]=1;ops.push_back(entry);
 auto colour=operation(26);colour.fields[5]=colour.fields[6]=8;colour.fields[12]=colour.fields[13]=8;colour.fields[19]=colour.fields[20]=2;
 colour.payload={0xcd,0xab,0xcd,0xab,0xcd,0xab,0xcd,0xab};colour.fields[18]=8;ops.push_back(colour);
 auto sprite=raster;sprite.fields[3]=1;sprite.fields[5]=sprite.fields[6]=8;sprite.fields[8]=sprite.fields[9]=6;sprite.fields[12]=sprite.fields[13]=8;ops.push_back(sprite);
 auto check=entry;check.fields[2]=10;check.fields[15]=4;ops.push_back(check);
 auto leave=entry;leave.fields[2]=12;ops.push_back(leave);
 return legacy::decodeRollingQueue(legacy::encodeRollingQueue(ops));
}
int main(int argc,char** argv)try{
 QGuiApplication app(argc,argv);QTemporaryDir temporary;require(temporary.isValid(),"Fixture root");
 const auto root=std::filesystem::path(temporary.path().toStdString());populate(root);QFile source(QString::fromStdString((root/"body.spr").string()));require(source.open(QIODevice::ReadOnly),"SPR fixture");const auto raw=source.readAll();
 auto word=[&](int at){const auto* p=reinterpret_cast<const unsigned char*>(raw.data()+at);return p[0]|unsigned(p[1])<<8|unsigned(p[2])<<16|unsigned(p[3])<<24;};
 const auto table=24+word(16)*768,base=table+word(12)*4,at=base+word(table),size=word(at);auto& r=raster.fields;
 r[2]=9;r[14]=0;r[15]=word(16)?1:8;r[17]=word(16)!=0;r[19]=size;r[20]=word(16)?512:0;r[18]=size+r[20];
 raster.payload.assign(raw.begin()+at,raw.begin()+at+size);std::fill(raster.payload.begin()+28,raster.payload.begin()+32,0);
 if(r[17])for(unsigned i=0;i<256;++i){raster.payload.push_back(0);raster.payload.push_back(0xfc);}
 render::GlBlitter renderer;
 {
  assets::ResourceManager resources(store(root));render::SceneRenderer scene(renderer,resources,{8,8,std::vector<std::uint32_t>(64,0x1234)});
  scene.beginFrame({});rejected([&]{scene.releaseCachedImages();});
  while(!scene.drawNext(32)){}scene.releaseCachedImages();
  require(scene.read().pixels==std::vector<std::uint32_t>(64,0x1234),"Idle reclamation changed complete scene");
 }

 {
  legacy::WorldRollingSession resident(renderer,store(root),false,true);
  resident.consumeResident(1,frame(true));const auto before=resident.diagnosticReadResident(renderer);
  require(before.pixels[0]==0xabcd&&before.pixels[2]==0x1234,"Initial owned canvas pixels");
  require(renderer.stats().surfaces==5,"Expected one canvas and four cached sprite/colour planes");
  legacy::CanvasProducerStream source;source.version=3;
  for(unsigned id=2;id<=60;++id)source.operations.push_back(create(id));
  resident.consumeSource(source);require(renderer.stats().surfaces==64,"Exact shared renderer cap");
  source.operations={create(61)};resident.consumeSource(source);
  require(renderer.stats().surfaces==61,"Rebuildable cache did not yield to original producer storage");
  require(resident.diagnosticReadResident(renderer).pixels==before.pixels,"Cache reclamation changed native canvas history");
  source.operations.clear();for(unsigned id=2;id<=61;++id)source.operations.push_back(operation(2,id));resident.consumeSource(source);
  resident.consumeResident(2,frame(false));require(renderer.stats().surfaces==5,"Colour cache was not rebuilt");
  require(resident.diagnosticReadResident(renderer).pixels==before.pixels,"Rebuilt cache changed exact owned words");
 }
 require(renderer.stats().surfaces==0,"Pressure fixture leaked surfaces");
 std::cout<<"Shared64surfacecap,cache eviction,unchanged canvas words,rebuild,and zero remaining surfaces\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
