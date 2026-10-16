#pragma once
#include "simulation/combat.hpp"
#include "simulation/creature_encounter.hpp"
namespace mnm::skirmish {
constexpr std::uint64_t creatureBattleIdentity = 0x4352424154543031ULL;
std::shared_ptr<const game::CombatDriver>
installedCreatureRuntime(const std::filesystem::path &);
std::shared_ptr<const game::CombatDriver>
resolveCreatureRuntime(const game::State &);
struct CreatureBattleView {
  game::Handle actor;
  game::CreaturePhase phase;
  std::optional<game::Handle> target;
};
std::vector<CreatureBattleView>
creatureBattleViews(const game::CombatSession &);
} // namespace mnm::skirmish
