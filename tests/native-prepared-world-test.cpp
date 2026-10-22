#include "../compat/legacy/canvas_world.hpp"
#include "../compat/legacy/world_rolling_session.hpp"
#include "../compat/legacy/world_rolling_handoff.hpp"
#include "../renderer/surface_copy.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <QTemporaryDir>
#include <QFile>
#include <iostream>
using namespace mnm;
using namespace resource_test;
namespace {
const render::PixelFormat rgb{16,{0xf800,0x7e0,31}};
std::vector<unsigned char> read(const char* name){QFile f(QString::fromLocal8Bit(name));require(f.open(QIODevice::ReadOnly),"Fixture open");auto b=f.readAll();return {b.begin(),b.end()};}
void baselines(render::GlBlitter& g){
 render::Image image{4,3,{0,1,2,3,4,5,6,7,8,9,10,65535}};
 const auto s=g.create(image,rgb);g.setClipper(s,{true,std::vector<render::Rect>{{1,1,3,2}}});
 const auto b=g.cloneSurface(s),d=g.cloneSurface(b);
 g.update(s,0,0,{4,3,std::vector<std::uint32_t>(12,0x1234)});g.destroy(s);
 require(g.read(b).pixels==image.pixels,"Baseline changed with source mutation/release");
 g.update(d,0,0,{4,3,std::vector<std::uint32_t>(12,0x4321)});g.restoreSurface(b,d);require(g.read(d).pixels==image.pixels,"Native baseline restoration differs");
 auto src=g.create({4,3,std::vector<std::uint32_t>(12,0xf800)},rgb);
 render::SurfaceCopyRequest req;req.api=render::SurfaceCopyApi::Blt;req.source={0,0,4,3};req.destination={0,0,4,3};
 require(g.surfaceCopy(src,d,req).hresult==0,"Restored clipper copy failed");auto clipped=image.pixels;clipped[5]=clipped[6]=0xf800;require(g.read(d).pixels==clipped,"Clipper not preserved");
 const auto unknown=g.allocate(4,3,rgb);g.update(unknown,1,1,{1,1,{0x5678}});const auto sparse=g.cloneSurface(unknown);rejected([&]{g.read(sparse);});g.update(sparse,0,0,image);g.restoreSurface(unknown,sparse);rejected([&]{g.read(sparse);});
 auto narrow=g.allocate(3,3,rgb),other=g.allocate(4,3,{16,{0x7c00,0x3e0,31}});rejected([&]{g.restoreSurface(b,narrow);});rejected([&]{g.restoreSurface(b,other);});rejected([&]{g.restoreSurface(b,b);});
 const auto indexed=g.create({2,1,{0,1}},{8,{}});g.setPalette(indexed,0,{{255,0,0},{0,0,255}});const auto palette=g.cloneSurface(indexed);g.setPalette(indexed,0,{{0,255,0},{255,255,255}});auto a=g.present(palette);require(a.pixelColor(0,0)==QColor(255,0,0)&&a.pixelColor(1,0)==QColor(0,0,255),"Indexed palette snapshot lost");
 for(auto id:{b,d,src,unknown,sparse,narrow,other,indexed,palette})g.destroy(id);
}
void prepared(render::GlBlitter& g,const std::filesystem::path& root){
 const auto canvas=g.create({16,16,std::vector<std::uint32_t>(256,0x001f)},rgb);
 {
  legacy::CanvasWorld w(g,store(root));w.enablePreparedReplay();rejected([&]{w.replayPrepared();});
  legacy::CanvasProducer e{};e.fields[2]=11;e.fields[3]=7;e.fields[5]=16;e.fields[6]=16;e.fields[14]=1;
  w.beginResident(e,canvas,1);
  legacy::CanvasProducer c{};c.fields[2]=26;c.fields[3]=7;c.fields[5]=16;c.fields[6]=16;c.fields[8]=3;c.fields[9]=4;c.fields[12]=16;c.fields[13]=16;c.fields[15]=2;c.fields[19]=2;c.fields[20]=2;c.payload={0,248,0,248,0,248,0,248};w.append(c);
  w.completeResident();auto end=e;end.fields[2]=12;w.end(end);
  auto expected=std::vector<std::uint32_t>(256,0x001f);for(auto at:{67,68,83,84})expected[at]=0x780f;
  require(g.read(canvas).pixels==expected,"Independently expected half-composite differs");
  c.payload.assign(8,0);auto before=w.profile();
  for(unsigned n=0;n<40;++n){
   const auto reads=g.stats().nativeReadbacks,rgba=g.stats().rgbaReadbacks;auto frame=w.replayPrepared();require(frame.valid()&&g.stats().nativeReadbacks==reads&&g.stats().rgbaReadbacks==rgba,"Replay exported native pixels");
   require(g.read(canvas).pixels==expected,"Repeated half-composite accumulated or source alias changed");
  }
  require(w.completedSequence()==1&&w.draws()==1&&w.preparedReplays()==40,"Extra rendering changed producer sequence/count");
  require(w.profile().identityHashes==before.identityHashes&&w.profile().visualChecks==before.visualChecks,"Replay repeated producer/binding preparation");
  // A later update takes a new native baseline, not the first update's history.
  g.update(canvas,0,0,{16,16,std::vector<std::uint32_t>(256,0x07e0)});w.beginResident(e,canvas,2);c.payload={0,248,0,248,0,248,0,248};w.append(c);w.completeResident();w.end(end);
  auto second=std::vector<std::uint32_t>(256,0x07e0);for(auto at:{67,68,83,84})second[at]=0x7be0;
  require(g.read(canvas).pixels==second,"Second native baseline mismatch");w.replayPrepared();require(g.read(canvas).pixels==second,"Replay retained stale first baseline");
  // A new source may replace both storage identity and extent. Two completed
  // subqueues freeze the baseline once, rather than freezing after the first.
  const auto replacement=g.create({8,8,std::vector<std::uint32_t>(64,31)},rgb);
  e.fields[5]=e.fields[6]=8;end=e;end.fields[2]=12;
  rejected([&]{w.beginResident(e,replacement,4);});
  w.beginResident(e,replacement,3);c.fields[5]=c.fields[6]=c.fields[12]=c.fields[13]=8;
  w.append(c);w.completeResident();w.append(c);w.completeResident();
  rejected([&]{w.replayPrepared();});auto invalid=end;invalid.fields[3]=8;
  rejected([&]{w.end(invalid);});w.end(end);
  auto third=std::vector<std::uint32_t>(64,31);for(auto at:{35,36,43,44})third[at]=0xb007;
  require(g.read(replacement).pixels==third,"Split native queues did not draw twice");
  for(unsigned i=0;i<12;++i){g.update(replacement,0,0,{8,8,std::vector<std::uint32_t>(64,0xffff)});w.replayPrepared();require(g.read(replacement).pixels==third,"Replacement baseline or split replay accumulated");}
  require(w.completedSequence()==3&&w.draws()==4,"Replay advanced replacement producer history");
  g.destroy(replacement);
 }
 g.destroy(canvas);
}
}
int main(int argc,char** argv)try{
 QGuiApplication app(argc,argv);QTemporaryDir temp;require(temp.isValid(),"Temporary fixture root");auto root=std::filesystem::path(temp.path().toStdString());populate(root);render::GlBlitter g;baselines(g);prepared(g,root);
 if(argc==4){
  const auto raw=read(argv[1]),request=read(argv[2]);auto stream=legacy::decodeCanvasProducers(raw,false);auto handoff=legacy::prepareRollingHandoff(stream,0,request,1,0,legacy::WorldBatchSourcePolicy::ownedV5,false);auto assets=store(std::filesystem::path(argv[3]));
  render::GlBlitter normal,extra;legacy::WorldRollingSession baseline(normal,assets,false,true),candidate(extra,assets,false,true,true);
  baseline.consumeResident(1,handoff.stream);candidate.consumeResident(1,handoff.stream);
  auto expected=baseline.diagnosticReadResident(normal).pixels;require(candidate.diagnosticReadResident(extra).pixels==expected,"Actual owned source primary differs");
  const auto decodes=candidate.producerCacheStats().decodes;
  for(unsigned i=0;i<4;++i){auto reads=extra.stats().nativeReadbacks;candidate.replayPrepared();require(extra.stats().nativeReadbacks==reads,"Actual-source replay readback");require(candidate.diagnosticReadResident(extra).pixels==expected,"Actual-source replay pixel parity");}
  require(candidate.completed()==1&&candidate.producerCacheStats().decodes==decodes,"Actual-source replay advanced producer preparation");
 }
 require(g.stats().surfaces==0,"Prepared fixture leaked native surfaces");std::cout<<"{\"baseline_controls\":9,\"destination_dependent_repeats\":53,\"source_updates\":3,\"replacement_repeats\":12,\"split_queues\":true,\"actual_source_repeats\":"<<(argc==4?4:0)<<",\"producer_history_unchanged\":true,\"ordinary_readbacks\":0,\"teardown_surfaces\":0}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
