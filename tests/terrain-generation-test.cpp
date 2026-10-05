#include "terrain_generation.hpp"
#include <cassert>
#include <stdexcept>
using namespace mnm::reconstruction;
int main(){
    RegionDescriptorBank bank;RegionBlockDescriptor block;block.selection.maximum=20;block.selection.edges={{0,0,0,0}};bank.blocks.push_back(block);
    auto initial=makeRegionPlacementState(2,2,bank);placeRegionSection(initial,0,0,0,0);initial.candidates[1]={{0,15}};
    initial.storedSourceRows[1]=71;
    auto reset=resetRegionGenerationAttempt(initial);
    assert(reset.bank.blocks[0].selection.placed==0 && !reset.assignments[0] && !reset.grid.cells[0].occupied && !reset.carry && reset.candidates[1].empty());
    assert(reset.storedSourceRows[1]==71 && initial.assignments[0] && initial.candidates[1].size()==1);
    auto complete=generateRegionPlacement(initial,{{}},0xffffffffU);
    assert(complete.complete && complete.attempts.size()==1 && complete.nextSeed==499 && complete.attempts[0].seed==0xffffffffU);
    assert(complete.state.bank.blocks[0].selection.placed==4 && initial.bank.blocks[0].selection.placed==1);
    // Counts reset before every failed attempt; fixed requests are retained.
    auto limited=makeRegionPlacementState(2,2,bank);limited.bank.blocks[0].selection.specific=true;limited.bank.blocks[0].selection.maximum=1;
    auto failed=generateRegionPlacement(limited,{{1,0,0}},0xfffffff0U);
    assert(!failed.complete && failed.attempts.size()==10 && failed.nextSeed==0xfffffff0U+std::uint32_t(5000));
    assert(failed.requests[0].column==1 && failed.requests[0].row==0 && failed.state.bank.blocks[0].selection.placed==1);
    for(unsigned i=0;i<10;++i) assert(failed.attempts[i].seed==0xfffffff0U+std::uint32_t(500*i) && !failed.attempts[i].complete);
    unsigned occupied=0;for(const auto& a:failed.state.assignments)occupied+=bool(a);assert(occupied==1);
    // One-coordinate wildcard normalization survives the successful attempt.
    auto mixed=initial;auto specific=block;specific.selection.specific=true;specific.selection.maximum=1;specific.selection.section=1;mixed.bank.blocks.push_back(specific);
    auto normalized=generateRegionPlacement(mixed,{{},{-1,1,0}},123);
    assert(normalized.complete && normalized.requests[1].column==-1 && normalized.requests[1].row==-1);
    bool caught=false;try{generateRegionPlacement(initial,{},0);}catch(const std::invalid_argument&){caught=true;}assert(caught && initial.assignments[0]);
}
