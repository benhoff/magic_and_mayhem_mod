#include "terrain_constraints.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool value){if(!value)throw std::runtime_error("Region constraints contract");}
template<class F> static void rejects(F f){try{f();}catch(const std::exception&){return;}throw std::runtime_error("Expected refusal");}
int main()try{
    mnm::assets::MapAsset map;map.width=map.height=40;map.layers=2;map.metadata[0]=map.metadata[1]=2;
    RegionConnectorState next;RegionDescriptorBank bank;bank.blocks=expandRegionDescriptors(map,7,false,2,next);
    require(bank.blocks.size()==4 && next.next==std::array<std::int32_t,4>{{1004,1005,1006,1007}});
    require(bank.blocks[0].selection.edges[1]==bank.blocks[1].selection.edges[3]);
    require(bank.blocks[0].selection.edges[2]==bank.blocks[2].selection.edges[0]);
    require(bank.blocks[1].selection.edges[2]==bank.blocks[3].selection.edges[0]);
    RegionSelectionGrid grid;grid.columns=grid.rows=3;grid.cells.resize(9);
    for(unsigned r=0;r<4;++r)require(admitRegionSection(grid,bank,0,0,0,r).admitted);
    grid.cells[3].occupied=true;require(!admitRegionSection(grid,bank,0,0,0,0).admitted);
    grid.cells[3].occupied=false;
    auto table=regionCandidates(grid,bank,0,0,false);require(table.size()==4);
    auto copy=table;auto pruned=pruneRegionCandidates(bank,table,0,0);
    require(table.size()==copy.size() && table[0].rotations==copy[0].rotations);
    require(pruned[0].descriptor==0 && pruned[0].rotations==14);
    bank.blocks[0].selection.placed=2;pruned=pruneRegionCandidates(bank,table,1,-1);
    for(const auto& c:pruned)require(c.descriptor!=0 && c.descriptor!=1);
    // Connector search priority is independent of section ID.
    RegionDescriptorBank duplicate;duplicate.blocks.resize(3);
    duplicate.blocks[1].selection.edges[0]=1000;duplicate.blocks[2].selection.edges[0]=1000;
    require(findRegionConnector(duplicate,1000,0,0)==2);
    duplicate.blocks[1].selection.edges[0]=0;duplicate.blocks[2].selection.edges[0]=0;
    duplicate.blocks[1].selection.edges[1]=1000;duplicate.blocks[2].selection.edges[1]=1000;
    require(findRegionConnector(duplicate,1000,0,0)==1);
    // Equal edge signatures in another descriptor are also pruned.
    duplicate.blocks[1].selection.edges=duplicate.blocks[0].selection.edges;
    pruned=pruneRegionCandidates(duplicate,{{0,1},{1,1}},0,0);require(pruned.empty());
    rejects([&]{pruneRegionCandidates(duplicate,std::vector<RegionCandidate>(50,{0,1}),0,0);});
    rejects([&]{pruneRegionCandidates(duplicate,{{0,0}},0,0);});
    rejects([&]{pruneRegionCandidates(duplicate,{{0,1}},0,4);});
    bank.blocks[0].selection.placed=0;bank.blocks.erase(bank.blocks.begin()+1);
    rejects([&]{admitRegionSection(grid,bank,0,0,0,0);});
    const auto saved=next.next;map.metadata[0]=3;rejects([&]{expandRegionDescriptors(map,7,false,2,next);});require(next.next==saved);
    map.metadata[0]=2;map.width=39;rejects([&]{expandRegionDescriptors(map,7,false,2,next);});
    std::cout<<"Owned connectors, admission, pruning, sentinel capacity and refusal pass\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
