#include "scene.hpp"
#include <map>
#include <stdexcept>
namespace mnm::preview {
TerrainPreviewResult renderTerrain(render::GlBlitter& renderer,const assets::TerrainCatalog& catalog,
 const assets::Sprite& sprite,std::vector<TerrainPreviewTile> tiles,reconstruction::TerrainAdmission admission,bool visibility,bool world,std::optional<reconstruction::PaletteShadingConfig> shading){
 if((!world && (tiles.empty() || tiles.size()>9)) || tiles.size()>16384 || sprite.frames.empty())throw std::invalid_argument("Terrain preview tile/SPR bounds exceeded");
 std::vector<reconstruction::ShadedPalette> palettes;
 if(shading){if(sprite.storage!=assets::SpriteStorage::indexed8)throw std::invalid_argument("Terrain shading requires indexed palettes");
  for(const auto& rgb:sprite.palettes){reconstruction::PaletteRgb copied{};for(unsigned i=0;i<256;++i)copied[i]={rgb[i].red,rgb[i].green,rgb[i].blue};palettes.push_back(reconstruction::buildShadedPalette(copied,*shading));}}
 TerrainPreviewResult result;std::map<std::size_t,std::size_t> cells;std::vector<std::size_t> ownerForTile;
 std::vector<TerrainPreviewDraw> draws;std::vector<reconstruction::SpriteQueueEntry> sorted;
 for(std::size_t i=0;i<tiles.size();++i){auto& tile=tiles[i];
  if(tile.definition>=catalog.records.size())throw std::invalid_argument("Terrain definition outside TTD");
  const auto [ownerIt,fresh]=cells.emplace(world?tile.cell:i,i);const auto ownerIndex=ownerIt->second;ownerForTile.push_back(ownerIndex);
  if(!fresh){tile.state.flags8=result.owners[ownerIndex].flags8;tile.state.flags10=result.owners[ownerIndex].flags10;}
  const auto d=reconstruction::decodeTerrainDefinition(catalog.records[tile.definition]);
  const auto queue=reconstruction::submitTerrain(d,tile.state,admission,sprite.frames.size()-1);
  const auto resolve=[&](unsigned frame){return sprite.frames[frame<sprite.frames.size()?frame:0].sourceOffset;};
  result.owners.push_back({resolve(d.body[admission.view]),resolve(d.first[admission.view]),resolve(d.second[admission.view]),tile.state.flags8,tile.state.flags10});
  if(!fresh)result.owners[ownerIndex]=result.owners.back();
  for(const auto& draw:queue){sorted.push_back({draw.key,std::uint32_t(draws.size())});draws.push_back({i,draw});}
 }
 reconstruction::sortSpriteQueue(sorted);
 std::vector<reconstruction::SpriteVisibilityEntry> entries;
 for(const auto& s:sorted){const auto& item=draws[s.payload];const auto& d=item.draw;const auto& frame=sprite.frames[d.frame];result.queue.push_back(item);
  entries.push_back({reconstruction::decodeSpriteVisibility(frame.auxiliaryData[0],frame.auxiliaryData[1],frame.originX,frame.originY),d.anchorX,d.anchorY,d.kind,frame.sourceOffset,ownerForTile[item.tile]});}
 if(visibility){reconstruction::SpriteVisibilityGrid grid;reconstruction::applySpriteVisibility(entries,grid,result.owners);}
 for(std::size_t i=0;i<result.owners.size();++i)result.owners[i]=result.owners[ownerForTile[i]];
 render::Image background{512,256,std::vector<std::uint32_t>(512*256,0x2124)};
 const auto canvas=renderer.create(background,render::spriteFormat);
 try{
  std::map<std::pair<std::uint32_t,std::size_t>,std::unique_ptr<render::UploadedSpriteFrame>> uploads;
  for(std::size_t i=0;i<result.queue.size();++i){auto& d=result.queue[i].draw;d.kind=entries[i].kind;if(d.kind==-2)continue;
   const auto& frame=sprite.frames[d.frame];const auto x=std::int64_t(d.anchorX)-frame.originX,y=std::int64_t(d.anchorY)-frame.originY;
   if(!world && (x<0 || y<0 || x+frame.width>512 || y+frame.height>256))throw std::invalid_argument("Selected terrain frame exceeds bounded preview canvas");
   if(shading && (!frame.paletteIndex || *frame.paletteIndex>=palettes.size()))throw std::invalid_argument("Missing terrain shading palette");
   const auto table=shading?palettes.at(*frame.paletteIndex).tableIndex(d.shade):0;
   const auto key=std::make_pair(d.frame,table);
   // Full scenes may touch many distinct frames; retain at most 16 uploads.
   if(world && uploads.find(key)==uploads.end() && uploads.size()>=16)uploads.erase(uploads.begin());
   auto& upload=uploads[key];if(!upload)upload=std::make_unique<render::UploadedSpriteFrame>(renderer,sprite,d.frame,shading?&palettes.at(*frame.paletteIndex).tables.at(table):nullptr);
   if(world)upload->drawClipped(canvas,d.anchorX,d.anchorY,{0,0,512,256});
   else upload->draw(canvas,d.anchorX,d.anchorY);
  }
  result.pixels=renderer.read(canvas);result.image=renderer.present(canvas);
 }catch(...){renderer.destroy(canvas);throw;}
 renderer.destroy(canvas);return result;
}
}
