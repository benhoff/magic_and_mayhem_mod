// Isolated execution of unmodified, hash-pinned PE32 sprite drawing routines.
// The Python runner verifies input hashes. No game process or file is patched.
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
#include <sys/mman.h>

using Bytes = std::vector<unsigned char>;
static Bytes read(const char* path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream) throw std::runtime_error("input open failed");
    return Bytes(std::istreambuf_iterator<char>(stream), {});
}
static std::uint32_t u32(const Bytes& b, std::size_t at) {
    if (at + 4 > b.size()) throw std::runtime_error("input extent");
    std::uint32_t n; std::memcpy(&n, b.data() + at, 4); return n;
}
static void global(std::uintptr_t address, std::uint32_t value) {
    *reinterpret_cast<std::uint32_t*>(address) = value;
}
static void save(const std::string& path, const std::vector<std::uint16_t>& pixels) {
    std::ofstream stream(path, std::ios::binary);
    stream.write(reinterpret_cast<const char*>(pixels.data()), pixels.size() * 2);
    if (!stream) throw std::runtime_error("output write failed");
}
static void map_image(const Bytes& b) {
    const auto pe = u32(b, 0x3c), opt = pe + 24;
    if (u32(b, opt + 28) != 0x400000) throw std::runtime_error("image base");
    const auto length = u32(b, opt + 56);
    auto* memory = static_cast<unsigned char*>(mmap(reinterpret_cast<void*>(0x400000), length,
        PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0));
    if (memory == MAP_FAILED) throw std::runtime_error("PE map failed");
    const auto count = static_cast<unsigned>(b.at(pe + 6) | (b.at(pe + 7) << 8));
    const auto opt_length = static_cast<unsigned>(b.at(pe + 20) | (b.at(pe + 21) << 8));
    for (unsigned i = 0; i < count; ++i) {
        const auto section = opt + opt_length + i * 40;
        const auto size = u32(b, section + 16), offset = u32(b, section + 20), rva = u32(b, section + 12);
        if (offset + size > b.size() || rva + size > length) throw std::runtime_error("PE section");
        std::memcpy(memory + rva, b.data() + offset, size);
    }
    if (*reinterpret_cast<unsigned char*>(0x597086) != 0x55 ||
        *reinterpret_cast<unsigned char*>(0x57d560) != 0x8b ||
        *reinterpret_cast<unsigned char*>(0x57e1b0) != 0x83)
        throw std::runtime_error("reference entry bytes");
}
int main(int argc, char** argv) try {
    if (argc != 5) throw std::runtime_error("expected PE, SPR, index, output prefix");
    map_image(read(argv[1]));
    auto sprite = read(argv[2]);
    if (u32(sprite, 8) != 4) throw std::runtime_error("reference harness requires SPR version 4");
    const auto count = u32(sprite, 12), pals = u32(sprite, 16);
    const auto index = static_cast<unsigned>(std::stoul(argv[3]));
    if (index >= count || pals > 4) throw std::runtime_error("frame/palette count");
    const auto table = 24 + pals * 768, base = table + count * 4;
    const auto offset = base + u32(sprite, table + index * 4);
    const auto size = u32(sprite, offset), w = u32(sprite, offset + 4), h = u32(sprite, offset + 8);
    if (offset + size > sprite.size() || w > 2048 || h > 2048) throw std::runtime_error("frame extent/budget");
    auto* frame = sprite.data() + offset;
    const auto stride = w + 2;
    std::vector<std::uint16_t> pixels(stride * (h + 2), 0x1234);
    global(0x658174, reinterpret_cast<std::uintptr_t>(pixels.data()));
    global(0x6a2dc8, stride);
    global(0x6e0008, 0); global(0x6cbb6c, 0);
    global(0x6a49b8, stride); global(0x656618, h + 2);
    global(0x6e1f68, 0); global(0x6e1f88, 0);
    const auto cx = static_cast<std::int32_t>(u32(sprite, offset + 12));
    const auto cy = static_cast<std::int32_t>(u32(sprite, offset + 16));
    std::vector<std::uint32_t> palette(3 + 128, 0);
    if (pals) {
        const auto pi = u32(sprite, offset + 28);
        if (pi >= pals) throw std::runtime_error("indexed palette outside file");
        auto* colours = reinterpret_cast<std::uint16_t*>(palette.data() + 3);
        for (unsigned i = 0; i < 256; ++i) {
            const auto at = 24 + pi * 768 + i * 3;
            colours[i] = ((sprite.at(at) >> 3) << 11) | ((sprite.at(at + 1) >> 2) << 5) | (sprite.at(at + 2) >> 3);
        }
        // The real loader replaces the raw palette index with a palette-chain
        // pointer. This fixture supplies one unshaded RGB565 table directly;
        // it does not test original palette construction or lighting.
        const auto pointer = reinterpret_cast<std::uintptr_t>(palette.data());
        std::memcpy(frame + 28, &pointer, 4);
    }
    auto draw = [&] {
        std::fill(pixels.begin(), pixels.end(), 0x1234);
        if (pals) {
            using Draw = unsigned (__attribute__((fastcall)) *)(void*, int, int, int, int);
            reinterpret_cast<Draw>(0x57e1b0)(frame, cx + 1, cy + 1, 0, 0);
        } else {
            using Draw = unsigned (*)(void*, int, int);
            reinterpret_cast<Draw>(0x597086)(frame, cx + 1, cy + 1);
        }
    };
    draw(); save(std::string(argv[4]) + ".565", pixels);
    if (!pals) {
        std::uint32_t object[9] = {};
        object[0] = reinterpret_cast<std::uintptr_t>(sprite.data());
        object[2] = reinterpret_cast<std::uintptr_t>(sprite.data() + table);
        object[3] = reinterpret_cast<std::uintptr_t>(sprite.data() + base);
        using Convert = void (__attribute__((thiscall)) *)(void*);
        reinterpret_cast<Convert>(0x57d560)(object);
        draw(); save(std::string(argv[4]) + ".555", pixels);
    }
    std::cout << "original draw completed\n";
    return 0;
} catch (const std::exception& exc) {
    std::cerr << exc.what() << '\n'; return 1;
}
