#include "../compat/legacy/canvas_world.hpp"
#include "../compat/legacy/world_rolling_session.hpp"
#include "../compat/legacy/world_rolling_handoff.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <QTemporaryDir>
#include <QFile>
#include <chrono>
#include <iostream>
using namespace mnm;
using namespace resource_test;
namespace {
const render::PixelFormat rgb{16,{0xf800,0x7e0,31}};
std::vector<unsigned char> read(const char* name){
 QFile f(QString::fromLocal8Bit(name));require(f.open(QIODevice::ReadOnly),"Fixture open");
 auto b=f.readAll();return {b.begin(),b.end()};
}
unsigned half(unsigned s,unsigned d){return ((s&0xf7de)+(d&0xf7de))>>1;}
legacy::CanvasProducer rectangle(unsigned x,unsigned y,unsigned colour){
 legacy::CanvasProducer c{};c.fields[2]=26;c.fields[3]=7;c.fields[5]=16;c.fields[6]=16;
 c.fields[8]=x;c.fields[9]=y;c.fields[12]=16;c.fields[13]=16;
 c.fields[15]=2;c.fields[19]=2;c.fields[20]=2;
 for(unsigned n=0;n<4;++n){c.payload.push_back(colour&255);c.payload.push_back(colour>>8);}
 return c;
}
void authored(render::GlBlitter& g,const std::filesystem::path& root){
 const auto canvas=g.create({16,16,std::vector<std::uint32_t>(256,0x001f)},rgb);
 {
  legacy::CanvasWorld w(g,store(root));w.enablePreparedReplay(true);
  rejected([&]{w.replayPrepared();});
  legacy::CanvasProducer e{};e.fields[2]=11;e.fields[3]=7;e.fields[5]=16;e.fields[6]=16;e.fields[14]=1;
  auto end=e;end.fields[2]=12;
  for(unsigned generation=1;generation<=2;++generation){
   const unsigned background=generation==1?0x001f:0x07e0;
   g.update(canvas,0,0,{16,16,std::vector<std::uint32_t>(256,background)});
   w.beginResident(e,canvas,generation);rejected([&]{w.replayPrepared();});
   auto red=rectangle(3,4,0xf800),blue=rectangle(4,5,0x001f);
   w.append(red);w.completeResident();w.append(blue);w.completeResident();
   rejected([&]{w.replayPrepared();});w.end(end);
   auto expected=std::vector<std::uint32_t>(256,background);
   for(auto at:{67,68,83,84})expected[at]=half(0xf800,expected[at]);
   for(auto at:{84,85,100,101})expected[at]=half(0x001f,expected[at]);
   require(g.read(canvas).pixels==expected,"Authored destination-dependent World differs");
   const auto before=w.profile();
   for(unsigned n=0;n<40;++n){
    // Simulate later native UI/history writes. They must not enter the frozen World.
    g.update(canvas,0,0,{16,16,std::vector<std::uint32_t>(256,n+0x1234)});
    const auto stats=g.stats();auto frame=w.replayPrepared();
    require(frame.valid()&&g.stats().nativeReadbacks==stats.nativeReadbacks&&g.stats().rgbaReadbacks==stats.rgbaReadbacks,"Image reuse read pixels");
    // The existing native-to-RGBA presentation resolve still draws once.
    require(g.stats().drawInstances==stats.drawInstances+1&&g.stats().copyBatches==stats.copyBatches&&g.stats().snapshotPixels==stats.snapshotPixels&&g.stats().nativeSnapshotCopies==stats.nativeSnapshotCopies+1,"Image reuse repeated World drawing");
    require(g.read(canvas).pixels==expected,"UI contamination, cumulative effect or stale generation");
   }
   require(w.profile().identityHashes==before.identityHashes&&w.profile().visualChecks==before.visualChecks,"Image reuse repeated preparation");
   require(w.retainedImageCaptures()==generation&&w.completedSequence()==generation,"Image reuse changed source sequence/capture count");
  }
  require(w.preparedReplays()==80&&w.draws()==4,"Image reuse advanced native producer draws");
 }
 g.destroy(canvas);require(g.stats().surfaces==0,"Retained image teardown leaked surfaces");
 rejected([&]{legacy::WorldRollingSession invalid(g,store(root),false,true,false,true);});
 rejected([&]{legacy::WorldRollingSession invalid(g,store(root),false,false,true,true);});
}
double benchmark(legacy::WorldRollingSession& w,render::GlBlitter& g){
 for(unsigned i=0;i<20;++i)w.replayPrepared();
 const auto start=std::chrono::steady_clock::now();
 for(unsigned i=0;i<200;++i)w.replayPrepared();
 // Explicit diagnostic synchronization after the batch, included in timing.
 // Ordinary presentation uses the existing GPU fences and never reads pixels.
 w.diagnosticReadResident(g);
 return std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()/200;
}
}
int main(int argc,char** argv)try{
 QGuiApplication app(argc,argv);QTemporaryDir temp;require(temp.isValid(),"Temporary fixture root");
 auto root=std::filesystem::path(temp.path().toStdString());populate(root);render::GlBlitter g;authored(g,root);
 double replayMs=0,reuseMs=0;unsigned actual=0;
 if(argc==4){
  const auto raw=read(argv[1]),request=read(argv[2]);auto stream=legacy::decodeCanvasProducers(raw,false);
  auto handoff=legacy::prepareRollingHandoff(stream,0,request,1,0,legacy::WorldBatchSourcePolicy::ownedV5,false);
  auto assets=store(std::filesystem::path(argv[3]));render::GlBlitter normal,retained;
  legacy::WorldRollingSession replay(normal,assets,false,true,true),reuse(retained,assets,false,true,true,true);
  replay.consumeResident(1,handoff.stream);reuse.consumeResident(1,handoff.stream);
  const auto expected=replay.diagnosticReadResident(normal).pixels;
  require(reuse.diagnosticReadResident(retained).pixels==expected,"Captured primary World differs");
  const auto decodes=reuse.producerCacheStats().decodes;
  for(unsigned i=0;i<8;++i){
   const auto before=retained.stats();replay.replayPrepared();reuse.replayPrepared();
   require(retained.stats().nativeReadbacks==before.nativeReadbacks&&retained.stats().rgbaReadbacks==before.rgbaReadbacks,"Captured image reuse readback");
   require(replay.diagnosticReadResident(normal).pixels==expected&&reuse.diagnosticReadResident(retained).pixels==expected,"Captured full-frame replay/reuse parity");
   ++actual;
  }
  replayMs=benchmark(replay,normal);reuseMs=benchmark(reuse,retained);
  require(reuse.completed()==1&&reuse.retainedImageCaptures()==1&&reuse.producerCacheStats().decodes==decodes,"Image reuse changed producer history");
 }
 std::cout<<"{\"success\":true,\"destination_dependent_repeats\":80,\"source_updates\":2,\"ui_contamination_checked\":true,\"active_world_refused\":true,\"actual_source_repeats\":"<<actual
 <<",\"prepared_replay_ms\":"<<replayMs<<",\"retained_image_ms\":"<<reuseMs<<",\"benchmark_repeats_per_policy\":"<<(argc==4?200:0)
 <<",\"ordinary_readbacks\":0,\"producer_history_unchanged\":true,\"teardown_surfaces\":0}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
