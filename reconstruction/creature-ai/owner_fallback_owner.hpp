#pragma once
#include "owner_task_storage.hpp"
#include "owner_task_scheduler.hpp"
#include "owner_fallback_task.hpp"
#include <cstring>

namespace mnm::reconstruction::combat {
// Single ordinary direct-root count-mode projection, using the recovered
// selector, replacement gate and caller. Owner0, zero baseline/counters, fixed
// source availability and maximum selection define this API domain.
// Perception/source/snapshot/training
// providers and original World cadence remain outside this composition.
template<class M> class OwnerFallbackOwner {
    using U=std::uint32_t;
    M& m;
    OwnerTaskStorage<M> objects;
    U source;
    static U bits(float f) { U u; std::memcpy(&u,&f,4);return u; }
    void retire() {
        if (!active) return;
        m.set32(source+5,0);
        objects.destroy(active); m.free(active); active=0;
        previous.reset(); scheduler.executionComplete=false; scheduler.pendingExecution=0;
    }
    void cleanup() {
        if(previous && retireOwnerTask(*previous,scheduler.executionComplete,tick,{})) retire();
    }
    void plan(int& budget) {
        if(selection.reset) { selection.value=-2;selection.selected=-1;selection.ties=0; }
        OwnerTaskSelectionServices p;
        p.count=1;p.root=true;
        p.prepare=[&](unsigned){
            m.set32(evaluator+8,source);
            m.set32(evaluator+0x27,0);
            prepareOwnerFallback(candidate,false);
            m.set8(evaluator+0x149,candidate.eligible);
            m.set8(evaluator+0x14a,candidate.emitted);
        };
        p.next=[&](unsigned,int&){
            auto value=nextOwnerFallback(candidate);
            m.set8(evaluator+0x14a,candidate.emitted);
            return value;
        };
        p.save=[](unsigned){};p.finish=[]{};
        advanceOwnerTaskSelection(selection,budget,random,p);
        if(selection.reset && selection.selected>=0 &&
           replaceOwnerTask(selection.value,previous ? &*previous : nullptr,tick,{})) {
            retire();active=evaluator;evaluator=0;
            m.set32(source+5,active);
            previous.emplace();previous->started=tick;previous->expires=tick+8;
            previous->value=selection.value;
            for(auto item:{std::pair<U,U>{8,source},{0xc,U(tick)},{0x10,U(tick+8)},
                    {0x1b,bits(selection.value)},{0x1f,nextSerial++},{0x27,0},
                    {0x2b,0},{0x2f,0},{0x33,0}})m.set32(active+item.first,item.second);
            m.set8(active+0x14,0);m.set8(active+0x15,0);m.set8(active+0x1a,0);
            evaluator=objects.fallback();
            if(!evaluator)throw std::runtime_error("fallback evaluator allocation");
        }
    }
public:
    U active=0,evaluator=0,nextSerial=0;
    int tick=0;
    OwnerTaskScheduler scheduler;
    OwnerTaskSelection selection;
    OwnerFallbackCandidate candidate;
    RandomState random;
    std::optional<OwnerPreviousTask> previous;
    OwnerFallbackOwner(M& storage,U borrowedSource,unsigned seed)
        :m(storage),objects(storage),source(borrowedSource),random(seedRandom(seed)) {
        if(m.get32(source+5))throw std::invalid_argument("fallback source already bound");
        evaluator=objects.fallback();
        if(!evaluator)throw std::runtime_error("fallback initial allocation");
    }
    OwnerFallbackOwner(const OwnerFallbackOwner&)=delete;
    OwnerFallbackOwner& operator=(const OwnerFallbackOwner&)=delete;
    // Explicit close allows the backend to verify destruction before its own
    // storage is retired. No destructor hides a potentially throwing provider.
    void close() { retire();if(evaluator){objects.destroy(evaluator);m.free(evaluator);evaluator=0;} }
    OwnerTaskSchedulerResult advance(int clock,int total,bool disabled=false) {
        if(!evaluator)throw std::invalid_argument("fallback owner closed");
        if(clock<tick || clock>2147483639)throw std::invalid_argument("fallback clock");
        tick=clock;
        OwnerTaskSchedulerServices p;
        p.cleanup=[&]{cleanup();};p.plan=[&](int& b){plan(b);};
        p.active=[&]{return OwnerScheduledTask{active?nextSerial:0,previous?previous->expires:0,true};};
        p.execute=[](int&){return executeOwnerFallback();};
        return advanceOwnerTaskScheduler(scheduler,tick,total,p,disabled);
    }
    void invalidate() { if(previous){previous->completed=true;m.set8(active+0x15,1);} }
    void expireAt(int clock) { tick=clock;cleanup(); }
};
}
