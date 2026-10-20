#pragma once
#include "world_dispatch.hpp"
#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace mnm::reconstruction {
struct WorldTrail {
    std::int32_t x1{}, y1{}, x2{}, y2{};
    std::uint32_t source{};
    std::int32_t shade{}, ordinal{};
};
struct WorldTrailStamp {
    std::uint32_t source;
    std::int32_t x, y, shade, ordinal;
};
// Kind9 overloads the queue's nominal x field with its frame token. The four
// pointed DWORDs are coordinates on this branch, never a vtable/receiver.
inline WorldTrail worldTrailForQueueCall(const QueueCall& call) {
    if (call.backend != QueueBackend::objectHelper || call.count != 7)
        throw std::invalid_argument("Expected a kind9 trail call");
    auto signedWord = [](std::uint32_t word) {
        return word <= std::uint32_t(INT32_MAX) ? std::int32_t(word) :
            -1 - std::int32_t(~word);
    };
    const auto& v = call.values;
    return {signedWord(v[0]), signedWord(v[1]), signedWord(v[2]),
        signedWord(v[3]), v[4], signedWord(v[5]), signedWord(v[6])};
}
// No-CD4df840 places a580a90 sprite at each Bresenham point, including
// both endpoints, with a strict > half-step tie. Source is an owned token.
// Native admission caps work before any effect; signed-overflow legacy inputs
// and source/raster ownership are outside this recovered contract.
template<class Backend>
std::uint32_t drawWorldTrail(const WorldTrail& trail, std::uint32_t maxPoints,
        Backend&& backend) {
    const auto deltaX = std::int64_t(trail.x2) - trail.x1;
    const auto deltaY = std::int64_t(trail.y2) - trail.y1;
    const auto dx = deltaX < 0 ? -deltaX : deltaX;
    const auto dy = deltaY < 0 ? -deltaY : deltaY;
    if (std::max(dx, dy) + 1 > maxPoints ||
            std::max(dx, dy) > std::numeric_limits<std::int32_t>::max() / 2)
        throw std::invalid_argument("World trail exceeds bounded work/arithmetic domain");
    const auto stepX = deltaX < 0 ? -1 : 1, stepY = deltaY < 0 ? -1 : 1;
    auto x = trail.x1, y = trail.y1;
    std::int64_t error = 0;
    auto stamp = [&] {return std::uint32_t(backend(WorldTrailStamp{
        trail.source, x, y, trail.shade, trail.ordinal}));};
    if (!dx) {
        while (y != trail.y2) { stamp(); y += stepY; }
    } else if (!dy) {
        while (x != trail.x2) { stamp(); x += stepX; }
    } else if (dy < dx) {
        while (x != trail.x2) {
            stamp(); error += dy * 2; x += stepX;
            if (error > dx) { y += stepY; error -= dx * 2; }
        }
    } else {
        while (y != trail.y2) {
            stamp(); error += dx * 2; y += stepY;
            if (error > dy) { x += stepX; error -= dy * 2; }
        }
    }
    return stamp(); // Preserve the last raster's entire EAX value.
}
}
