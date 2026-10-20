#pragma once
#include <chrono>
#include <cstdint>

// Native display policy, independent of original simulation/update cadence.
// Keep the display phase through late turns and through early source imports.
// A completed source counts as one display slot; missed slots are skipped,
// never rendered as a catch-up burst.
inline void advanceNativeFrameDeadline(std::chrono::steady_clock::time_point& deadline,
                                       std::chrono::steady_clock::time_point now,
                                       std::uint64_t& missed) {
    using Clock = std::chrono::steady_clock;
    const auto period = std::chrono::duration_cast<Clock::duration>(std::chrono::seconds(1)) / 60;
    if (!deadline.time_since_epoch().count()) deadline = now;
    if (now >= deadline) {
        const auto skipped = (now - deadline) / period;
        missed += std::uint64_t(skipped);
        deadline += period * (skipped + 1);
    } else {
        deadline += period;
    }
}
