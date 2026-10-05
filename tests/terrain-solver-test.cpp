#include "terrain_solver.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool b){if(!b)throw std::runtime_error("Region solver contract");}
template<class F> static void rejects(F f){try{f();}catch(const std::exception&){return;}throw std::runtime_error("Expected refusal");}
int main()try{
    RegionDescriptorBank bank;bank.blocks.resize(1);bank.blocks[0].selection.maximum=4;
    auto empty=makeRegionPlacementState(2,2,bank);require(empty.storedSourceRows[0]==0);
    auto complete=solveRegionPlacement(empty,123);require(complete.complete && complete.state.bank.blocks[0].selection.placed==4);
    require(empty.bank.blocks[0].selection.placed==0 && !empty.assignments[0]);
    require(complete.state.assignments[0] && complete.state.candidates[0].size()==1);
    bank.blocks[0].selection.maximum=3;auto failed=solveRegionPlacement(makeRegionPlacementState(2,2,bank),123);
    require(!failed.complete && failed.backtracks>0);
    auto state=empty;placeRegionSection(state,0,3,1,1);require(state.assignments[3]->rotation==3 && state.bank.blocks[0].selection.placed==1);
    removeRegionBlock(state,1,1);require(!state.assignments[3] && state.grid.cells[3].edges==RegionEdges{{-1,-1,-1,-1}} && state.storedSourceRows[3]==0xffffffffU);
    rejects([&]{removeRegionBlock(state,1,1);});rejects([&]{placeRegionSection(state,0,4,1,1);});
    mnm::assets::MapAsset map;map.width=map.height=40;map.layers=1;map.metadata[0]=map.metadata[1]=2;
    RegionConnectorState next;bank.blocks=expandRegionDescriptors(map,7,false,2,next);state=makeRegionPlacementState(3,3,bank);
    placeRegionSection(state,0,1,0,0);require(state.assignments[0] && state.assignments[6] && state.assignments[1] && state.assignments[7]);
    auto saved=state;rejects([&]{placeRegionSection(state,0,0,0,0);});require(state.bank.blocks[0].selection.placed==saved.bank.blocks[0].selection.placed);
    auto broken=makeRegionPlacementState(3,3,bank);broken.bank.blocks.erase(broken.bank.blocks.begin()+1);
    rejects([&]{placeRegionSection(broken,0,0,0,0);});require(!broken.assignments[0] && broken.bank.blocks[0].selection.placed==0);
    state=makeRegionPlacementState(2,2,{});require(!solveRegionPlacement(state,1).complete);
    state.assignments.pop_back();rejects([&]{solveRegionPlacement(state,1);});
    rejects([&]{makeRegionPlacementState(6,1,bank);});
    std::cout<<"Owned placement, count mutation, partial failure, rollback and refusal pass\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
