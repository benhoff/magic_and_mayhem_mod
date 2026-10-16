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
game::Bytes initial();
void validate(const game::Bytes &, const game::CombatSession &,
              const game::SkirmishState &);
game::SkirmishRefusal cast(game::Bytes &, game::CombatSession &,
                           game::SkirmishState &, game::Handle, game::Handle);
game::CombatReport step(game::Bytes &, game::CombatSession &);
void cancel(game::Bytes &, game::CombatSession &);
void removed(game::Bytes &, game::CombatSession &, game::Handle);
std::vector<View> views(const game::Bytes &);
} // namespace mnm::skirmish::sphere
