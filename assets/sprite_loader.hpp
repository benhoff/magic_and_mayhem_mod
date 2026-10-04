#pragma once
#include "asset_file.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace mnm::assets {
enum class SpriteStorage { indexed8, rgb565 };
struct SpriteRgb { std::uint8_t red = 0, green = 0, blue = 0; };
using SpritePalette = std::array<SpriteRgb, 256>;
using SpritePixels = std::variant<std::vector<std::uint8_t>, std::vector<std::uint16_t>>;
struct SpriteFrame {
    std::uint32_t width = 0, height = 0;
    std::int32_t originX = 0, originY = 0;
    // Preserve all eight bytes, including padding/non-text bytes.
    std::array<std::uint8_t, 8> name{};
    // Version-2 serialized palette word, preserved only as opaque data.
    std::optional<std::uint32_t> legacyPaletteWord;
    std::optional<std::uint32_t> paletteIndex; // absent for direct RGB565
    std::uint32_t sourceOffset = 0, encodedSize = 0;
    // Original trailing-plane offsets: preserved, not interpreted as effects.
    std::array<std::uint32_t, 2> auxiliaryOffsets{};
    // Owned opaque trailing planes; interpretation belongs to consumers.
    std::array<std::vector<std::uint8_t>,2> auxiliaryData;
    SpritePixels pixels;
    // Top-down, width*height entries, 0 transparent / 1 opaque. Transparent
    // pixel slots contain zero; opaque index/word zero remains distinguishable.
    std::vector<std::uint8_t> opaqueMask;
    bool empty() const { return width == 0 && height == 0; }
};
struct Sprite {
    SpriteStorage storage = SpriteStorage::indexed8;
    std::uint32_t version = 4, headerFlags = 0;
    std::uint64_t sourceBytes = 0;
    std::vector<SpritePalette> palettes;
    std::vector<SpriteFrame> frames;
};
struct SpriteLimits {
    std::uint64_t inputBytes = 32ULL * 1024 * 1024;
    std::uint32_t frames = 4096, width = 2048, height = 2048;
    std::uint64_t pixels = 16ULL * 1024 * 1024;
    std::uint64_t decodedBytes = 48ULL * 1024 * 1024;
    // Sum of frame extents processed, including table aliases; bounds work
    // even when an input repeats a large compressed frame many times.
    std::uint64_t scannedBytes = 32ULL * 1024 * 1024;
};
enum class SpriteErrorCode {
    invalidArgument, invalidFormat, unsupportedVersion, malformedData,
    limitExceeded, assetInput
};
struct SpriteError {
    SpriteErrorCode code;
    std::size_t offset = 0;
    std::optional<std::uint32_t> frame;
    std::string detail;
    std::optional<Error> input; // original structured AssetFile error, unchanged
};
using SpriteResult = std::variant<Sprite, SpriteError>;
// Complete owned output or structured error. No borrowed source/file/palette
// pointers survive. Version-4 SPR and single-palette version-2 SPR; SFT/ANI are separate.
SpriteResult decodeSprite(const std::vector<std::uint8_t>& bytes, const SpriteLimits& limits = {});
SpriteResult loadSprite(AssetFile& input, const SpriteLimits& limits = {});
}
