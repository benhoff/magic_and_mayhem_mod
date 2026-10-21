#include "view_refresh.hpp"
#include "effect_animation_selection.hpp"
#include "effect_view_tables.hpp"
#include <cmath>
#include <stdexcept>
namespace mnm::reconstruction {
namespace {
using Target=AnimationViewAction::Target;using Operation=AnimationViewAction::Operation;
void viewIndex(unsigned view){if(view>3)throw std::invalid_argument("Animation camera view");}
std::uint32_t quantize(float x,float y,bool sixteen){
 if(!std::isfinite(x)||!std::isfinite(y))throw std::invalid_argument("Effect direction finite velocity");
 auto angle=std::atan2(static_cast<long double>(x),-static_cast<long double>(y));
 if(angle<EffectAngleZero)angle+=sixteen?EffectAnglePi+EffectAnglePi:EffectAngle8Turn;
 const auto value=sixteen?(angle*EffectAngle16Scale)/EffectAnglePi:angle*EffectAngle8Scale;
 // Original stores the x87 expression to a double before floor/__ftol.
 const auto rounded=std::floor(static_cast<double>(value+EffectAngleHalf));
 return std::uint32_t(rounded)&(sixteen?15:7);
}
std::int32_t delta(std::int32_t a,std::int32_t b,unsigned period){
 if(!period||period>4096)throw std::invalid_argument("Effect direction extent");
 auto difference=(std::int64_t(a)-b)%period;if(difference<0)difference+=period;
 if(difference>period/2)difference-=period;
 return std::int32_t(difference);
}
void dimensions(const EffectViewInputs& input){if(!input.width||input.width>128||!input.height||input.height>128)throw std::invalid_argument("Effect direction map dimensions");}
const EffectViewCreature* creature(const EffectViewInputs& input,std::uint32_t ordinal){
 return ordinal<input.creatures.size()&&input.creatures[ordinal]?&*input.creatures[ordinal]:nullptr;
}
std::uint32_t sequence(const EffectViewInputs& input,std::uint32_t ordinal){return input.sequenceBases.at(ordinal);}
}
std::uint32_t effectViewDirection8(float x,float y){return quantize(x,y,false);}
std::uint32_t effectViewDirection16(float x,float y){return quantize(x,y,true);}
std::vector<AnimationViewAction> refreshCreatureView(CreatureViewState& state,unsigned view,std::optional<CreatureSpecialView> special){
 viewIndex(view);auto next=state;next.viewDirection=(next.direction-2*view-(next.reverseFacing?4:0))&7;
 std::vector<AnimationViewAction> actions;actions.reserve(4);
 if(next.animationKind==3){
  if(!special)throw std::invalid_argument("Creature special view requires owned catalog selection");
  actions.push_back({Target::special,Operation::bindCatalog,special->catalogIndex});
  actions.push_back({Target::special,Operation::startForward,special->sequenceBase+next.viewDirection});
 }else if(next.animationKind==2&&next.linkedEffect)actions.push_back({Target::linkedEffect,Operation::refreshLinkedEffect,next.viewDirection});
 if(next.secondary)actions.push_back({Target::secondary,Operation::switchSequence,next.viewDirection+8});
 actions.push_back({Target::main,Operation::switchSequence,next.mainSequenceBase+next.viewDirection});state=next;return actions;
}
std::vector<AnimationViewAction> refreshEffectView(EffectViewState& state,unsigned view,const EffectViewInputs& input){
 auto next=state;Operation operation=Operation::switchSequence;std::uint32_t selected=0;
 switch(next.type){
 case 0:case 4:case 7:{
  const auto ordinal=selectEffectAnimation(next.type,next.kind,input.kindClasses,next.creatureOrdinal,input.creatureDescriptorClasses);
  if(ordinal==NoEffectAnimation)throw std::invalid_argument("Effect view lacks selected metadata");
  selected=sequence(input,ordinal);operation=Operation::startForward;
  if(next.kind==93||next.kind==95||next.kind==98){
   viewIndex(view);next.worldDirection=effectViewDirection16(next.velocityX,next.velocityY);next.viewDirection=(next.worldDirection-4*view)&15;
   selected+=next.stage*16+next.viewDirection;
  }
  break;
 }
 case 20:viewIndex(view);selected=sequence(input,64)+std::uint32_t(EffectType20Views.at(view).at(next.orientationIndex));operation=Operation::startForward;break;
 case 63:viewIndex(view);selected=sequence(input,68)+std::uint32_t(EffectType63Views.at(view).at(next.variant));break;
 case 81:{
  viewIndex(view);dimensions(input);
  if(next.targetOrdinal!=0xffffffffu)next.cachedTarget=creature(input,next.targetOrdinal)?std::optional<std::uint32_t>(next.targetOrdinal):std::nullopt;
  std::int32_t x=0,y=0;
  if(next.mode==4){x=delta(next.destinationX,next.initialX,input.width*32);y=delta(next.destinationY,next.initialY,input.height*32);}
  else{
   const auto* target=next.cachedTarget?creature(input,*next.cachedTarget):nullptr;
   if(!target)throw std::invalid_argument("Effect view requires a live owned cached target");
   x=delta(next.column,target->column,input.width);y=delta(next.row,target->row,input.height);
  }
  next.worldDirection=effectViewDirection8(float(x),float(y));selected=sequence(input,72)+((next.worldDirection-2*view)&7);break;
 }
 case 51:{
  const auto* target=creature(input,next.creatureOrdinal);
  next.cachedTarget=target?std::optional<std::uint32_t>(next.creatureOrdinal):std::nullopt;
  if(!target){state=next;return {};}
  viewIndex(view);dimensions(input);
  const auto x=delta(target->column,next.column,input.width),y=delta(target->row,next.row,input.height);
  next.worldDirection=effectViewDirection8(float(x),float(y));selected=sequence(input,44)+((next.worldDirection-2*view)&7);break;
 }
 default:return {};
 }
 std::vector<AnimationViewAction> actions{{Target::effect,operation,selected}};state=next;return actions;
}
std::vector<AnimationViewAction> refreshCreatureLinkedEffect(EffectViewState& state,const EffectViewInputs& input){
 if(state.type!=52)return {};
 const auto* owner=creature(input,state.targetOrdinal);if(!owner)throw std::invalid_argument("Linked effect requires owned creator");
 const auto selected=sequence(input,45)+owner->viewDirection;
 std::vector<AnimationViewAction> actions{{Target::effect,Operation::switchSequence,selected}};state.cachedTarget=state.targetOrdinal;return actions;
}
}
