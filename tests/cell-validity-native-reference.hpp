#pragma once
#include "movement-native-reference.hpp"
namespace native_reference {
inline std::vector<unsigned char> validity_context(const CellValidityMapView& v,int boundary=0) {
    const auto& m=v.map;
    std::vector<unsigned char> context(0x64b2+m.row_count*4,0);
    assert(0x6432+m.layer_offset_count*4<=0x64b2);
    const auto write=[&](std::size_t offset,std::uint32_t value) { std::memcpy(context.data()+offset,&value,4); };
    write(4,m.dimensions.x); write(8,m.dimensions.y); write(0x10,m.plane_stride);
    write(0x4c,reinterpret_cast<std::uintptr_t>(m.cells)); write(0x7f0,boundary);
    std::memcpy(context.data()+0x6432,m.layer_offsets,m.layer_offset_count*4);
    std::memcpy(context.data()+0x64b2,m.row_offsets,m.row_count*4);
    *reinterpret_cast<int*>(0x6c5494)=m.dimensions.x;
    *reinterpret_cast<int*>(0x6c5498)=m.dimensions.y;
    *reinterpret_cast<int*>(0x5e1780)=m.layer_count;
    *reinterpret_cast<std::uintptr_t*>(0x65660c)=reinterpret_cast<std::uintptr_t>(v.terrain);
    return context;
}
inline bool run_cell_rules(Coordinates pos,const CreatureMovementParameters& p,
    const CellValidityMapView& v,bool footprint=false) {
    const auto context=validity_context(v);
    auto receiver=context; // caller's context is distinct from fixed occupancy map
    using Fn=int (__attribute__((thiscall)) *)(void*,int,int,int,const CreatureMovementParameters*);
    const auto result=reinterpret_cast<Fn>(footprint?0x4f46b0:0x4f3440)(receiver.data(),pos.x,pos.y,pos.z,&p)!=0;
    assert(receiver==context);
    return result;
}
inline bool run_cell_rules_movement(Coordinates from,Coordinates to,const CreatureMovementParameters& p,
    int& category,int record,const MovementHelpers& h,const CellValidityMapView& v) {
    const auto context=validity_context(v,h.boundary());
    auto* receiver=reinterpret_cast<void*>(0x6c5490);
    std::memcpy(receiver,context.data(),context.size());
    const auto result=run(from,to,p,category,record,h);
    assert(std::memcmp(receiver,context.data(),context.size())==0);
    return result;
}
}
