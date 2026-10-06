#include <array>
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
static void json(const State& state,const MovementSession* session=nullptr) {
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
            if(m.terrainMotion) std::cout<<",\"terrainMotion\":true";
            if(m.sampleMotion) {
                std::cout<<",\"sampleMotion\":true";
                if(m.continuousMotion) {
                    std::cout<<",\"continuousMotion\":true,\"segmentTicks\":"<<m.segmentTicks;
                    if(m.previous) {const auto& h=*m.previous;std::cout<<",\"previous\":["<<h.direction<<','<<h.motion.rate<<','<<h.motion.accumulator<<','<<h.motion.progress<<','<<h.motion.frame<<']';}
                    if(m.fine) std::cout<<",\"rate\":"<<m.fine->rate<<",\"duration\":"<<m.fine->duration;
                }
                if(m.fine && m.fine->animation) {
                    const auto& a=*m.fine->animation;std::cout<<",\"animation\":["<<a.sequence<<','<<a.pc<<','<<(a.displayed?static_cast<int>(*a.displayed):-1)<<','<<a.active<<','<<a.delay<<','<<a.elapsed<<','<<a.repeats<<','<<a.breakFlag<<']';
                }
                if(m.fine) {const auto& f=*m.fine;std::cout<<",\"fine\":["<<f.fine.x<<','<<f.fine.y<<','<<f.fine.z<<"],\"progress\":"<<f.progress<<",\"accumulator\":"<<f.accumulator<<",\"frame\":"<<f.frame;if(m.terrainMotion) std::cout<<",\"heightOrigin\":"<<f.heightOrigin<<",\"heightDelta\":"<<f.heightDelta;}
                else if(m.terrainMotion && !session) std::cout<<",\"fine\":null";
                else {const auto at=session?session->finePosition(e):Point{e.x*32,e.y*32,e.z*16};std::cout<<",\"fine\":["<<at.x<<','<<at.y<<','<<at.z<<']';}
            }
        }
        std::cout<<'}';
    }
    std::cout<<"]}\n";
}
static MovementSession session(State state) {
    auto navigation=mnm::sandbox::loadFrozenNavigation(state.map,state.animation);
    if(state.navigation && !(navigation->binding()==*state.navigation))
        navigation=mnm::sandbox::loadFrozenNavigation(state.map,state.animation,true);
    if(state.navigation && !(navigation->binding()==*state.navigation))
        navigation=mnm::sandbox::loadFrozenNavigation(state.map,state.animation,true,true);
    World world(0);world.restore(std::move(state));return MovementSession(std::move(world),std::move(navigation));
}
int main(int argc,char** argv) try {
    if(argc==19 && std::string_view(argv[1])=="move-pair-terrain-ani") {
        auto base=integer(argv[4]);if(base<0 || base>4088) throw std::invalid_argument("ANI sequence base outside 0..4088");
        auto animation=mnm::sandbox::loadMovementAnimation(argv[3],base);
        auto navigation=mnm::sandbox::loadFrozenNavigation(argv[2],animation,true,true);World world(8);
        auto state=world.state();state.map=std::filesystem::absolute(argv[2]).lexically_normal().string();state.navigation=navigation->binding();state.animation=std::move(animation);world.restore(std::move(state));
        MovementSession movement(std::move(world),navigation);std::array<Handle,2> actors;
        for(unsigned i=0;i<2;++i) {
            Entity e;e.type=navigation->creatureType();e.x=integer(argv[6+i*6]);e.y=integer(argv[7+i*6]);e.z=integer(argv[8+i*6]);
            actors[i]=movement.spawn(e,true,true,true);
        }
        for(unsigned i=0;i<2;++i) movement.move(actors[i],{integer(argv[9+i*6]),integer(argv[10+i*6]),integer(argv[11+i*6])});
        for(std::uint32_t i=0,count=ticks(argv[18]);i<count;++i) movement.step();
        auto commit=writeSnapshot(argv[5],movement.world().state());if(!commit.durable) throw std::runtime_error(commit.detail);
        json(movement.world().state(),&movement);return 0;
    }
    if(argc==17 && (std::string_view(argv[1])=="move-pair" || std::string_view(argv[1])=="move-pair-fine")) {
        auto navigation=mnm::sandbox::loadFrozenNavigation(argv[2],{},true,true);World world(8);
        auto state=world.state();state.map=std::filesystem::absolute(argv[2]).lexically_normal().string();state.navigation=navigation->binding();world.restore(std::move(state));
        MovementSession movement(std::move(world),navigation);std::array<Handle,2> actors;
        for(unsigned i=0;i<2;++i) {
            Entity e;e.type=navigation->creatureType();e.x=integer(argv[4+i*6]);e.y=integer(argv[5+i*6]);e.z=integer(argv[6+i*6]);
            const auto fine=std::string_view(argv[1])=="move-pair-fine";actors[i]=movement.spawn(e,fine,fine);
        }
        for(unsigned i=0;i<2;++i) movement.move(actors[i],{integer(argv[7+i*6]),integer(argv[8+i*6]),integer(argv[9+i*6])});
        for(std::uint32_t i=0,count=ticks(argv[16]);i<count;++i) movement.step();
        auto commit=writeSnapshot(argv[3],movement.world().state());if(!commit.durable) throw std::runtime_error(commit.detail);
        json(movement.world().state(),&movement);return 0;
    }
    if(argc==14 && std::string_view(argv[1])=="move-occupied") {
        auto navigation=mnm::sandbox::loadFrozenNavigation(argv[2],{},true);World world(8);
        auto state=world.state();state.map=std::filesystem::absolute(argv[2]).lexically_normal().string();state.navigation=navigation->binding();world.restore(std::move(state));
        MovementSession movement(std::move(world),navigation);
        Entity e;e.type=navigation->creatureType();e.x=integer(argv[4]);e.y=integer(argv[5]);e.z=integer(argv[6]);
        auto creature=movement.spawn(e);
        e.x=integer(argv[10]);e.y=integer(argv[11]);e.z=integer(argv[12]);movement.spawnBlocker(e);
        movement.move(creature,{integer(argv[7]),integer(argv[8]),integer(argv[9])});
        for(std::uint32_t i=0,count=ticks(argv[13]);i<count;++i) movement.step();
        auto commit=writeSnapshot(argv[3],movement.world().state());if(!commit.durable) throw std::runtime_error(commit.detail);
        json(movement.world().state(),&movement);return 0;
    }
    if(argc==9 && std::string_view(argv[1])=="spawn-terrain-ani") {
        const auto base=integer(argv[4]);if(base<0 || base>4088) throw std::invalid_argument("ANI sequence base outside 0..4088");
        auto animation=mnm::sandbox::loadMovementAnimation(argv[3],base);
        auto navigation=mnm::sandbox::loadFrozenNavigation(argv[2],animation);World world(8);
        auto state=world.state();state.map=std::filesystem::absolute(argv[2]).lexically_normal().string();state.navigation=navigation->binding();state.animation=std::move(animation);world.restore(std::move(state));
        MovementSession movement(std::move(world),navigation);
        Entity e;e.type=navigation->creatureType();e.x=integer(argv[6]);e.y=integer(argv[7]);e.z=integer(argv[8]);
        movement.spawn(e,true,true,true,true);
        const auto commit=writeSnapshot(argv[5],movement.world().state());if(!commit.durable) throw std::runtime_error(commit.detail);
        json(movement.world().state(),&movement);return 0;
    }
    if(argc==13 && (std::string_view(argv[1])=="move-ani" || std::string_view(argv[1])=="move-terrain-ani")) {
        auto base=integer(argv[4]);if(base<0 || base>4088) throw std::invalid_argument("ANI sequence base outside 0..4088");
        auto animation=mnm::sandbox::loadMovementAnimation(argv[3],base);
        auto navigation=mnm::sandbox::loadFrozenNavigation(argv[2],animation);World world(8);
        auto state=world.state();state.map=std::filesystem::absolute(argv[2]).lexically_normal().string();state.navigation=navigation->binding();state.animation=std::move(animation);world.restore(std::move(state));
        MovementSession movement(std::move(world),navigation);
        Entity e;e.type=navigation->creatureType();e.x=integer(argv[6]);e.y=integer(argv[7]);e.z=integer(argv[8]);
        auto creature=movement.spawn(e,true,true,std::string_view(argv[1])=="move-terrain-ani");movement.move(creature,{integer(argv[9]),integer(argv[10]),integer(argv[11])});
        for(std::uint32_t i=0,count=ticks(argv[12]);i<count;++i) movement.step();
        auto commit=writeSnapshot(argv[5],movement.world().state());if(!commit.durable) throw std::runtime_error(commit.detail);
        json(movement.world().state(),&movement);return 0;
    }
    if(argc==11 && (std::string_view(argv[1])=="move" || std::string_view(argv[1])=="move-fine" || std::string_view(argv[1])=="move-continuous" || std::string_view(argv[1])=="move-terrain")) {
        auto navigation=mnm::sandbox::loadFrozenNavigation(argv[2]);World world(8);
        auto state=world.state();state.map=std::filesystem::absolute(argv[2]).lexically_normal().string();state.navigation=navigation->binding();world.restore(std::move(state));
        MovementSession movement(std::move(world),navigation);
        Entity e;e.type=navigation->creatureType();e.x=integer(argv[4]);e.y=integer(argv[5]);e.z=integer(argv[6]);
        const bool terrain=std::string_view(argv[1])=="move-terrain";
        auto creature=movement.spawn(e,std::string_view(argv[1])!="move",terrain || std::string_view(argv[1])=="move-continuous",terrain);movement.move(creature,{integer(argv[7]),integer(argv[8]),integer(argv[9])});
        for(std::uint32_t i=0,count=ticks(argv[10]);i<count;++i) movement.step();
        auto commit=writeSnapshot(argv[3],movement.world().state());if(!commit.durable) throw std::runtime_error(commit.detail);
        json(movement.world().state(),&movement);return 0;
    }
    if(argc==3 && std::string_view(argv[1])=="inspect-json") {json(readSnapshot(argv[2]));return 0;}
    if(argc==4 && std::string_view(argv[1])=="trace") {
        auto movement=session(readSnapshot(argv[2]));json(movement.world().state(),&movement);
        for(std::uint32_t i=0,count=ticks(argv[3]);i<count;++i) {movement.step();json(movement.world().state(),&movement);}return 0;
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
            json(movement.world().state(),&movement);return 0;
        }
        World world;world.restore(std::move(state));
        for(std::uint32_t i=0;i<count;++i) world.step();
        auto commit=writeSnapshot(argv[3],world.state());if(!commit.durable) throw std::runtime_error(commit.detail);
        std::cout<<"resumed "<<count<<" admitted idle ticks\n";return 0;
    }
    std::cerr<<"usage: mnm-world-sandbox move-pair|move-pair-fine MAP OUTPUT SX1 SY1 SZ1 TX1 TY1 TZ1 SX2 SY2 SZ2 TX2 TY2 TZ2 TICKS | move-occupied MAP OUTPUT SX SY SZ TX TY TZ BX BY BZ TICKS | create FILE | inspect FILE | inspect-json FILE | resume INPUT OUTPUT TICKS | trace INPUT TICKS | move|move-fine|move-continuous|move-terrain MAP OUTPUT SX SY SZ TX TY TZ TICKS | spawn-terrain-ani MAP ANI BASE OUTPUT X Y Z | move-ani|move-terrain-ani MAP ANI BASE OUTPUT SX SY SZ TX TY TZ TICKS\n";return 2;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
