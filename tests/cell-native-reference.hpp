#pragma once
#include "movement-native-reference.hpp"
namespace native_reference {
inline const CellHelpers* cells = nullptr;
extern "C" int __attribute__((thiscall)) cell_stub(void* receiver, int x, int y, int z,
    int object, const CreatureMovementParameters* p) {
    assert(receiver == reinterpret_cast<void*>(0x6c5490));
    return cells->check({x,y,z}, object, *p) ? 1 : 0;
}
inline void initialize_cell(const char* path) {
    initialize(path);
    assert(*reinterpret_cast<unsigned char*>(0x4f3550) == 0x8b);
    redirect(0x4f3550, reinterpret_cast<std::uintptr_t>(&cell_stub));
}
inline bool run_cell(Coordinates pos, int object, const CreatureMovementParameters& p,
    const CellHelpers& h) {
    cells = &h;
    *reinterpret_cast<int*>(0x6c5494) = h.dimensions.x;
    *reinterpret_cast<int*>(0x6c5498) = h.dimensions.y;
    using Fn = int (__attribute__((thiscall)) *)(void*, int,int,int,int,
        const CreatureMovementParameters*);
    return reinterpret_cast<Fn>(0x4f47d0)(reinterpret_cast<void*>(0x6c5490),
        pos.x,pos.y,pos.z,object,&p) != 0;
}
}
