#ifdef MNM_ORIGINAL_REFERENCE
#define main sprite_reference_unused_main
#include "sprite-binary-reference.cpp"
#undef main
#else
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>
#endif
#include "../reconstruction/rendering/kind8_method.hpp"

using namespace mnm::reconstruction;
static std::uint32_t* current;
static std::vector<std::uint32_t> observed;
static std::uint32_t capture(void* shape, int x, int y) {
    auto* bytes = reinterpret_cast<unsigned char*>(current);
    const auto offset = static_cast<unsigned char*>(shape) - bytes;
    observed.push_back(std::uint32_t(offset));
    observed.push_back(std::uint32_t(x)); observed.push_back(std::uint32_t(y));
    observed.push_back(bytes[0x40]); observed.push_back(bytes[0x44]);
    auto* words = static_cast<std::uint32_t*>(shape);
    for (unsigned i = 0; i < 5; ++i) observed.push_back(words[i]);
    return 0x1234;
}
#ifdef MNM_ORIGINAL_REFERENCE
static unsigned __attribute__((thiscall)) raster(void* shape, int x, int y) {
    return capture(shape, x, y);
}
static void bindRaster(std::uintptr_t address) {
    auto* entry = reinterpret_cast<unsigned char*>(address);
    entry[0] = 0xe9;
    const auto relative = std::uint32_t(reinterpret_cast<std::uintptr_t>(&raster) - address - 5);
    std::memcpy(entry + 1, &relative, 4);
}
#endif
int main(int argc, char** argv) try {
#ifdef MNM_ORIGINAL_REFERENCE
    if (argc != 2) throw std::invalid_argument("Expected pinned PE32 image");
    map_image(read(argv[1]));
    bindRaster(0x545c10); bindRaster(0x547d80);
#else
    (void)argv;
    if (argc != 1) throw std::invalid_argument("Expected no native inputs");
#endif
    constexpr std::uint32_t pairs[6][2]{{0x5c7420, 0x54ab00}, {0x5c73f0, 0x54aaa0},
        {0x5c7408, 0x546540}, {0x5c7494, 0x54ab60}, {0x5c74b0, 0x54ab60}, {0x5c74cc, 0x54ac00}};
    for (const auto& pair : pairs) for (bool present : {false, true})
    for (unsigned gate : {0u, 1u}) for (int player = -1; player <= 8; ++player)
    for (unsigned enabled : {0u, 0x80000000u}) for (unsigned seed : {0u, 0xa5u}) {
        std::uint32_t object[26], owner[58]{};
        for (unsigned i = 0; i < 26; ++i) object[i] = i * 0x01010101u + 0x123;
        object[0] = pair[0];
        auto* bytes = reinterpret_cast<unsigned char*>(object);
        bytes[0x40] = bytes[0x44] = static_cast<unsigned char>(seed);
        current = object; observed.clear();
        owner[57] = gate;
        for (unsigned i = 0; i < 8; ++i) owner[9 + i] = enabled;
        unsigned result;
#ifdef MNM_ORIGINAL_REFERENCE
        object[24] = present ? std::uint32_t(reinterpret_cast<std::uintptr_t>(owner)) : 0;
        global(0x644520, std::uint32_t(player));
        if (*reinterpret_cast<const std::uint32_t*>(pair[0] + 12) != pair[1])
            throw std::runtime_error("Pinned original vtable/method changed");
        using Method = unsigned (__attribute__((thiscall)) *)(void*, int, int);
        result = std::uint16_t(reinterpret_cast<Method>(pair[1])(object, -123, 456));
#else
        (void)owner;
        Kind8Owner nativeOwner{present, gate, {}}; nativeOwner.players.fill(enabled);
        Kind8Flags flags{bytes[0x40], bytes[0x44]};
        result = drawKind8(kind8Class(pair[0], pair[1]), flags, nativeOwner, player, -123, 456,
            [&](Kind8Class type, int x, int y) {
                bytes[0x40] = flags.at40; bytes[0x44] = flags.at44;
                return capture(bytes + type.shapeOffset, x, y);
            });
        bytes[0x40] = flags.at40; bytes[0x44] = flags.at44;
#endif
        object[24] = present ? 777 : 0; // Normalize the private owner's process pointer.
        std::cout << "{\"vtable\":" << pair[0] << ",\"owner\":" << present
                  << ",\"gate\":" << gate << ",\"player\":" << player
                  << ",\"enabled\":" << enabled << ",\"seed\":" << seed
                  << ",\"result\":" << result << ",\"calls\":[";
        for (unsigned i = 0; i < observed.size(); ++i) { if (i) std::cout << ','; std::cout << observed[i]; }
        std::cout << "],\"object\":[";
        for (unsigned i = 0; i < 26; ++i) { if (i) std::cout << ','; std::cout << object[i]; }
        std::cout << "]}\n";
    }
#ifndef MNM_ORIGINAL_REFERENCE
    unsigned refused = 0;
    for (const auto& pair : pairs) {
        try { kind8Class(pair[0], pair[1] + 1); }
        catch (const std::invalid_argument&) { ++refused; }
    }
    try { kind8Class(0, 0); } catch (const std::invalid_argument&) { ++refused; }
    if (refused != 7) throw std::runtime_error("Unknown class pair admitted");
    Kind8Flags flags{};
    try {
        drawKind8(kind8Class(0x5c7420, 0x54ab00), flags, {}, -1, 0, 0,
            [](Kind8Class, int, int) -> unsigned { throw std::runtime_error("Backend failure"); });
    } catch (const std::runtime_error&) {}
    if (flags.at44 != 1) throw std::runtime_error("Original flag/raster ordering changed");
#endif
    return 0;
} catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
