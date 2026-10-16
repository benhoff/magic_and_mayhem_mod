#include "../renderer/canvas_sequence.hpp"
#include "../renderer/gpu_canvas_mirror.hpp"
#include <QGuiApplication>
#include <iostream>
#include <stdexcept>
using namespace mnm::render;
void require(bool ok){if(!ok)throw std::runtime_error("Native CPU reader control failed");}
int main(int argc,char** argv)try{
 QGuiApplication app(argc,argv);GlBlitter renderer;
 {
 GpuCanvasMirror mirror(renderer);CanvasSequence actual(&mirror),expected;
 for(auto* c:{&actual,&expected}){c->create(1,16,16);c->fill(1,{0,0,16,16},0x1234);}
 actual.publishWrites();actual.discardCpu(1);
 renderer.update(mirror.surface(1),0,0,Image{16,16,std::vector<std::uint32_t>(256,0x6bd2)});
 expected.fill(1,{0,0,16,16},0x6bd2);
 // Sparse CPU writes must publish without uploading stale words between them.
 for(auto* c:{&actual,&expected}){c->fill(1,{2,2,3,3},0x7fff);c->fill(1,{8,2,9,3},0x2222);}
 actual.publishWrites();require(mirror.cpuReaderSnapshots()==0);
 for(auto* c:{&actual,&expected})c->panel(1,{1,1,10,10},15,20,true,true);
 require(mirror.cpuReaderSnapshots()==1&&actual.read(1).pixels==expected.read(1).pixels);
 actual.publishWrites();require(renderer.read(mirror.surface(1)).pixels==expected.read(1).pixels);
 // GPU-only copies must propagate GPU authority without trusting stale keys.
 for(auto* c:{&actual,&expected}){c->create(2,16,16);c->fill(2,{0,0,16,16},0x4321);}
 actual.publishWrites();actual.discardCpu(1);
 for(auto* c:{&actual,&expected}){c->copy(1,2,{0,0,16,16},0,0,0x1234);c->panel(2,{1,1,10,10},16);}
 require(mirror.cpuReaderSnapshots()==2&&actual.read(2).pixels==expected.read(2).pixels);
 actual.create(3,16,16);actual.fill(3,{1,1,2,2},0x1234);actual.publishWrites();actual.discardCpu(3);
 // A native snapshot of one owned pixel must not define its unknown neighbours.
 bool refused=false;try{actual.panel(3,{1,1,3,3},17);}catch(const std::runtime_error&){refused=true;}
 require(refused);refused=false;try{actual.read(3);}catch(const std::runtime_error&){refused=true;}
 require(refused);actual.release(1);actual.release(2);actual.release(3);
 }
 // Menu/loading chunks can encounter these readers before the next World.
 // Every reader must use current native GPU words, never stale CPU colours.
 for(unsigned kind=0;kind<9;++kind){
  GpuCanvasMirror mirror(renderer);CanvasSequence actual(&mirror),expected;
  for(auto* c:{&actual,&expected}){c->create(1,16,16);c->fill(1,{0,0,16,16},0x1234);}
  actual.publishWrites();actual.discardCpu(1);
  renderer.update(mirror.surface(1),0,0,Image{16,16,std::vector<std::uint32_t>(256,0x6bd2)});
  expected.fill(1,{0,0,16,16},0x6bd2);
  mnm::assets::SpriteFrame rgb{};rgb.width=2;rgb.height=4;rgb.opaqueMask.assign(8,1);
  rgb.pixels=std::vector<std::uint16_t>(8,0xf800);
  auto font=rgb;font.pixels=std::vector<std::uint8_t>(8,31);
  std::array<float,64> weights{};for(unsigned i=0;i<64;++i)weights[i]=float(i)/63;
  std::array<std::uint16_t,256> colours{};std::array<int,16> offsets{};offsets.fill(1);
  auto draw=[&](CanvasSequence& c,unsigned id){
   if(kind==0)c.glyph(id,font,2,2,{0,0,16,16},{255,120,40},weights);
   else if(kind==1)c.fade(id);
   else if(kind==2)c.addRgb(id,{2,2,4,6},{1,2,3});
   else if(kind==8)c.panel(id,{2,2,4,6},17);
   else c.raster(id,rgb,2,2,{0,0,16,16},kind-2,8,colours,offsets);
  };
  draw(actual,1);draw(expected,1);
  require(mirror.cpuReaderSnapshots()==1&&actual.read(1).pixels==expected.read(1).pixels);
  actual.publishWrites();require(renderer.read(mirror.surface(1)).pixels==expected.read(1).pixels);
  actual.create(2,16,16);actual.fill(2,{2,2,3,3},0x1234);actual.publishWrites();actual.discardCpu(2);
  bool refused=false;try{draw(actual,2);}catch(const std::runtime_error&){refused=true;}
  require(refused);refused=false;try{actual.read(2);}catch(const std::runtime_error&){refused=true;}
  require(refused);
 }
 require(!renderer.stats().surfaces);std::cout<<"{\"success\":true,\"sparse_writes\":true,\"gpu_keyed_copy\":true,\"unknown_history_refused\":true,\"gpu_reader_families\":9,\"remaining_surfaces\":0}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
