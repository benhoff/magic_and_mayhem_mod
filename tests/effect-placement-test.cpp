#include "effect_placement.hpp"
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool v){if(!v)throw std::runtime_error("Effect placement assertion failed");}
template<class F> void refuses(F f){bool refused=false;try{f();}catch(const std::invalid_argument&){refused=true;}require(refused);}
int main(){
    EffectPlacementPool pool(8,8,3,4);std::array<std::uint32_t,63> parameters{};
    parameters[0]=std::uint32_t(-1);parameters[1]=256;parameters[2]=99;
    parameters[3]=256;parameters[4]=std::uint32_t(-256);parameters[5]=48;parameters[6]=0xffffffff;
    pool.cells()[135].flags=0x80;pool.place(0,3,parameters);
    const auto& r=pool.records()[0];require(r.active && r.initialized && r.type==3 && r.cell==135);
    require(r.units==std::array<unsigned,3>{255,0,47} && r.position==std::array<unsigned,3>{7,0,2});
    require(r.parameters[3]==0 && r.parameters[4]==0 && r.parameters[5]==47);
    require(pool.cells()[135].head==0 && pool.cells()[135].flags==0 && pool.scan()==1);
    pool.place(2,13,parameters);require(pool.records()[0].next==2 && pool.records()[2].next==NoEffect && pool.scan()==3);
    auto fields=EffectLightingTable::Fields{};fields[3][0]="4";EffectLightingTable table;table.reload(fields);
    refuses([&]{pool.records()[0].lightingSource(table);});
    refuses([&]{pool.place(0,3,parameters);});refuses([&]{pool.place(4,3,parameters);});
    for(unsigned type:{0u,21u,89u})refuses([&]{pool.place(1,type,parameters);});
    auto bad=parameters;bad[6]=0;refuses([&]{pool.place(1,3,bad);});
    bad=parameters;bad[0]=512;refuses([&]{pool.place(1,3,bad);});
    bad=parameters;bad[2]=0xffffffff;refuses([&]{pool.place(1,3,bad);});
    require(!pool.records()[1].active && pool.records()[0].next==2 && pool.scan()==3);
    pool.records()[2].next=0;refuses([&]{pool.place(1,3,parameters);});require(!pool.records()[1].active && pool.scan()==3);
    pool.records()[2].next=99;refuses([&]{pool.place(1,3,parameters);});require(!pool.records()[1].active);
    pool.cells()[135].flags=0x40000080;pool.place(1,22,parameters);
    require(pool.cells()[135].flags==0x40000080 && pool.cells()[135].head==0 && pool.records()[1].active);
    refuses([&]{pool.setScan(5);});refuses([&]{EffectPlacementPool invalid(0,8,3,4);});
}
