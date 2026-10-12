#include "simulation/combat.hpp"
#include "persistence/snapshot.hpp"
#include "frozen_navigation.hpp"
#include <charconv>
#include <iostream>
#include <stdexcept>
#include <string_view>

namespace {
unsigned count(const char* text) {
    const std::string_view s(text);unsigned n=0;
    const auto result=std::from_chars(s.data(),s.data()+s.size(),n);
    if(result.ec!=std::errc{} || result.ptr!=s.data()+s.size() || n>4096) throw std::invalid_argument("tick count must be 0..4096");
    return n;
}
mnm::game::CombatSession load(const char* path) {
    auto state=mnm::game::readSnapshot(path);
    auto nav=mnm::sandbox::loadFrozenNavigation(state.map,state.animation,true,true);
    if(state.systems.empty()) throw std::invalid_argument("checkpoint contains no combat subsystem");
    mnm::game::World world(0);world.restore(std::move(state));
    return {std::move(world),std::move(nav)};
}
void print(const mnm::game::CombatSession& session,const mnm::game::CombatReport& report={}) {
    using namespace mnm::game;
    constexpr const char* actions[]={"idle","approaching","windingUp","recovering","dead"};
    constexpr const char* events[]={"acquired","approach","windup","hit","missed","died"};
    std::cout<<"{\"tick\":"<<session.world().state().tick<<",\"fighters\":[";
    bool comma=false;
    for(const auto& f:session.combat().fighters) {
        if(comma) std::cout<<',';
        comma=true;const auto& e=*session.world().find(f.actor);
        std::cout<<"{\"slot\":"<<f.actor.slot<<",\"generation\":"<<f.actor.generation<<",\"owner\":"<<e.owner
                 <<",\"position\":["<<e.x<<','<<e.y<<','<<e.z<<"],\"health\":"<<f.health<<",\"maximumHealth\":"<<f.stats.maximumHealth
                 <<",\"action\":\""<<actions[static_cast<unsigned>(f.action)]<<"\",\"remaining\":"<<f.remaining<<",\"target\":";
        if(f.target)std::cout<<'['<<f.target->slot<<','<<f.target->generation<<']';else std::cout<<"null";
        std::cout<<",\"cleaned\":"<<(e.cleaned?"true":"false")<<'}';
    }
    std::cout<<"],\"events\":[";comma=false;
    for(const auto& e:report.events) {
        if(comma)std::cout<<',';
        comma=true;std::cout<<"{\"kind\":\""<<events[static_cast<unsigned>(e.kind)]<<"\",\"actor\":["<<e.actor.slot<<','<<e.actor.generation
                           <<"],\"target\":["<<e.target.slot<<','<<e.target.generation<<"],\"amount\":"<<e.amount<<'}';
    }
    std::cout<<"],\"searchExpansions\":"<<report.movement.searchExpansions<<"}\n";
}
void save(const char* path,const mnm::game::CombatSession& session) {
    const auto result=mnm::game::writeSnapshot(path,session.world().state());
    if(!result.durable)throw std::runtime_error(result.detail);
}
}
int main(int argc,char** argv) try {
    using namespace mnm::game;
    if(argc<2) throw std::invalid_argument("Usage: mnm-combat-sandbox create MAP OUTPUT | resume INPUT OUTPUT TICKS | trace INPUT TICKS | inspect INPUT");
    const std::string mode=argv[1];
    if((mode=="create" || mode=="create-fine") && argc==4) {
        auto nav=mnm::sandbox::loadFrozenNavigation(std::filesystem::absolute(argv[2]).string(),{},true,true);
        World world(2);auto state=world.state();state.map=std::filesystem::absolute(argv[2]).string();state.navigation=nav->binding();world.restore(std::move(state));
        CombatSession session(std::move(world),nav);
        Entity a;a.type=nav->creatureType();a.x=1;a.y=1;a.z=1;a.owner=0;
        Entity b=a;b.x=5;b.owner=1;
        session.spawn(a,{20,6,2,3},mode=="create-fine");session.spawn(b,{20,4,2,3},mode=="create-fine");save(argv[3],session);print(session);return 0;
    }
    // The create form deliberately has no implicit simulation ticks.
    if(mode=="resume" && argc==5) {
        auto session=load(argv[2]);const auto ticks=count(argv[4]);
        for(unsigned i=0;i<ticks;++i){const auto report=session.step();print(session,report);}
        save(argv[3],session);return 0;
    }
    if(mode=="trace" && argc==4) {
        auto session=load(argv[2]);const auto ticks=count(argv[3]);print(session);
        for(unsigned i=0;i<ticks;++i){const auto report=session.step();print(session,report);}return 0;
    }
    if(mode=="inspect" && argc==3) {print(load(argv[2]));return 0;}
    throw std::invalid_argument("Usage: mnm-combat-sandbox create MAP OUTPUT (starts at tick zero) | resume INPUT OUTPUT TICKS | trace INPUT TICKS | inspect INPUT");
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
