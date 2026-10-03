// Host reconstruction of no-CD search control flow; not an injected x86 ABI.
#pragma once
#include "route_request.hpp"

#include <functional>
#include <limits>
#include <map>
#include <vector>

namespace mnm::reconstruction {

using NodeId = std::uint32_t; // engine pointer token, never a host pointer
struct Coordinates { std::int32_t x, y, z; };
struct MapDimensions { std::int32_t x, y, z; };

// Payload copied from candidate +8 to node record +8. Only category is needed
// for waypoint emission here; movement-dependent fields stay opaque.
struct MovementPayload {
    std::int32_t category = 5;
    std::array<std::byte, 24> unknown_04{};
};
struct Candidate {
    std::int32_t edge_cost;
    NodeId node;
    MovementPayload movement;
};
struct NodeRecord {
    std::int32_t priority = std::numeric_limits<std::int32_t>::max();
    NodeId predecessor = 0;
    MovementPayload movement{};
};
struct Waypoint {
    std::int32_t x, y, z, direction, vertical_delta;
    std::uint32_t category_is_1_to_3;
    std::int32_t category;
};
static_assert(sizeof(MovementPayload) == 28);
static_assert(sizeof(Candidate) == 36);
static_assert(offsetof(Candidate, movement) == 8);
static_assert(sizeof(NodeRecord) == 36);
static_assert(offsetof(NodeRecord, movement) == 8);
static_assert(sizeof(Waypoint) == 28);

// The map, creature collision checks, movement costs and movement payloads
// must be supplied explicitly. A toy grid is not the recovered game world.
struct SearchWorld {
    MapDimensions dimensions;
    std::function<NodeId(Coordinates)> node_at;
    std::function<Coordinates(NodeId)> coordinates;
    std::function<std::vector<Candidate>(NodeId, const MovementPayload&,
        const ObjectPrefix&, std::uint32_t unknown_argument)> expand;
};

// Container representation is host-side. Signed priority order, duplicate
// entries and FIFO ordering among equal priorities follow the inspected tree.
struct SearchState {
    std::multimap<std::int32_t, NodeId> open;
    std::map<NodeId, NodeRecord> records;
    NodeId best_node = 0;
    std::int32_t best_heuristic = 0;
    bool initialized = false;
};
enum class SearchStop { queue_empty, target_bound, budget_exhausted };
struct SearchResult {
    SearchStop stop;
    std::size_t expansions;
    std::vector<NodeId> path; // start through closest discovered node, inclusive
};

// 0x004eac10 and 0x004ec780, including x86 DWORD wrapping arithmetic.
std::int32_t distance_metric(std::int32_t dx, std::int32_t dy, std::int32_t dz);
std::int32_t wrapped_difference(std::int32_t a, std::int32_t b, std::int32_t dimension);
std::int32_t route_heuristic(Coordinates a, Coordinates b, MapDimensions dimensions);

// 26 candidate offsets in the exact 0x005e178c table order. This enumerates
// wrapped cells only; it makes no claim that they are legal creature moves.
std::vector<Coordinates> adjacent_cells(Coordinates from, MapDimensions dimensions);

SearchResult route_search(RouteContextPrefix& context, const ObjectPrefix& object,
    std::uint32_t unknown_argument, Coordinates target, std::int32_t& remaining_budget,
    SearchState& state, const SearchWorld& world);

// Connect the search to the existing route-request reconstruction.
struct ReconstructedSearch { SearchState state; SearchWorld world; };
SearchBackend search_backend(ReconstructedSearch& search);

} // namespace mnm::reconstruction
