#pragma once
#include "world.hpp"
#include <map>

namespace mnm::game {
// Owned native policy: bounded logical-cell boxes resolved per actor. No legacy IDs/pointers.
using FootprintResolver = std::function<Point(const Entity&)>;
class Occupancy {
    Point dimensions_;
    std::vector<std::optional<Handle>> cells_;
    std::size_t index(Point) const;
public:
    Occupancy(const State&,Point dimensions,std::uint32_t creatureType,Point footprint);
    Occupancy(const State&,Point dimensions,const FootprintResolver&);
    std::optional<Handle> owner(Point) const;
    bool blocked(Point,Handle self) const;
};
}

namespace mnm::game {
// Native conservative grid reservations, reconstructed from ongoing fine edges.
// Logical positions and existing edges stay occupied for the whole phase.
class MovementReservations {
    Occupancy logical_;
    Point dimensions_;
    std::map<std::uint32_t,Point> footprints_;
    std::vector<std::optional<Handle>> claims_;
    std::map<std::uint32_t,std::pair<Handle,Point>> origins_;
    std::size_t index(Point) const;
public:
    MovementReservations(const State&,Point dimensions,std::uint32_t creatureType,Point footprint);
    MovementReservations(const State&,Point dimensions,const FootprintResolver&);
    std::optional<Handle> owner(Point) const;
    bool tryReserve(Handle,Point from,Point to); // Atomic refusal; no partial claims.
};
}
