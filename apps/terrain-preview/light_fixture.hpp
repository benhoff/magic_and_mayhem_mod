#pragma once
#include "terrain_lighting_cycle.hpp"
#include <istream>
namespace mnm::preview {
// Authored text snapshots for bounded offline presentation, not a game format.
// Maximum 64 ticks and 256 records per collection in each snapshot.
std::vector<reconstruction::TerrainLightingSnapshot> readLightingFixture(std::istream&);
}
