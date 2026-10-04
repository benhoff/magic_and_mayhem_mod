#include "terrain_catalog.hpp"
#include <algorithm>
#include <new>
namespace mnm::assets {
TerrainCatalogResult decodeTerrainCatalog(const std::vector<std::uint8_t>& bytes)try{
    const auto word=[&](std::size_t at){return std::uint32_t(bytes[at])|(std::uint32_t(bytes[at+1])<<8)|(std::uint32_t(bytes[at+2])<<16)|(std::uint32_t(bytes[at+3])<<24);};
    if(bytes.size()<16 || bytes.size()>16+65536ULL*356 || bytes[0]!='T' || bytes[1]!='T' || bytes[2]!='D' || bytes[3]!=0)
        return TerrainCatalogError{"Invalid or oversized TTD header",{}};
    if(word(8)!=4 || word(4)!=bytes.size() || word(12)>65536 || 16+std::uint64_t(word(12))*356!=bytes.size())
        return TerrainCatalogError{"Unsupported TTD version or inconsistent record extent",{}};
    TerrainCatalog catalog;catalog.records.resize(word(12));
    for(std::size_t i=0;i<catalog.records.size();++i)std::copy_n(bytes.begin()+16+i*356,356,catalog.records[i].begin());
    return catalog;
}catch(const std::bad_alloc&){return TerrainCatalogError{"TTD allocation failed",{}};}
TerrainCatalogResult loadTerrainCatalog(AssetFile& file){
    auto bytes=readWhole(file,16+65536LL*356);
    if(const auto* error=std::get_if<Error>(&bytes))return TerrainCatalogError{error->detail,*error};
    return decodeTerrainCatalog(std::get<std::vector<std::uint8_t>>(bytes));
}
}
