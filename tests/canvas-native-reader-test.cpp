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
 require(!renderer.stats().surfaces);std::cout<<"{\"success\":true,\"sparse_writes\":true,\"gpu_keyed_copy\":true,\"unknown_history_refused\":true,\"remaining_surfaces\":0}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
