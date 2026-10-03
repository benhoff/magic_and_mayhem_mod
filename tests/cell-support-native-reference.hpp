#pragma once
#include "cell-validity-native-reference.hpp"
namespace native_reference {
inline std::uint32_t run_cell_support(Coordinates pos,const CreatureMovementParameters& p,
    const CellValidityMapView& v,int boundary,bool aggregate=false) {
    const auto original=validity_context(v,boundary);
    auto context=original;
    using Fn=std::uint32_t (__attribute__((thiscall)) *)(void*,int,int,int,const CreatureMovementParameters*);
    const auto result=reinterpret_cast<Fn>(aggregate?0x4f4330:0x4f3320)(context.data(),pos.x,pos.y,pos.z,&p);
    assert(context==original);
    return result;
}
}
