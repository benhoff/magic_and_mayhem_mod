#pragma once
#include "simulation/skirmish.hpp"
#include <ostream>
namespace mnm::skirmish {
void trace(std::ostream &, const game::SkirmishSession &, const game::SkirmishReport & = {});
}
