#pragma once
#include "owner_task_storage.hpp"
#include "owner_task_sources.hpp"
#include <cstring>
#include <utility>
namespace mnm::reconstruction::combat {
// Ownership-only projection of a preselected kind6 evaluator. Caller supplies
// the cached source, duration and winner value. No planner/World policy inferred.
template<class M> class OwnerTask6Owner {
    using U=std::uint32_t;
    M& m;
    OwnerTaskStorage<M> objects;
    OwnerTaskSources<M> sources;
    U borrowed;
    bool prepared=false;
    void release(U p){objects.destroy(p);m.free(p);}
public:
    U evaluator=0,active=0,serial=0;
    OwnerTask6Owner(M& memory,U source):m(memory),objects(memory),sources(memory),borrowed(source){
        if(m.get32(source+5))throw std::invalid_argument("task6 source bound");
        evaluator=objects.task6();if(!evaluator)throw std::runtime_error("task6 initial allocation");
    }
    OwnerTask6Owner(const OwnerTask6Owner&)=delete;
    OwnerTask6Owner& operator=(const OwnerTask6Owner&)=delete;
    U prepareCached(U root,U duration){
        if(!evaluator||!duration||duration>0x7fffffffu)throw std::invalid_argument("task6 cached winner");
        U copy=sources.clone(evaluator,root);m.set32(evaluator+0x16f,copy);m.set32(evaluator+0x177,duration);prepared=true;return copy;
    }
    void install(U tick,float value){
        if(active||!evaluator||!prepared||tick>0x7fffffffu-m.get32(evaluator+0x177))throw std::invalid_argument("task6 install domain");
        active=evaluator;prepared=false;U bits;std::memcpy(&bits,&value,4);
        for(auto v:{std::pair<U,U>{8,borrowed},{0xc,tick},{0x10,tick+m.get32(active+0x177)},
                    {0x23,~U(0)},{0x1b,bits},{0x1f,serial++},{0x27,0},{0x2b,0},{0x2f,0},{0x33,0}})m.set32(active+v.first,v.second);
        m.set8(active+0x14,0);m.set8(active+0x15,0);m.set8(active+0x1a,0);m.set32(borrowed+5,active);
        // 4289e0 leaves its first traversal occupied after resolving the owner.
        U cursor=sources.beginTraversal(borrowed);
        if(!sources.nextActor(borrowed,cursor))throw std::invalid_argument("task6 source actor absent");
        evaluator=objects.task6();if(!evaluator)throw std::runtime_error("task6 fresh evaluator allocation");
    }
    bool retire(U tick,bool completed=false){
        if(!active||(!completed&&tick<m.get32(active+0x10)))return false;
        if(completed)m.set8(active+0x14,1);
        m.set32(borrowed+5,0);sources.drain(borrowed,3);release(active);active=0;return true;
    }
    void close(){if(active){m.set32(borrowed+5,0);release(active);active=0;}if(evaluator){release(evaluator);evaluator=0;}}
};
}
