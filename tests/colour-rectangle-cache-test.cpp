#include "../renderer/scenes/scene_renderer.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <QTemporaryDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>
#include <iostream>
using namespace mnm;
using namespace resource_test;
static std::uint32_t blend(std::uint32_t s,std::uint32_t d,render::CompositeMode mode){
 const auto m=0x7befu;
 switch(mode){
 case render::CompositeMode::copy:return s;
 case render::CompositeMode::half:return ((s>>1)&m)+((d>>1)&m);
 case render::CompositeMode::quarterSource:{auto h=(d>>1)&m;return h+((h>>1)&m)+((s>>2)&(m>>1)&m);}
 case render::CompositeMode::quarterDestination:{auto h=(s>>1)&m;return h+((h>>1)&m)+((d>>2)&(m>>1)&m);}
 default:throw std::runtime_error("Unexpected fixture mode");
 }
}
static render::SceneDraw tile(unsigned n){
 render::SceneDraw d;render::Image image{int(2+n%63),int(1+n%64),{}};image.pixels.resize(std::size_t(image.width)*image.height);
 for(std::size_t i=0;i<image.pixels.size();++i)image.pixels[i]=(n*863+i*97)&65535;
 d.colourRectangle=render::ColourRectangle{std::move(image)};d.anchorX=int(n*13%170)-20;d.anchorY=int(n*7%115)-10;
 d.composite.mode=render::CompositeMode(n%4==3?4:n%4); // copy,half,quarter-source,quarter-destination
 if(n%3==0)d.viewport=render::Rect{3,5,119,87};
 return d;
}
static render::Image expected(const render::Image& base,const std::vector<render::SceneDraw>& draws){
 auto image=base;
 for(const auto& d:draws){const auto& src=d.colourRectangle->pixels;auto c=d.viewport.value_or(render::Rect{0,0,base.width,base.height});
  for(int y=std::max(d.anchorY,c.top);y<std::min(d.anchorY+src.height,c.bottom);++y)
   for(int x=std::max(d.anchorX,c.left);x<std::min(d.anchorX+src.width,c.right);++x){auto& out=image.pixels[std::size_t(y)*image.width+x];out=blend(src.pixels[std::size_t(y-d.anchorY)*src.width+x-d.anchorX],out,d.composite.mode);}
 }
 return image;
}
int main(int argc,char** argv)try{
 QGuiApplication app(argc,argv);QTemporaryDir temp;require(temp.isValid(),"Fixture directory");const auto root=std::filesystem::path(temp.path().toStdString());populate(root);auto assetStore=store(root);assets::ResourceManager resources(assetStore);render::GlBlitter renderer;
 render::Image base{128,96,std::vector<std::uint32_t>(128*96,0x1234)};
 std::uint64_t hits=0,uploads=0,evictions=0;unsigned checked=0;
 {
  render::SceneLimits limits;limits.cacheColourRectangles=true;render::SceneRenderer cached(renderer,resources,base,limits),serial(renderer,resources,base);
  const auto compare=[&](const std::vector<render::SceneDraw>& draws){const auto oracle=expected(base,draws);cached.draw(draws);serial.draw(draws);require(cached.read().pixels==oracle.pixels&&serial.read().pixels==oracle.pixels,"Cached/serial colour pixels differ from independent arithmetic/clip oracle");++checked;};
  std::vector<render::SceneDraw> eviction;for(unsigned n=0;n<100;++n){auto d=tile(n);d.anchorX=int(n%40);d.anchorY=int(n%20);eviction.push_back(std::move(d));}compare(eviction);
  require(cached.colourCacheStats().evictions>0&&cached.colourCacheStats().entries==64&&cached.colourCacheStats().pixels==524288,"LRU capacity/storage bound");
  std::vector<render::SceneDraw> warm;for(unsigned n=0;n<32;++n)warm.push_back(tile(n));compare(warm);compare(warm);
  const auto before=renderer.stats();cached.draw(warm);const auto after=renderer.stats();require(after.textureUploadCalls==before.textureUploadCalls&&after.textureUploadBytes==before.textureUploadBytes,"Warm native redraw uploaded colour source pixels");require(cached.read().pixels==expected(base,warm).pixels,"Warm native colour pixels");++checked;
  auto changed=warm;changed[10].colourRectangle->pixels.pixels[0]^=65535;changed[10].anchorX=10;changed[10].anchorY=10;compare(changed);compare(warm);
  std::vector<render::SceneDraw> batching;for(unsigned n=0;n<32;++n){auto d=tile(n);d.anchorX=d.anchorY=0;d.composite.mode=render::CompositeMode::copy;batching.push_back(std::move(d));}cached.draw(batching);serial.draw(batching);
  auto a=renderer.stats();cached.draw(batching);auto b=renderer.stats();serial.draw(batching);auto c=renderer.stats();require(b.drawCalls-a.drawCalls<c.drawCalls-b.drawCalls,"Native atlas did not batch colour copies");require(cached.read().pixels==serial.read().pixels&&cached.read().pixels==expected(base,batching).pixels,"Batched overlap ordering mismatch");++checked;
  const auto prior=cached.read().pixels;auto bad=tile(0);bad.colourRectangle->pixels.pixels[0]=65536;rejected([&]{cached.draw({bad});});require(cached.read().pixels==prior,"Refusal changed completed frame");bad=tile(0);bad.colourRectangle->pixels={65,1,std::vector<std::uint32_t>(65)};rejected([&]{cached.draw({bad});});require(cached.read().pixels==prior,"Oversize refusal changed frame");
  const auto stats=cached.colourCacheStats();hits=stats.hits;uploads=stats.uploads;evictions=stats.evictions;require(hits>0&&stats.entries<=64&&stats.pixels==524288,"Cache statistics/bounds");
 }
 require(renderer.stats().surfaces==0,"Native colour cache leaked surfaces");
 std::cout<<QJsonDocument(QJsonObject{{"success",true},{"independent_pixel_cases",int(checked)},{"words_per_case",128*96},{"hits",double(hits)},{"uploads",double(uploads)},{"evictions",double(evictions)},{"warm_source_uploads",0},{"refusals",2},{"remaining_surfaces",0}}).toJson(QJsonDocument::Compact).constData()<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
