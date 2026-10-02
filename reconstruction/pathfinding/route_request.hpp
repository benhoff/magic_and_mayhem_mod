// Readable host-side model, NOT a live hook or a recovered original header.
// Build: no-CD SHA-256 40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168.
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace mnm::reconstruction {

// Known accessed offsets only. Unknown regions deliberately remain opaque.
// These packed prefixes describe byte layout, not a complete engine class.
#pragma pack(push, 1)
struct RouteSnapshot {
    std::int32_t target_x;                    // +0x00: coordinate labels inferred
    std::int32_t target_y;                    // +0x04
    std::int32_t target_z;                    // +0x08
    std::uint32_t unknown_0c;                 // +0x0c
    std::uint32_t waypoint_count;             // +0x10: checked for nonzero
    std::array<std::byte, 0x1f8> unknown_14;   // +0x14..+0x20b
};

struct ObjectPrefix {
    std::array<std::byte, 0x96b> unknown_000;
    RouteSnapshot route;                      // +0x96b, copied 0x20c bytes
    std::array<std::byte, 0x14> unknown_b77;
    std::uint32_t route_present;              // +0xb8b: assigned 1, possibly 0
    std::array<std::byte, 0x174> unknown_b8f;
    std::uint32_t unknown_d03;                // +0xd03: reset to 0
};

struct RouteContextPrefix {
    RouteSnapshot route;                      // global preferred VA 0x00690148
    std::uint8_t unknown_route_flag;           // +0x20c aliases VA 0x00690354
    std::array<std::byte, 0x5c> unknown_20d;   // search touches through +0x268
};
#pragma pack(pop)

static_assert(sizeof(RouteSnapshot) == 0x20c);
static_assert(offsetof(RouteSnapshot, waypoint_count) == 0x10);
static_assert(offsetof(ObjectPrefix, route) == 0x96b);
static_assert(offsetof(ObjectPrefix, route_present) == 0xb8b);
static_assert(offsetof(ObjectPrefix, unknown_d03) == 0xd03);
static_assert(sizeof(ObjectPrefix) == 0xd07);
static_assert(offsetof(RouteContextPrefix, unknown_route_flag) == 0x20c);
static_assert(offsetof(RouteContextPrefix, unknown_20d) == 0x20d);
static_assert(sizeof(RouteContextPrefix) == 0x269);

struct EngineState {
    std::int32_t x_dimension;                 // VA 0x006c5494
    std::int32_t y_dimension;                 // VA 0x006c5498
    std::int32_t node_budget;                 // VA 0x005e174c
    RouteContextPrefix scratch{};
};

// Test seam for unresolved 0x0054b800. This is a host ABI, NOT the x86 ABI.
using SearchFunction = void (*)(RouteContextPrefix&, ObjectPrefix&,
    std::uint32_t unknown_argument, std::int32_t x, std::int32_t y,
    std::int32_t z, std::int32_t& remaining_budget, void* user_data);

struct SearchBackend {
    SearchFunction run;
    void* user_data = nullptr;
};

// 0x0040e290 / 0x0040e8a0: require a positive signed map dimension.
std::int32_t normalize_coordinate(std::int32_t value, std::int32_t dimension);
// 0x0040eeb0: a one-DWORD copy, with no normalization.
std::int32_t copy_z(std::int32_t value);
// 0x00512800: reconstruction of observed side effects with injected search.
bool route_request(ObjectPrefix& object, EngineState& engine,
    std::int32_t x, std::int32_t y, std::int32_t z,
    std::uint32_t unknown_argument, SearchBackend search);

} // namespace mnm::reconstruction
