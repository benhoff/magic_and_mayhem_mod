#include "sprite_loader.hpp"
#include <QFileInfo>
#include <QTemporaryDir>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>

using namespace mnm::assets;
using Bytes = std::vector<std::uint8_t>;
void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
void put(Bytes& b, std::size_t at, std::uint32_t n) {
    for (unsigned i = 0; i < 4; ++i) b.at(at + i) = static_cast<std::uint8_t>(n >> (i * 8));
}
Bytes fixture(bool direct = false) {
    const unsigned palettes = direct ? 0 : 2;
    const auto firstSize = 40 + 16 + 7 + 6 * (direct ? 2 : 1) + 3 + 8;
    const auto base = 24 + palettes * 768 + 8;
    Bytes b(base + firstSize + 40, 0);
    put(b, 0, 0x00525053); put(b, 4, b.size()); put(b, 8, 4);
    put(b, 12, 2); put(b, 16, palettes); put(b, 20, 0x12345678);
    for (unsigned p = 0; p < palettes; ++p) {
        for (unsigned i = 0; i < 256; ++i) {
            b[24 + p * 768 + i * 3] = i;
            b[24 + p * 768 + i * 3 + 1] = p + 10;
            b[24 + p * 768 + i * 3 + 2] = 255 - i;
        }
    }
    put(b, base - 8, 0); put(b, base - 4, firstSize);
    put(b, base, firstSize); put(b, base + 4, 5); put(b, base + 8, 2);
    put(b, base + 12, static_cast<std::uint32_t>(-7)); put(b, base + 16, 0x80000000U);
    const Bytes name = {'A', 0, 0xff, 'Z', 0, 0, '!', 'x'};
    std::copy(name.begin(), name.end(), b.begin() + base + 20);
    put(b, base + 28, direct ? 0xffffffffU : 1);
    put(b, base + 32, firstSize - 8); put(b, base + 36, firstSize - 4);
    put(b, base + 40, 56); put(b, base + 44, 63);
    put(b, base + 48, 59); put(b, base + 52, 63 + 3 * (direct ? 2 : 1));
    const Bytes deltas = {1, 3, 1, 0, 1, 2, 2};
    std::copy(deltas.begin(), deltas.end(), b.begin() + base + 56);
    const std::vector<std::uint16_t> words = {0, 0xf800, 0x07e0, 0x001f, 0xffff, 0x1234};
    const Bytes indices = {0, 255, 2, 1, 3, 0};
    for (unsigned i = 0; i < 6; ++i) {
        if (direct) {
            b[base + 63 + i * 2] = words[i] & 255;
            b[base + 64 + i * 2] = words[i] >> 8;
        } else b[base + 63 + i] = indices[i];
    }
    std::fill(b.begin() + base + firstSize - 8, b.begin() + base + firstSize, 0xa5);
    put(b, base + firstSize, 40);
    put(b, base + firstSize + 28, direct ? 0xffffffffU : 0);
    return b;
}
Sprite take(SpriteResult result) {
    if (const auto* e = std::get_if<SpriteError>(&result)) throw std::runtime_error(e->detail);
    return std::get<Sprite>(std::move(result));
}
SpriteError rejects(const Bytes& b, SpriteErrorCode code, const SpriteLimits& limits = {}) {
    auto result = decodeSprite(b, limits);
    const auto* error = std::get_if<SpriteError>(&result);
    require(error && error->code == code, "Wrong sprite rejection");
    return *error;
}
class Stream final : public AssetFile {
public:
    explicit Stream(Bytes b) : AssetFile("fixture.spr", "fixture.spr"), bytes(std::move(b)) {}
    Bytes bytes;
    std::size_t at = 0;
    bool failRead = false, failSeek = false;
    std::int64_t extraSize = 0;
    Result<std::int64_t> size() override { return static_cast<std::int64_t>(bytes.size()) + extraSize; }
    Result<std::int64_t> position() override { return static_cast<std::int64_t>(at); }
    Status seek(std::int64_t offset) override {
        if (failSeek) return failure(ErrorCode::ioError, "seek", "fixture seek failure");
        at = static_cast<std::size_t>(offset); return std::monostate{};
    }
    ReadResult read(void* out, std::int64_t capacity) override {
        if (failRead && at >= 6) return {0, failure(ErrorCode::ioError, "read", "fixture read failure")};
        const auto count = std::min<std::size_t>({3, static_cast<std::size_t>(capacity), bytes.size() - at});
        std::memcpy(out, bytes.data() + at, count); at += count;
        return {static_cast<std::int64_t>(count), {}};
    }
};
int main() try {
    auto bytes = fixture();
    auto sprite = take(decodeSprite(bytes));
    require(sprite.storage == SpriteStorage::indexed8 && sprite.frames.size() == 2 && sprite.palettes.size() == 2, "Indexed container");
    const auto& frame = sprite.frames[0];
    require(frame.width == 5 && frame.height == 2 && frame.originX == -7 && frame.originY == (-2147483647 - 1), "Frame metadata/signs");
    require(frame.name[1] == 0 && frame.name[2] == 255 && frame.name[7] == 'x' && frame.paletteIndex == 1, "Exact name/palette");
    require(sprite.headerFlags == 0x12345678 && frame.auxiliaryOffsets[0] == frame.encodedSize - 8, "Flags/auxiliary metadata");
    require(std::get<Bytes>(frame.pixels) == Bytes({0,0,255,2,0,1,0,0,3,0}), "Indexed runs/rows");
    require(frame.auxiliaryData[0]==Bytes(4,0xa5) && frame.auxiliaryData[1]==Bytes(4,0xa5), "Owned auxiliary plane extents");
    require(frame.opaqueMask == Bytes({0,1,1,1,0,1,0,0,1,1}), "Opaque zero versus transparency");
    require(sprite.palettes[1][255].red == 255 && sprite.palettes[1][255].green == 11, "Owned palette bytes");
    require(sprite.frames[1].empty() && sprite.frames[1].opaqueMask.empty(), "Empty frame preserved");
    std::fill(bytes.begin(), bytes.end(), 0);
    require(frame.auxiliaryData[0]==Bytes(4,0xa5) && frame.auxiliaryData[1]==Bytes(4,0xa5), "Source-independent auxiliary ownership");
    require(std::get<Bytes>(frame.pixels)[2] == 255 && sprite.palettes[1][255].red == 255, "Source-independent ownership");
    auto direct = take(decodeSprite(fixture(true)));
    require(direct.storage == SpriteStorage::rgb565 && direct.palettes.empty() && !direct.frames[0].paletteIndex, "Direct colour contract");
    require(std::get<std::vector<std::uint16_t>>(direct.frames[0].pixels) ==
        std::vector<std::uint16_t>({0,0,0xf800,0x07e0,0,0x001f,0,0,0xffff,0x1234}), "Little-endian RGB565 runs");
    require(std::holds_alternative<std::vector<std::uint16_t>>(direct.frames[1].pixels), "Empty direct type");
    auto b = fixture(); const std::size_t base = 24 + 2 * 768 + 8;
    auto changed = [&](std::size_t at, std::uint32_t value) { auto x = b; put(x, at, value); return x; };
    rejects(Bytes{}, SpriteErrorCode::malformedData);
    rejects(changed(0, 0), SpriteErrorCode::invalidFormat);
    rejects(changed(4, b.size() + 1), SpriteErrorCode::malformedData);
    rejects(changed(8, 2), SpriteErrorCode::unsupportedVersion);
    rejects(changed(16, 5), SpriteErrorCode::malformedData);
    rejects(changed(12, 4097), SpriteErrorCode::limitExceeded);
    rejects(changed(base - 8, 0xffffffffU), SpriteErrorCode::malformedData);
    rejects(changed(base, 39), SpriteErrorCode::malformedData);
    rejects(changed(base, 0xffffffffU), SpriteErrorCode::malformedData);
    const auto e = rejects(changed(base + 28, 2), SpriteErrorCode::malformedData);
    require(e.frame == 0 && e.offset == base + 28, "Frame/offset error diagnostics");
    rejects(changed(base + 28, 0xffffffffU), SpriteErrorCode::malformedData);
    auto wrongDirect = fixture(true); put(wrongDirect, 32 + 28, 0);
    rejects(wrongDirect, SpriteErrorCode::malformedData);
    rejects(changed(base + 4, 0), SpriteErrorCode::malformedData);
    rejects(changed(base + 8, 2049), SpriteErrorCode::limitExceeded);
    rejects(changed(base + 32, 55), SpriteErrorCode::malformedData);
    rejects(changed(base + 36, 0xffffffffU), SpriteErrorCode::malformedData);
    rejects(changed(base + 40, 0), SpriteErrorCode::malformedData);
    rejects(changed(base + 48, 55), SpriteErrorCode::malformedData);
    rejects(changed(base + 44, 0xffffffffU), SpriteErrorCode::malformedData);
    rejects(changed(base + 52, 64), SpriteErrorCode::malformedData); // first row needs three bytes
    auto run = b; run[base + 56] = 6; rejects(run, SpriteErrorCode::malformedData);
    run = b; run[base + 56] = 0; rejects(run, SpriteErrorCode::malformedData);
    // Table order is authoritative; duplicate table entries produce independently
    // owned frames and are charged against aggregate budgets.
    auto order = b; put(order, base - 8, frame.encodedSize); put(order, base - 4, 0);
    require(take(decodeSprite(order)).frames[0].empty(), "Frame table order");
    auto alias = b; put(alias, base - 4, 0);
    auto aliased = take(decodeSprite(alias));
    std::get<Bytes>(aliased.frames[0].pixels)[2] = 9;
    require(std::get<Bytes>(aliased.frames[1].pixels)[2] == 255, "Aliased encoded frames have owned output");
    SpriteLimits limits;
    limits.inputBytes = b.size() - 1; rejects(b, SpriteErrorCode::limitExceeded, limits);
    limits = {}; limits.frames = 1; rejects(b, SpriteErrorCode::limitExceeded, limits);
    limits = {}; limits.width = 4; rejects(b, SpriteErrorCode::limitExceeded, limits);
    limits = {}; limits.pixels = 9; rejects(b, SpriteErrorCode::limitExceeded, limits);
    limits = {}; limits.decodedBytes = 19; rejects(b, SpriteErrorCode::limitExceeded, limits);
    limits = {}; limits.scannedBytes = frame.encodedSize + 39; rejects(b, SpriteErrorCode::limitExceeded, limits);
    limits = {}; limits.scannedBytes = frame.encodedSize; rejects(alias, SpriteErrorCode::limitExceeded, limits);
    Stream shortReads(b); shortReads.at = 10;
    require(take(loadSprite(shortReads)).frames[0].width == 5 && shortReads.at == b.size(), "AssetFile seek/short reads");
    Stream fault(b); fault.failRead = true;
    auto failed = loadSprite(fault);
    const auto& inputError = std::get<SpriteError>(failed);
    require(inputError.code == SpriteErrorCode::assetInput && inputError.input && inputError.input->operation == "read" &&
            inputError.input->requestedPath == "fixture.spr", "Structured input errors preserved");
    Stream seekFault(b); seekFault.failSeek = true;
    require(std::get<SpriteError>(loadSprite(seekFault)).input->operation == "seek", "Seek error retained");
    Stream truncated(b); truncated.extraSize = 6;
    require(std::get<SpriteError>(loadSprite(truncated)).input->code == ErrorCode::unexpectedEof, "Premature EOF retained");
    Stream limited(b); limited.at = 7; limits = {}; limits.inputBytes = 2;
    require(std::get<SpriteError>(loadSprite(limited, limits)).code == SpriteErrorCode::limitExceeded && limited.at == 7, "Read limit before seek");
    limits = {}; limits.inputBytes = ~std::uint64_t(0);
    require(std::get<SpriteError>(loadSprite(limited, limits)).code == SpriteErrorCode::invalidArgument, "Signed read limit checked");
    auto overlap = b;
    put(overlap, base - 4, frame.encodedSize - 4);
    put(overlap, base + frame.encodedSize - 4, 40);
    rejects(overlap, SpriteErrorCode::malformedData);
    Sprite owned;
    {
        QTemporaryDir temp;
        require(temp.isValid(), "Fixture directory");
        const auto root = QFileInfo(temp.path()).filesystemCanonicalFilePath();
        const auto path = root / "Example.spr";
        std::ofstream out(path, std::ios::binary);
        out.write(reinterpret_cast<const char*>(b.data()), b.size()); out.close();
        auto store = std::get<AssetStore>(AssetStore::create(root, {"C:/Game"}));
        auto opened = store.open("c:\\gAmE\\eXaMpLe.SpR");
        auto handle = std::get<std::unique_ptr<AssetFile>>(std::move(opened));
        owned = take(loadSprite(*handle));
        handle.reset();
    }
    require(std::get<Bytes>(owned.frames[0].pixels)[2] == 255 && owned.frames[0].opaqueMask[1] == 1 &&
            owned.palettes[1][255].green == 11, "Output survives file/store/installation destruction");
    std::cout << "SPR ownership, indexed/direct runs, malformed ranges and budgets passed\n";
    return 0;
} catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
