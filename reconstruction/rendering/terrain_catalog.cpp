#include "terrain_catalog.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>
namespace mnm::reconstruction {
namespace {
std::int32_t signedWord(std::uint32_t v) {
    return v<=0x7fffffffU?std::int32_t(v):-1-std::int32_t(0xffffffffU-v);
}
}
TerrainRegionCatalog buildTerrainRegionCatalog(const assets::RegionRecipe& recipe,
    const std::vector<const assets::MapAsset*>& sources) {
    if(!recipe.columns || recipe.columns>5 || !recipe.rows || recipe.rows>5 || sources.empty() ||
       sources.size()!=recipe.specific.size()+recipe.random.size() || sources.size()>50)
        throw std::invalid_argument("Region catalog recipe/source extent");
    TerrainRegionCatalog result;result.columns=recipe.columns;result.rows=recipe.rows;
    std::int32_t maximum=0;
    for(const auto* map:sources) {
        if(!map || !map->metadata[0] || map->metadata[0]>2 || !map->metadata[1] || map->metadata[1]>2 ||
           !map->width || map->width>128 || !map->height || map->height>128 || !map->layers || map->layers>32 ||
           map->width%map->metadata[0] || map->height%map->metadata[1] || map->width/map->metadata[0]!=map->height/map->metadata[1])
            throw std::invalid_argument("Region catalog MAP header");
        if(recipe.columns<std::min(map->metadata[0],map->metadata[1]) || recipe.rows<std::min(map->metadata[0],map->metadata[1]))
            throw std::invalid_argument("Region catalog section too large in every rotation");
        const unsigned side=map->width/map->metadata[0];
        if(result.side && result.side!=side) throw std::invalid_argument("Region catalog section side mismatch");
        result.side=side;result.layers=std::max(result.layers,map->layers);
        std::array<std::int32_t,8> e{};for(unsigned i=0;i<8;++i)e[i]=signedWord(map->metadata[2+i]);
        for(unsigned i=0;i<4;++i)if(maximum<e[i])maximum=e[i];
        // Preserve the selected CFG caller's two assignments to earlier edges.
        if(maximum<e[4])maximum=e[0];
        if(maximum<e[5])maximum=e[1];
        for(unsigned i=6;i<8;++i)if(maximum<e[i])maximum=e[i];
    }
    if(maximum>std::numeric_limits<std::int32_t>::max()-4)
        throw std::out_of_range("Region catalog connector threshold overflow");
    result.bank.internalThreshold=maximum+1;
    for(unsigned i=0;i<4;++i)result.connectors.next[i]=maximum+1+int(i);
    for(unsigned i=0;i<sources.size();++i) {
        const bool specific=i<recipe.specific.size();unsigned section=0;int count=1;RegionSpecificRequest request;
        if(specific) {
            const auto& q=recipe.specific[i];section=q.section;request={q.column,q.row,q.rotation};
            if(q.column< -1 || q.row< -1 || q.column>=int(recipe.columns) || q.row>=int(recipe.rows) || q.rotation< -1 || q.rotation>3)
                throw std::invalid_argument("Region catalog Specific fields");
        } else {
            const auto& q=recipe.random[i-recipe.specific.size()];section=q.section;
            if(!q.occurrences || q.occurrences>999)throw std::invalid_argument("Region catalog Random maximum");
            count=int(q.occurrences);
        }
        if(result.bank.blocks.size()+sources[i]->metadata[0]*sources[i]->metadata[1]>50)
            throw std::invalid_argument("Region catalog expanded descriptor capacity");
        auto blocks=expandRegionDescriptors(*sources[i],section,specific,count,result.connectors);
        result.firstDescriptors.push_back(result.bank.blocks.size());
        for(const auto& block:blocks){result.bank.blocks.push_back(block);result.requests.push_back(request);result.sourceForDescriptor.push_back(i);}
    }
    return result;
}
}
