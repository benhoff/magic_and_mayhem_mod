#pragma once
#include "movement-native-reference.hpp"
namespace native_reference {
inline bool run_occupancy(Coordinates pos, int object, const CreatureMovementParameters& p,
    const OccupancyMapView& map, bool footprint = false) {
    // Both 0x4f3550 and 0x4f47d0 remain original machine code in this test.
    std::vector<unsigned char> context(0x64b2 + map.row_count*4, 0);
    assert(0x6432 + map.layer_offset_count*4 <= 0x64b2);
    const auto write = [&](std::size_t offset, std::uint32_t value) {
        std::memcpy(context.data()+offset, &value, 4);
    };
    write(4, map.dimensions.x); write(8, map.dimensions.y);
    write(0x10, map.plane_stride);
    write(0x4c, reinterpret_cast<std::uintptr_t>(map.cells));
    std::memcpy(context.data()+0x6432, map.layer_offsets, map.layer_offset_count*4);
    std::memcpy(context.data()+0x64b2, map.row_offsets, map.row_count*4);
    *reinterpret_cast<int*>(0x5e1780) = map.layer_count;
    using Fn = int (__attribute__((thiscall)) *)(void*, int,int,int,int,
        const CreatureMovementParameters*);
    void* receiver = context.data();
    if (footprint) {
        receiver = reinterpret_cast<void*>(0x6c5490);
        std::memcpy(receiver, context.data(), context.size());
    }
    const auto original_context = context;
    const auto result = reinterpret_cast<Fn>(footprint ? 0x4f47d0 : 0x4f3550)(
        receiver, pos.x,pos.y,pos.z,object,&p) != 0;
    assert(std::memcmp(receiver, original_context.data(), context.size()) == 0);
    return result;
}
}
