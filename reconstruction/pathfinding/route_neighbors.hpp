#pragma once
#include "route_search.hpp"

namespace mnm::reconstruction {

// Packed engine descriptor. Unknown fields remain byte-exact for helper replay.
#pragma pack(push, 1)
struct DescriptorCoordinates {
    std::int32_t x = 0, y = 0, z = 0;
    DescriptorCoordinates() = default;
    DescriptorCoordinates(std::int32_t px, std::int32_t py, std::int32_t pz) : x(px), y(py), z(pz) {}
    DescriptorCoordinates(Coordinates p) : x(p.x), y(p.y), z(p.z) {}
    operator Coordinates() const { return {x,y,z}; }
};
struct NeighborDescriptor {
    std::int32_t mode = 0;               // +00: standard acceptance path
    std::uint32_t object = 0;            // +04: engine token, never dereferenced
    std::uint32_t unknown_08 = 0;
    DescriptorCoordinates start{};      // +0c
    std::array<std::byte, 22> unknown_18{};
    DescriptorCoordinates target{};     // +2e
    std::int32_t linked = 0;             // +3a: selects six-link branch
    std::int32_t radius_limit = -1;      // +3e
    std::int32_t float_limit = -1;       // +42
};
#pragma pack(pop)
static_assert(sizeof(NeighborDescriptor) == 0x46);
static_assert(offsetof(NeighborDescriptor, target) == 0x2e);

// Typed view of the existing 28-byte MovementPayload, accessed through memcpy.
struct NeighborMovement {
    std::int32_t category = 5, direction = 0, vertical_delta = 0;
    std::uint32_t scalar = 0;
    float accumulated = 0;
    std::int32_t accumulated_cost = 0, unknown_18 = 0;
};
static_assert(sizeof(NeighborMovement) == 28);
NeighborMovement neighbor_movement(const MovementPayload& payload);
MovementPayload movement_payload(const NeighborMovement& movement);

struct NeighborCell { std::uint16_t first_word = 0, links = 0; std::uint8_t flags = 0; };
struct NeighborRecord {
    std::int32_t cost_factor = 0; // record +589
    bool flag_penalty = false;   // type record +5ad (VA 6a652d)
    std::uint32_t engine_token = 0; // setup record passed as 0x4f3990 source override
};
struct NeighborHelpers {
    MapDimensions dimensions;
    std::function<Coordinates(NodeId)> coordinates;
    std::function<NodeId(Coordinates)> node_at;
    std::function<NeighborCell(NodeId)> cell;
    // Resolve type record or 0x4f4330 output once per standard expansion.
    std::function<NeighborRecord(const NeighborDescriptor&, Coordinates)> record;
    std::function<std::int32_t(std::uint32_t)> object_d03;
    // 0x514360 / 0x4f3990: nonzero accepts, category is an out-parameter.
    std::function<bool(const NeighborDescriptor&, Coordinates, Coordinates,
        const NeighborRecord&, bool, std::int32_t&)> accept;
    // 0x5205b0: scalar is in/out; last argument from 0x4eb030 is always 4.
    std::function<void(const NeighborDescriptor&, Coordinates, Coordinates,
        std::int32_t category, std::uint32_t& scalar, std::int32_t argument)> scalar;
};

// Generator only. The budgeted entry models 0x4ebae0; use the unbudgeted form
// inside SearchWorld::expand, because route_search already charges one unit.
std::vector<Candidate> generate_neighbors(NodeId current, const MovementPayload& prior,
    const NeighborDescriptor& descriptor, const NeighborHelpers& helpers);
void expand_neighbors(NodeId current, const MovementPayload& prior,
    const NeighborDescriptor& descriptor, const NeighborHelpers& helpers,
    std::int32_t& remaining_budget, std::vector<Candidate>& output);

// Descriptor initialized by 0x54b800's creature search path. Object token is
// supplied by the harness; the host address is not an engine pointer.
NeighborDescriptor creature_descriptor(const ObjectPrefix& object,
    std::uint32_t object_token, std::uint32_t unknown_argument, Coordinates target);
SearchWorld neighbor_search_world(NeighborHelpers helpers, Coordinates target,
    std::uint32_t object_token);
} // namespace mnm::reconstruction
