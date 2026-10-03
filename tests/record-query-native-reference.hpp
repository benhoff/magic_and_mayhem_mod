#pragma once
#include "movement-native-reference.hpp"
namespace native_reference {
inline const RecordQueryHelpers* support=nullptr;
inline void* support_receiver=nullptr;
extern "C" std::uint32_t __attribute__((thiscall)) support_stub(void* receiver,int x,int y,int z,
    const CreatureMovementParameters* p) {
    assert(receiver==support_receiver);
    return support->query({x,y,z},*p);
}
inline void initialize_record_query(const char* path) {
    initialize(path,false,true); // Preserve 4330; remaining movement checks stay fixtures.
    assert(*reinterpret_cast<unsigned char*>(0x4f3320)==0x8b);
    redirect(0x4f3320,reinterpret_cast<std::uintptr_t>(&support_stub));
}
inline std::uint32_t run_record_query(Coordinates pos,const CreatureMovementParameters& p,
    const RecordQueryHelpers& h) {
    support=&h; support_receiver=reinterpret_cast<void*>(0x6c4bd0);
    *reinterpret_cast<int*>(0x6c5494)=h.dimensions.x;
    *reinterpret_cast<int*>(0x6c5498)=h.dimensions.y;
    *reinterpret_cast<int*>(0x6c4bd0+0x7f0)=h.boundary?h.boundary():0;
    using Fn=std::uint32_t (__attribute__((thiscall)) *)(void*,int,int,int,const CreatureMovementParameters*);
    return reinterpret_cast<Fn>(0x4f4330)(support_receiver,pos.x,pos.y,pos.z,&p);
}
inline bool run_support_movement(Coordinates from,Coordinates to,const CreatureMovementParameters& p,
    int& category,int record,const MovementHelpers& h,const RecordQueryHelpers& q) {
    support=&q; support_receiver=reinterpret_cast<void*>(0x6c5490);
    return run(from,to,p,category,record,h);
}
}
