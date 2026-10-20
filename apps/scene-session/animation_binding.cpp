#include "animation_binding.hpp"
#include <set>
#include <stdexcept>
namespace mnm::scene {
SceneAnimationBindings::SceneAnimationBindings(assets::ResourceManager& resources,SceneAnimationLimits limits):resources_(resources),limits_(limits),thread_(std::this_thread::get_id()){
 if(!limits.receivers||limits.receivers>4096||!limits.bytes||limits.bytes>1024ULL*1024*1024)throw std::invalid_argument("Scene animation bounds");
}
void SceneAnimationBindings::checkThread() const {if(thread_!=std::this_thread::get_id())throw std::runtime_error("Scene animation owner thread");}
const assets::VisualResource& SceneAnimationBindings::current(const Track& track) const {
 const auto& resource=resources_.resident(track.resource);
 if(resource.revision!=track.revision||!resource.animation)throw std::invalid_argument("Stale scene animation resource revision");
 return resource;
}
std::vector<assets::AnimationRecord> SceneAnimationBindings::sequence(const Track& track,std::uint32_t ordinal) const {
 const auto& resource=current(track);const auto& animation=*resource.animation;
 if(std::uint64_t(ordinal)+1>=animation.starts.size())throw std::out_of_range("Scene animation sequence ordinal");
 const auto first=animation.starts[ordinal],end=animation.starts[ordinal+1];
 if(first>=end||end>animation.records.size()||end-first>65536)throw std::out_of_range("Scene animation sequence extent");
 std::vector<assets::AnimationRecord> result(animation.records.begin()+first,animation.records.begin()+end);
 for(const auto& record:result)if(record.opcode==0&&(record.argument<0||std::uint32_t(record.argument)>=resource.frameCount()))throw std::out_of_range("Scene animation displayed frame");
 return result;
}
std::uint64_t SceneAnimationBindings::trackCharge(const Track& track){
 // Owned record capacities, receiver/track storage and resource-name buffers.
 // Allocator/map-node overhead is excluded, matching ResourceManager's budget.
 return sizeof(std::pair<const AnimationReceiverId,Track>)+track.resource.name.capacity()+(track.controller?track.controller->retainedRecordBytes():0);
}
std::uint64_t SceneAnimationBindings::charge(const std::map<AnimationReceiverId,Track>& tracks) const {
 std::uint64_t bytes=0;for(const auto& entry:tracks)bytes+=trackCharge(entry.second);return bytes;
}
void SceneAnimationBindings::budget(const std::map<AnimationReceiverId,Track>& tracks) const {if(tracks.size()>limits_.receivers||charge(tracks)>limits_.bytes)throw std::length_error("Scene animation retained budget");}
void SceneAnimationBindings::apply(const std::vector<SceneAnimationCommand>& commands){
 checkThread();if(commands.empty())return;if(commands.size()>65536)throw std::length_error("Scene animation command count");auto staged=tracks_;auto bytes=charge(staged);
 for(const auto& command:commands){
  if(!command.receiver.owner||command.receiver.channel>15)throw std::invalid_argument("Scene animation receiver identity");
  const auto previous=staged.find(command.receiver);const auto prior=previous==staged.end()?0:trackCharge(previous->second);
  switch(command.operation){
  case SceneAnimationCommand::Operation::bind:{
   if(!command.resource)throw std::invalid_argument("Animation bind requires resource identity");
   const auto& resource=resources_.resident(*command.resource);if(!resource.animation)throw std::invalid_argument("Animation bind requires resident ANI");
   staged.insert_or_assign(command.receiver,Track{*command.resource,resource.revision,0,{}});break;
  }
  case SceneAnimationCommand::Operation::startForward:{
   if(command.resource)throw std::invalid_argument("Animation start cannot implicitly rebind");
   auto& track=staged.at(command.receiver);auto records=sequence(track,command.sequence);
   track.controller.emplace(std::move(records));track.controller->start(false);track.sequence=command.sequence;break;
  }
  case SceneAnimationCommand::Operation::switchSequence:{
   if(command.resource)throw std::invalid_argument("Animation switch cannot implicitly rebind");
   auto& track=staged.at(command.receiver);if(!track.controller)throw std::invalid_argument("Animation switch requires started receiver");
   auto records=sequence(track,command.sequence);track.controller->switchSequence(std::move(records));track.sequence=command.sequence;break;
  }
  default:throw std::invalid_argument("Unknown scene animation operation");
  }
  bytes=bytes-prior+trackCharge(staged.at(command.receiver));
  if(staged.size()>limits_.receivers||bytes>limits_.bytes)throw std::length_error("Scene animation retained budget");
 }
 tracks_.swap(staged);
}
SceneAnimationSnapshot SceneAnimationBindings::snapshot(AnimationReceiverId id) const {
 checkThread();const auto& track=tracks_.at(id);current(track);if(!track.controller)throw std::invalid_argument("Animation display requires started receiver");
 return {track.resource,track.revision,track.sequence,track.controller->sprite(),track.controller->state()};
}
std::vector<std::pair<AnimationReceiverId,std::int32_t>> SceneAnimationBindings::advance(const std::vector<AnimationReceiverId>& ids){
 checkThread();if(ids.empty())return {};if(ids.size()>limits_.receivers)throw std::length_error("Animation advancement count");std::set<AnimationReceiverId> unique;auto staged=tracks_;std::vector<std::pair<AnimationReceiverId,std::int32_t>> events;events.reserve(ids.size());
 for(const auto& id:ids){if(!unique.insert(id).second)throw std::invalid_argument("Duplicate animation advancement");auto& track=staged.at(id);current(track);if(!track.controller)throw std::invalid_argument("Animation advancement requires started receiver");events.push_back({id,track.controller->tick()});}
 budget(staged);tracks_.swap(staged);return events;
}
void SceneAnimationBindings::restore(AnimationReceiverId id,const reconstruction::AnimationControllerState& state){
 checkThread();auto staged=tracks_;auto& track=staged.at(id);current(track);if(!track.controller)throw std::invalid_argument("Animation restore requires started receiver");track.controller->restore(state);budget(staged);tracks_.swap(staged);
}
void SceneAnimationBindings::retireOwner(std::uint64_t owner){checkThread();for(auto it=tracks_.begin();it!=tracks_.end();)if(it->first.owner==owner)it=tracks_.erase(it);else ++it;}
void SceneAnimationBindings::clear(){checkThread();tracks_.clear();}
}
