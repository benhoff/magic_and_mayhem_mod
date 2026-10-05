#include "creature_profile.hpp"
#include "creature_movement.hpp"
#include <stdexcept>
#include <iostream>
using namespace mnm;
static void check(bool b){if(!b)throw std::runtime_error("Creature profile assertion");}
template<class F>void refuses(F f){try{f();}catch(const std::invalid_argument&){return;}throw std::runtime_error("Expected profile refusal");}
int main(){
    assets::Config cfg;
    cfg.sections["creature_10"]={{"tileheight","3"},{"tilesizexy","1"},{"acceleration","10"},{"swimmingability","1"},{"canfly","FALSE"},{"groundspeed","4"},{"flyingspeed","0"}};
    auto c=reconstruction::normalizeCreatureMovementConfig(assets::creatureMovementConfig(cfg,10));check(c.height==3 && c.width==1 && c.acceleration==10 && !c.canFly && c.swimming==1 && c.groundSpeed==4);
    refuses([&]{assets::creatureMovementConfig(cfg,28);});
    auto bad=cfg;bad.sections["creature_10"]["acceleration"]="10junk";refuses([&]{assets::creatureMovementConfig(bad,10);});
    bad=cfg;bad.sections["creature_10"].erase("tileheight");refuses([&]{assets::creatureMovementConfig(bad,10);});
    bad=cfg;bad.sections["creature_10"]["canfly"]="1";refuses([&]{assets::creatureMovementConfig(bad,10);});
    bad=cfg;bad.sections["creature_10"]["tileheight"]="9";bad.sections["creature_10"]["acceleration"]="-2";
    const auto raw=assets::creatureMovementConfig(bad,10);check(raw.height==9 && raw.acceleration==-2);
    const auto bounded=reconstruction::normalizeCreatureMovementConfig(raw);check(bounded.height==5 && bounded.acceleration==0);
    assets::Animation ani;ani.starts.push_back(0);
    for(unsigned facing=0;facing<8;++facing){
        for(unsigned i=0;i<12;++i){assets::AnimationRecord r;r.argument=i;r.metadata[0]=0;ani.records.push_back(r);}
        assets::AnimationRecord event;event.opcode=5;event.argument=2;ani.records.push_back(event);
        assets::AnimationRecord repeat;repeat.metadata[0]=facing==1?30:16;ani.records.push_back(repeat);
        assets::AnimationRecord stop;stop.opcode=6;stop.argument=-1;ani.records.push_back(stop);ani.starts.push_back(ani.records.size());
    }
    const auto samples=reconstruction::groundMovementSamples(ani);
    for(unsigned i=0;i<12;++i)check(samples[i]==8 && samples[12+i]==(i%2?8u:7u));
    for(unsigned i=24;i<48;++i)check(!samples[i]);
    check(reconstruction::groundMovementMaximum(samples)==235);
    auto broken=ani;broken.records[12].argument=1;refuses([&]{reconstruction::groundMovementSamples(broken);});
    broken=ani;broken.records[13].argument=5;refuses([&]{reconstruction::groundMovementSamples(broken);});
    broken=ani;broken.records[13].metadata[0]=0;refuses([&]{reconstruction::groundMovementSamples(broken);});
    broken=ani;broken.starts[1]=broken.records.size()+1;refuses([&]{reconstruction::groundMovementSamples(broken);});
    std::cout<<"Selected profile admission, ground ANI shape and derived samples pass\n";
}
