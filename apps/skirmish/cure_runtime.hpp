#pragma once
namespace mnm::reconstruction::casting { struct CureAnimationDefinition; }
#include "simulation/skirmish.hpp"
namespace mnm::skirmish {
constexpr std::uint64_t cureBattleIdentity = 0x4355524542415431ULL;
constexpr unsigned cureCost = 3, cureMagnitude = 8;
using CureAnimationDefinition =
    reconstruction::casting::CureAnimationDefinition;
// Default arena uses a declared authored ANI sequence and existing fixture
// cost/ magnitude. Callers may supply installed selected records; snapshots own
// them.
std::shared_ptr<const game::SkirmishEffects> cureRuntime();
std::shared_ptr<const game::SkirmishEffects>
    cureRuntime(CureAnimationDefinition);
std::shared_ptr<const game::SkirmishEffects>
resolveCureRuntime(const game::State &);
struct CureView {
  game::Handle effect, target;
  game::Point position;
};
std::vector<CureView> cureViews(const game::CombatSession &);
} // namespace mnm::skirmish
