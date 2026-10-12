#pragma once
#include "simulation/melee_exchange.hpp"
#include <filesystem>
namespace mnm::melee_lab {
struct Profile {
  game::Bytes bytes;
  std::shared_ptr<const game::MeleeDriver> driver;
};
Profile loadProfile(const std::filesystem::path &installation);
Profile decodeProfile(game::Bytes);
game::MeleeExchangeSession create(Profile, std::uint32_t seed = 1);
game::MeleeExchangeSession load(const std::filesystem::path &);
} // namespace mnm::melee_lab
