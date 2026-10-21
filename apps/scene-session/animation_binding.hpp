#pragma once
#include "../../assets/resources/resource_manager.hpp"
#include "../../reconstruction/animation/controller.hpp"
namespace mnm::scene {
struct AnimationReceiverId {
 // Caller-owned semantic generation identity, never an address or dense slot.
 std::uint64_t owner=0;unsigned channel=0;
 bool operator<(const AnimationReceiverId& other) const{return owner<other.owner||(owner==other.owner&&channel<other.channel);}
};
struct SceneAnimationCommand {
 enum class Operation {bind,startForward,switchSequence};
 AnimationReceiverId receiver;Operation operation;
 std::uint32_t sequence=0;std::optional<assets::ResourceId> resource;
};
struct SceneAnimationSnapshot {
 assets::ResourceId resource;std::uint64_t revision;std::uint32_t sequence;
 std::optional<std::uint32_t> frame;reconstruction::AnimationControllerState state;
};
struct SceneAnimationLimits {std::size_t receivers=1024;std::uint64_t bytes=64ULL*1024*1024;};
class SceneAnimationBindings {
 struct Track {
  assets::ResourceId resource;std::uint64_t revision=0;std::uint32_t sequence=0;
  std::optional<reconstruction::NoCdAnimationController> controller;
 };
 assets::ResourceManager& resources_;
 std::map<AnimationReceiverId,Track> tracks_;
 SceneAnimationLimits limits_;std::thread::id thread_;
 struct StageTag {};
 std::shared_ptr<const int> identity_=std::make_shared<const int>(0);
 std::weak_ptr<const int> origin_;
 std::uint64_t version_=0,baseVersion_=0;
 SceneAnimationBindings(const SceneAnimationBindings&,StageTag);
 void changed();
 void checkThread() const;
 const assets::VisualResource& current(const Track&) const;
 std::vector<assets::AnimationRecord> sequence(const Track&,std::uint32_t) const;
 static std::uint64_t trackCharge(const Track&);
 std::uint64_t charge(const std::map<AnimationReceiverId,Track>&) const;
 void budget(const std::map<AnimationReceiverId,Track>&) const;
public:
 explicit SceneAnimationBindings(assets::ResourceManager&,SceneAnimationLimits={});
 SceneAnimationBindings(const SceneAnimationBindings&)=delete;
 SceneAnimationBindings& operator=(const SceneAnimationBindings&)=delete;
 void assertOwnerThread() const {checkThread();}
 // Independently owned candidate for application-level frame transactions.
 // Publication refuses intervening owner mutation or changed resource revisions.
 SceneAnimationBindings stage() const;
 void publish(SceneAnimationBindings&&);
 // All commands commit together. Bind resets controller state; switches retain
 // relative ANI positions/timing and require a compatible active display.
 void apply(const std::vector<SceneAnimationCommand>&);
 SceneAnimationSnapshot snapshot(AnimationReceiverId) const;
 // The simulation owns tick selection; duplicate receivers are refused. This
 // service never installs a second timer or implicitly advances creature motion.
 std::vector<std::pair<AnimationReceiverId,std::int32_t>> advance(const std::vector<AnimationReceiverId>&);
 void restore(AnimationReceiverId,const reconstruction::AnimationControllerState&);
 void restore(const std::vector<std::pair<AnimationReceiverId,reconstruction::AnimationControllerState>>&);
 void retireOwner(std::uint64_t);
 void retireReceiver(AnimationReceiverId);
 void clear();
 std::uint64_t retainedBytes() const {checkThread();return charge(tracks_);}
};
}
