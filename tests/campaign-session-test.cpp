#include "simulation/campaign_session.hpp"
#include "simulation/occupancy.hpp"
#include <iostream>
#include <functional>
#include <limits>
#include <stdexcept>

using namespace mnm::game;
namespace {
void need(bool value,const char* why) {if(!value) throw std::runtime_error(why);}
template<class F> void refuses(F action) {
    bool failed=false;try {action();} catch(const std::exception&) {failed=true;}
    need(failed,"operation should refuse");
}
struct Grid final:Navigation {
    std::uint64_t fingerprint=1;
    NavigationBinding binding() const override {return {{8,8,3},fingerprint};}
    std::uint32_t creatureType() const override {return 0;}
    std::uint32_t maxMovingCreatures() const override {return 32;}
    void validateSpawnPosition(Point p) const override {need(arenaCell(p),"synthetic arena placement refused");}
    void validateOccupants(const State& s) const override {(void)MovementReservations(s,binding().dimensions,0,{1,1,1});}
    bool accepts(const Entity& e,const RoutePoint& p) const override {
        return contains(p.position,binding()) && p.position.z==1 && p.position!=Point{e.x,e.y,e.z} &&
            std::abs(p.position.x-e.x)<=1 && std::abs(p.position.y-e.y)<=1;
    }
    RoutePlan plan(const Entity& e,Point target,std::uint32_t budget) const override {
        RoutePlan r{PlanStatus::reachable,{},0};Point p{e.x,e.y,e.z};
        while(p!=target && r.route.size()<16) {
            if(++r.expansions>budget) return {PlanStatus::budgetExhausted,{},budget};
            p.x+=(p.x<target.x)-(p.x>target.x);p.y+=(p.y<target.y)-(p.y>target.y);
            r.route.push_back({p,2,0,0,720});
        }
        return r;
    }
};
struct Rules final:HealthRules {
    bool fail=false,wrongIdentity=false;
    std::uint64_t identity() const override {return nativeFixtureHealthRules()->identity()+(wrongIdentity?1:0);}
    std::uint32_t change(std::uint32_t current,std::uint32_t maximum,std::int32_t delta,bool defended) const override {
        if(fail) throw std::runtime_error("injected health failure");
        return nativeFixtureHealthRules()->change(current,maximum,delta,defended);
    }
};
struct Resources final:CampaignResources {
    std::shared_ptr<Grid> nav=std::make_shared<Grid>();
    std::shared_ptr<Rules> rules=std::make_shared<Rules>();
    bool failCreate=false,failRestore=false,wrongMapBinding=false,wrongRules=false,alterRestore=false,draw=false;
    std::uint32_t playerHealth=48,enemyHealth=1;
    std::function<void()> beforeCreate;
    SkirmishSession create(const CampaignMission&) const override {
        if(beforeCreate) beforeCreate();
        if(failCreate) throw std::runtime_error("injected create failure");
        SkirmishSession s(nav,rules);auto state=s.world().state();auto combat=decodeCombat(state.systems);
        combat.fighters[0].health=playerHealth;combat.fighters[1].health=enemyHealth;
        state.systems=encodeCombat(combat);
        if(draw) {
            auto meta=decodeSkirmish(state.campaign);
            meta.nextProjectile=3;
            meta.projectiles={{1,0,meta.wizards[0].actor,meta.wizards[1].actor,{1,6,1},{6,1,1},1},
                              {2,1,meta.wizards[1].actor,meta.wizards[0].actor,{6,1,1},{1,6,1},1}};
            state.campaign=encodeSkirmish(meta);
        }
        World w(0);w.restore(std::move(state));return {std::move(w),nav,rules};
    }
    SkirmishSession restore(const CampaignMission&,State state) const override {
        if(failRestore) throw std::runtime_error("injected resolver failure");
        auto rebound=std::make_shared<Grid>();rebound->fingerprint=wrongMapBinding?2:1;
        auto health=std::make_shared<Rules>();health->wrongIdentity=wrongRules;
        if(alterRestore) ++state.tick;
        World w(0);w.restore(std::move(state));return {std::move(w),rebound,health};
    }
};
CampaignDefinition definition() {
    CampaignDefinition d;auto& p=d.initial;
    p.identity={"session-test","content-v1"};p.player="player";p.realm="one";p.region="first";
    CampaignWizard player;player.id="player";player.spells={"retained"};player.inventory={"token","token"};
    CampaignWizard enemy;enemy.id="enemy";p.wizards={player,enemy};
    p.realms={{"one",{{"first","enemy",false}}},{"two",{{"last","enemy",false}}}};
    d.missions={{"one","first","native-skirmish-arena-v1","enemy"},{"two","last","native-skirmish-arena-v1","enemy"}};
    return d;
}
Bytes bytes(const CampaignSession& s) {return encodeCampaignSnapshot(s.checkpoint(),s.progress().identity);}
SkirmishOrder bolt() {return {SkirmishOrderKind::bolt,{0,1},Handle{1,1},{}};}
std::optional<CampaignResult> finish(CampaignSession& s) {
    std::optional<CampaignResult> result;
    for(unsigned n=0;n<200 && s.battle();++n) {
        auto report=s.step();if(report.result) {need(!result,"duplicate outcome report");result=report.result;}
    }
    need(bool(result),"battle did not finish");return result;
}
void contract() {
    auto resources=std::make_shared<Resources>();CampaignSession session(definition(),resources);
    auto initial=bytes(session);
    need(!session.battle() && !session.step().result && bytes(session)==initial,"idle session advanced");
    refuses([&]{session.enqueue(bolt());});need(bytes(session)==initial,"idle command mutated state");
    resources->failCreate=true;refuses([&]{session.startMission();});resources->failCreate=false;
    need(bytes(session)==initial,"failed creation mutated progress");
    resources->beforeCreate=[&]{session.startMission();};
    refuses([&]{session.startMission();});need(bytes(session)==initial,"recursive creation changed session");
    resources->beforeCreate={};
    session.startMission();need(session.progress().battle->attempt==1 && session.progress().transition==1,"attempt not issued");
    auto started=bytes(session);refuses([&]{session.startMission();});need(bytes(session)==started,"duplicate start changed state");
    session.enqueue(bolt());auto queued=bytes(session);
    need(!session.step({false,false,false}).battle && bytes(session)==queued,"nonadmitted tick advanced");
    session.step();session.step();session.step();need(!session.battle()->state().projectiles.empty(),"missing in-flight projectile");
    const auto pending=bytes(session);
    const auto saved=session.checkpoint();
    for(int failure=0;failure<4;++failure) {
        resources->failRestore=failure==0;resources->wrongMapBinding=failure==1;
        resources->wrongRules=failure==2;resources->alterRestore=failure==3;
        refuses([&]{session.restore(saved);});need(bytes(session)==pending,"failed restoration published part of checkpoint");
    }
    resources->failRestore=resources->wrongMapBinding=resources->wrongRules=resources->alterRestore=false;
    auto reject=[&](auto edit) {auto bad=saved;edit(bad);refuses([&]{session.restore(std::move(bad));});need(bytes(session)==pending,"invalid checkpoint changed session");};
    reject([](auto& c){c.campaign.identity.contentRevision="changed";});
    reject([](auto& c){c.campaign.battle->attempt=0;});
    reject([](auto& c){c.campaign.battle->attempt=3;});
    reject([](auto& c){c.campaign.battle->participants={"enemy","player"};});
    reject([](auto& c){c.campaign.realms[1].regions[0].completed=true;});
    reject([](auto& c){c.campaign.realm="two";c.campaign.region="last";});
    reject([](auto& c){c.world.map="other";});
    CampaignSession restored(definition(),resources);restored.restore(saved);
    need(bytes(restored)==pending,"resource restoration changed checkpoint bytes");
    for(unsigned i=0;i<3;++i) session.step();
    const auto beforeFailure=bytes(session);resources->rules->fail=true;
    refuses([&]{session.step();});need(bytes(session)==beforeFailure,"failed terminal tick partially published");
    resources->rules->fail=false;
    auto result=finish(session);auto resumedResult=finish(restored);
    need(result->outcome==CampaignOutcome::victory && result->attempt==1 &&
         resumedResult->attempt==1 && bytes(session)==bytes(restored),"continued outcome differs");
    need(!session.battle() && session.progress().realm=="two" && session.progress().region=="last" &&
         session.progress().transition==2 && session.progress().realms[0].regions[0].owner=="player","victory progress not committed");
    const auto settled=bytes(session);
    for(unsigned i=0;i<4;++i) need(!session.step().result && bytes(session)==settled,"settled result replayed");
    restored.restore(session.checkpoint());need(!restored.step().result && bytes(restored)==settled,"result replayed after restore");
    auto idleWithWorld=session.checkpoint();idleWithWorld.world=saved.world;
    refuses([&]{session.restore(idleWithWorld);});need(bytes(session)==settled,"idle world refusal changed state");
    session.startMission();need(session.progress().battle->attempt==3,"second attempt reused identity");
    session.enqueue(bolt());result=finish(session);
    need(result->attempt==3 && session.progress().completed && session.progress().transition==4,"final mission not completed");
    const auto done=bytes(session);refuses([&]{session.startMission();});need(bytes(session)==done,"completed campaign restarted");
    need(session.progress().wizards[0].inventory==definition().initial.wizards[0].inventory &&
         session.progress().wizards[0].experience==0,"settlement invented rewards");
    resources->enemyHealth=48;resources->playerHealth=1;
    CampaignSession defeated(definition(),resources);defeated.startMission();result=finish(defeated);
    need(result->outcome==CampaignOutcome::defeat && defeated.progress().region=="first" &&
         !defeated.progress().realms[0].regions[0].completed,"defeat advanced campaign");
    auto defeatBytes=bytes(defeated);restored.restore(defeated.checkpoint());
    need(!restored.step().result && bytes(restored)==defeatBytes,"defeat replayed on reopen");
    defeated.startMission();need(defeated.progress().battle->attempt==3,"retry reused attempt");
    resources->draw=true;resources->enemyHealth=1;
    CampaignSession drawn(definition(),resources);drawn.startMission();result=finish(drawn);
    need(result->outcome==CampaignOutcome::draw && drawn.progress().region=="first","draw advanced campaign");
    auto overflow=drawn.checkpoint();overflow.campaign.lastResult->attempt=std::numeric_limits<std::uint64_t>::max()-2;
    overflow.campaign.transition=std::numeric_limits<std::uint64_t>::max()-1;
    drawn.restore(overflow);const auto exhausted=bytes(drawn);
    refuses([&]{drawn.startMission();});need(bytes(drawn)==exhausted,"exhaustion changed campaign");
    auto bad=definition();bad.missions[0].opponent="player";refuses([&]{CampaignSession invalid(bad,resources);});
    bad=definition();bad.missions.pop_back();refuses([&]{CampaignSession invalid(bad,resources);});
    auto continuation=session.progress();const auto encoded=encodeCampaign(continuation);
    need(encoded[8]==2 && encodeCampaign(decodeCampaign(encoded))==encoded,"version-2 continuation roundtrip");
    for(std::size_t n=0;n<encoded.size();++n)
        refuses([&]{decodeCampaign(Bytes(encoded.begin(),encoded.begin()+n));});
    auto invalid=encoded;invalid.back()=255;refuses([&]{decodeCampaign(invalid);});
    continuation.lastResult->attempt=0;refuses([&]{encodeCampaign(continuation);});
}
}
int main() try {
    contract();std::cout<<"Campaign session: transactional attempts, resource restoration, victory/defeat/draw and one-time settlement passed.\n";return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
