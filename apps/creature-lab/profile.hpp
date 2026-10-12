#pragma once
#include "simulation/creature_encounter.hpp"
#include <filesystem>
namespace mnm::creature_lab {
struct Profile {
  game::Bytes bytes;
  std::shared_ptr<const game::CreatureRules> rules;
};
Profile loadProfile(const std::filesystem::path &);
Profile decodeProfile(game::Bytes);
game::CreatureEncounterSession create(Profile, std::uint32_t seed = 1);
game::CreatureEncounterSession load(const std::filesystem::path &);
} // namespace mnm::creature_lab
