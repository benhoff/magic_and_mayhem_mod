#pragma once
#include "simulation/skirmish.hpp"
namespace mnm::skirmish::sphere {
constexpr unsigned spell = 73, cost = 8, magnitude = 20, speed = 20,
                   lifetime = 100;
// Authored arena profile. Numerals are declared fixtures; installed CFG binding
// and complete original ricochet/physical presentation remain separate.
struct View {
  game::Handle effect;
  game::Point fine;
  unsigned remaining;
};
struct TerrainCell { unsigned terrain = 0, flags = 0; };
// Empty cells retain the legacy open8x8x3 arena. A bound mission owns every
// catalog bitmap/cell flag; resource restoration checks the complete binding.
struct Environment {
  game::Point dimensions{8,8,3};
  std::vector<TerrainCell> cells;
  std::vector<std::array<std::uint8_t,32>> occupancy{4};
  unsigned creatureWidth=1, creatureHeight=1;
  bool actorProfiles=false; // SHP4: resolve body/aim dimensions per bound actor.
};
bool sameEnvironment(const Environment &, const Environment &);
Environment environment(const game::Bytes &);
game::Bytes initial(Environment = {});
void validate(const game::Bytes &, const game::CombatSession &,
              const game::SkirmishState &, const Environment & = {});
game::SkirmishRefusal cast(game::Bytes &, game::CombatSession &,
                           game::SkirmishState &, game::Handle, game::Handle);
game::CombatReport step(game::Bytes &, game::CombatSession &);
void cancel(game::Bytes &, game::CombatSession &);
void removed(game::Bytes &, game::CombatSession &, game::Handle);
std::vector<View> views(const game::Bytes &);
} // namespace mnm::skirmish::sphere
