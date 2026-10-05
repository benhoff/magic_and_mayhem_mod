#pragma once
#include "creature_occupancy.hpp"
namespace mnm::reconstruction {
// Recovered 004e1260: every nonzero selector uses the large rectangular template.
CreatureOccupancy initializeCreatureFootprint(std::uint32_t height,
                                             std::uint32_t selector) noexcept;
}
