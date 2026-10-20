#pragma once
#include "world_dispatch.hpp"
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <vector>

namespace mnm::reconstruction {
struct WorldQueueRow {
    QueueRow draw;
    std::uint32_t objectHeight{}, objectOriginY{};
    std::uint16_t first{0x8ad0}, prior{0x8ad0};
};
struct WorldQueueState {
    bool video{}, tall{}, postlude{};
    // Original clip helper arguments are top,left,right,bottom, in that order.
    std::array<std::uint32_t, 4> clip{};
    std::uint32_t pitch{};
    std::array<std::uint32_t, 3> pending{};
};
inline std::int32_t queueSigned(std::uint32_t value) {
    std::int32_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}
// Backend owns clip/draw/postlude effects. Row state and pending counters are
// owned values. No legacy pointers or implicit GPU/CPU coherence are assumed.
template<class Backend>
void consumeWorldQueue(std::vector<WorldQueueRow>& rows, std::size_t count,
        WorldQueueState& state, Backend& backend) {
    if (count > rows.size()) throw std::invalid_argument("World queue count exceeds owned rows");
    constexpr std::uint32_t unset = 0x88ca6c00;
    auto setClip = [&](std::uint32_t top, std::uint32_t left, std::uint32_t bottom) {
        backend.clip(std::array<std::uint32_t, 4>{top, left,
            state.clip[2] + state.pitch * 2u, bottom});
    };
    for (std::size_t i = 0; i < count; ++i) {
        auto& item = rows[i];
        auto& draw = item.draw;
        std::uint32_t firstBase = unset, priorBase = unset;
        auto opening = [&](std::uint32_t end, bool clamp) {
            if (queueSigned(end) < queueSigned(state.clip[3]) &&
                    queueSigned(end) > queueSigned(state.clip[0]))
                setClip(state.clip[0], state.clip[1] + state.pitch,
                    clamp ? std::uint32_t(std::max(queueSigned(end), 0)) : end);
        };
        if (state.video) {
            if (item.first != 0x8ad0) {
                firstBase = draw.y + item.objectHeight - item.objectOriginY;
                opening(firstBase + std::uint32_t(std::int32_t(std::int16_t(item.first))), true);
            }
            if (item.prior != 0x8ad0) {
                priorBase = draw.y + item.objectHeight - item.objectOriginY;
                draw.y += item.objectHeight - std::uint32_t(std::int32_t(std::int16_t(item.prior)));
                opening(priorBase, false);
            }
        }
        auto dispatch = [&](QueueTable table) {
            if (const auto result = dispatchQueueRow(table, draw,
                    [&](const QueueCall& call) { return backend.draw(table, call); }))
                item.prior = *result;
        };
        dispatch(state.video ? QueueTable::primary : QueueTable::software);
        if (state.video) {
            if (firstBase != unset && item.first != 0x8ad0) {
                const auto end = firstBase + std::uint32_t(std::int32_t(std::int16_t(item.first)));
                if (queueSigned(end) < (state.tall ? 600 : 480) &&
                        queueSigned(end) < queueSigned(state.clip[3])) {
                    setClip(std::uint32_t(std::max(queueSigned(end), 0)), state.clip[1], state.clip[3]);
                    dispatch(QueueTable::auxiliary);
                }
            }
            if (firstBase != unset || priorBase != unset)
                setClip(state.clip[0], state.clip[1], state.clip[3]);
        }
    }
    if (state.postlude) {
        backend.postlude(state.pending);
        state.pending.fill(0);
    }
}
}
