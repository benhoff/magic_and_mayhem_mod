#pragma once
#include "simulation/movement.hpp"

namespace mnm::sandbox {
// Bounded same-profile frozen-input adapter; multi-mover policy is opt-in. No live addresses are dereferenced.
std::shared_ptr<const game::Navigation> loadFrozenNavigation(const std::string& path,const std::optional<game::AnimationBinding>& = {},bool stationaryOccupancy=false,bool multiMovement=false);
std::shared_ptr<const game::Navigation> loadFrozenNavigationBytes(const std::vector<std::byte>&,const std::optional<game::AnimationBinding>& = {},bool stationaryOccupancy=false,bool multiMovement=false);
game::AnimationBinding loadMovementAnimation(const std::string& path,std::uint32_t sequenceBase);
}
