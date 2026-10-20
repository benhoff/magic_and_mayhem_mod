#pragma once
#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>

namespace mnm::reconstruction {
// No-CD 0x5965f7: mov edx,[edi+eax*2]; mov [edi],dx.
// Models a selected, already resolved logical source WORD. The caller supplies
// the contiguous readable extent; this does not infer a live allocation/alias.
struct DistortionWordRead {
    std::size_t byteOffset;
    bool crossesLogicalEnd;
    std::optional<std::uint32_t> operand;
    std::optional<std::size_t> firstUnreadable;
    std::optional<std::uint16_t> storedWord() const {
        if (!operand) return {};
        return static_cast<std::uint16_t>(*operand);
    }
};

inline DistortionWordRead distortionWordRead(const std::uint8_t* bytes,
        std::size_t readableBytes, std::size_t logicalBytes, std::size_t sourceWord) {
    if (!bytes || !logicalBytes || logicalBytes % 2 || logicalBytes > readableBytes ||
            sourceWord >= logicalBytes / 2)
        throw std::invalid_argument("Distortion source must be a mapped logical WORD");
    // Subtraction avoids overflow even for a caller-supplied SIZE_MAX extent.
    const auto offset = sourceWord * 2;
    DistortionWordRead result{offset, logicalBytes - offset < 4, {}, {}};
    if (readableBytes - offset < 4) {
        result.firstUnreadable = readableBytes;
        return result; // No partial read or destination store on a fault.
    }
    result.operand = std::uint32_t(bytes[offset]) |
        std::uint32_t(bytes[offset + 1]) << 8 |
        std::uint32_t(bytes[offset + 2]) << 16 |
        std::uint32_t(bytes[offset + 3]) << 24;
    return result;
}
}
