#include "terrain_selection.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool value){if(!value)throw std::runtime_error("Region selection contract");}
template<class F> static void rejects(F f){try{f();}catch(const std::exception&){return;}throw std::runtime_error("Expected refusal");}
int main()try{
    RegionSelectionGrid g;g.columns=g.rows=3;g.cells.resize(9);
    SingleRegionDescriptor d;d.edges={{1,2,3,4}};
    require(admitSingleRegionSection(g,d,1,1,0));
    g.cells[1*3+0].edges[2]=4;require(!admitSingleRegionSection(g,d,1,1,0));require(admitSingleRegionSection(g,d,1,1,1));
    g.cells[1*3+0].edges[2]=-1;
    // Confirm the recovered west sentinel asymmetry explicitly.
    g.cells[0*3+1].edges[1]=99;require(admitSingleRegionSection(g,d,1,1,0));
    g.cells[0*3+1].edges[3]=0;require(!admitSingleRegionSection(g,d,1,1,0));
    g.cells[0*3+1].edges={{-1,-1,-1,-1}};
    d.placed=d.maximum;require(!admitSingleRegionSection(g,d,1,1,0));
    ++d.placed;require(admitSingleRegionSection(g,d,1,1,0));d.placed=0;
    auto specific=d;specific.specific=true;auto special=d;special.special=true;
    auto table=singleRegionCandidates(g,{specific,d,special},1,1,false);
    require(table.size()==1 && table[0].descriptor==1 && table[0].rotations==15);
    require(singleRegionCandidates(g,{specific,d,special},1,1,true)[0].descriptor==2);
    g.cells[0].occupied=true;auto locations=singleRegionLocations(g,d,-1);
    require(locations.size()==8 && locations[0].column==1 && locations[0].row==0);
    require(singleRegionLocations(g,d,2)[0].rotations==4);
    for(unsigned seed=0;seed<64;++seed){
        auto a=chooseRegionCandidate(seed,table),b=chooseRegionCandidate(seed,table);
        require(a && b && a->index==b->index && a->rotation==b->rotation);
        require(chooseRegionRotation(seed,4)==2);
    }
    require(!chooseRegionCandidate(0,{}));require(!chooseRegionLocation(0,{}));require(!chooseRegionRotation(0,0));
    mnm::assets::MapAsset map;map.width=map.height=20;map.layers=1;map.metadata[0]=map.metadata[1]=1;
    map.metadata[2]=7;auto descriptor=describeSingleRegionSection(map,99,false,8);
    map.metadata[2]=0;require(descriptor.special && descriptor.edges[0]==7 && descriptor.maximum==8);
    require(!describeSingleRegionSection(map,99,true,8).special);
    map.metadata[0]=2;rejects([&]{describeSingleRegionSection(map,1,false,1);});
    rejects([&]{singleRegionLocations(g,d,4);});rejects([&]{admitSingleRegionSection(g,d,3,0,0);});
    rejects([&]{chooseRegionCandidate(0,{{50,1}});});rejects([&]{chooseRegionCandidate(0,{{0,0}});});
    rejects([&]{chooseRegionLocation(0,{{5,0,1}});});rejects([&]{chooseRegionRotation(0,16);});
    g.cells.pop_back();rejects([&]{singleRegionLocations(g,d,-1);});
    std::cout<<"Owned region selection, sentinels, ordering, reseeding and refusal pass\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
