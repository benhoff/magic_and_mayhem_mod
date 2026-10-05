#include "terrain_generated.hpp"
#include <stdexcept>
namespace mnm::reconstruction {
FixedRegionPlan planGeneratedTerrainRegion(const TerrainRegionCatalog& catalog,const RegionGenerationResult& generation) {
    if(!generation.complete)throw std::invalid_argument("Cannot assemble incomplete generated region");
    const auto& state=generation.state;validateRegionPlacementState(state);
    if(catalog.columns!=state.grid.columns || catalog.rows!=state.grid.rows || !catalog.side || catalog.side>128 ||
       catalog.columns>128/catalog.side || catalog.rows>128/catalog.side || !catalog.layers || catalog.layers>32 ||
       catalog.bank.internalThreshold!=state.bank.internalThreshold || catalog.bank.blocks.size()!=state.bank.blocks.size() ||
       catalog.bank.blocks.empty() || catalog.sourceForDescriptor.size()!=catalog.bank.blocks.size() ||
       catalog.firstDescriptors.empty() || catalog.firstDescriptors.front()!=0 || catalog.requests.size()!=catalog.bank.blocks.size())
        throw std::invalid_argument("Generated region catalog/state mismatch");
    for(unsigned source=0;source<catalog.firstDescriptors.size();++source) {
        const auto begin=catalog.firstDescriptors[source];const auto end=source+1<catalog.firstDescriptors.size()?catalog.firstDescriptors[source+1]:catalog.bank.blocks.size();
        if(begin>=end || end>catalog.bank.blocks.size())throw std::invalid_argument("Generated region source spans");
        for(unsigned i=begin;i<end;++i)if(catalog.sourceForDescriptor[i]!=source)throw std::invalid_argument("Generated region source identity");
    }
    for(unsigned i=0;i<catalog.bank.blocks.size();++i) {
        const auto& a=catalog.bank.blocks[i];const auto& b=state.bank.blocks[i];
        if(a.selection.section!=b.selection.section || a.selection.specific!=b.selection.specific || a.selection.special!=b.selection.special ||
           a.selection.maximum!=b.selection.maximum || a.selection.edges!=b.selection.edges || a.sourceX!=b.sourceX || a.sourceY!=b.sourceY ||
           a.columns!=b.columns || a.rows!=b.rows || a.layers!=b.layers || a.layers>catalog.layers)
            throw std::invalid_argument("Generated region descriptor identity");
    }
    FixedRegionPlan plan;plan.columns=catalog.columns;plan.rows=catalog.rows;plan.side=catalog.side;plan.layers=catalog.layers;
    // Canonical row-first destination order, independent of placement walk order.
    for(unsigned y=0;y<plan.rows;++y)for(unsigned x=0;x<plan.columns;++x) {
        const auto& assignment=state.assignments[x*plan.rows+y];
        if(!assignment)throw std::invalid_argument("Generated region has unfilled slots");
        const auto& descriptor=state.bank.blocks[assignment->descriptor];
        plan.blocks.push_back({catalog.sourceForDescriptor[assignment->descriptor],descriptor.sourceX*plan.side,descriptor.sourceY*plan.side,x,y,assignment->rotation});
    }
    return plan;
}
GeneratedTerrainRegionPlan generateTerrainRegionPlan(const assets::RegionRecipe& recipe,
    const std::vector<const assets::MapAsset*>& sources,std::uint32_t seed) {
    GeneratedTerrainRegionPlan result;result.catalog=buildTerrainRegionCatalog(recipe,sources);
    result.generation=generateRegionPlacement(makeRegionPlacementState(result.catalog.columns,result.catalog.rows,result.catalog.bank),result.catalog.requests,seed);
    if(result.generation.complete)result.plan=planGeneratedTerrainRegion(result.catalog,result.generation);
    return result;
}
}
