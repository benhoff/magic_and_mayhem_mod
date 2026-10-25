#include "terrain_generation.hpp"
namespace mnm::reconstruction {
RegionPlacementState resetRegionGenerationAttempt(RegionPlacementState state) {
    validateRegionPlacementState(state);
    for(auto& block:state.bank.blocks) block.selection.placed=0;
    for(unsigned i=0;i<state.grid.cells.size();++i) {
        state.grid.cells[i]={};state.assignments[i].reset();state.candidates[i].clear();
    }
    state.carry.reset();return state;
}
RegionGenerationResult stepRegionGeneration(RegionPlacementState state,
    std::vector<RegionSpecificRequest> requests,std::uint32_t seed) {
    validateRegionSpecificRequests(state,requests);
    auto specifics=placeRegionSpecifics(resetRegionGenerationAttempt(std::move(state)),std::move(requests),seed);
    auto solved=solveRegionPlacement(std::move(specifics.state),seed);
    RegionGenerationResult result;
    result.attempts.push_back({seed,solved.complete,solved.backtracks,solved.multiBlockBacktracks,std::move(specifics.diagnostics)});
    result.state=std::move(solved.state);result.requests=std::move(specifics.requests);result.locations=std::move(specifics.locations);
    result.complete=solved.complete;result.nextSeed=seed+std::uint32_t(500);
    return result;
}
RegionGenerationResult generateRegionPlacement(RegionPlacementState state,
    std::vector<RegionSpecificRequest> requests,std::uint32_t seed) {
    RegionGenerationResult result;result.state=std::move(state);result.requests=std::move(requests);result.nextSeed=seed;
    for(unsigned attempt=0;attempt<10;++attempt) {
        auto next=stepRegionGeneration(std::move(result.state),std::move(result.requests),result.nextSeed);
        result.attempts.push_back(std::move(next.attempts.front()));
        result.state=std::move(next.state);result.requests=std::move(next.requests);result.locations=std::move(next.locations);
        result.complete=next.complete;result.nextSeed=next.nextSeed;
        if(result.complete) break;
    }
    return result;
}
}
