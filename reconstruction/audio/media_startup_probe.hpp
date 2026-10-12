#pragma once
#include <cstdint>

namespace mnm::reconstruction::audio {
// Recovered return gates only. The separate original instruction fixture
// observes COM call/resource order; this model never constructs a graph.
constexpr bool mediaStartupAvailable(std::int32_t initialization,
                                     std::int32_t creation) noexcept {
    return initialization >= 0 && creation >= 0;
}
}
