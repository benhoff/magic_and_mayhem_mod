#pragma once
#include "../command-session/native_providers.hpp"
#include "../world-scene/map_navigation.hpp"
#include <filesystem>
namespace mnm::encounter {
namespace cs = command_session;
struct Profile {
  game::Bytes bytes;
  cs::NativeCreatureProfile native;
  cs::Continuation initial;
  reconstruction::combat::MeleeInputs melee;
  unsigned maximum = 0;
  unsigned projectedObjects = 0, projectedReferences = 0;
};
// Portable immutable inputs: encrypted CFG/HTH, ANI, MAP, TTD and crop.
game::Bytes loadInputs(const std::filesystem::path &, scene::MapCrop);
Profile decodeInputs(game::Bytes);
cs::NativeConsequenceFactory consequences(const Profile &);
std::shared_ptr<const cs::Runtime> runtime(const Profile &);
game::CreatureRuleSession create(const Profile &, unsigned seed);
struct Statistics {
  unsigned hits = 0, damage = 0, stuns = 0, knockbacks = 0, deaths = 0,
           cues = 0;
};
Statistics statistics(const cs::Continuation &);
} // namespace mnm::encounter
