#pragma once
#include "simulation/skirmish.hpp"
namespace mnm::skirmish {
// App-side frozen reconstruction adapter for an authored, sealed arena.
std::shared_ptr<const game::Navigation> arenaNavigation();
game::SkirmishSession newSession();
game::SkirmishSession load(const std::filesystem::path &);
} // namespace mnm::skirmish
