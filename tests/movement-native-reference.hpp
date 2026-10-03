// Included only by the optional -m32 differential test. This maps a hash-checked
// disposable PE snapshot, not a running game, and redirects lower helpers.
#pragma once
#include "route_movement.hpp"
#include <cassert>
#include <cstring>
#include <vector>
#include <fstream>
#include <sys/mman.h>
#include <iterator>

namespace native_reference {
using namespace mnm::reconstruction;
inline const MovementHelpers* helpers = nullptr;
extern "C" std::uint32_t __attribute__((thiscall)) record_stub(void*, int x, int y, int z,
    const CreatureMovementParameters* p) { return helpers->record({x,y,z}, *p); }
// Engine callers test full EAX; a C++ bool return only guarantees AL.
template<MovementCheck Kind>
int __attribute__((thiscall)) check_stub(void*, int x, int y, int z,
    const CreatureMovementParameters* p) { return helpers->check(Kind, {x,y,z}, *p) ? 1 : 0; }
inline void redirect(std::uint32_t va, std::uintptr_t target) {
    // Each replacement is in the private mapping and starts at a verified VA.
    auto* code = reinterpret_cast<unsigned char*>(va);
    const std::uint32_t relative = static_cast<std::uint32_t>(target) - va - 5;
    code[0] = 0xe9;
    std::memcpy(code + 1, &relative, 4);
}
inline void initialize(const char* path, bool preserve_validity = false, bool preserve_record = false, bool preserve_boundary = false, bool preserve_clearance = false) {
    std::ifstream input(path, std::ios::binary);
    const std::vector<unsigned char> data((std::istreambuf_iterator<char>(input)), {});
    assert(data.size() > 0x1000);
    const auto read16 = [&](std::size_t o) { std::uint16_t v; std::memcpy(&v,data.data()+o,2); return v; };
    const auto read32 = [&](std::size_t o) { std::uint32_t v; std::memcpy(&v,data.data()+o,4); return v; };
    const auto pe = read32(0x3c), opt = pe + 24;
    assert(read16(opt) == 0x10b && read32(opt + 28) == 0x400000);
    const auto size = read32(opt + 56);
    auto* image = static_cast<unsigned char*>(mmap(reinterpret_cast<void*>(0x400000), size,
        PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0));
    assert(image != MAP_FAILED);
    for (unsigned i = 0; i < read16(pe + 6); ++i) {
        const auto section = opt + read16(pe + 20) + i * 40;
        const auto rva = read32(section+12), length = read32(section+16), offset = read32(section+20);
        assert(offset + length <= data.size() && rva + length <= size);
        std::memcpy(image + rva, data.data() + offset, length);
    }
    // Hash verification is performed by test-native-movement.py before it
    // creates this snapshot. Also check the expected helper prologues locally.
    assert(*reinterpret_cast<unsigned char*>(0x4f4330) == 0x83);
    assert(*reinterpret_cast<unsigned char*>(0x4f46b0) == 0x56);
    assert(*reinterpret_cast<unsigned char*>(0x4f41a0) == 0x53);
    assert(*reinterpret_cast<unsigned char*>(0x4f44e0) == 0x51);
    assert(*reinterpret_cast<unsigned char*>(0x4f45b0) == 0x83);
    if (!preserve_record) redirect(0x4f4330, reinterpret_cast<std::uintptr_t>(&record_stub));
    if (!preserve_validity) redirect(0x4f46b0, reinterpret_cast<std::uintptr_t>(&check_stub<MovementCheck::check_46b0>));
    if (!preserve_boundary) redirect(0x4f41a0, reinterpret_cast<std::uintptr_t>(&check_stub<MovementCheck::check_41a0>));
    if (!preserve_clearance) redirect(0x4f44e0, reinterpret_cast<std::uintptr_t>(&check_stub<MovementCheck::check_44e0>));
    if (!preserve_clearance) redirect(0x4f45b0, reinterpret_cast<std::uintptr_t>(&check_stub<MovementCheck::check_45b0>));
}
inline bool run(Coordinates from, Coordinates to, const CreatureMovementParameters& p,
    int& category, int record_override, const MovementHelpers& h) {
    helpers = &h;
    *reinterpret_cast<int*>(0x6c5494) = h.dimensions.x;
    *reinterpret_cast<int*>(0x6c5498) = h.dimensions.y;
    *reinterpret_cast<int*>(0x6c5c80) = h.boundary();
    *reinterpret_cast<int*>(0x5e1780) = h.layer_count();
    using Fn = int (__attribute__((thiscall)) *)(void*, int,int,int,int,int,int,
        const CreatureMovementParameters*, int*, int);
    const auto fn = reinterpret_cast<Fn>(0x4f3990);
    return fn(reinterpret_cast<void*>(0x6c5490),from.x,from.y,from.z,to.x,to.y,to.z,
        &p,&category,record_override) != 0;
}
} // namespace native_reference
