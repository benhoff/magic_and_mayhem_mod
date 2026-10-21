#include "view_refresh.hpp"
#include <limits>
#include <iostream>
#include <stdexcept>
using namespace mnm::reconstruction;
using Target=AnimationViewAction::Target;using Operation=AnimationViewAction::Operation;
static void check(bool ok){if(!ok)throw std::runtime_error("View refresh unit mismatch");}
template<class F> static void refused(F f){bool caught=false;try{f();}catch(const std::exception&){caught=true;}check(caught);}
int main()try{
 check(effectViewDirection8(0,-1)==0&&effectViewDirection8(1,0)==2&&effectViewDirection8(0,1)==4&&effectViewDirection8(-1,0)==6);
 check(effectViewDirection16(0,-1)==0&&effectViewDirection16(1,0)==4&&effectViewDirection16(0,1)==8&&effectViewDirection16(-1,0)==12);
 refused([]{effectViewDirection8(std::numeric_limits<float>::quiet_NaN(),0);});refused([]{effectViewDirection16(0,std::numeric_limits<float>::infinity());});
 CreatureViewState actor;actor.direction=1;actor.reverseFacing=true;actor.animationKind=3;actor.secondary=true;actor.mainSequenceBase=0xffffffffu;
 auto actions=refreshCreatureView(actor,3,CreatureSpecialView{16,2});check(actor.viewDirection==7&&actions.size()==4&&actions[0].target==Target::special&&actions[0].value==2&&actions[1].value==23&&actions[2].value==15&&actions[3].value==6);
 actor.viewDirection=99;refused([&]{refreshCreatureView(actor,1);});check(actor.viewDirection==99);refused([&]{refreshCreatureView(actor,4,CreatureSpecialView{});});check(actor.viewDirection==99);
 actor.animationKind=2;actor.linkedEffect=true;actions=refreshCreatureView(actor,0);check(actions[0].operation==Operation::refreshLinkedEffect&&actions.back().target==Target::main);
 EffectViewInputs inputs;inputs.width=17;inputs.height=31;inputs.sequenceBases.resize(89);for(unsigned i=0;i<89;++i)inputs.sequenceBases[i]=i*100+3;inputs.kindClasses.assign(103,0);inputs.creatureDescriptorClasses.assign(1,0);inputs.creatures={EffectViewCreature{0,0,7}};
 EffectViewState state;state.type=81;state.mode=4;state.initialX=0;state.initialY=0;state.destinationX=32;state.destinationY=0;actions=refreshEffectView(state,1,inputs);check(state.worldDirection==2&&actions[0].value==7203);
 state.mode=0;state.targetOrdinal=0;state.column=16;actions=refreshEffectView(state,1,inputs);check(state.worldDirection==6&&actions[0].value==7207&&state.cachedTarget==0);
 state.targetOrdinal=9;const auto before=state;refused([&]{refreshEffectView(state,1,inputs);});check(state.cachedTarget==before.cachedTarget&&state.worldDirection==before.worldDirection);
 state.type=51;state.creatureOrdinal=0;actions=refreshEffectView(state,1,inputs);check(state.worldDirection==2&&actions[0].value==4403);state.creatureOrdinal=9;check(refreshEffectView(state,99,{}).empty()&&!state.cachedTarget);
 state.type=52;state.targetOrdinal=0;actions=refreshCreatureLinkedEffect(state,inputs);check(actions[0].value==4510&&state.cachedTarget==0);state.targetOrdinal=9;refused([&]{refreshCreatureLinkedEffect(state,inputs);});check(state.cachedTarget==0);
 state.type=20;state.orientationIndex=35;actions=refreshEffectView(state,1,inputs);check(actions[0].value==6417&&actions[0].operation==Operation::startForward);state.orientationIndex=36;refused([&]{refreshEffectView(state,1,inputs);});
 state.type=63;state.variant=3;actions=refreshEffectView(state,2,inputs);check(actions[0].value==6805&&actions[0].operation==Operation::switchSequence);
 state.type=0;state.kind=0;actions=refreshEffectView(state,99,inputs);check(actions[0].value==1503&&actions[0].operation==Operation::startForward);state.kind=103;refused([&]{refreshEffectView(state,0,inputs);});
 state.type=999;check(refreshEffectView(state,99,{}).empty()&&refreshCreatureLinkedEffect(state,{}).empty());
 std::cout<<"View directions, branch requests and atomic owned target refusals passed\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
