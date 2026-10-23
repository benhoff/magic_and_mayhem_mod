#include "scene.hpp"
#include "resource-fixtures.hpp"
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <stdexcept>
int main(int argc,char** argv)try{
 QApplication app(argc,argv);
 mnm::render::GlBlitter renderer;
 mnm::assets::TerrainCatalog catalog;catalog.records.resize(1);catalog.records[0][0x84]=1;
 mnm::assets::Sprite sprite;sprite.storage=mnm::assets::SpriteStorage::indexed8;sprite.palettes.resize(1);sprite.palettes[0][1]={80,120,160};
 mnm::assets::SpriteFrame frame;frame.width=1;frame.height=1;frame.paletteIndex=0;frame.pixels=std::vector<std::uint8_t>{1};frame.opaqueMask={1};sprite.frames={frame,frame};
 std::vector<mnm::preview::TerrainPreviewTile> tiles(3);
 for(unsigned i=0;i<3;++i){auto& tile=tiles[i];tile.state.column=i;tile.state.anchorX=100+i;tile.state.anchorY=100;tile.state.flags8=4;tile.state.light=std::int8_t(int(i)*127-127);}
 const auto original=mnm::preview::renderTerrain(renderer,catalog,sprite,tiles,{},false,false,mnm::reconstruction::PaletteShadingConfig{});
 if(original.pixels.pixels[100*512+100]!=0||original.pixels.pixels[100*512+101]!=0x53d4||original.pixels.pixels[100*512+102]!=65535||renderer.stats().surfaces)throw std::runtime_error("Repeated terrain frame reused another shade or leaked surfaces");
 unsigned frames=1;
 for(unsigned fault=0;fault<4;++fault) {
  auto selected=tiles;auto admission=mnm::reconstruction::TerrainAdmission{};
  mnm::preview::TerrainPaletteEffects effects;
  if(fault==0)admission.view=4;
  if(fault==1)selected.back().definition=1;
  if(fault==2)selected.back().state.anchorX=512;
  if(fault==3)effects.ordinal=99;
  bool refused=false;
  try {mnm::preview::renderTerrain(renderer,catalog,sprite,selected,admission,false,false,mnm::reconstruction::PaletteShadingConfig{},effects);}
  catch(const std::exception&){refused=true;}
  if(!refused||renderer.stats().surfaces)throw std::runtime_error("Terrain admission leaked or partially accepted an invalid late frame");
 }
 const auto recovered=mnm::preview::renderTerrain(renderer,catalog,sprite,tiles,{},false,false,mnm::reconstruction::PaletteShadingConfig{});
 if(recovered.pixels.pixels!=original.pixels.pixels)throw std::runtime_error("Terrain refusal poisoned a later frame");
 mnm::reconstruction::PaletteRgb rgb{};rgb[1]={80,120,160};
 for(unsigned mode=0;mode<10;++mode){
  auto selected=tiles;if(mode>=8)for(unsigned i=0;i<3;++i)selected[i].state.light=std::int8_t(int(i)*63-127);
  mnm::preview::TerrainPaletteEffects effects;effects.append={mode};effects.profile.metalContrast=200;
  const auto result=mnm::preview::renderTerrain(renderer,catalog,sprite,selected,{},false,false,mnm::reconstruction::PaletteShadingConfig{},effects);
  mnm::reconstruction::PaletteEffectChain expected({rgb});expected.append(0);expected.append(mode,{},effects.profile);
  for(const auto& item:result.queue){const auto& draw=item.draw;const auto colour=expected.colours(1,0,draw.shade)[1];if(result.pixels.pixels.at(std::size_t(draw.anchorY)*512+draw.anchorX)!=colour)throw std::runtime_error("Terrain effect chain/ordinal/shade binding differs");}
  if(renderer.stats().surfaces||sprite.palettes[0][1].red!=80)throw std::runtime_error("Terrain effect changed shared RGB or leaked surfaces");
  ++frames;
 }
 mnm::preview::TerrainPaletteEffects base;base.append={1};base.ordinal=0;
 auto baseline=mnm::preview::renderTerrain(renderer,catalog,sprite,tiles,{},false,false,mnm::reconstruction::PaletteShadingConfig{},base);if(baseline.pixels.pixels!=original.pixels.pixels)throw std::runtime_error("Explicit base effect ordinal differs");++frames;
 for(auto& tile:tiles)tile.state.light=0;
 for(unsigned mode:{0u,8u}){mnm::preview::TerrainPaletteEffects effects;effects.append={mode};auto result=mnm::preview::renderTerrain(renderer,catalog,sprite,tiles,{},false,false,mnm::reconstruction::PaletteShadingConfig{17,1,1,2,2},effects);for(unsigned i=0;i<3;++i)if(result.pixels.pixels[100*512+100+i]!=0x53d4)throw std::runtime_error("Non-power terrain palette admission differs");++frames;}
 if(argc==2){
  const auto root=std::filesystem::path(argv[1]);std::filesystem::create_directories(root/"realm");auto ttd=resource_test::terrain();resource_test::word(ttd,20,0);ttd[16+0x84]=1;resource_test::write(root/"realm","Terrain.ttd",ttd);resource_test::write(root/"realm","Terrain.spr",resource_test::sprite(true));
  const std::string prefs="[VIDEO]\nTerrainLightLevels=17\n",effects="[STONE_EFFECT]\nSpecalValue=1\nDarkest=9\n[METAL_EFFECT]\nContrast=200\n";resource_test::write(root,"prefs.cfg",resource_test::Bytes(prefs.begin(),prefs.end()));resource_test::write(root,"effects.ini",resource_test::Bytes(effects.begin(),effects.end()));
 }
 const auto driver=renderer.driver();std::cout<<QJsonDocument(QJsonObject{{"success",true},{"gpu_frames",int(frames)},{"modes",10},{"nonpower_counts_passed",true},{"driver",QJsonObject{{"vendor",QString::fromStdString(driver.vendor)},{"renderer",QString::fromStdString(driver.renderer)}}}}).toJson(QJsonDocument::Compact).constData()<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
