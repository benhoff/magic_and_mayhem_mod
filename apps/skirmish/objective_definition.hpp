#pragma once
#include "simulation/mission.hpp"
#include <string_view>
namespace mnm::skirmish {
// Strict version-2 authoring adapter. The simulation itself has no JSON/Qt dependency.
game::MissionPlan decodeObjectiveDefinition(std::string_view);
}
