#include "simulation/campaign_session.hpp"
#include "simulation/occupancy.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
using namespace mnm::game;
void need(bool ok,const char* why) {if(!ok) throw std::runtime_error(why);}
template<class F> void refuses(F action) {
    bool rejected=false;try {action();} catch(const std::invalid_argument&) {rejected=true;}
    need(rejected,"expected campaign graph refusal");
}
struct Grid final:Navigation {
    NavigationBinding binding() const override {return {{8,8,3},731};}
    unsigned creatureType() const override {return 0;}
    unsigned maxMovingCreatures() const override {return 32;}
    void validateSpawnPosition(Point p) const override {if(!arenaCell(p)) throw std::invalid_argument("fixture placement");}
    void validateOccupants(const State& s) const override {(void)MovementReservations(s,binding().dimensions,0,{1,1,1});}
    bool accepts(const Entity&,const RoutePoint&) const override {return false;}
    RoutePlan plan(const Entity&,Point,unsigned) const override {return {};}
};
struct Resources final:CampaignResources {
    std::shared_ptr<const Grid> nav=std::make_shared<Grid>();bool fail=false;
    SkirmishSession create(const CampaignMission&) const override {
        if(fail) throw std::invalid_argument("injected factory refusal");
        return SkirmishSession(nav);
    }
    SkirmishSession restore(const CampaignMission&,State state) const override {
        World w(0);w.restore(std::move(state));return {std::move(w),nav};
    }
};
CampaignGraph graph() {return {{{{}, {}, "", {}},{{0}, {}, "road", {"cure"}},{{0}, {}, "road", {"magic-sphere"}},{{}, {1,2}, "", {}}}};}
CampaignDefinition definition() {
    CampaignDefinition d;auto& p=d.initial;p.identity={"graph-core-test","fixture-v1"};p.player="player";p.realm="celtic";p.region="opening";
    CampaignWizard a;a.id="player";a.spells={"summon-crocodile","arc-bolt"};CampaignWizard b;b.id="rival";p.wizards={a,b};p.realms={{"celtic",{}}};p.graph=CampaignGraphProgress{};
    d.spellLoadout=true;d.graph=graph();
    for(const auto* id:{"opening","left","right","finale"}) {
        p.realms[0].regions.push_back({id,"rival",false});d.missions.push_back({"celtic",id,arenaMission().map,"rival"});
        MissionPlan plan;plan.version=2;plan.id=id;plan.briefing="Timer fixture";
        plan.nodes={{"timer","Survive",ObjectiveKind::survive,MissionTarget::group,0,1,{}},{"guard","Protect",ObjectiveKind::protect,MissionTarget::player,0,0,{0}}};d.plans.push_back(plan);
    }
    return d;
}
Bytes bytes(const CampaignSession& s) {return encodeCampaignSnapshot(s.checkpoint(),s.progress().identity);}
void finish(CampaignSession& s) {for(unsigned i=0;i<4 && s.battle();++i) s.step();need(!s.battle() && s.progress().lastResult->outcome==CampaignOutcome::victory,"fixture victory");}
void invalidDefinitions() {
    validateCampaignGraph(graph());auto bad=graph();bad.nodes[1].all={1};refuses([&]{validateCampaignGraph(bad);});
    bad=graph();bad.nodes[1].all={0,0};refuses([&]{validateCampaignGraph(bad);});
    bad=graph();bad.nodes[3].all={1,2};bad.nodes[3].any.clear();refuses([&]{validateCampaignGraph(bad);});
    bad=graph();bad.nodes[3].any={1};refuses([&]{validateCampaignGraph(bad);}); // Right route dead-ends.
    bad=graph();bad.nodes[2].choice="other";refuses([&]{validateCampaignGraph(bad);});
    bad=graph();bad.nodes[2].all.clear();refuses([&]{validateCampaignGraph(bad);});
    bad=graph();bad.nodes[1].rewards={"unknown"};refuses([&]{validateCampaignGraph(bad);});
    bad=graph();bad.nodes[1].rewards={"cure","cure"};refuses([&]{validateCampaignGraph(bad);});
    bad=graph();bad.nodes[0].choice="road";refuses([&]{validateCampaignGraph(bad);});
    bad.nodes.resize(17);refuses([&]{validateCampaignGraph(bad);});
    CampaignGraph chain;for(unsigned i=0;i<16;++i) {CampaignNode n;if(i) n.all={i-1};chain.nodes.push_back(n);}validateCampaignGraph(chain);
}
int main() try {
    invalidDefinitions();unsigned continuations=0;
    for(unsigned route:{1U,2U}) {
        auto resources=std::make_shared<Resources>();const auto d=definition();CampaignSession s(d,resources);
        need(encodeCampaign(s.progress())[8]==5 && s.availableMissions().size()==1,"initial graph/version");
        const auto idle=bytes(s);refuses([&]{s.selectMission("celtic","finale");});need(bytes(s)==idle,"locked selection published");
        s.startMission();finish(s);need(s.availableMissions().size()==2 && s.progress().graph->choices.empty(),"fork frontier");
        s.selectMission("celtic","right");s.selectMission("celtic","left");s.selectMission("celtic",route==1?"left":"right");
        const auto selected=bytes(s);resources->fail=true;refuses([&]{s.startMission();});resources->fail=false;
        need(bytes(s)==selected && s.progress().graph->choices.empty(),"failed start committed route");
        s.startMission();need(s.progress().graph->choices==std::vector<unsigned>{route},"route start commitment");
        auto before=bytes(s);refuses([&]{s.selectMission("celtic",route==1?"right":"left");});need(bytes(s)==before,"active selection changed battle");
        refuses([&]{s.enqueue({SkirmishOrderKind::cure,s.battle()->state().wizards[0].actor,s.battle()->state().wizards[1].actor,{}});});need(bytes(s)==before,"locked spell changed battle");
        need(!s.step({false,false,false}).battle && bytes(s)==before,"unadmitted graph advancement");
        auto wire=encodeCampaign(s.progress());
        for(std::size_t i=0;i<wire.size();++i) {auto cut=wire;cut.resize(i);refuses([&]{decodeCampaign(cut);});}
        auto bad=wire;bad[8]=6;refuses([&]{decodeCampaign(bad);});bad=wire;bad.push_back(0);refuses([&]{decodeCampaign(bad);});
        auto saved=s.checkpoint();auto forged=saved;forged.campaign.graph->choices={route==1?2U:1U};refuses([&]{s.restore(forged);});need(bytes(s)==before,"forged choice published");
        forged=saved;forged.campaign.graph->choices.push_back(15);refuses([&]{s.restore(forged);});
        forged=saved;forged.campaign.graph.reset();refuses([&]{s.restore(forged);});
        // Ordinary base defeat beats a timer completion; retry keeps the chosen route.
        auto c=s.battle()->combat();auto meta=s.battle()->state();c.damage(meta.wizards[0].actor,47,meta.wizards[1].actor);
        meta.projectiles={{1,1,meta.wizards[1].actor,meta.wizards[0].actor,{6,1,1},{1,6,1},1}};meta.nextProjectile=2;
        auto defeat=s.checkpoint();defeat.world=c.world().state();defeat.world.campaign=encodeSkirmish(meta);s.restore(defeat);s.step();
        need(!s.battle() && s.progress().lastResult->outcome==CampaignOutcome::defeat && s.progress().graph->completed==std::vector<unsigned>{0},"defeat awarded progress");
        before=bytes(s);refuses([&]{s.selectMission("celtic",route==1?"right":"left");});need(bytes(s)==before && s.availableMissions().size()==1,"failed route switched");
        CampaignSession lossClone(d,resources);lossClone.restore(s.checkpoint());need(bytes(lossClone)==bytes(s),"loss continuation");
        s.startMission();need(s.progress().battle->attempt==5 && s.progress().graph->choices==std::vector<unsigned>{route},"retry lost route/attempt");
        for(unsigned i=0;i<2;++i) {CampaignSession clone(d,resources);clone.restore(s.checkpoint());s.step();clone.step();need(bytes(s)==bytes(clone),"route continuation differs");++continuations;}
        need(!s.battle() && s.progress().region=="finale" && s.progress().graph->completed==std::vector<unsigned>({0,route}),"route settlement/frontier");
        need(s.spellAvailable(Spell::cure)==(route==1) && s.spellAvailable(Spell::sphere)==(route==2),"route spell rewards");
        const auto rewarded=bytes(s);s.step();s.step();need(bytes(s)==rewarded,"idle reward replay");
        forged=s.checkpoint();forged.campaign.wizards[0].spells.push_back(route==1?"magic-sphere":"cure");refuses([&]{s.restore(forged);});
        forged=s.checkpoint();forged.campaign.graph->completed={0,3};refuses([&]{s.restore(forged);});
        forged=s.checkpoint();forged.campaign.realms[0].regions[route==1?2:1].completed=true;refuses([&]{s.restore(forged);});
        forged=s.checkpoint();forged.campaign.graph->completed={0,route,route};refuses([&]{s.restore(forged);});
        s.startMission();finish(s);need(s.progress().completed && s.progress().graph->completed==std::vector<unsigned>({0,route,3}) && s.availableMissions().empty(),"finale completion");
        const auto complete=bytes(s);CampaignSession clone(d,resources);clone.restore(s.checkpoint());need(bytes(clone)==complete,"complete save roundtrip");
        refuses([&]{s.startMission();});refuses([&]{s.selectMission("celtic","opening");});need(!s.step().result && bytes(s)==complete,"completed campaign replay");
    }
    std::cout<<"{\"success\":true,\"routes\":2,\"continuations\":"<<continuations<<",\"retry\":true,\"wire_refusals\":true}\n";
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
