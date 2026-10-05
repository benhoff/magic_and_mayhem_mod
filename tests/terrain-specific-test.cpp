#include "terrain_specific.hpp"
#include <cassert>
#include <stdexcept>
using namespace mnm::reconstruction;
template<class F> void refused(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}assert(caught);}
int main(){
    RegionDescriptorBank bank;RegionBlockDescriptor d;d.selection.specific=true;d.selection.edges={{0,0,0,0}};bank.blocks.push_back(d);
    const auto initial=makeRegionPlacementState(2,2,bank);
    auto fixed=placeRegionSpecifics(initial,{{1,1,3}},123);
    assert(fixed.state.assignments[3] && fixed.state.assignments[3]->rotation==3);
    assert(!initial.assignments[3] && initial.bank.blocks[0].selection.placed==0);
    assert(fixed.diagnostics.empty() && fixed.locations.empty());
    auto wildcard=placeRegionSpecifics(initial,{{-1,1,-1}},123);
    assert(wildcard.requests[0].column==-1 && wildcard.requests[0].row==-1);
    assert(wildcard.locations.size()==4 && wildcard.state.bank.blocks[0].selection.placed==1);
    // Ordinary descriptors are skipped and their request fields retained.
    auto ordinary=initial;ordinary.bank.blocks[0].selection.specific=false;
    auto ignored=placeRegionSpecifics(ordinary,{{1,-1,2}},123);
    assert(ignored.requests[0].column==1 && ignored.state.bank.blocks[0].selection.placed==0 && !ignored.state.carry);
    // Exhaustion suppresses the descriptor without inventing an assignment.
    auto blocked=initial;blocked.bank.blocks[0].selection.specific=false;
    placeRegionSection(blocked,0,0,0,0);placeRegionSection(blocked,0,0,1,0);placeRegionSection(blocked,0,0,0,1);placeRegionSection(blocked,0,0,1,1);
    auto extra=d;extra.selection.section=1;blocked.bank.blocks.push_back(extra);
    auto exhausted=placeRegionSpecifics(blocked,{{},{-1,-1,2}},0);
    assert(exhausted.state.bank.blocks[1].selection.placed==1 && exhausted.locations.empty());
    assert(exhausted.diagnostics.size()==2 && exhausted.diagnostics[0].notice==RegionSpecificNotice::RequestedLocationsEmpty && exhausted.diagnostics[1].notice==RegionSpecificNotice::AllLocationsEmpty);
    // Requested orientation fails beside a single asymmetric fixed neighbor;
    // offset correction succeeds and preserves the request's authored rotation.
    auto correction=initial;correction.bank.blocks[0].selection.specific=false;correction.bank.blocks[0].selection.edges={{-1,2,-1,3}};
    correction.bank.blocks.push_back(d);correction.bank.blocks[1].selection.edges={{2,0,3,0}};
    placeRegionSection(correction,0,0,0,0);
    auto corrected=placeRegionSpecifics(correction,{{},{1,0,0}},123);
    assert(corrected.diagnostics.size()==1 && corrected.diagnostics[0].notice==RegionSpecificNotice::RequestedRotationRejected);
    assert(corrected.state.assignments[2] && corrected.state.assignments[2]->rotation==3);
    refused([&]{placeRegionSpecifics(initial,{},0);});
    refused([&]{placeRegionSpecifics(initial,{{2,0,0}},0);});
    refused([&]{placeRegionSpecifics(initial,{{0,0,4}},0);});
    refused([&]{auto malformed=initial;malformed.assignments.clear();placeRegionSpecifics(malformed,{{}},0);});
    refused([&]{auto malformed=initial;malformed.grid.cells[0].occupied=true;placeRegionSpecifics(malformed,{{}},0);});
    // A suppressed or malformed connector partner must fail within owned state.
    auto missing=initial;missing.bank.blocks[0].columns=2;missing.bank.blocks[0].selection.edges[1]=1000;
    refused([&]{placeRegionSpecifics(missing,{{-1,-1,-1}},0);});
    assert(!missing.assignments[0] && missing.bank.blocks[0].selection.placed==0);
    assert(initial.bank.blocks[0].selection.placed==0);
}
