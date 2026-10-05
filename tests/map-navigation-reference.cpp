// Isolated original terrain predicates on imported MAP inputs, not live search.
#include "route_world.hpp"
#include "route_cell_support.hpp"
#include "route_boundary.hpp"
#include "route_clearance.hpp"
#include "clearance-native-reference.hpp"
#include "cell-support-native-reference.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
static void check(bool ok){if(!ok)throw std::runtime_error("Imported MAP original predicate mismatch");}
int main(int argc,char** argv)try{
    if(argc!=3)throw std::invalid_argument("Supply hash-checked PE and frozen MAP");
    native_reference::initialize(argv[1],true,true,true,true);
    const auto s=read_route_world(argv[2]);
    if(std::uint64_t(s->dimensions.x)*s->dimensions.y*s->dimensions.z>4096)throw std::invalid_argument("Reference crop budget");
    const CellValidityMapView v{{s->dimensions,s->cells.data(),s->cells.size(),s->rows.data(),s->rows.size(),s->layers.data(),s->layers.size(),s->plane_stride,s->dimensions.z},s->terrain.data(),s->terrain.size()};
    const CreatureMovementParameters p{1,1,0,0,0,0,0};
    auto h=with_boundary_test(with_clearance_tests(MovementHelpers{s->dimensions,[&]{return s->boundary;},[&]{return s->dimensions.z;},{},{}},v));
    h=with_validity_test(h,with_cell_validity_test(ValidityHelpers{s->dimensions,{}},v));
    h=with_record_query(h,with_cell_support(RecordQueryHelpers{s->dimensions,[&]{return s->boundary;},{}},v));
    unsigned cells=0,moves=0,accepted=0,refused=0,sealAccepted=0;
    std::array<unsigned,6> categories{};
    const auto originalCells=s->cells;const auto originalTerrain=s->terrain;
    for(int z=0;z<s->dimensions.z;++z)for(int y=0;y<s->dimensions.y;++y)for(int x=0;x<s->dimensions.x;++x){
        const Coordinates from{x,y,z};
        const auto valid=test_cell_validity(from,p,v),support=test_cell_support(from,p,v)!=0;
        check(native_reference::run_cell_rules(from,p,v)==valid);
        check((native_reference::run_cell_support(from,p,v,s->boundary)!=0)==support);++cells;
        if(!valid || !support || !x || !y || x+1==s->dimensions.x || y+1==s->dimensions.y)continue;
        for(int dz=-1;dz<=1;++dz)for(int dy=-1;dy<=1;++dy)for(int dx=-1;dx<=1;++dx){
            if(!dx && !dy && !dz)continue;
            const Coordinates to{x+dx,y+dy,z+dz};if(to.z<0 || to.z>=s->dimensions.z)continue;
            int category=5,expectedCategory=5;
            const auto actual=test_movement(from,to,p,category,-1,h);
            const auto original=native_reference::run_complete_movement(from,to,p,expectedCategory,-1,v,v,s->boundary);
            check(actual==original && category==expectedCategory);++moves;
            if(actual){++accepted;check(category>=0 && category<=5);++categories[category];if(!to.x || !to.y || to.x+1==s->dimensions.x || to.y+1==s->dimensions.y)++sealAccepted;}
            else ++refused;
        }
    }
    check(!sealAccepted);
    check(!std::memcmp(originalCells.data(),s->cells.data(),s->cells.size()*12));
    check(!std::memcmp(originalTerrain.data(),s->terrain.data(),s->terrain.size()*356));
    std::cout<<"{\"all_match\":true,\"cells\":"<<cells<<",\"predicate_comparisons\":"<<cells*2<<",\"moves\":"<<moves<<",\"accepted\":"<<accepted<<",\"rejected\":"<<refused<<",\"sealed_moves_accepted\":"<<sealAccepted<<",\"categories\":[";
    for(unsigned i=0;i<categories.size();++i){if(i)std::cout<<',';std::cout<<categories[i];}std::cout<<"]}\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
