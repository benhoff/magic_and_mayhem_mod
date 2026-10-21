#include "camera_binding.hpp"
#include <set>
#include <stdexcept>
namespace mnm::scene {
namespace {
using Action=reconstruction::AnimationViewAction;
using Target=Action::Target;using Operation=Action::Operation;
using Command=SceneAnimationCommand;
template<class Pool> bool live(const std::optional<SceneActorReference>& ref,const Pool& pool){return ref&&ref->identity&&ref->ordinal<pool.size()&&pool[ref->ordinal].active&&pool[ref->ordinal].identity==ref->identity;}
unsigned channel(Target target){switch(target){case Target::main:return 0;case Target::special:return 1;case Target::secondary:return 2;case Target::effect:return 3;default:throw std::invalid_argument("Unresolved linked animation target");}}
}
void applyCameraViews(SceneAnimationBindings& animations,const std::vector<reconstruction::CameraRefresh>& requests,
 std::vector<SceneCreatureView>& creatures,std::vector<SceneEffectView>& effects,
 SceneMinimapView& minimap,reconstruction::EffectViewInputs input,const std::vector<assets::ResourceId>& catalog){
 animations.assertOwnerThread();if(requests.empty())return;
 if(creatures.size()>65536||effects.size()>65536||requests.size()>131073||catalog.size()>4096)throw std::length_error("Scene view pool bounds");
 auto nextCreatures=creatures;auto nextEffects=effects;auto nextMinimap=minimap;std::set<std::uint64_t> owners;
 for(const auto& creature:creatures)if(creature.active&&(!creature.identity||!owners.insert(creature.identity).second))throw std::invalid_argument("Creature view ownership identity");
 for(const auto& effect:effects)if(effect.active&&(!effect.identity||!owners.insert(effect.identity).second))throw std::invalid_argument("Effect view ownership identity");
 input.creatures.assign(creatures.size(),{});
 for(std::size_t i=0;i<creatures.size();++i)if(creatures[i].active){auto position=creatures[i].position;position.viewDirection=creatures[i].state.viewDirection;input.creatures[i]=position;}
 std::vector<Command> commands;
 const auto append=[&](std::uint64_t owner,const Action& action){
  const AnimationReceiverId receiver{owner,channel(action.target)};
  switch(action.operation){
  case Operation::bindCatalog:commands.push_back({receiver,Command::Operation::bind,0,catalog.at(action.value)});break;
  case Operation::startForward:commands.push_back({receiver,Command::Operation::startForward,action.value,{}});break;
  case Operation::switchSequence:commands.push_back({receiver,Command::Operation::switchSequence,action.value,{}});break;
  default:throw std::invalid_argument("Unresolved scene animation operation");
  }
 };
 const auto prepareEffect=[&](SceneEffectView& effect){
  // An explicit stale target differs from the no-target sentinel: it clears
  // the cached target if the selected original branch performs a lookup.
  effect.state.targetOrdinal=effect.target?(live(effect.target,nextCreatures)?unsigned(effect.target->ordinal):0xfffffffeu):0xffffffffu;
  effect.state.creatureOrdinal=live(effect.creator,nextCreatures)?unsigned(effect.creator->ordinal):0xffffffffu;
  effect.state.cachedTarget=live(effect.cachedTarget,nextCreatures)?std::optional<std::uint32_t>(unsigned(effect.cachedTarget->ordinal)):std::nullopt;
 };
 const auto cacheEffect=[&](SceneEffectView& effect){
  if(effect.state.cachedTarget){const auto index=*effect.state.cachedTarget;if(index>=nextCreatures.size()||!nextCreatures[index].active)throw std::invalid_argument("Effect refresh produced unowned cache");effect.cachedTarget=SceneActorReference{index,nextCreatures[index].identity};}
  else effect.cachedTarget.reset();
 };
 for(const auto& request:requests){
  switch(request.kind){
  case reconstruction::CameraRefresh::Kind::creature:{
   auto& creature=nextCreatures.at(request.ordinal);if(!creature.active)throw std::invalid_argument("Inactive creature refresh");creature.state.linkedEffect=bool(creature.linkedEffect);
   const auto actions=reconstruction::refreshCreatureView(creature.state,request.view,creature.special);input.creatures[request.ordinal]->viewDirection=creature.state.viewDirection;
   for(const auto& action:actions){
    if(action.operation==Operation::refreshLinkedEffect){
     if(!live(creature.linkedEffect,nextEffects))throw std::invalid_argument("Stale creature linked-effect generation");
     auto& effect=nextEffects[creature.linkedEffect->ordinal];prepareEffect(effect);const auto linked=reconstruction::refreshCreatureLinkedEffect(effect.state,input);cacheEffect(effect);for(const auto& item:linked)append(effect.identity,item);
    }else append(creature.identity,action);
   }
   break;
  }
  case reconstruction::CameraRefresh::Kind::effect:{
   auto& effect=nextEffects.at(request.ordinal);if(!effect.active)throw std::invalid_argument("Inactive effect refresh");prepareEffect(effect);const auto actions=reconstruction::refreshEffectView(effect.state,request.view,input);cacheEffect(effect);for(const auto& action:actions)append(effect.identity,action);break;
  }
  case reconstruction::CameraRefresh::Kind::minimap:
   if(request.view>3||request.ordinal)throw std::invalid_argument("Minimap view request");
   nextMinimap.view=request.view;nextMinimap.dirty=true;break;
  default:throw std::invalid_argument("Unknown camera refresh request");
  }
 }
 // Actual controller application can fail on stale revisions, incompatible
 // phases or budgets. Publish the other owned state only after it succeeds.
 animations.apply(commands);creatures.swap(nextCreatures);effects.swap(nextEffects);minimap=nextMinimap;
}
void rotateSceneCamera(reconstruction::TerrainCameraLifecycle& state,reconstruction::CameraTurn turn,
 SceneAnimationBindings& animations,std::vector<SceneCreatureView>& creatures,std::vector<SceneEffectView>& effects,
 SceneMinimapView& minimap,reconstruction::EffectViewInputs input,const std::vector<assets::ResourceId>& catalog,std::optional<unsigned> primary){
 animations.assertOwnerThread();
 if(creatures.size()>65536||effects.size()>65536)throw std::length_error("Scene view pool bounds");
 auto next=state;std::vector<bool> activeCreatures,activeEffects;
 activeCreatures.reserve(creatures.size());activeEffects.reserve(effects.size());
 for(const auto& actor:creatures)activeCreatures.push_back(actor.active);
 for(const auto& effect:effects)activeEffects.push_back(effect.active);
 const auto requests=reconstruction::rotateTerrainCamera(next,turn,activeCreatures,activeEffects,primary);
 applyCameraViews(animations,requests,creatures,effects,minimap,std::move(input),catalog);state=next;
}
void finishSceneCameraFrame(reconstruction::TerrainCameraLifecycle& state,SceneAnimationBindings& animations,
 std::vector<SceneCreatureView>& creatures,std::vector<SceneEffectView>& effects,SceneMinimapView& minimap,
 reconstruction::EffectViewInputs input,const std::vector<assets::ResourceId>& catalog,std::optional<unsigned> primary){
 animations.assertOwnerThread();
 if(creatures.size()>65536||effects.size()>65536)throw std::length_error("Scene view pool bounds");
 auto next=state;std::vector<bool> activeCreatures,activeEffects;
 activeCreatures.reserve(creatures.size());activeEffects.reserve(effects.size());
 for(const auto& actor:creatures)activeCreatures.push_back(actor.active);
 for(const auto& effect:effects)activeEffects.push_back(effect.active);
 const auto requests=reconstruction::finishTerrainCameraFrame(next,activeCreatures,activeEffects,primary);
 applyCameraViews(animations,requests,creatures,effects,minimap,std::move(input),catalog);state=next;
}

}
