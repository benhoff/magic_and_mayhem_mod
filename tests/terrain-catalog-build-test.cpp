#include "terrain_catalog.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool ok){if(!ok)throw std::runtime_error("Region catalog boundary differs");}
template<class F> void refused(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}require(caught);}
int main()try{
    mnm::assets::MapAsset map;map.width=map.height=40;map.layers=2;map.metadata[0]=map.metadata[1]=2;
    map.metadata[2]=3;map.metadata[3]=7;map.metadata[6]=90;map.metadata[7]=80;
    mnm::assets::RegionRecipe recipe;recipe.columns=recipe.rows=4;recipe.specific.push_back({4,2,1,-1});recipe.random.push_back({4,3});
    auto result=buildTerrainRegionCatalog(recipe,{&map,&map});
    require(result.bank.internalThreshold==8 && result.side==20 && result.layers==2);
    require(result.bank.blocks.size()==8 && result.firstDescriptors[1]==4 && result.sourceForDescriptor[7]==1);
    require(result.requests[0].column==1 && result.requests[0].row==-1 && result.requests[0].rotation==2);
    require(result.requests[4].column==-1 && result.bank.blocks[4].selection.maximum==3 && result.bank.blocks[0].selection.maximum==1);
    require(result.connectors.next[0]==16 && map.metadata[6]==90);
    map.metadata[2]=50;require(result.bank.blocks[0].selection.edges[0]==3); // Owned header projection.
    refused([&]{buildTerrainRegionCatalog(recipe,{&map});});
    refused([&]{buildTerrainRegionCatalog(recipe,{nullptr,&map});});
    auto other=map;other.width=other.height=20;refused([&]{buildTerrainRegionCatalog(recipe,{&map,&other});});
    auto oversized=recipe;oversized.random.clear();for(unsigned i=0;i<13;++i)oversized.random.push_back({4,1});refused([&]{buildTerrainRegionCatalog(oversized,std::vector<const mnm::assets::MapAsset*>(14,&map));});
    other=map;other.metadata[8]=0x7fffffffU;refused([&]{buildTerrainRegionCatalog(recipe,{&other,&map});});
    // Capacity counts expanded descriptors, including repeated source entries.
    mnm::assets::MapAsset single;single.width=single.height=1;single.layers=32;single.metadata[0]=single.metadata[1]=1;
    mnm::assets::RegionRecipe full;full.columns=full.rows=5;
    for(unsigned i=0;i<50;++i)full.random.push_back({99,999});
    const auto boundary=buildTerrainRegionCatalog(full,std::vector<const mnm::assets::MapAsset*>(50,&single));
    require(boundary.bank.blocks.size()==50 && boundary.layers==32 && boundary.firstDescriptors.back()==49);
    for(unsigned i=0;i<50;++i)require(boundary.sourceForDescriptor[i]==i && boundary.bank.blocks[i].selection.section==99 && boundary.bank.blocks[i].selection.maximum==999);
    full.random.push_back({99,1});refused([&]{buildTerrainRegionCatalog(full,std::vector<const mnm::assets::MapAsset*>(51,&single));});
    full.random.pop_back();full.random.back().occurrences=0;
    refused([&]{buildTerrainRegionCatalog(full,std::vector<const mnm::assets::MapAsset*>(50,&single));});
    full.random.back().occurrences=1000;
    refused([&]{buildTerrainRegionCatalog(full,std::vector<const mnm::assets::MapAsset*>(50,&single));});
    require(boundary.bank.blocks.back().selection.maximum==999 && single.metadata[0]==1);
    std::cout<<"Catalog requests, owned headers, threshold quirks and exact expanded capacity pass\n";
    return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
