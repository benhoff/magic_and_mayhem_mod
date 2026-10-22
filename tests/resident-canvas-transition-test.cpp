#include "../compat/legacy/world_rolling_session.hpp"
#include "resource-fixtures.hpp"
#include "../protocols/include/mnm/canvas_producers_v3.h"
#include <QGuiApplication>
#include <QFile>
#include <iostream>
#include <map>
using namespace mnm;
using namespace resource_test;
int main(int argc,char** argv)try{
 QGuiApplication app(argc,argv);QTemporaryDir temporary;
 require(temporary.isValid(),"Fixture root");
 const auto root=std::filesystem::path(temporary.path().toStdString());populate(root);
 QFile source(QString::fromStdString((root/"body.spr").string()));
 require(source.open(QIODevice::ReadOnly),"SPR source");const auto raw=source.readAll();
 auto word=[&](int at){const auto* p=reinterpret_cast<const unsigned char*>(raw.data()+at);return p[0]|unsigned(p[1])<<8|unsigned(p[2])<<16|unsigned(p[3])<<24;};
 const auto table=24+word(16)*768,base=table+word(12)*4,at=base+word(table),size=word(at);
 legacy::CanvasProducer raster{};auto& r=raster.fields;
 r[2]=9;r[8]=2;r[14]=2;r[15]=word(16)?1:8;r[17]=word(16)!=0;r[19]=size;r[20]=word(16)?512:0;r[18]=size+r[20];
 raster.payload.assign(raw.begin()+at,raw.begin()+at+size);std::fill(raster.payload.begin()+28,raster.payload.begin()+32,0);
 if(r[17])for(unsigned i=0;i<256;++i){raster.payload.push_back(0);raster.payload.push_back(0xfc);}
 auto packet=[&](unsigned id,unsigned width,unsigned height,bool create,bool release,bool fill){
  std::vector<legacy::CanvasProducer> operations;
  if(release){legacy::CanvasProducer c{};c.fields[2]=2;c.fields[3]=id;operations.push_back(c);}
  if(create){legacy::CanvasProducer c{};c.fields[2]=1;c.fields[3]=id;c.fields[5]=width;c.fields[6]=height;operations.push_back(c);}
  if(fill){legacy::CanvasProducer c{};c.fields[2]=5;c.fields[3]=id;c.fields[12]=width;c.fields[13]=height;c.fields[14]=0x1234+id;operations.push_back(c);}
  legacy::CanvasProducer entry{};entry.fields[2]=11;entry.fields[3]=id;entry.fields[5]=width;entry.fields[6]=height;entry.fields[14]=1;
  auto draw=raster;draw.fields[3]=id;draw.fields[5]=width;draw.fields[6]=height;draw.fields[12]=width;draw.fields[13]=height;
  auto checkpoint=entry;checkpoint.fields[2]=10;checkpoint.fields[15]=4;
  auto returned=entry;returned.fields[2]=12;
  operations.insert(operations.end(),{entry,draw,checkpoint,returned});
  return legacy::decodeRollingQueue(legacy::encodeRollingQueue(operations));
 };
 {
  legacy::CanvasProducerReplay p([](const std::string&){return std::vector<std::uint8_t>{};});
  for(unsigned id:{71u,72u}){
   legacy::CanvasProducer create{};create.fields[2]=1;create.fields[3]=id;create.fields[5]=2;create.fields[6]=1;p.apply(create);
   legacy::CanvasProducer fill{};fill.fields[2]=5;fill.fields[3]=id;fill.fields[12]=2;fill.fields[13]=1;fill.fields[14]=0x2345;p.apply(fill);
  }
  legacy::CanvasProducer copy{};auto& f=copy.fields;
  f[2]=6;f[3]=72;f[4]=71;f[5]=2;f[6]=1;f[12]=3;f[13]=1;f[14]=0xffffffff;
  f[21]=MNM_PRODUCER_NATIVE_COPY_RECT_HRESULT;f[23]=MNM_PRODUCER_NATIVE_COPY_RECT_REFUSED;
  auto good=packet(7,5,3,true,false,true);good.operations.insert(good.operations.begin()+2,copy);
  const auto decoded=legacy::decodeRollingQueue(legacy::encodeRollingQueue(good.operations));
  require(decoded.operations[2].fields[23]==MNM_PRODUCER_NATIVE_COPY_RECT_REFUSED,"Marked copy refusal did not survive wire decoding");
  p.apply(copy);require(p.copyRectRefusals()==1&&p.read(72).pixels==std::vector<std::uint32_t>({0x2345,0x2345}),"Failed native copy mutated its destination");
  for(unsigned mode=0;mode<4;++mode){auto bad=copy;
   if(mode==0)bad.fields[23]=0;
   if(mode==1)bad.fields[21]=0;
   if(mode==2)bad.fields[21]=0x80004005;
   if(mode==3)bad.fields[23]^=1;
   rejected([&]{p.apply(bad);});require(p.copyRectRefusals()==1&&p.read(72).pixels==std::vector<std::uint32_t>({0x2345,0x2345}),"Rejected copy metadata changed history");
   if(mode){auto wire=packet(7,5,3,true,false,true);wire.operations.insert(wire.operations.begin()+2,bad);
    rejected([&]{legacy::decodeRollingQueue(legacy::encodeRollingQueue(wire.operations));});}
  }
  // A no-op refusal cannot smuggle source bytes or an unknown marker.
  for(unsigned mode=0;mode<3;++mode){auto bad=copy;
   if(mode==0){bad.fields[18]=1;bad.payload={0xa5};}
   if(mode==1)bad.payload={0xa5};
   if(mode==2){bad.fields[23]=1;bad.fields[21]=0;}
   rejected([&]{p.apply(bad);});
   require(p.copyRectRefusals()==1&&p.read(72).pixels==std::vector<std::uint32_t>({0x2345,0x2345}),"Payload refusal changed history");
  }
 }
 render::GlBlitter renderer;
 for(unsigned mode=0;mode<3;++mode){
  legacy::WorldRollingSession resident(renderer,store(root),false,true,mode!=0,mode==2);
  std::map<unsigned,std::unique_ptr<legacy::WorldRollingSession>> references;
  std::map<unsigned,unsigned> sequences;
  render::GpuFrame frozen;render::Image frozenPixels;
  for(unsigned step=0;step<7;++step){
   const unsigned id=(step==2||step==6)?9:7;
   const unsigned width=id==9?7:step>=5?4:5,height=id==9?4:step>=5?2:3;
   const bool recreate=step==4||step==5,create=step==0||step==2||recreate;
   if(create){references[id]=std::make_unique<legacy::WorldRollingSession>(renderer,store(root),true);sequences[id]=0;}
   const auto expected=references.at(id)->consumeOwned(++sequences[id],packet(id,width,height,create,false,create));
   const auto frame=resident.consumeResident(step+1,packet(id,width,height,create,recreate,create));
   require(frame.size()==QSize(int(width),int(height)),"Changed World extent was not presented");
   require(resident.diagnosticReadResident(renderer).pixels==expected.pixels,"Changed World did not retain independent CPU-verified pixels");
   // A resident lease shares its current owner's writes. Freeze the last
   // version before switching away; release/recreation must retain that
   // physical allocation for existing leases, even when the logical ID repeats.
   if(step==1||step==3){frozen=frame;frozenPixels=expected;}
   if(mode){require(resident.replayPrepared().valid(),"Prepared replay unavailable after transition");
    require(resident.diagnosticReadResident(renderer).pixels==expected.pixels,"Replay used an old canvas baseline");}
   if(frozen.valid()){
    const auto target=renderer.allocate(frozenPixels.width,frozenPixels.height,{16,{0xf800,0x7e0,31}});
    renderer.importGpu565(target,frozen);
    require(renderer.read(target).pixels==frozenPixels.pixels,"Old GPU lease changed after release/recreation");renderer.destroy(target);
   }
   require(!resident.worldReadbacks(),"World transition exported ordinary pixels");
  }
  require(resident.completed()==7&&resident.firstWorldCanvas()==7&&resident.worldCanvas()==9&&resident.worldCanvasChanges()==3,"Global completion/canvas accounting changed");
  require(resident.worldContextChanges()==5,"Same-ID allocation or extent replacement was missed");
  rejected([&]{resident.consumeResident(9,packet(9,7,4,false,false,false));});
  require(resident.failed()&&resident.completed()==7,"Sequence gap did not retire the session");
  rejected([&]{resident.consumeResident(8,packet(9,7,4,false,false,false));});
 }
 {
  legacy::WorldRollingSession resident(renderer,store(root),false,true);
  resident.consumeResident(1,packet(7,5,3,true,false,true));
  rejected([&]{resident.consumeResident(2,packet(9,7,4,true,false,false));});
  require(resident.failed()&&resident.completed()==1,"Undefined new destination was admitted");
 }
 require(!renderer.stats().surfaces,"Transition resources retained after teardown");
 std::cout<<"{\"success\":true,\"modes\":3,\"queues\":21,\"copy_refusal_controls\":8,\"ordinary_world_readbacks\":0,\"remaining_surfaces\":0,\"independent_cpu_pixels\":true}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
