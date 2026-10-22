#include "../renderer/blit.hpp"
#include "../renderer/scenes/scene_renderer.hpp"
#include "../compat/legacy/canvas_world.hpp"
#include "../compat/legacy/world_raster_batch.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
using namespace mnm;
using namespace resource_test;
static render::Image oracle(const render::Image& base,const assets::SpriteFrame& f,const render::SceneDraw& d){
 auto result=base;const auto left=std::int64_t(d.anchorX)-f.originX,top=std::int64_t(d.anchorY)-f.originY;const auto clip=d.viewport.value_or(render::Rect{0,0,base.width,base.height});
 for(unsigned row=0;row<f.height;++row)for(unsigned col=0;col<f.width;++col){const auto x=left+col,y=top+row;
  if(x<clip.left||x>=clip.right||y<clip.top||y>=clip.bottom||!f.opaqueMask.at(std::size_t(row)*f.width+col))continue;
  const auto at=y*base.width+x,source=at+d.composite.rowOffsets[unsigned(y)%d.composite.rowPeriod];
  if(source>=std::int64_t(base.pixels.size())&&d.composite.retainOutsideDisplacement)continue;
  require(source>=0&&source<std::int64_t(base.pixels.size()),"Actual covered overflow in expected source fixture");result.pixels.at(std::size_t(at))=base.pixels.at(std::size_t(source));
 }
 return result;
}
int main(int argc,char** argv)try{
 QGuiApplication app(argc,argv);render::GlBlitter g;const render::PixelFormat rgb{16,{0xf800,0x7e0,31}};
 const render::Image base{4,2,{1,2,3,4,5,6,7,8}};
 auto dst=g.create(base,rgb),src=g.create({2,1,{0,0}},rgb),mask=g.create({2,1,{1,0}},{8,{}});
 render::SpriteComposite c;c.mode=render::CompositeMode::displace;c.rowPeriod=2;c.rowOffsets[0]=3;c.rowOffsets[1]=1;
 g.composite(src,dst,{0,0,2,1},2,1,mask,c);auto expected=base;expected.pixels[6]=8;require(g.read(dst).pixels==expected.pixels,"Transparent last pixel wrongly sampled or refused");
 auto cloned=g.cloneSurface(mask);g.update(mask,0,0,{2,1,{1,1}});rejected([&]{g.composite(src,dst,{0,0,2,1},2,1,mask,c);});g.restoreSurface(cloned,mask);g.composite(src,dst,{0,0,2,1},2,1,mask,c);
 auto other=g.create({2,1,{1,1}},{8,{}});g.swapContents(mask,other);rejected([&]{g.composite(src,dst,{0,0,2,1},2,1,mask,c);});g.swapContents(mask,other);g.composite(src,dst,{0,0,2,1},2,1,mask,c);
 g.copy(cloned,mask,{0,0,2,1},0,0);rejected([&]{g.composite(src,dst,{0,0,2,1},2,1,mask,c);});g.update(mask,0,0,{2,1,{1,0}});g.setPalette(mask,0,{{0,0,0},{255,255,255}});g.composite(src,dst,{0,0,2,1},2,1,mask,c);
 g.update(mask,0,0,{2,1,{1,2}});rejected([&]{g.composite(src,dst,{0,0,2,1},2,1,mask,c);});g.update(mask,0,0,{2,1,{1,0}});g.invalidateContents(mask);rejected([&]{g.composite(src,dst,{0,0,2,1},2,1,mask,c);});g.update(mask,0,0,{2,1,{1,1}});c.rowOffsets[1]=0;g.composite(src,dst,{0,0,2,1},2,1,mask,c); // unused larger phase must not refuse
 // Every preflight failure must leave the complete destination unchanged,
 // including malformed phases and mask proof invalidation/recovery.
 const auto unchanged=g.read(dst).pixels;
 for(unsigned variant=0;variant<6;++variant){
  auto bad=c;
  if(variant==0)bad.rowPeriod=0;else if(variant==1)bad.rowPeriod=17;
  else if(variant==2)bad.rowOffsets[1]=-1;else if(variant==3)bad.rowOffsets[1]=17;
  else {bad.rowOffsets[1]=1;if(variant==5)g.copy(cloned,mask,{0,0,2,1},0,0);}
  rejected([&]{g.composite(src,dst,{0,0,2,1},2,1,mask,bad);});
  require(g.read(dst).pixels==unchanged,"Displacement refusal mutated destination");
 }
 g.update(mask,0,0,{2,1,{1,0}});c.rowOffsets[1]=1;g.composite(src,dst,{0,0,2,1},2,1,mask,c);
 require(g.read(dst).pixels==unchanged,"Binary reload did not restore conservative proof admission");
 g.update(mask,0,0,{2,1,{1,1}});
 require(g.stats().coverageProofBytes<=g.stats().pixels,"Mask proof exceeded native pixel budget");
 // Defined edge policy must preserve invalid taps, still draw valid taps and
 // wrap horizontally into an existing next row, even without CPU mask proof.
 c.retainOutsideDisplacement=true;c.rowOffsets[1]=1;g.update(dst,0,0,base);
 g.composite(src,dst,{0,0,2,1},2,1,mask,c);require(g.read(dst).pixels==expected.pixels,"Finite edge policy changed valid sample or failed to retain invalid tap");
 g.copy(cloned,mask,{0,0,2,1},0,0);g.composite(src,dst,{0,0,2,1},2,1,mask,c);
 g.update(dst,0,0,base);c.rowOffsets[0]=1;g.composite(src,dst,{0,0,2,1},2,0,mask,c);auto wrapped=base;wrapped.pixels[2]=4;require(g.read(dst).pixels==wrapped.pixels,"Finite edge policy lost valid row wrapping");
 // A prior wider/taller snapshot leaves reusable storage beyond the current
 // logical freeze. Invalid taps must not sample that stale scratch allocation.
 auto large=g.create({8,8,std::vector<std::uint32_t>(64,0xbeef)},rgb);
 g.additiveRect(large,{0,0,8,8},{0,0,0});
 g.update(mask,0,0,{2,1,{1,1}});g.update(dst,0,0,base);
 g.composite(src,dst,{0,0,2,1},2,1,mask,c);
 require(g.read(dst).pixels==expected.pixels,"Finite edge sampled stale high-water scratch outside logical canvas");
 for(int shift=0;shift<=16;++shift){
  c.rowPeriod=1;c.rowOffsets[0]=shift;g.update(dst,0,0,base);
  auto edge=base;
  for(unsigned at=6;at<8;++at)if(at+shift<base.pixels.size())edge.pixels[at]=base.pixels[at+shift];
  g.composite(src,dst,{0,0,2,1},2,1,mask,c);
  require(g.read(dst).pixels==edge.pixels,"High-water finite edge offset differs from logical word oracle");
 }
 g.destroy(large);
 for(auto id:{dst,src,mask,cloned,other})g.destroy(id);
 unsigned cases=0,oldRefusals=0,edgeRetains=0;
 if(argc==3||argc==4){
  QFile file(QString::fromLocal8Bit(argv[1]));require(file.open(QIODevice::ReadOnly),"Captured source open");auto bytes=file.readAll();std::vector<std::uint8_t> raw(bytes.begin(),bytes.end());auto stream=legacy::decodeCanvasProducers(raw,false);auto assetStore=store(std::filesystem::path(argv[2]));assets::ResourceManager resources(assetStore);legacy::WorldResources bindings(assetStore,resources);
  render::Image background{800,600,std::vector<std::uint32_t>(480000)};for(std::size_t i=0;i<background.pixels.size();++i)background.pixels[i]=i&65535;
  for(const auto& producer:stream.operations){if(producer.fields[2]!=9||producer.fields[14]!=3||legacy::worldRasterAX(producer))continue;
   auto original=legacy::producerWorldDraw(producer);legacy::WorldFrame frame{1,800,600,800,{original}};auto draws=bindings.display(frame);require(draws.size()==1,"Captured displacement binding");const auto& d=draws.front();const auto& sprite=std::get<assets::Sprite>(resources.resident(d.resource).image);const auto& f=sprite.frames.at(d.frame);
   const auto x=std::int64_t(d.anchorX)-f.originX,y=std::int64_t(d.anchorY)-f.originY;const auto right=std::min<std::int64_t>(x+f.width,800),bottom=std::min<std::int64_t>(y+f.height,600);const auto reach=*std::max_element(d.composite.rowOffsets.begin(),d.composite.rowOffsets.begin()+d.composite.rowPeriod);
   if(argc==4&&(x+f.width<780||y+f.height<590))continue;
   if(x>=800||y>=600||x+f.width<=0||y+f.height<=0)continue;
   const auto expectedPixels=oracle(background,f,d);if((bottom-1)*800+right-1+reach>=480000)++oldRefusals;
   for(auto row=std::max<std::int64_t>(0,y);row<bottom;++row)for(auto col=std::max<std::int64_t>(0,x);col<right;++col)
    if(f.opaqueMask.at(std::size_t(row-y)*f.width+col-x)&&row*800+col+d.composite.rowOffsets[unsigned(row)%d.composite.rowPeriod]>=480000)++edgeRetains;
   for(bool atlas:{false,true}){render::SceneLimits limits;limits.cache.indexedAtlas=atlas;render::SceneRenderer scene(g,resources,background,limits);auto before=g.stats();scene.draw(draws);require(g.stats().nativeReadbacks==before.nativeReadbacks&&g.stats().rgbaReadbacks==before.rgbaReadbacks,"Captured displacement delivery readback");require(scene.read().pixels==expectedPixels.pixels,"Captured covered displacement differs from scalar oracle");++cases;}
  }
  require(oldRefusals>0&&cases>0,"Failed public geometry not exercised");
  if(argc==4)require(edgeRetains>0,"Captured bottom-edge failure not exercised");
 }
 require(g.stats().surfaces==0,"Displacement controls leaked surfaces");std::cout<<QJsonDocument(QJsonObject{{"success",true},{"mask_controls",20},{"atomic_refusals",6},{"high_water_edge_checks",18},{"captured_pixel_cases",int(cases)},{"baseline_envelope_refusals",int(oldRefusals)},{"edge_pixels_retained",int(edgeRetains)},{"words_per_captured_case",480000},{"ordinary_readbacks",0},{"remaining_surfaces",0}}).toJson(QJsonDocument::Compact).constData()<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
