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
}
