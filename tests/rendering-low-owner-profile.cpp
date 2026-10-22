#include "../compat/legacy/world_rolling_session.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <cmath>
#include <iostream>
using namespace mnm;
using namespace resource_test;
int main(int argc,char** argv)try{
 QGuiApplication app(argc,argv);QTemporaryDir temporary;require(temporary.isValid(),"Fixture directory");
 const auto root=std::filesystem::path(temporary.path().toStdString());populate(root);
 render::GlBlitter renderer;
 {
  qunsetenv("MNM_WORLD_OWNER_PROFILE");legacy::WorldRollingSession off(renderer,store(root),false,true);
  qputenv("MNM_WORLD_OWNER_PROFILE","1");legacy::WorldRollingSession on(renderer,store(root),false,true);qunsetenv("MNM_WORLD_OWNER_PROFILE");
  legacy::CanvasProducer create{};create.fields[2]=1;create.fields[3]=7;create.fields[5]=16;create.fields[6]=16;
  auto fill=create;fill.fields[2]=5;fill.fields[12]=16;fill.fields[13]=16;fill.fields[14]=31;
  auto entry=create;entry.fields[2]=11;entry.fields[14]=1;
  // A rolling World requires an actual raster request as well as its return.
  // Keep this source above the independent destination-dependent rectangle.
  const auto spriteBytes=sprite();const unsigned frame=36,frameBytes=55;
  auto raster=create;raster.fields[2]=9;raster.fields[8]=2;raster.fields[12]=16;raster.fields[13]=16;raster.fields[15]=8;raster.fields[19]=frameBytes;
  raster.payload.assign(spriteBytes.begin()+frame,spriteBytes.begin()+frame+frameBytes);
  for(unsigned at=28;at<32;++at)raster.payload[at]=0;
  auto draw=create;draw.fields[2]=26;draw.fields[8]=3;draw.fields[9]=4;draw.fields[12]=16;draw.fields[13]=16;draw.fields[15]=2;draw.fields[19]=2;draw.fields[20]=2;draw.payload={0,248,0,248,0,248,0,248};
  auto checkpoint=entry;checkpoint.fields[2]=10;checkpoint.fields[15]=4;
  auto returned=entry;returned.fields[2]=12;
  for(unsigned seq=1;seq<=64;++seq){
   std::vector<legacy::CanvasProducer> ops;
   if(seq==1)ops.push_back(create);
   ops.insert(ops.end(),{fill,entry,raster,draw,checkpoint,returned});
   const auto owned=legacy::decodeRollingQueue(legacy::encodeRollingQueue(ops));
   const auto before=renderer.stats().nativeReadbacks;
   require(off.consumeResident(seq,owned).valid()&&on.consumeResident(seq,owned).valid(),"Resident delivery");
   require(renderer.stats().nativeReadbacks==before,"Ordinary profiling exported pixels");
   const auto& a=off.ownerCosts();const auto& b=on.ownerCosts();
   require(!a.enabled&&b.enabled,"Profile opt-in was not session local");
   for(auto value:{a.validationMs,a.entryMs,a.appendMs,a.completeMs,a.returnMs,a.presentMs,a.totalMs})require(value==0,"Disabled owner clocks reported work");
   for(auto value:{b.validationMs,b.entryMs,b.appendMs,b.completeMs,b.returnMs,b.presentMs,b.totalMs})require(std::isfinite(value)&&value>=0&&value<=b.totalMs,"Invalid owner interval");
   require(b.totalMs>0,"Missing total owner timing");
   for(unsigned op=0;op<28;++op){
    const unsigned expected=op==1?(seq==1):unsigned(op==5||op==11||op==9||op==25||op==26||op==10||op==12);
    require(b.producerCalls[op]==expected&&!a.producerCalls[op]&&a.producerMs[op]==0,"Per-World opcode counters accumulated or changed");
    require(std::isfinite(b.producerMs[op])&&b.producerMs[op]>=0&&b.producerMs[op]<=b.totalMs,"Invalid producer interval");
   }
   const auto expected=off.diagnosticReadResident(renderer).pixels,actual=on.diagnosticReadResident(renderer).pixels;
   require(actual==expected,"Profile on/off changed pixels");
   for(auto at:{67,68,83,84})require(actual[at]==0x780f,"Independently expected half-composite changed");
  }
  require(on.completed()==64&&off.completed()==64,"Profiling advanced source history");
  // The byte-packet and owned-packet entry points must also reset each World.
  qunsetenv("MNM_WORLD_OWNER_PROFILE");legacy::WorldRollingSession packetOff(renderer,store(root),false,false);
  qputenv("MNM_WORLD_OWNER_PROFILE","1");legacy::WorldRollingSession packetOn(renderer,store(root),false,false);
  legacy::WorldRollingSession ownedOn(renderer,store(root),false,false);qunsetenv("MNM_WORLD_OWNER_PROFILE");
  for(unsigned seq=1;seq<=2;++seq){
   std::vector<legacy::CanvasProducer> ops;if(seq==1)ops.push_back(create);
   ops.insert(ops.end(),{fill,entry,raster,draw,checkpoint,returned});
   const auto bytes=legacy::encodeRollingQueue(ops);const auto stream=legacy::decodeRollingQueue(bytes);
   const auto expected=packetOff.consume(seq,bytes).pixels;
   require(packetOn.consume(seq,bytes).pixels==expected&&ownedOn.consumeOwned(seq,stream).pixels==expected,"Packet profiling changed native words");
   for(const auto* costs:{&packetOn.ownerCosts(),&ownedOn.ownerCosts()}){
    require(costs->enabled&&costs->totalMs>0&&costs->validationMs<=costs->totalMs,"Missing packet timing parent");
    require(costs->producerCalls[5]==1&&costs->producerCalls[1]==unsigned(seq==1),"Packet profile accumulated across Worlds");
   }
   require(!packetOff.ownerCosts().enabled&&packetOff.ownerCosts().totalMs==0,"Disabled packet profile measured work");
  }
 }
 require(renderer.stats().surfaces==0,"Profiling retained surfaces");
 std::cout<<"{\"success\":true,\"worlds\":64,\"compared_words\":32768,\"packet_worlds\":2,\"packet_entry_points\":2,\"ordinary_readbacks\":0,\"remaining_surfaces\":0}\n";return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
