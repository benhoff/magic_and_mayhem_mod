#include "arena.hpp"
#include "simulation/battle_systems.hpp"
#include "persistence/snapshot.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::game;
namespace {
void need(bool yes,const char *why) {if(!yes) throw std::runtime_error(why);}
template<class F> void refuses(F f) {bool failed=false;try{f();}catch(const std::exception&){failed=true;}need(failed,"missing refusal");}
class Runtime final:public SkirmishEffects {
public:
    mutable bool fail=false;
    std::uint64_t identity()const override{return 0x1234567887654321ULL;}
    Bytes initial()const override{return {0};}
    void validate(const CombatSession& c,const SkirmishState&)const override {
        need(c.effects().identity==identity() && c.effects().data.size()==1,"effect runtime binding");
    }
    SkirmishRefusal cast(CombatSession&,SkirmishState&,Handle,Handle)const override {return SkirmishRefusal::target;}
    CombatReport step(CombatSession& c,SkirmishState&,TickInput)const override {
        auto bytes=c.effects().data;++bytes[0];c.updateEffects(bytes);
        if(fail) throw std::runtime_error("injected effect failure after movement");
        return {};
    }
    void cancel(CombatSession& c,SkirmishState&)const override {for(auto h:c.effects().entities)c.releaseEffect(h);}
};
}
int main()try {
    auto nav=mnm::skirmish::arenaNavigation();
    SkirmishSession legacy(nav);auto raw=legacy.world().state().systems;
    need(!isBattleSystems(raw) && encodeBattleSystems(decodeBattleSystems(raw))==raw,"legacy exact bytes");
    auto runtime=std::make_shared<Runtime>();SkirmishSession s(nav,{},runtime);
    s.enqueue({SkirmishOrderKind::summon,{0,1},{},Point{2,5,1}});
    for(unsigned i=0;i<3;++i)s.step();
    need(s.state().units.size()>=3,"new creature enrollment");
    auto c=s.combat();Entity e;e.family=Family::missileEffect;e.type=41;e.owner=0;e.x=2;e.y=5;e.z=1;e.target=Handle{2,1};
    const auto effect=c.spawnEffect(e);auto payload=c.effects().data;
    c.damage({2,1},5,{0,1});c.heal({2,1},3,{0,1});
    need(c.combat().fighters[2].health==22 && c.effects().data==payload,"one health owner and payload preservation");
    c.enqueue({CombatOrderKind::move,{2,1},{},Point{3,5,1}});
    refuses([&]{c.step();}); // Opaque effect clock must be owned by a battle runtime.
    auto state=c.world().state();state.campaign=encodeSkirmish(s.state());
    // Lower-level queued controls are consumed before a skirmish checkpoint.
    auto combat=decodeCombat(state.systems);combat.pending.clear();auto sections=decodeBattleSystems(state.systems);sections.combat=encodeCombat(combat);state.systems=encodeBattleSystems(sections);
    World w(0);w.restore(state);SkirmishSession combined(std::move(w),nav,{},runtime);
    combined.enqueue({SkirmishOrderKind::move,{2,1},{},Point{3,5,1}});
    auto before=encodeSnapshot(combined.world().state());runtime->fail=true;refuses([&]{combined.step();});
    need(encodeSnapshot(combined.world().state())==before,"whole movement/effect rollback");runtime->fail=false;
    combined.step();need(combined.world().find(effect),"effect survives creature movement");
    auto snapshot=encodeSnapshot(combined.world().state());World copy(0);copy.restore(decodeSnapshot(snapshot));
    refuses([&]{SkirmishSession wrong(copy,nav);});
    SkirmishSession resumed(std::move(copy),nav,{},runtime);
    for(unsigned i=0;i<6;++i) {combined.step();resumed.step();need(encodeSnapshot(combined.world().state())==encodeSnapshot(resumed.world().state()),"combined continuation");}
    c=combined.combat();auto oldTick=c.world().state().tick;c.releaseEffect(effect);need(!c.world().find(effect) && c.world().state().tick==oldTick,"retire without second tick");
    const auto reused=c.spawnEffect(e);need(reused.slot==effect.slot && reused.generation!=effect.generation,"effect generation reuse");refuses([&]{c.releaseEffect(effect);});
    auto blob=encodeBattleSystems(decodeBattleSystems(c.world().state().systems));
    for(std::size_t n=8;n<blob.size();++n) {Bytes truncated(blob.begin(),blob.begin()+n);refuses([&]{decodeBattleSystems(truncated);});}
    auto bad=decodeBattleSystems(blob);bad.effects.entities.push_back(reused);refuses([&]{encodeBattleSystems(bad);});
    bad=decodeBattleSystems(blob);bad.effects.identity=0;refuses([&]{encodeBattleSystems(bad);});
    before=encodeSnapshot(combined.world().state());combined.step({false,false,false});need(encodeSnapshot(combined.world().state())==before,"unadmitted freeze");
    oldTick=combined.world().state().tick;auto clock=combined.combat().effects().data[0];combined.step({true,true,false});need(combined.world().state().tick==oldTick && combined.combat().effects().data[0]==clock+1,"alternate effects-only clock");
    std::cout<<"{\"success\":true,\"legacy_exact\":true,\"rollback\":true,\"continuation\":true,\"generation_safe\":true}\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
