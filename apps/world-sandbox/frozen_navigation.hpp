#pragma once
#include "simulation/movement.hpp"

namespace mnm::sandbox {
// Bounded one-creature frozen-input adapter. No live addresses are dereferenced.
std::shared_ptr<const game::Navigation> loadFrozenNavigation(const std::string& path,const std::optional<game::AnimationBinding>& = {},bool stationaryOccupancy=false);
std::shared_ptr<const game::Navigation> loadFrozenNavigationBytes(const std::vector<std::byte>&,const std::optional<game::AnimationBinding>& = {},bool stationaryOccupancy=false);
game::AnimationBinding loadMovementAnimation(const std::string& path,std::uint32_t sequenceBase);
}
