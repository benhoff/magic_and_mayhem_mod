#include "terrain_generated.hpp"
#include <cassert>
#include <stdexcept>
using namespace mnm::reconstruction;
template<class F>void refused(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}assert(caught);}
int main(){
    mnm::assets::MapAsset map;map.width=map.height=2;map.layers=1;map.metadata[0]=map.metadata[1]=1;map.cells.resize(4);
    for(unsigned i=0;i<4;++i){map.cells[i].definition=i;map.cells[i].references.fill(7);map.cells[i].flags10=8;}
    mnm::assets::RegionRecipe recipe;recipe.columns=recipe.rows=2;recipe.random.push_back({4,4});
    auto result=generateTerrainRegionPlan(recipe,{&map},123);assert(result.plan && result.generation.complete && result.plan->blocks.size()==4);
    mnm::assets::TerrainCatalog terrain;terrain.records.resize(4);for(unsigned i=0;i<4;++i)for(unsigned r=1;r<4;++r)terrain.records[i][0x94-4*r]=i;
    const auto assembled=assembleFixedTerrainRegion(*result.plan,{&map},terrain);
    assert(assembled.map.width==4 && assembled.map.height==4 && assembled.projectedObjects==4 && assembled.projectedReferences==12);
    for(const auto& cell:assembled.map.cells){assert(cell.references[0]==0xffff && !(cell.flags10&8));}
    assert(map.cells[0].references[0]==7 && map.cells[0].flags10==8);
    // Duplicate section IDs retain separate source-entry ownership/provenance.
    recipe.specific={{4,3,0,0}};recipe.random[0].occurrences=3;
    auto mixed=generateTerrainRegionPlan(recipe,{&map,&map},123);assert(mixed.plan);
    assert(mixed.plan->blocks[0].source==0 && mixed.plan->blocks[0].rotation==3);
    for(unsigned i=1;i<4;++i)assert(mixed.plan->blocks[i].source==1);
    auto malformed=mixed.catalog;malformed.sourceForDescriptor[1]=0;refused([&]{planGeneratedTerrainRegion(malformed,mixed.generation);});
    malformed=mixed.catalog;malformed.bank.blocks[0].selection.section=5;refused([&]{planGeneratedTerrainRegion(malformed,mixed.generation);});
    auto incomplete=mixed.generation;incomplete.complete=false;refused([&]{planGeneratedTerrainRegion(mixed.catalog,incomplete);});
    incomplete=mixed.generation;incomplete.state.grid.cells[1]={};incomplete.state.assignments[1].reset();refused([&]{planGeneratedTerrainRegion(mixed.catalog,incomplete);});
    recipe.specific.clear();recipe.random[0].occurrences=1;
    auto exhausted=generateTerrainRegionPlan(recipe,{&map},123);assert(!exhausted.plan && !exhausted.generation.complete && exhausted.generation.attempts.size()==10);
}
