#include "effect_transition.hpp"
#include "terrain_lighting_cycle.hpp"
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool v){if(!v)throw std::runtime_error("Effect transition assertion failed");}
static EffectPlacementPool created(){
    EffectPlacementPool pool(32,32,3,3);for(auto& c:pool.cells()){c.terrain=1;c.flags=0x80;}
    std::array<std::uint32_t,63> p{};p[0]=8*32+1;p[1]=8*32+1;p[2]=1;p[6]=0xffffffffu;p[9]=1;
    pool.place(0,3,p);pool.place(1,13,p);p[0]=16*32+1;pool.place(2,22,p);return pool;
}
int main(){
    auto pool=created();EffectTransitionState state;state.previousPosition=pool.records()[1].position;
    state.motion.trajectory.words[4]=8*32;
    const auto oldCell=pool.records()[1].cell,newCell=pool.records()[2].cell;
    EffectLightingTable table;table.entries()[13].diameter=4;
    // First produce the source at its original position, then move it.
    auto still=state;still.motion.trajectory.words[4]=0;
    transitionEffectEmptyWorld(pool,1,still,32,32,3);
    TerrainLightingCycle cycle(32,32,3,{-50,7});TerrainLightingSnapshot input;
    input.changed=1;input.view={12,8,24};input.objectScan=1;input.objects={pool.records()[1].lightingSource(table)};
    cycle.step(input);require(cycle.field().at(8,8,0)==-85);
    require(transitionEffectEmptyWorld(pool,1,state,32,32,3)==3);
    const auto& moved=pool.records()[1];require(moved.cell==newCell && moved.position[0]==16 && state.previousPosition[0]==8);
    require(moved.initialPosition[0]==8 && moved.sentinels[6]==16 && pool.cells()[oldCell].head==0);
    require(pool.records()[0].next==NoEffect && pool.cells()[newCell].head==2 && pool.records()[2].next==1);
    input.objects={moved.lightingSource(table)};cycle.step(input);
    require(cycle.field().at(16,8,0)==-85 && cycle.field().at(8,8,0)==-127);
    // Zero-ordinal final departure: column veto preserves clear flag;
    // absent table permits flag 80, while another effect still blocks cleanup.
    for(unsigned mode=0;mode<3;++mode){
        pool=created();pool.cells()[oldCell].terrain=0;pool.cells()[oldCell].flags=0;
        state={};state.terrain=0;state.previousPosition=pool.records()[1].position;state.motion.trajectory.words[4]=8*32;
        if(mode!=2){pool.cells()[oldCell].head=1;pool.records()[0].active=false;pool.records()[0].next=NoEffect;}
        std::vector<EffectCleanupColumn> columns;
        if(mode==1){columns.resize(32*32);columns[oldCell%(32*32)]={1,0,1};}
        transitionEffectEmptyWorld(pool,1,state,32,32,3,columns);
        require(pool.cells()[oldCell].flags==(mode==0?0x80u:0u));
    }
    for(unsigned mode=0;mode<7;++mode){
        pool=created();state={};state.previousPosition=pool.records()[1].position;state.motion.trajectory.words[4]=8*32;
        if(mode==0)pool.records()[0].next=0;
        if(mode==1)pool.cells()[newCell].terrain=4;
        if(mode==2)pool.cells()[newCell].flags|=0x40000000u;
        if(mode==3)state.motion.trajectory.words[1]=0xfffffffeu;
        if(mode==4){pool.records()[1].parameters[9]=8;state.motion.trajectory.words[4]=32;pool.cells()[oldCell+4].terrain=4;}
        if(mode==5)pool.cells()[oldCell].head=0xffff;
        std::vector<EffectCleanupColumn> columns;
        if(mode==6)columns.resize(1);
        auto before=pool;auto oldState=state;bool refused=false;
        try{transitionEffectEmptyWorld(pool,1,state,32,32,3,columns);}catch(const std::invalid_argument&){refused=true;}
        require(refused && pool.records()[1].parameters==before.records()[1].parameters && pool.records()[1].position==before.records()[1].position);
        require(pool.records()[0].next==before.records()[0].next && pool.records()[2].next==before.records()[2].next);
        for(unsigned i=0;i<pool.cells().size();++i)require(pool.cells()[i].head==before.cells()[i].head && pool.cells()[i].flags==before.cells()[i].flags);
        require(state.motion.trajectory.words==oldState.motion.trajectory.words && state.motion.changes==oldState.motion.changes && state.previousPosition==oldState.previousPosition);
    }
}
