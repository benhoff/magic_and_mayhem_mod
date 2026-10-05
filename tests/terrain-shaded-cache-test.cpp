#include "scene.hpp"
#include <QApplication>
#include <stdexcept>
int main(int argc,char** argv){
 QApplication app(argc,argv);
 mnm::render::GlBlitter renderer;
 mnm::assets::TerrainCatalog catalog;catalog.records.resize(1);catalog.records[0][0x84]=1;
 mnm::assets::Sprite sprite;sprite.storage=mnm::assets::SpriteStorage::indexed8;sprite.palettes.resize(1);sprite.palettes[0][1]={80,120,160};
 mnm::assets::SpriteFrame frame;frame.width=1;frame.height=1;frame.paletteIndex=0;frame.pixels=std::vector<std::uint8_t>{1};frame.opaqueMask={1};sprite.frames={frame,frame};
 std::vector<mnm::preview::TerrainPreviewTile> tiles(3);
 for(unsigned i=0;i<3;++i){auto& tile=tiles[i];tile.state.column=i;tile.state.anchorX=100+i;tile.state.anchorY=100;tile.state.flags8=4;tile.state.light=std::int8_t(int(i)*127-127);}
 const auto result=mnm::preview::renderTerrain(renderer,catalog,sprite,tiles,{},false,false,mnm::reconstruction::PaletteShadingConfig{});
 if(result.pixels.pixels[100*512+100]!=0 || result.pixels.pixels[100*512+101]!=0x53d4 || result.pixels.pixels[100*512+102]!=65535 || renderer.stats().surfaces)
  throw std::runtime_error("Repeated terrain frame reused another shade or leaked surfaces");
}
