#include "terrain_catalog.hpp"
#include <cassert>
int main(){
    using namespace mnm::assets;
    std::vector<std::uint8_t> b(372);b[0]='T';b[1]='T';b[2]='D';b[4]=116;b[5]=1;b[8]=4;b[12]=1;b.back()=91;
    auto result=decodeTerrainCatalog(b);assert(std::holds_alternative<TerrainCatalog>(result));
    b.back()=0;assert(std::get<TerrainCatalog>(result).records[0].back()==91);
    b.pop_back();assert(std::holds_alternative<TerrainCatalogError>(decodeTerrainCatalog(b)));
    b.push_back(0);b[8]=5;assert(std::holds_alternative<TerrainCatalogError>(decodeTerrainCatalog(b)));
    b[8]=4;b[12]=255;assert(std::holds_alternative<TerrainCatalogError>(decodeTerrainCatalog(b)));
    assert(std::holds_alternative<TerrainCatalogError>(decodeTerrainCatalog({})));
}
