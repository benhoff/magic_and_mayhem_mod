#pragma once
#include <cstdint>
#include <vector>
#include <optional>
namespace mnm::reconstruction {
struct AnimationViewAction {
 enum class Target {main,special,secondary,linkedEffect,effect};
 enum class Operation {bindCatalog,startForward,switchSequence,refreshLinkedEffect};
 Target target;Operation operation;std::uint32_t value=0;
};
struct CreatureViewState {
 std::uint32_t direction=0,viewDirection=0,animationKind=0,mainSequenceBase=0;
 bool reverseFacing=false,linkedEffect=false,secondary=false;
};
struct CreatureSpecialView {std::uint32_t sequenceBase=0,catalogIndex=0;};
// 00508a10: directions and ordered requests; special catalog fields are explicit
// owned inputs from the original shared selection, not guessed creature offsets.
std::vector<AnimationViewAction> refreshCreatureView(CreatureViewState&,unsigned view,
 std::optional<CreatureSpecialView> = {});
struct EffectViewCreature {std::int32_t column=0,row=0;std::uint32_t viewDirection=0;};
struct EffectViewState {
 // type=+28, kind=+4c. These are distinct original fields.
 std::uint32_t type=0,kind=0,stage=0,variant=0,orientationIndex=0,mode=0;
 std::uint32_t worldDirection=0,viewDirection=0;
 std::uint32_t targetOrdinal=0xffffffffu,creatureOrdinal=0xffffffffu;
 std::optional<std::uint32_t> cachedTarget;
 std::int32_t column=0,row=0,initialX=0,initialY=0,destinationX=0,destinationY=0;
 float velocityX=0,velocityY=0;
};
struct EffectViewInputs {
 unsigned width=0,height=0;
 std::vector<std::uint32_t> sequenceBases;
 std::vector<std::uint32_t> kindClasses,creatureDescriptorClasses;
 // Missing native slots are never interpreted as stale original objects.
 std::vector<std::optional<EffectViewCreature>> creatures;
};
std::uint32_t effectViewDirection8(float x,float y);
std::uint32_t effectViewDirection16(float x,float y);
// 004945f0. Only the selected branch consults metadata/targets/coordinates.
// Unknown types are successful no-ops. All refusals preserve owned state.
std::vector<AnimationViewAction> refreshEffectView(EffectViewState&,unsigned view,const EffectViewInputs&);
// 00494a50 ignores its original direction argument and reads the creator's
// updated view direction. Non-type52 is a successful no-op.
std::vector<AnimationViewAction> refreshCreatureLinkedEffect(EffectViewState&,const EffectViewInputs&);
}
