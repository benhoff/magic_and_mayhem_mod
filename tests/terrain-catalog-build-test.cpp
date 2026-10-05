#include "terrain_catalog.hpp"
#include <cassert>
#include <stdexcept>
using namespace mnm::reconstruction;
template<class F> void refused(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}assert(caught);}
int main(){
    mnm::assets::MapAsset map;map.width=map.height=40;map.layers=2;map.metadata[0]=map.metadata[1]=2;
    map.metadata[2]=3;map.metadata[3]=7;map.metadata[6]=90;map.metadata[7]=80;
    mnm::assets::RegionRecipe recipe;recipe.columns=recipe.rows=4;recipe.specific.push_back({4,2,1,-1});recipe.random.push_back({4,3});
    auto result=buildTerrainRegionCatalog(recipe,{&map,&map});
    assert(result.bank.internalThreshold==8 && result.side==20 && result.layers==2);
    assert(result.bank.blocks.size()==8 && result.firstDescriptors[1]==4 && result.sourceForDescriptor[7]==1);
    assert(result.requests[0].column==1 && result.requests[0].row==-1 && result.requests[0].rotation==2);
    assert(result.requests[4].column==-1 && result.bank.blocks[4].selection.maximum==3 && result.bank.blocks[0].selection.maximum==1);
    assert(result.connectors.next[0]==16 && map.metadata[6]==90);
    map.metadata[2]=50;assert(result.bank.blocks[0].selection.edges[0]==3); // Owned header projection.
    refused([&]{buildTerrainRegionCatalog(recipe,{&map});});
    refused([&]{buildTerrainRegionCatalog(recipe,{nullptr,&map});});
    auto other=map;other.width=other.height=20;refused([&]{buildTerrainRegionCatalog(recipe,{&map,&other});});
    auto oversized=recipe;oversized.random.clear();for(unsigned i=0;i<13;++i)oversized.random.push_back({4,1});refused([&]{buildTerrainRegionCatalog(oversized,std::vector<const mnm::assets::MapAsset*>(14,&map));});
    other=map;other.metadata[8]=0x7fffffffU;refused([&]{buildTerrainRegionCatalog(recipe,{&other,&map});});
}
