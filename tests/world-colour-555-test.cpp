#include "../compat/legacy/world_frame.hpp"
#include "resource-fixtures.hpp"
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>
#include <iostream>
using namespace mnm;
using namespace resource_test;
static void put(QByteArray& b,unsigned at,unsigned v){for(unsigned i=0;i<4;++i)b[at+i]=char(v>>(8*i));}
int main(int argc,char** argv)try{
 QGuiApplication app(argc,argv);require(argc==3,"Expected owned input directory and new native output directory");
 const auto inputs=std::filesystem::path(argv[1]),outputs=std::filesystem::path(argv[2]);
 require(!std::filesystem::exists(outputs),"Refusing output overwrite");std::filesystem::create_directory(outputs);
 QTemporaryDir empty;require(empty.isValid(),"Asset-free fixture root");const auto assets=store(std::filesystem::path(empty.path().toStdString()));
 assets::ResourceManager resources(assets);legacy::SnapshotResources bindings(assets,resources);render::GlBlitter renderer;
 std::vector<std::filesystem::path> paths;for(const auto& e:std::filesystem::directory_iterator(inputs))if(e.path().extension()==".world")paths.push_back(e.path());std::sort(paths.begin(),paths.end());require(!paths.empty()&&paths.size()<=256,"Fixture count");
 unsigned refusals=0,rgb555=0,rgb565=0;std::uint64_t cacheHits=0;
 for(const auto& path:paths){
  require(std::filesystem::file_size(path)<=2*1024*1024,"Fixture input budget");std::ifstream f(path,std::ios::binary);const Bytes bytes((std::istreambuf_iterator<char>(f)),{});
  const QByteArray raw(reinterpret_cast<const char*>(bytes.data()),qsizetype(bytes.size()));
  const auto frame=legacy::decodeWorldFrame(raw,legacy::WorldInputPolicy::colourRgb555);const auto draws=legacy::worldDisplay(frame,bindings);
  if(frame.colourRgb555){
   ++rgb555;rejected([&]{legacy::decodeWorldFrame(raw);});++refusals;
   for(const auto mutation:std::vector<std::pair<unsigned,unsigned>>{{72,2},{84,6},{144,0},{152,4}}){auto b=raw;put(b,mutation.first,mutation.second);rejected([&]{legacy::decodeWorldFrame(b,legacy::WorldInputPolicy::colourRgb555);});++refusals;}
   auto high=raw;high[157]=char(static_cast<unsigned char>(high[157])|0x80);rejected([&]{legacy::decodeWorldFrame(high,legacy::WorldInputPolicy::colourRgb555);});++refusals;
  }else{++rgb565;require(legacy::decodeWorldFrame(raw).draws.size()==draws.size(),"RGB565 default changed");}
  for(const bool cached:{false,true}){
   render::SceneLimits limits;limits.colourRgb555=frame.colourRgb555;limits.cacheColourRectangles=cached;
   render::SceneRenderer scene(renderer,resources,{int(frame.width),int(frame.height),std::vector<std::uint32_t>(std::size_t(frame.width)*frame.height,0xffff)},limits);
   scene.draw(draws);const auto image=scene.read();
   std::ofstream out(outputs/(path.filename().string()+(cached?".cached":".plain")),std::ios::binary);for(auto v:image.pixels){out.put(char(v));out.put(char(v>>8));}require(bool(out),"Native output write");
   if(frame.colourRgb555){
    const auto before=image.pixels;auto invalid=draws;invalid.back().colourRectangle.reset();invalid.back().additive=render::AdditiveRectangle{1,1,{1,1,1}};
    rejected([&]{scene.draw(invalid);});require(scene.read().pixels==before,"RGB555 late refusal changed completed frame");++refusals;
    invalid=draws;invalid.back().colourRectangle->pixels.pixels.back()|=0x8000;
    rejected([&]{scene.draw(invalid);});require(scene.read().pixels==before,"RGB555 high-bit refusal changed completed frame");++refusals;
   }
   if(cached){scene.draw(draws);require(scene.read().pixels==image.pixels,"Repeated cached frame differs");cacheHits+=scene.colourCacheStats().hits;}
  }
 }
 require(rgb555&&rgb565&&cacheHits,"Missing format or cache controls");require(renderer.stats().surfaces==0,"World fixture leaked native surfaces");const auto d=renderer.driver();
 std::cout<<QJsonDocument(QJsonObject{{"success",true},{"cases",int(paths.size())},{"rgb555",int(rgb555)},{"rgb565",int(rgb565)},{"refusals",int(refusals)},{"cache_hits",qint64(cacheHits)},{"remaining_surfaces",0},{"original_oracles_read",false},{"driver",QJsonObject{{"vendor",QString::fromStdString(d.vendor)},{"renderer",QString::fromStdString(d.renderer)},{"version",QString::fromStdString(d.version)}}}}).toJson(QJsonDocument::Compact).constData()<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
