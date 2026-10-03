#pragma once
#include "cell-validity-native-reference.hpp"
namespace native_reference {
inline std::vector<unsigned char> install_clearance_map(const CellValidityMapView& v,int boundary=0) {
    const auto context=validity_context(v,boundary);
    std::memcpy(reinterpret_cast<void*>(0x6c5490),context.data(),context.size());
    return context;
}
inline bool run_clearance(Coordinates pos,const CreatureMovementParameters& p,const CellValidityMapView& v,bool tall) {
    // Empty scans are also tested with absent host tables. Native setup supplies
    // inert tables but a null cell base, which must never be dereferenced.
    auto native_view=v;
    std::vector<int> rows(v.map.row_count,0),layers(v.map.layer_offset_count,0);
    if(!native_view.map.row_offsets) native_view.map.row_offsets=rows.data();
    if(!native_view.map.layer_offsets) native_view.map.layer_offsets=layers.data();
    const auto context=install_clearance_map(native_view);
    using Fn=int (__attribute__((thiscall)) *)(void*,int,int,int,const CreatureMovementParameters*);
    // Incoming receiver is not dereferenced; global map supplies both predicates.
    const auto result=reinterpret_cast<Fn>(tall?0x4f45b0:0x4f44e0)(reinterpret_cast<void*>(1),pos.x,pos.y,pos.z,&p)!=0;
    assert(std::memcmp(reinterpret_cast<void*>(0x6c5490),context.data(),context.size())==0);
    return result;
}
inline bool run_complete_movement(Coordinates from,Coordinates to,const CreatureMovementParameters& p,
    int& category,int override_record,const CellValidityMapView& receiver_map,
    const CellValidityMapView& global_map,int boundary) {
    const auto global_context=install_clearance_map(global_map,boundary);
    const auto original=validity_context(receiver_map,boundary); auto context=original;
    using Fn=int (__attribute__((thiscall)) *)(void*,int,int,int,int,int,int,
        const CreatureMovementParameters*,int*,int);
    const auto result=reinterpret_cast<Fn>(0x4f3990)(context.data(),from.x,from.y,from.z,
        to.x,to.y,to.z,&p,&category,override_record)!=0;
    assert(context==original);
    assert(std::memcmp(reinterpret_cast<void*>(0x6c5490),global_context.data(),global_context.size())==0);
    return result;
}

}
