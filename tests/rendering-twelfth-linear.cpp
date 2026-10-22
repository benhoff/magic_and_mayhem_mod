#include "../renderer/scenes/scene_renderer.hpp"
#include "../renderer/canvas_sequence.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <iostream>
using namespace mnm;
using namespace resource_test;
// Authored source rows; no original pixels or captures enter these fixtures.
Bytes encoded(bool sparse){
 const unsigned h=4,w=4,base=28,table=40+8*h;
 Bytes frame(table,0),runs,pixels;std::array<unsigned,4> pixelOffsets{};
 for(unsigned row=0;row<h;++row){
  word(frame,40+row*8,table+runs.size());
  pixelOffsets[row]=pixels.size();
  const unsigned n=sparse&&row==3?0:w;
  if(n){runs.push_back(0);runs.push_back(n);}else runs.push_back(w);
  for(unsigned x=0;x<n;++x){pixels.push_back(0x10);pixels.push_back(0x42);}
 }
 for(unsigned row=0;row<h;++row)word(frame,44+row*8,table+runs.size()+pixelOffsets[row]);
 frame.insert(frame.end(),runs.begin(),runs.end());frame.insert(frame.end(),pixels.begin(),pixels.end());
 word(frame,0,frame.size());word(frame,4,w);word(frame,8,h);word(frame,28,0xffffffff);
 Bytes result(base,0);word(result,0,0x00525053);word(result,4,base+frame.size());word(result,8,4);word(result,12,1);
 result.insert(result.end(),frame.begin(),frame.end());return result;
}
int main(int argc,char** argv)try{
 QGuiApplication app(argc,argv);QTemporaryDir temporary;require(temporary.isValid(),"Owned sprite directory");
 const auto root=std::filesystem::path(temporary.path().toStdString());write(root,"dense.spr",encoded(false));write(root,"sparse.spr",encoded(true));
 assets::ResourceManager resources(store(root));
 render::Image base{16,8,{}};for(unsigned i=0;i<128;++i)base.pixels.push_back((i*997+13)&65535);
 render::GlBlitter renderer;unsigned displaced=0,refused=0,shadows=0,wrapped=0;std::uint64_t compared=0;
 for(bool atlas:{false,true})for(bool sparse:{false,true}){
  const assets::ResourceId id{assets::ResourceKind::ui,sparse?"sparse":"dense"};resources.bind(id,spr(sparse?"sparse.spr":"dense.spr"));
  const auto& frame=std::get<assets::Sprite>(resources.load(id).image).frames.at(0);
  render::SceneLimits limits;limits.cache.indexedAtlas=atlas;render::SceneRenderer scene(renderer,resources,base,limits);
  render::SpriteComposite composite;composite.mode=render::CompositeMode::displace;composite.rowPeriod=2;composite.rowOffsets.fill(2);
  auto previous=base;
  for(int x:{0,13,14})for(int y:{0,1,4}){
   auto expected=base;bool outside=false;
   for(unsigned row=0;row<4;++row)for(unsigned col=0;col<4;++col){
    const int dx=x+int(col),dy=y+int(row);if(dx>=16||dy>=8||!frame.opaqueMask[row*4+col])continue;
    const int at=dy*16+dx,sample=at+2;if(sample>=128){outside=true;continue;}
    expected.pixels[at]=base.pixels[sample];wrapped+=dx+2>=16;
   }
   render::CanvasSequence cpu;cpu.create(1,16,8);cpu.update(1,0,0,base);
   const auto draw=render::SceneDraw{id,0,x,y,true,true,{},render::Rect{0,0,16,8},composite};
   const auto before=renderer.stats();
   if(outside){rejected([&]{cpu.raster(1,frame,x,y,{0,0,16,8},3,1,{},composite.rowOffsets,2);});rejected([&]{scene.draw({draw});});++refused;}
   else {cpu.raster(1,frame,x,y,{0,0,16,8},3,1,{},composite.rowOffsets,2);scene.draw({draw});++displaced;}
   const auto after=renderer.stats();require(before.nativeReadbacks==after.nativeReadbacks&&before.rgbaReadbacks==after.rgbaReadbacks,"Displacement delivery readback");
   require(cpu.read(1).pixels==(outside?base:expected).pixels,"Independent CPU linear oracle differs");
   require(scene.read().pixels==(outside?previous:expected).pixels,"Independent GPU linear oracle differs");compared+=256;if(!outside)previous=expected;
  }
  composite.mode=render::CompositeMode::projectedShadow;
  for(int x:{10,11,12,13})for(int y:{0,3})for(int bottom:{5,8}){
   auto expected=base;const int top=y+2,first=0,last=std::min(4,bottom-top);
   if(x+6<=16&&top<bottom)for(int row=0;row<(last-first+1)/2;++row)for(int col=0;col<4;++col){
    if(!frame.opaqueMask[(first+row*2)*4+col])continue;
    const int at=(top+first+row)*16+x+col+last/2-row;expected.pixels.at(at)=(base.pixels.at(at)>>1)&0x7bef;
   }
   render::CanvasSequence cpu;cpu.create(1,16,8);cpu.update(1,0,0,base);
   const auto before=renderer.stats();cpu.raster(1,frame,x,y,{0,0,16,bottom},5,10,{});
   scene.draw({render::SceneDraw{id,0,x,y,true,true,{},render::Rect{0,0,16,bottom},composite}});
   const auto after=renderer.stats();require(before.nativeReadbacks==after.nativeReadbacks&&before.rgbaReadbacks==after.rgbaReadbacks,"Shadow delivery readback");
   require(cpu.read(1).pixels==expected.pixels&&scene.read().pixels==expected.pixels,"Independent full projected envelope oracle differs");++shadows;compared+=256;
  }
 }
 require(!renderer.stats().surfaces,"Linear fixtures leaked surfaces");
 std::cout<<"{\"success\":true,\"displacement_admitted\":"<<displaced<<",\"atomic_displacement_refusals\":"<<refused<<",\"shadow_cases\":"<<shadows<<",\"wrapped_taps\":"<<wrapped<<",\"compared_words\":"<<compared<<",\"ordinary_readbacks\":0,\"remaining_surfaces\":0,\"renderer\":\""<<renderer.driver().renderer<<"\"}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
