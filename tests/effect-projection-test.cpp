#include "effect_projection.hpp"
#include "terrain_lighting_cycle.hpp"
#include <stdexcept>
using namespace mnm::reconstruction;
static void require(bool v){if(!v)throw std::runtime_error("Effect projection assertion failed");}
static EffectPlacementRecord created(unsigned w=32,unsigned h=32){
    EffectPlacementPool pool(w,h,3,1);std::array<std::uint32_t,63> p{};
    p[0]=w==1?1:8*32+1;p[1]=h==1?1:8*32+1;p[2]=1;p[6]=0xffffffffu;p[9]=1;
    pool.place(0,3,p);return pool.records()[0];
}
int main(){
    auto r=created();EffectProjectionState s;s.trajectory.words[1]=1;s.trajectory.words[4]=1;
    EffectLightingTable table;table.entries()[3].diameter=4;
    bool refused=false;try{r.lightingSource(table);}catch(const std::invalid_argument&){refused=true;}require(refused);
    const auto before=r.units;require(projectEffectSameCell(r,s,32,32,3)==3);
    require(s.previousUnits==before && s.changes==4 && r.units[0]==258 && r.units[2]==2);
    require(r.sentinels[6]==8 && r.sentinels[7]==8 && r.sentinels[8]==0);
    auto light=r.lightingSource(table);require(light.active && light.admissionColumn==8 && light.admissionRow==8);
    TerrainLightingSnapshot input;input.changed=1;input.view={8,8,16};input.objectScan=1;input.objects={light};
    TerrainLightingCycle cycle(32,32,3,{-50,7});cycle.step(input);require(cycle.field().at(8,8,0)==-85);
    // A later failing iteration must roll back earlier valid steps as well.
    r=created();r.parameters[9]=8;s={};s.trajectory.words[4]=4;
    auto old=r;auto oldState=s;
    refused=false;try{projectEffectSameCell(r,s,32,32,3);}catch(const std::invalid_argument&){refused=true;}
    require(refused && r.parameters==old.parameters && r.units==old.units && r.sentinels==old.sentinels);
    require(s.trajectory.words==oldState.trajectory.words && s.changes==oldState.changes && s.previousUnits==oldState.previousUnits);
    for(unsigned mode=0;mode<7;++mode){
        r=created();s={};
        if(mode==0)s.trajectory.words[1]=0xfffffffeu;
        if(mode==1)s.trajectory.words[1]=48;
        if(mode==2)s.trajectory.words[4]=2048;
        if(mode==3)r.parameters[8]=1;
        if(mode==4)r.parameters[9]=9;
        if(mode==5)s.changes=33;
        if(mode==6)r.position[0]=0;
        old=r;oldState=s;refused=false;
        try{projectEffectSameCell(r,s,32,32,3);}catch(const std::invalid_argument&){refused=true;}
        require(refused && r.parameters==old.parameters && r.units==old.units && r.position==old.position && r.sentinels==old.sentinels);
        require(s.trajectory.words==oldState.trajectory.words && s.changes==oldState.changes);
    }
    r=created(1,1);s={};s.trajectory.words[4]=32;s.trajectory.words[5]=0xffffffe0u;
    require(projectEffectSameCell(r,s,1,1,3)==3 && r.units[0]==1 && r.units[1]==1);
    r=created();r.parameters[9]=0;old=r;s={};oldState=s;
    require(projectEffectSameCell(r,s,32,32,3)==3 && r.sentinels==old.sentinels && s.changes==0);
}
