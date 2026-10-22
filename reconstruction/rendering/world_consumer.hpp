#pragma once
#include "world_dispatch.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

namespace mnm::reconstruction {
// No-CD 40209ca7: owned queue envelope around dispatchQueueRow. Backends must
// leave the queue, source facts and configuration unchanged during consumption.
// Original helper bodies and allocation/object lifetime remain separate.
struct ConsumerClip { std::int32_t left{}, top{}, right{}, bottom{}; };
struct ConsumerRow {
    QueueRow draw;
    std::uint32_t sourceHeight{}, sourceOriginY{};
    std::uint16_t auxiliaryMarker{0x8ad0}, metadata{0x8ad0};
};
struct ConsumerState {
    bool hardware{}, tallVideo{}, finalize{};
    std::uint32_t padding{};
    ConsumerClip base{}, active{};
    std::array<std::uint32_t, 3> finalArguments{};
};

template<class Backend, class Clip, class Finalize>
void consumeWorldRows(std::vector<ConsumerRow>& rows, ConsumerState& state,
                      Backend&& backend, Clip&& clip, Finalize&& finalize) {
    constexpr std::uint32_t unset = 0x88ca6c00u;
    const auto signedWord = [](std::uint16_t value) {
        return std::int32_t(value < 0x8000 ? value : std::int32_t(value) - 0x10000);
    };
    // Bit-preserving signed conversion, including x86 wrap at coordinate edges.
    const auto signedDword = [](std::uint32_t value) {
        return value <= 0x7fffffff ? std::int32_t(value) :
            -1 - std::int32_t(0xffffffffu - value);
    };
    auto setClip = [&](std::uint32_t left, std::uint32_t top,
                       std::uint32_t right, std::uint32_t bottom) {
        state.active = {signedDword(left), signedDword(top),
                        signedDword(right), signedDword(bottom)};
        clip(state.active);
    };
    const auto right = std::uint32_t(state.base.right) + 2 * state.padding;
    auto draw = [&](ConsumerRow& row, QueueTable table) {
        const auto result = dispatchQueueRow(table, row.draw, backend);
        if (result) row.metadata = *result;
    };
    for (auto& row : rows) {
        if (!state.hardware) {
            draw(row, QueueTable::software);
            continue;
        }
        std::uint32_t lower = unset, upper = unset;
        if (row.auxiliaryMarker != 0x8ad0) {
            lower = row.draw.y + row.sourceHeight - row.sourceOriginY;
            const auto edge = signedDword(lower + std::uint32_t(signedWord(row.auxiliaryMarker)));
            if (edge < state.base.bottom && edge > state.base.top)
                setClip(std::uint32_t(state.base.left) + state.padding,
                        state.base.top, right, std::uint32_t(std::max(edge, 0)));
        }
        if (row.metadata != 0x8ad0) {
            upper = row.draw.y + row.sourceHeight - row.sourceOriginY;
            row.draw.y += row.sourceHeight - std::uint32_t(signedWord(row.metadata));
            const auto edge = signedDword(upper);
            if (edge < state.base.bottom && edge > state.base.top)
                setClip(std::uint32_t(state.base.left) + state.padding,
                        state.base.top, right, upper);
        }
        draw(row, QueueTable::primary);
        if (lower != unset && row.auxiliaryMarker != 0x8ad0) {
            const auto edge = signedDword(lower + std::uint32_t(signedWord(row.auxiliaryMarker)));
            if (edge < (state.tallVideo ? 600 : 480) && edge < state.base.bottom) {
                setClip(state.base.left, std::uint32_t(std::max(edge, 0)), right,
                        state.base.bottom);
                draw(row, QueueTable::auxiliary);
            }
        }
        if (lower != unset || upper != unset)
            setClip(state.base.left, state.base.top, right, state.base.bottom);
    }
    if (state.finalize) {
        finalize(state.finalArguments);
        state.finalArguments = {};
    }
}
}
