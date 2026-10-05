#pragma once
#include "world.hpp"

namespace mnm::game {
// Owned native policy: logical-cell boxes for one profile. No legacy IDs/pointers.
class Occupancy {
    Point dimensions_;
    std::vector<std::optional<Handle>> cells_;
    std::size_t index(Point) const;
public:
    Occupancy(const State&,Point dimensions,std::uint32_t creatureType,Point footprint);
    std::optional<Handle> owner(Point) const;
    bool blocked(Point,Handle self) const;
};
}
