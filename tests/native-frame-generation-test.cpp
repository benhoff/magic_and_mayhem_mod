#include "../renderer/commands.hpp"
#include <QGuiApplication>
#include <iostream>
#include <stdexcept>
using namespace mnm::render;
namespace {
void require(bool okay,const char* reason){if(!okay)throw std::runtime_error(reason);}
const PixelFormat rgb{16,{0xf800,0x7e0,31}};
void movingOverlay(bool inherit){
 GlBlitter g;
 const auto source=g.create({800,600,std::vector<std::uint32_t>(480000,0x07e0)},rgb);
 const auto sample=g.allocate(800,600,rgb);
 std::vector<std::vector<std::uint32_t>> shown;
 {
  CommandConsumer consumer(g,[&](auto frame){g.importGpu565(sample,frame);shown.push_back(g.read(sample).pixels);},
   {CommandDiagnostics::Skip,false,CommandStreamMode::Streaming,[&](auto,auto){return g.presentGpu(source);},true,inherit});
  unsigned sequence=0;
  auto submit=[&](unsigned operation,std::initializer_list<unsigned> fields,Image image=Image{}){
   SurfaceCommand c;c.version=4;c.operation=operation;c.sequence=++sequence;
   std::copy(fields.begin(),fields.end(),c.words.begin());c.image=std::move(image);c.format=rgb;consumer.submit(c);
  };
  for(unsigned id:{1u,2u})submit(1,{id,800,600,16,0xf800,0x7e0,31},{800,600,std::vector<std::uint32_t>(480000)});
  submit(19,{1,1,0,123,0});submit(3,{1,2,0,0,800,600,0,0,0,0});
  submit(2,{2,1,2,1,1},{1,1,{0xf800}});submit(6,{2});
  auto first=std::vector<std::uint32_t>(480000,0x07e0);first[1601]=0xf800;
  require(shown.size()==1&&shown.back()==first,"First completed overlay differs");
  g.update(source,0,0,{800,600,std::vector<std::uint32_t>(480000,0x001f)});
  submit(19,{1,2,0,123,0});
  const bool published=consumer.presentImportedSealed();
  require(published==inherit,"First-seal policy did not control inherited presentation");
  if(inherit){
   auto stale=std::vector<std::uint32_t>(480000,0x001f);stale[1601]=0xf800;
   require(shown.size()==2&&shown.back()==stale,"Inherited negative control did not expose the old overlay position");
  }else{
   require(shown.size()==1&&!consumer.sealedGeneration(),"Incomplete new World exposed old UI");
  }
  // The complete second frame moves the overlay from x=1 to x=5.
  submit(3,{1,2,0,0,800,600,0,0,0,0});submit(2,{2,5,2,1,1},{1,1,{0xf800}});submit(6,{2});
  auto second=std::vector<std::uint32_t>(480000,0x001f);second[1605]=0xf800;
  require(shown.back()==second&&shown.size()==(inherit?3u:2u),"Completed second overlay differs");
  require(consumer.sealedGeneration()==2,"Matching second UI was not sealed");
  consumer.presentSealed(g.presentGpu(source),2);
  require(shown.back()==second,"Same-generation repeat moved the completed overlay");
  bool refused=false;try{consumer.presentSealed(g.presentGpu(source),1);}catch(const std::exception&){refused=true;}
  require(refused,"Stale generation accepted");consumer.abort();
 }
 g.destroy(sample);g.destroy(source);require(g.stats().surfaces==0,"Generation fixture leaked surfaces");
}
}
int main(int argc,char** argv)try{
 QGuiApplication app(argc,argv);movingOverlay(true);movingOverlay(false);
 std::cout<<"{\"success\":true,\"inherited_ui_old_position_reproduced\":true,\"matching_ui_wait_checked\":true,\"same_generation_repeat_stable\":true,\"stale_generation_refused\":true,\"diagnostic_pixel_reads_only\":true,\"teardown_surfaces\":0}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
