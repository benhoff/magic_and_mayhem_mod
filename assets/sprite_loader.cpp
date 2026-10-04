#include "sprite_loader.hpp"
#include <algorithm>
#include <cstring>
#include <limits>
#include <new>
#include <stdexcept>
#include <utility>

namespace mnm::assets {
namespace {
struct Failure { SpriteError error; };
class Reader {
public:
    explicit Reader(const std::vector<std::uint8_t>& bytes) : bytes_(bytes) {}
    std::optional<std::uint32_t> frame;
    [[noreturn]] void fail(SpriteErrorCode code, std::size_t at, const std::string& detail) const {
        throw Failure{{code, at, frame, detail, {}}};
    }
    void extent(std::uint64_t at, std::uint64_t length, std::uint64_t end) const {
        if (at > end || length > end - at || end > bytes_.size())
            fail(SpriteErrorCode::malformedData, static_cast<std::size_t>(std::min<std::uint64_t>(at, bytes_.size())),
                 "Byte extent exceeds containing data");
    }
    std::uint32_t u32(std::size_t at) const {
        extent(at, 4, bytes_.size());
        return std::uint32_t(bytes_[at]) | std::uint32_t(bytes_[at + 1]) << 8 |
            std::uint32_t(bytes_[at + 2]) << 16 | std::uint32_t(bytes_[at + 3]) << 24;
    }
    std::int32_t i32(std::size_t at) const {
        const auto value = u32(at);
        const auto signedValue = value <= 0x7fffffffU ? std::int64_t(value) : std::int64_t(value) - 0x100000000LL;
        return static_cast<std::int32_t>(signedValue);
    }
    const std::vector<std::uint8_t>& bytes() const { return bytes_; }
private:
    const std::vector<std::uint8_t>& bytes_;
};
void budget(Reader& reader, std::uint64_t& used, std::uint64_t amount, std::uint64_t limit, std::size_t at) {
    if (used > limit || amount > limit - used)
        reader.fail(SpriteErrorCode::limitExceeded, at, "Sprite exceeds configured aggregate budget");
    used += amount;
}
Sprite parse(Reader& r, const SpriteLimits& limits) {
    const auto& bytes = r.bytes();
    if (bytes.size() > limits.inputBytes) r.fail(SpriteErrorCode::limitExceeded, 0, "SPR exceeds input byte limit");
    r.extent(0, 24, bytes.size());
    if (r.u32(0) != 0x00525053) r.fail(SpriteErrorCode::invalidFormat, 0, "Expected SPR signature");
    if (r.u32(4) != bytes.size()) r.fail(SpriteErrorCode::malformedData, 4, "Declared SPR size differs from input");
    const auto version = r.u32(8);
    if (version != 4) r.fail(SpriteErrorCode::unsupportedVersion, 8, "Only SPR version 4 is supported");
    const auto count = r.u32(12), palettes = r.u32(16);
    if (count > limits.frames) r.fail(SpriteErrorCode::limitExceeded, 12, "SPR frame count exceeds limit");
    if (palettes > 4) r.fail(SpriteErrorCode::malformedData, 16, "SPR exceeds four embedded palettes");
    const std::uint64_t table = 24 + std::uint64_t(palettes) * 768;
    const auto base = table + std::uint64_t(count) * 4;
    r.extent(24, base - 24, bytes.size());
    Sprite sprite;
    sprite.storage = palettes ? SpriteStorage::indexed8 : SpriteStorage::rgb565;
    sprite.version = version;
    sprite.headerFlags = r.u32(20);
    sprite.sourceBytes = bytes.size();
    sprite.palettes.resize(palettes);
    for (std::uint32_t p = 0; p < palettes; ++p) {
        for (std::size_t c = 0; c < 256; ++c) {
            const auto at = 24 + p * 768 + c * 3;
            sprite.palettes[p][c] = {bytes[at], bytes[at + 1], bytes[at + 2]};
        }
    }
    // Validate table-selected frame extents before decoding or output allocation.
    // Identical aliases are permitted; partial overlaps are rejected.
    std::vector<std::pair<std::size_t, std::size_t>> spans;
    spans.reserve(count);
    for (std::uint32_t i = 0; i < count; ++i) {
        r.frame = i;
        const auto at = base + r.u32(static_cast<std::size_t>(table + i * 4));
        r.extent(at, 40, bytes.size());
        const auto size = r.u32(static_cast<std::size_t>(at));
        if (size < 40) r.fail(SpriteErrorCode::malformedData, static_cast<std::size_t>(at), "Frame is shorter than its header");
        r.extent(at, size, bytes.size());
        spans.emplace_back(static_cast<std::size_t>(at), static_cast<std::size_t>(at + size));
    }
    auto sorted = spans;
    std::sort(sorted.begin(), sorted.end());
    for (std::size_t i = 1; i < sorted.size(); ++i) {
        if (sorted[i].first < sorted[i - 1].second && sorted[i] != sorted[i - 1]) {
            r.frame.reset();
            r.fail(SpriteErrorCode::malformedData, sorted[i].first, "Partially overlapping frames");
        }
    }
    sprite.frames.reserve(count);
    std::uint64_t pixelBudget = 0, byteBudget = 0, scanBudget = 0;
    for (std::uint32_t i = 0; i < count; ++i) {
        r.frame = i;
        const auto start = spans[i].first, end = spans[i].second;
        SpriteFrame f;
        f.sourceOffset = static_cast<std::uint32_t>(start);
        f.encodedSize = static_cast<std::uint32_t>(end - start);
        budget(r, scanBudget, f.encodedSize, limits.scannedBytes, start);
        f.width = r.u32(start + 4); f.height = r.u32(start + 8);
        f.originX = r.i32(start + 12); f.originY = r.i32(start + 16);
        std::copy_n(bytes.begin() + static_cast<std::ptrdiff_t>(start + 20), 8, f.name.begin());
        const auto rawPalette = r.u32(start + 28);
        if (palettes) {
            if (rawPalette >= palettes) r.fail(SpriteErrorCode::malformedData, start + 28, "Palette index outside embedded palettes");
            f.paletteIndex = rawPalette;
        } else {
            if (rawPalette != 0xffffffffU) r.fail(SpriteErrorCode::malformedData, start + 28, "Direct-colour frame must have -1 palette member");
            f.pixels = std::vector<std::uint16_t>{};
        }
        f.auxiliaryOffsets = {r.u32(start + 32), r.u32(start + 36)};
        if (f.width > limits.width || f.height > limits.height)
            r.fail(SpriteErrorCode::limitExceeded, start + 4, "Frame dimensions exceed limits");
        if ((f.width == 0) != (f.height == 0))
            r.fail(SpriteErrorCode::malformedData, start + 4, "Mixed zero/nonzero dimensions are unsupported");
        const auto rowTableEnd = 40 + std::uint64_t(f.height) * 8;
        r.extent(start + 40, rowTableEnd - 40, end);
        for (const auto offset : f.auxiliaryOffsets) {
            if (offset && (offset < rowTableEnd || offset > f.encodedSize))
                r.fail(SpriteErrorCode::malformedData, start + 32, "Auxiliary offset outside trailing frame data");
        }
        for (unsigned plane=0;plane<2;++plane) {
            const auto offset=f.auxiliaryOffsets[plane];if(!offset)continue;
            auto stop=f.encodedSize;
            for(const auto other:f.auxiliaryOffsets)if(other>offset)stop=std::min(stop,other);
            budget(r, byteBudget, stop-offset, limits.decodedBytes, start+offset);
            f.auxiliaryData[plane].assign(bytes.begin()+start+offset,bytes.begin()+start+stop);
        }
        if (f.empty()) { sprite.frames.push_back(std::move(f)); continue; }
        const auto total = std::uint64_t(f.width) * f.height;
        const auto bytesPerPixel = palettes ? 1U : 2U;
        if (total > std::numeric_limits<std::size_t>::max() ||
            total > std::numeric_limits<std::uint64_t>::max() / (bytesPerPixel + 1))
            r.fail(SpriteErrorCode::limitExceeded, start + 4, "Decoded extent exceeds host allocation range");
        budget(r, pixelBudget, total, limits.pixels, start + 4);
        budget(r, byteBudget, total * (bytesPerPixel + 1), limits.decodedBytes, start + 4);
        const auto firstPixel = r.u32(start + 44);
        const auto pixelEnd = f.auxiliaryOffsets[0] ? f.auxiliaryOffsets[0] :
            f.auxiliaryOffsets[1] ? f.auxiliaryOffsets[1] : f.encodedSize;
        if (firstPixel < rowTableEnd || firstPixel > pixelEnd)
            r.fail(SpriteErrorCode::malformedData, start + 44, "Pixel plane overlaps headers or trailing data");
        f.opaqueMask.resize(static_cast<std::size_t>(total), 0);
        if (palettes) std::get<std::vector<std::uint8_t>>(f.pixels).resize(static_cast<std::size_t>(total), 0);
        else std::get<std::vector<std::uint16_t>>(f.pixels).resize(static_cast<std::size_t>(total), 0);
        for (std::uint32_t y = 0; y < f.height; ++y) {
            const auto row = start + 40 + std::size_t(y) * 8;
            const auto delta = r.u32(row), pixel = r.u32(row + 4);
            const auto deltaEnd = y + 1 < f.height ? r.u32(row + 8) : firstPixel;
            const auto rowPixelEnd = y + 1 < f.height ? r.u32(row + 12) : pixelEnd;
            if (delta < rowTableEnd || delta > deltaEnd || deltaEnd > firstPixel ||
                pixel < firstPixel || pixel > rowPixelEnd || rowPixelEnd > pixelEnd)
                r.fail(SpriteErrorCode::malformedData, row, "Invalid row delta/pixel ranges");
            std::uint64_t x = 0, cursor = start + pixel;
            bool colour = false;
            for (std::uint64_t at = start + delta; at < start + deltaEnd; ++at) {
                const auto run = bytes[static_cast<std::size_t>(at)];
                if (run > f.width - x) r.fail(SpriteErrorCode::malformedData, static_cast<std::size_t>(at), "Run exceeds row width");
                if (colour) {
                    r.extent(cursor, std::uint64_t(run) * bytesPerPixel, start + rowPixelEnd);
                    for (std::uint32_t k = 0; k < run; ++k) {
                        const auto slot = static_cast<std::size_t>(std::uint64_t(y) * f.width + x + k);
                        f.opaqueMask[slot] = 1;
                        if (palettes) std::get<std::vector<std::uint8_t>>(f.pixels)[slot] = bytes[static_cast<std::size_t>(cursor++)];
                        else {
                            auto& pixels = std::get<std::vector<std::uint16_t>>(f.pixels);
                            pixels[slot] = std::uint16_t(bytes[static_cast<std::size_t>(cursor)]) |
                                std::uint16_t(bytes[static_cast<std::size_t>(cursor + 1)]) << 8;
                            cursor += 2;
                        }
                    }
                }
                x += run;
                colour = !colour;
            }
            if (x != f.width) r.fail(SpriteErrorCode::malformedData, row, "Runs do not cover row width");
            // Padding between stored pixel rows/planes is allowed. Some installed
            // frames align the last row before auxiliary data.
        }
        sprite.frames.push_back(std::move(f));
    }
    return sprite;
}
}
SpriteResult decodeSprite(const std::vector<std::uint8_t>& bytes, const SpriteLimits& limits) {
    try { Reader reader(bytes); return parse(reader, limits); }
    catch (const Failure& failure) { return failure.error; }
    catch (const std::bad_alloc&) { return SpriteError{SpriteErrorCode::limitExceeded, 0, {}, "Sprite allocation failed", {}}; }
    catch (const std::length_error&) { return SpriteError{SpriteErrorCode::limitExceeded, 0, {}, "Sprite vector size is unsupported", {}}; }
}
SpriteResult loadSprite(AssetFile& input, const SpriteLimits& limits) {
    if (limits.inputBytes > std::uint64_t(std::numeric_limits<std::int64_t>::max()))
        return SpriteError{SpriteErrorCode::invalidArgument, 0, {}, "Input limit exceeds signed AssetFile range", {}};
    auto read = readWhole(input, static_cast<std::int64_t>(limits.inputBytes));
    if (const auto* error = std::get_if<Error>(&read))
        return SpriteError{error->code == ErrorCode::limitExceeded ? SpriteErrorCode::limitExceeded : SpriteErrorCode::assetInput,
                           0, {}, error->detail, *error};
    return decodeSprite(std::get<std::vector<std::uint8_t>>(read), limits);
}
}
