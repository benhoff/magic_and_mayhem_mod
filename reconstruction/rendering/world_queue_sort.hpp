#pragma once
#include <array>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

namespace mnm::reconstruction {
using PhysicalWorldRow = std::array<std::uint32_t, 9>;
inline std::int32_t worldOrderKey(std::uint32_t value) {
    return value <= 0x7fffffffu ? std::int32_t(value)
                              : -1 - std::int32_t(0xffffffffu - value);
}
// Original 4fff60 partitions complete 36-byte records by signed first DWORD.
// Equal keys have observable, unstable ordering. No object is dereferenced.
inline void sortOriginalWorldRows(std::vector<PhysicalWorldRow>& rows,
                                  std::uint32_t count) {
    if (count > 12000 || count > rows.size()) throw std::invalid_argument("World sort extent");
    if (!count) return;
    std::vector<std::pair<std::int32_t, std::int32_t>> pending;
    pending.emplace_back(0, std::int32_t(count - 1));
    while (!pending.empty()) {
        auto [lo, hi] = pending.back(); pending.pop_back();
        std::swap(rows[lo], rows[lo + (hi - lo) / 2]);
        const auto key = worldOrderKey(rows[lo][0]);
        auto left = lo, right = hi + 1;
        for (;;) {
            do { ++left; } while (left <= hi && worldOrderKey(rows[left][0]) <= key);
            do { --right; } while (right > lo && key <= worldOrderKey(rows[right][0]));
            if (left > right) break;
            std::swap(rows[left], rows[right]);
        }
        std::swap(rows[lo], rows[right]);
        if (lo + 1 < right) pending.emplace_back(lo, right - 1);
        if (left < hi) pending.emplace_back(left, hi);
    }
}
} // namespace mnm::reconstruction
