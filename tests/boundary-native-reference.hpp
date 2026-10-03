#pragma once
#include "cell-validity-native-reference.hpp"
namespace native_reference {
inline const BoundaryHelpers* boundary_helpers=nullptr;
inline void* boundary_receiver=nullptr;
extern "C" std::uint32_t __attribute__((thiscall)) boundary_support_stub(void* receiver,int x,int y,int z,
    const CreatureMovementParameters* p) {
    assert(receiver==boundary_receiver); return boundary_helpers->support({x,y,z},*p);
}
extern "C" int __attribute__((thiscall)) boundary_valid_stub(void* receiver,int x,int y,int z,
    const CreatureMovementParameters* p) {
    assert(receiver==boundary_receiver); return boundary_helpers->valid({x,y,z},*p)?1:0;
}
inline void initialize_boundary(const char* path,bool full_chain) {
    initialize(path,full_chain,full_chain,true);
    if(!full_chain) {
        redirect(0x4f4330,reinterpret_cast<std::uintptr_t>(&boundary_support_stub));
        redirect(0x4f46b0,reinterpret_cast<std::uintptr_t>(&boundary_valid_stub));
    }
}
inline bool run_boundary(Coordinates pos,const CreatureMovementParameters& p,const BoundaryHelpers& h) {
    boundary_helpers=&h; boundary_receiver=reinterpret_cast<void*>(0x6c4bd0);
    *reinterpret_cast<int*>(0x6c4bd0+0x7f0)=h.boundary();
    using Fn=int (__attribute__((thiscall)) *)(void*,int,int,int,const CreatureMovementParameters*);
    return reinterpret_cast<Fn>(0x4f41a0)(boundary_receiver,pos.x,pos.y,pos.z,&p)!=0;
}
inline bool run_boundary_map(Coordinates pos,const CreatureMovementParameters& p,const CellValidityMapView& v,int boundary) {
    const auto original=validity_context(v,boundary); auto context=original;
    using Fn=int (__attribute__((thiscall)) *)(void*,int,int,int,const CreatureMovementParameters*);
    const auto result=reinterpret_cast<Fn>(0x4f41a0)(context.data(),pos.x,pos.y,pos.z,&p)!=0;
    assert(context==original); return result;
}
}
