#pragma once
#include "world.hpp"
#include <map>

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

namespace mnm::game {
// Native conservative grid reservations, reconstructed from ongoing fine edges.
// Logical positions and existing edges stay occupied for the whole phase.
class MovementReservations {
    Occupancy logical_;
    Point dimensions_,footprint_;
    std::vector<std::optional<Handle>> claims_;
    std::map<std::uint32_t,std::pair<Handle,Point>> origins_;
    std::size_t index(Point) const;
public:
    MovementReservations(const State&,Point dimensions,std::uint32_t creatureType,Point footprint);
    std::optional<Handle> owner(Point) const;
    bool tryReserve(Handle,Point from,Point to); // Atomic refusal; no partial claims.
};
}
