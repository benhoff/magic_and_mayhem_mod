#include "scene_renderer.hpp"
#include "resource-fixtures.hpp"
#include "rendering-wave-fixture.hpp"
#include "../reconstruction/rendering/startup_canvas.hpp"
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <fstream>
using namespace mnm;
int main(int argc,char** argv)try{
  QGuiApplication app(argc,argv);QTemporaryDir directory;
  std::ofstream pixelFile;
  if(argc==3 && std::string(argv[1])=="--wave-pixels")pixelFile.open(argv[2],std::ios::binary);
  else resource_test::require(argc==1,"GPU arguments");
  resource_test::require(argc==1||bool(pixelFile),"GPU pixel output");
  resource_test::require(directory.isValid(),"GPU fixture directory");
  const auto root=std::filesystem::path(directory.path().toStdString());
  assets::ResourceManager resources(resource_test::store(root));
  const assets::ResourceId ids[]={{assets::ResourceKind::ui,"whole"},{assets::ResourceKind::ui,"split"}};
  for(unsigned split:{0u,1u}){const auto name=std::to_string(split)+".spr";resource_test::write(root,name,wave_fixture::sprite(split));resources.bind(ids[split],resource_test::spr(name));}
  render::Image initial{64,32,std::vector<std::uint32_t>(64*32)};wave_fixture::pattern(initial.pixels);
  render::GlBlitter renderer;std::vector<std::uint32_t> hashes;
  {
    render::SceneLimits limits;limits.cache.indexedAtlas=true;
    render::SceneRenderer scene(renderer,resources,initial,limits);
    const auto table=reconstruction::startupWaveTable();
    wave_fixture::cases([&](unsigned amplitude,unsigned,unsigned phase,int x,int y,unsigned split){
      scene.adoptNativeCanvas(initial);
      render::SceneDraw draw;draw.resource=ids[split];draw.anchorX=x;draw.anchorY=y;draw.viewport=render::Rect{2,2,40,25};
      draw.composite.mode=render::CompositeMode::displace;draw.composite.rowPeriod=2*amplitude;
      draw.composite.rowOffsets=reconstruction::startupWaveOffsets(table,amplitude,phase);draw.wholeOpaqueRunClipping=true;
      if(x<2||x+7>40){draw.composite.rowPeriod=16;for(unsigned row=0;row<16;++row)draw.composite.rowOffsets[row]=std::abs(8-int((row+phase)&15));}
      scene.beginFrame({draw},render::SceneStart::retainedCanvas);while(!scene.drawNext(32)){}
      const auto completed=scene.read();hashes.push_back(wave_fixture::hash(completed.pixels));
      if(pixelFile.is_open())wave_fixture::write(pixelFile,completed.pixels);
    });
  }
  resource_test::require(renderer.stats().surfaces==0,"GPU wave leaked storage");
  const auto driver=renderer.driver();
  std::cout<<"{\"driver\":"<<QJsonDocument(QJsonObject{{"vendor",QString::fromStdString(driver.vendor)},{"renderer",QString::fromStdString(driver.renderer)}}).toJson(QJsonDocument::Compact).constData()<<",\"success\":true,\"remaining_surfaces\":0,\"wave_pixel_hashes\":[";
  for(unsigned i=0;i<hashes.size();++i){if(i)std::cout<<',';std::cout<<hashes[i];}std::cout<<"]}\n";
  return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
