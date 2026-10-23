#pragma once
#include "owner_memory_groups.hpp"
#include "owner_task_storage.hpp"
#include "owner_task_sources.hpp"
#include "owner_task6_features.hpp"
#include <algorithm>
#include <cstring>
#include <utility>
#include <vector>
namespace mnm::reconstruction::combat {
// Generated logical groups -> task-owned source records and feature suffixes.
// NoCD normal ordinary-owner0 domain. Planner selection and commands remain
// separate services; intermediate planner cursor/probe-map bytes are not modeled.
// M provides checked value-address storage and destroySource. Snapshot addresses
// and resolved live-actor addresses are borrowed. Their owners must keep them
// live until every task referring to them releases its sources.
template<class M> class OwnerTask6Composition {
    using U=std::uint32_t;
    M& m;
    OwnerTaskStorage<M> objects;
    OwnerTaskSources<M> sources;
    OwnerTask6Features<M> features;
    U borrowed;
    std::vector<U> mapping,activeMapping;
    void release(U p){objects.destroy(p);m.free(p);}
    template<class Borrow> U node(const OwnerMemoryGroups& groups,U id,Borrow& snapshot){
        const auto& n=groups.nodes.at(id);
        U p=n.memoryKey<0?sources.anonymous(evaluator):sources.actor(evaluator,snapshot(n.memoryKey));mapping[id]=p;
        if(id){
            if(n.memoryKey>=0)m.set32(p+9,U(n.memoryKey));
            for(auto v:{std::pair<U,U>{0xd,U(n.position.x)},{0x11,U(n.position.y)},{0x15,U(n.position.z)},
                        {0x19,U(n.region)},{0x1d,U(n.uncertaintySquared)},{0x21,U(n.flag104)}})m.set32(p+v.first,v.second);
        }
        // Anonymous key/region/flag bytes of the top container are not written
        // by413330. Preserve allocator provenance, rather than inventing values.
        for(U child:n.children)sources.attach(p,node(groups,child,snapshot));
        return p;
    }
public:
    U evaluator=0,active=0,serial=0;
    OwnerTask6Composition(M& memory,U friendly):m(memory),objects(memory),sources(memory),features(memory),borrowed(friendly){
        if(m.get32(friendly+5))throw std::invalid_argument("task6 source already bound");
        evaluator=objects.task6();if(!evaluator)throw std::runtime_error("task6 factory refused");
    }
    OwnerTask6Composition(const OwnerTask6Composition&)=delete;
    OwnerTask6Composition& operator=(const OwnerTask6Composition&)=delete;
    template<class Borrow> void prepare(const OwnerMemoryGroups& groups,Borrow snapshot){
        validateOwnerMemoryGroups(groups,{32,32,8});if(groups.nodes.empty())throw std::invalid_argument("task6 missing group container");
        mapping.assign(groups.nodes.size(),0);
        // 431be0/413330 append a fresh clone. The enclosing sweep calls
        // finish() on every remaining evaluator after any selected installation.
        m.set32(evaluator+0x14a,node(groups,0,snapshot));
    }
    void candidate(U root,int risk,int duration,const std::vector<int>& values){
        if(root==0||root>=mapping.size()||risk<0||duration<=0)throw std::invalid_argument("task6 generated candidate");
        m.set32(evaluator+0x153,mapping[root]);m.set32(evaluator+0x157,U(risk));m.set32(evaluator+0x15b,U(duration));features.candidate(evaluator,values);
    }
    void save(){features.save(evaluator);}
    void install(U tick,float value){
        U duration=m.get32(evaluator+0x177);if(!duration||duration>0x7fffffffu||tick>0x7fffffffu-duration)throw std::invalid_argument("task6 generated install");
        if(active){m.set32(borrowed+5,0);release(active);}
        active=evaluator;activeMapping=mapping;U bits;std::memcpy(&bits,&value,4);
        for(auto v:{std::pair<U,U>{8,borrowed},{0xc,tick},{0x10,tick+duration},{0x23,~U(0)},
                    {0x1b,bits},{0x1f,serial++},{0x27,0},{0x2b,0},{0x2f,0},{0x33,0}})m.set32(active+v.first,v.second);
        m.set8(active+0x14,0);m.set8(active+0x15,0);m.set8(active+0x1a,0);m.set32(borrowed+5,active);
        U cursor=sources.beginTraversal(borrowed);if(!sources.nextActor(borrowed,cursor))throw std::invalid_argument("task6 friendly actor missing");
        evaluator=objects.task6();if(!evaluator)throw std::runtime_error("task6 next factory refused");mapping.clear();
    }
    // 423590 ->40a340: release evaluator sources, retaining vector capacity,
    // dangling root/saved source fields and populated suffix feature buffers.
    // Those source fields are invalid until the next preparation/save.
    void finish(){
        for(U at=m.get32(evaluator+0x3b);at!=m.get32(evaluator+0x3f);at+=4)
            if(U p=m.get32(at)){sources.destroy(p);m.free(p);}
        m.set32(evaluator+0x3f,m.get32(evaluator+0x3b));mapping.clear();
    }
    template<class Live> void syncActive(const OwnerMemoryGroups& groups,U root,const OwnerKnowledge& knowledge,int tick,bool resolved,Live live){
        if(!active||groups.nodes.size()!=activeMapping.size())throw std::invalid_argument("task6 active group mapping");
        for(U i=1;i<groups.nodes.size();++i){const auto& n=groups.nodes[i];U p=activeMapping[i];m.set32(p+0xd,U(n.position.x));m.set32(p+0x11,U(n.position.y));m.set32(p+0x15,U(n.position.z));if(n.memoryKey>=0)m.set32(p+0x89,n.activity);}
        if(resolved){
            std::vector<U> stack{root};
            while(!stack.empty()){
                U i=stack.back();stack.pop_back();const auto& n=groups.nodes.at(i);
                if(n.memoryKey<0){stack.insert(stack.end(),n.children.rbegin(),n.children.rend());continue;}
                const auto& rows=knowledge.records[0];auto at=std::find_if(rows.begin(),rows.end(),[&](const auto& row){return row.memory.key==n.memoryKey;});
                // 407c10 replaces35 only for current observations. Stale/missing
                // observations change activity, retaining the previous pointer.
                // Constructor byte25 remains the ORIGINAL snapshot pointer tag.
                if(at!=rows.end()&&at->markedTick==tick)m.set32(activeMapping[i]+0x35,live(at->memory.actor.slot));
            }
        }
    }
    bool retire(U tick,bool completed=false){
        if(!active||(!completed&&tick<m.get32(active+0x10)))return false;
        if(completed)m.set8(active+0x14,1);
        m.set32(borrowed+5,0);sources.drain(borrowed,3);release(active);active=0;activeMapping.clear();return true;
    }
    // Same base path retains feature suffix backing; M/host lifetime owns the
    // later disposition. No derived cleanup policy or exception unwind inferred.
    void close(){if(active)throw std::invalid_argument("retire active task before close");if(evaluator){release(evaluator);evaluator=0;}mapping.clear();}
};
}
