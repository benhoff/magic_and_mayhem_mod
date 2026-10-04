#include "scene.hpp"
#include <stdexcept>
namespace mnm::preview {
std::vector<TerrainPreviewTile> mapRegionTiles(const assets::MapAsset& map,TerrainMapRegion region,bool overlap){
    if(!region.width || !region.height || region.width>3 || region.height>3 ||
       region.layer>=map.layers || region.x>=map.width || region.y>=map.height ||
       region.width>map.width-region.x || region.height>map.height-region.y)
        throw std::invalid_argument("MAP preview region requires an in-bounds 1..3 by 1..3 slice");
    std::vector<TerrainPreviewTile> result;
    for(unsigned row=0;row<region.height;++row)for(unsigned col=0;col<region.width;++col){
        const auto& cell=map.cell(region.x+col,region.y+row,region.layer);
        result.push_back({cell.definition,{std::int32_t(region.y+row),std::int32_t(region.x+col),std::int32_t(region.layer),
            overlap?256:256+32*(int(col)-int(row)),overlap?160:96+16*int(col+row),0,cell.flags8,cell.flags10,0}});
    }
    return result;
}
std::vector<TerrainPreviewTile> worldTerrainTiles(const assets::MapAsset& map,const reconstruction::TerrainCamera& camera){
    const auto visits=reconstruction::traverseTerrain(map.width,map.height,map.layers,camera,{},[&](unsigned x,unsigned y,unsigned z){const auto& c=map.cell(x,y,z);return std::array<std::uint16_t,2>{c.flags8,c.flags10};});
    std::vector<TerrainPreviewTile> tiles;tiles.reserve(visits.size());
    for(const auto& v:visits){const auto& c=map.cells.at(v.cell);tiles.push_back({c.definition,{int(v.row),int(v.column),int(v.layer),v.anchorX,v.anchorY,v.priority,c.flags8,c.flags10,0},v.cell});}
    return tiles;
}
}
