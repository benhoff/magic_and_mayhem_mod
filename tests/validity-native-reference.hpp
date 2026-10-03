#pragma once
#include "movement-native-reference.hpp"
namespace native_reference {
inline const ValidityHelpers* validity = nullptr;
inline void* expected_receiver = nullptr;
extern "C" int __attribute__((thiscall)) validity_stub(void* receiver, int x, int y, int z,
    const CreatureMovementParameters* p) {
    assert(receiver == expected_receiver);
    return validity->check({x,y,z},*p) ? 1 : 0;
}
inline void initialize_validity(const char* path) {
    initialize(path,true); // Leave 46b0 original, including movement's calls to it.
    assert(*reinterpret_cast<unsigned char*>(0x4f3440) == 0x8b);
    redirect(0x4f3440, reinterpret_cast<std::uintptr_t>(&validity_stub));
}
inline bool run_validity(Coordinates pos, const CreatureMovementParameters& p,
    const ValidityHelpers& h) {
    validity = &h;
    expected_receiver = reinterpret_cast<void*>(0x6c4bd0); // Distinct from fixed occupancy context.
    *reinterpret_cast<int*>(0x6c5494)=h.dimensions.x;
    *reinterpret_cast<int*>(0x6c5498)=h.dimensions.y;
    using Fn = int (__attribute__((thiscall)) *)(void*,int,int,int,const CreatureMovementParameters*);
    return reinterpret_cast<Fn>(0x4f46b0)(expected_receiver,pos.x,pos.y,pos.z,&p) != 0;
}
inline bool run_valid_movement(Coordinates from, Coordinates to, const CreatureMovementParameters& p,
    int& category, int record, const MovementHelpers& h, const ValidityHelpers& v) {
    validity=&v; expected_receiver=reinterpret_cast<void*>(0x6c5490);
    return run(from,to,p,category,record,h);
}
}
