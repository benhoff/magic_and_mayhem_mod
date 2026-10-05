#include "persistence/snapshot.hpp"
#include "frozen_navigation.hpp"
#include <charconv>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string_view>

using namespace mnm::game;
static World fixture() {
    World world(4);
    Entity creature;creature.type=17;creature.owner=2;creature.x=-7;creature.y=12;creature.state={0,255,9};
    auto h=world.spawn(creature);
    Entity effect;effect.family=Family::missileEffect;effect.type=8;effect.target=h;effect.state={1,2,3};world.spawn(effect);
    auto state=world.state();state.sequence=std::numeric_limits<std::uint32_t>::max();state.tick=42;
    state.phase20=19;state.phase90=89;state.expansionBudget=7;state.map="fixture/map";
    state.campaign={4,0,5};state.systems={6,7};world.restore(std::move(state));
    world.enqueue({Operation::cleanup,h,{}});return world;
}
static std::int32_t integer(const char* argument) {
    std::string_view text(argument);std::int32_t value=0;
    auto result=std::from_chars(text.data(),text.data()+text.size(),value);
    if(result.ec!=std::errc{} || result.ptr!=text.data()+text.size()) throw std::invalid_argument("invalid integer argument");
    return value;
}
static std::uint32_t ticks(const char* argument) {
    auto count=integer(argument);if(count<0 || count>100000) throw std::invalid_argument("tick count must be 0..100000");return count;
}
static void json(const State& state) {
    std::cout<<"{\"tick\":"<<state.tick<<",\"sequence\":"<<state.sequence<<",\"pending\":"<<state.pending.size()<<",\"entities\":[";
    bool first=true;
    for(std::uint32_t i=0;i<state.slots.size();++i) if(state.slots[i].entity) {
        const auto& e=*state.slots[i].entity;if(!first) std::cout<<',';first=false;
        std::cout<<"{\"slot\":"<<i<<",\"generation\":"<<state.slots[i].generation<<",\"position\":["<<e.x<<','<<e.y<<','<<e.z<<']';
        if(e.motion) {
            const auto& m=*e.motion;
            std::cout<<",\"action\":"<<static_cast<std::uint32_t>(m.action)<<",\"next\":"<<m.next<<",\"destination\":["<<m.destination.x<<','<<m.destination.y<<','<<m.destination.z<<"],\"route\":[";
            bool initial=true;for(const auto& p:m.route) {if(!initial) std::cout<<',';initial=false;std::cout<<'['<<p.position.x<<','<<p.position.y<<','<<p.position.z<<','<<p.direction<<','<<p.verticalDelta<<','<<p.category<<','<<p.scalar<<']';}
            std::cout<<']';
            if(m.sampleMotion) {
                std::cout<<",\"sampleMotion\":true";
                if(m.fine) {const auto& f=*m.fine;std::cout<<",\"fine\":["<<f.fine.x<<','<<f.fine.y<<','<<f.fine.z<<"],\"progress\":"<<f.progress<<",\"accumulator\":"<<f.accumulator<<",\"frame\":"<<f.frame;}
                else std::cout<<",\"fine\":["<<e.x*32<<','<<e.y*32<<','<<e.z*16<<']';
            }
        }
        std::cout<<'}';
    }
    std::cout<<"]}\n";
}
static MovementSession session(State state) {
    auto navigation=mnm::sandbox::loadFrozenNavigation(state.map);
    World world(0);world.restore(std::move(state));return MovementSession(std::move(world),std::move(navigation));
}
int main(int argc,char** argv) try {
    if(argc==11 && (std::string_view(argv[1])=="move" || std::string_view(argv[1])=="move-fine")) {
        auto navigation=mnm::sandbox::loadFrozenNavigation(argv[2]);World world(8);
        auto state=world.state();state.map=std::filesystem::absolute(argv[2]).lexically_normal().string();state.navigation=navigation->binding();world.restore(std::move(state));
        MovementSession movement(std::move(world),navigation);
        Entity e;e.type=navigation->creatureType();e.x=integer(argv[4]);e.y=integer(argv[5]);e.z=integer(argv[6]);
        auto creature=movement.spawn(e,std::string_view(argv[1])=="move-fine");movement.move(creature,{integer(argv[7]),integer(argv[8]),integer(argv[9])});
        for(std::uint32_t i=0,count=ticks(argv[10]);i<count;++i) movement.step();
        auto commit=writeSnapshot(argv[3],movement.world().state());if(!commit.durable) throw std::runtime_error(commit.detail);
        json(movement.world().state());return 0;
    }
    if(argc==3 && std::string_view(argv[1])=="inspect-json") {json(readSnapshot(argv[2]));return 0;}
    if(argc==4 && std::string_view(argv[1])=="trace") {
        auto movement=session(readSnapshot(argv[2]));json(movement.world().state());
        for(std::uint32_t i=0,count=ticks(argv[3]);i<count;++i) {movement.step();json(movement.world().state());}return 0;
    }
    if(argc==3 && std::string_view(argv[1])=="create") {
        auto world=fixture();auto commit=writeSnapshot(argv[2],world.state());
        if(!commit.durable) throw std::runtime_error(commit.detail);
        std::cout<<"created native fixture checkpoint (pending cleanup retained)\n";return 0;
    }
    if(argc==3 && std::string_view(argv[1])=="inspect") {
        auto state=readSnapshot(argv[2]);std::size_t active=0;for(const auto& slot:state.slots) if(slot.entity) ++active;
        std::cout<<"tick="<<state.tick<<" sequence="<<state.sequence<<" active="<<active<<" pending="<<state.pending.size()<<'\n';return 0;
    }
    if(argc==5 && std::string_view(argv[1])=="resume") {
        auto count=ticks(argv[4]);auto state=readSnapshot(argv[2]);
        if(state.navigation) {
            auto movement=session(std::move(state));for(std::uint32_t i=0;i<count;++i) movement.step();
            auto commit=writeSnapshot(argv[3],movement.world().state());if(!commit.durable) throw std::runtime_error(commit.detail);
            json(movement.world().state());return 0;
        }
        World world;world.restore(std::move(state));
        for(std::uint32_t i=0;i<count;++i) world.step();
        auto commit=writeSnapshot(argv[3],world.state());if(!commit.durable) throw std::runtime_error(commit.detail);
        std::cout<<"resumed "<<count<<" admitted idle ticks\n";return 0;
    }
    std::cerr<<"usage: mnm-world-sandbox create FILE | inspect FILE | inspect-json FILE | resume INPUT OUTPUT TICKS | trace INPUT TICKS | move[-fine] MAP OUTPUT SX SY SZ TX TY TZ TICKS\n";return 2;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
