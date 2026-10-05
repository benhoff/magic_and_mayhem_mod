#pragma once
#include "simulation/movement.hpp"

namespace mnm::sandbox {
// Bounded one-creature frozen-input adapter. No live addresses are dereferenced.
std::shared_ptr<const game::Navigation> loadFrozenNavigation(const std::string& path);
}
