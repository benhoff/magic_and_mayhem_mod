#include "map_campaign.hpp"
#include "objective_definition.hpp"
#include "creature_runtime.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <stdexcept>
namespace g=mnm::game;
namespace app=mnm::skirmish;
void need(bool ok,const char* why) {if(!ok) throw std::runtime_error(why);}
template<class F> void rejects(F action,const char* why) {
  bool rejected=false;try {action();} catch(const std::invalid_argument&) {rejected=true;}
  need(rejected,why);
}
g::Bytes bytes(const g::CampaignSession& s) {return g::encodeCampaignSnapshot(s.checkpoint(),s.progress().identity);}
std::shared_ptr<const app::MapCampaignResources> resources;
g::CampaignSession fresh(const g::MissionPlan* plan=nullptr) {
  auto d=resources->definition();
  if(plan) {d.plans[0]=*plan;d.initial.identity.contentRevision="objective-test-fixture-v1";}
  g::CampaignSession s(d,resources);s.startMission();return s;
}
void pilot(g::CampaignSession& s) {
  if(!s.battle()) return;
  const auto& b=*s.battle();const auto m=b.state();const auto wizard=m.wizards[0];
  const auto fighters=b.combat().combat().fighters;
  // Clear a pending arrival area with ordinary player movement, then rejoin combat.
  if(s.progress().mission && s.progress().mission->version==2) {
    const auto& progress=*s.progress().mission;const auto& plan=*s.missionPlan();
    for(std::size_t i=0;i<plan.groups.size();++i) {
      if(!progress.groups[i].empty() || !std::all_of(plan.groups[i].after.begin(),plan.groups[i].after.end(),[&](auto n){return progress.objectives[n].complete;})) continue;
      for(const auto& f:fighters) {
        const auto* actor=b.world().find(f.actor);
        if(!f.health || actor->owner!=0 || f.actor==wizard.actor || f.control==g::CombatControl::move) continue;
        bool blocking=false;
        for(auto p:plan.groups[i].locations) blocking|=actor->z==p.z && std::abs(actor->x-p.x)<2 && std::abs(actor->y-p.y)<2;
        if(!blocking) continue;
        auto candidates=b.mission().standing;
        std::stable_sort(candidates.begin(),candidates.end(),[&](auto a,auto c){return std::max(std::abs(a.x-actor->x),std::abs(a.y-actor->y))<std::max(std::abs(c.x-actor->x),std::abs(c.y-actor->y));});
        for(auto p:candidates) {
          bool clear=true;
          for(std::size_t j=0;j<plan.groups.size();++j) if(progress.groups[j].empty())
            for(auto origin:plan.groups[j].locations) clear&=p.z!=origin.z || std::abs(p.x-origin.x)>=2 || std::abs(p.y-origin.y)>=2;
          if(clear && b.combat().driver()->canSpawn(b.combat(),p) && b.combat().navigation()->planInWorld(b.world().state(),f.actor,p,256).status==g::PlanStatus::reachable) {s.enqueue({g::SkirmishOrderKind::move,f.actor,{},p});return;}
        }
      }
    }
    for(const auto& f:fighters) if(f.health && b.world().find(f.actor)->owner==0 && !(f.actor==wizard.actor) && f.control!=g::CombatControl::automatic && f.control!=g::CombatControl::attack) {
      for(const auto& group:progress.groups) for(auto h:group)
        for(const auto& target:fighters) if(target.actor==h && target.health) {s.enqueue({g::SkirmishOrderKind::attack,f.actor,h,{}});return;}
    }
  }
  if(wizard.spell!=g::Spell::none) return;
  unsigned allies=0;
  for(const auto& f:b.combat().combat().fighters)
    if(f.health && b.world().find(f.actor)->owner==0 && !(f.actor==wizard.actor)) ++allies;
  if(wizard.mana>=g::SkirmishRules::summonCost && allies<3) {
    const auto* w=b.world().find(wizard.actor);
    for(auto p:b.mission().standing)
      if(std::max(std::abs(p.x-w->x),std::abs(p.y-w->y))<=2 && b.combat().driver()->canSpawn(b.combat(),p)) {
        s.enqueue({g::SkirmishOrderKind::summon,wizard.actor,{},p});return;
      }
  }
  if(wizard.mana>=g::SkirmishRules::boltCost) {
    auto target=m.wizards[1].actor;bool found=false;
    if(s.progress().mission && s.progress().mission->version==2)
      for(const auto& group:s.progress().mission->groups) for(auto h:group)
        for(const auto& f:b.combat().combat().fighters) if(!found && f.actor==h && f.health) {target=h;found=true;}
    const auto fighters=b.combat().combat().fighters;
    if(std::none_of(fighters.begin(),fighters.end(),[&](const auto& f){return f.actor==target && f.health;})) return;
    s.enqueue({g::SkirmishOrderKind::bolt,wizard.actor,target,{}});
  }
}
g::MissionPlan timerPlan(unsigned ticks=4) {
  g::MissionPlan p;p.version=2;p.id="timer-fixture";p.briefing="Timer and protection fixture";p.rewardSpell="cure";
  p.nodes={{"timer","Survive",g::ObjectiveKind::survive,g::MissionTarget::group,0,ticks,{}},
           {"guard","Protect",g::ObjectiveKind::protect,g::MissionTarget::player,0,0,{0}}};
  return p;
}
void health(g::CampaignSession& s,const std::vector<g::Handle>& group,unsigned value) {
  auto c=s.battle()->combat();const auto fighters=c.combat().fighters;
  for(auto h:group) for(const auto& f:fighters) if(f.actor==h && f.health>value)
    c.damage(h,f.health-value,s.battle()->state().wizards[0].actor);
  auto checkpoint=s.checkpoint();checkpoint.world=c.world().state();s.restore(checkpoint);
}
void bolt(g::CampaignSession& s,g::Handle target) {s.enqueue({g::SkirmishOrderKind::bolt,s.battle()->state().wizards[0].actor,target,{}});}
int main(int argc,char** argv) try {
  need(argc>=2,"mission-objectives-test ASSET_ROOT [emit DIR | pilot INPUT OUTPUT TICKS]");
  resources=app::mapCampaignResources(argv[1],true,true,true,true,true,true);
  if(argc==6 && std::string(argv[2])=="pilot") {
    auto s=app::loadMapCampaign(resources,argv[3]);const auto ticks=std::stoul(argv[5]);need(ticks<=4096,"pilot tick limit");
    for(unsigned i=0;i<ticks && s.battle();++i) {pilot(s);s.step();}
    need(s.save(argv[4]).durable,"pilot save");std::cout<<"{\"success\":true}\n";return 0;
  }
  const bool emitSnapshots=argc==4 && std::string(argv[2])=="emit";
  need(argc==2 || emitSnapshots,"unknown test mode");
  if(emitSnapshots) std::filesystem::create_directory(argv[3]);
  auto s=fresh();const auto original=bytes(s);
  need(s.missionPlan()->nodes.size()==6 && !s.spellAvailable(g::Spell::cure),"authored plan/loadout");
  need(!s.step({false,false,false}).battle && bytes(s)==original,"unadmitted tick mutated objectives");
  s.step({true,true,false});
  need(s.progress().mission->objectives[0].elapsed==0 && s.progress().mission->groups[0].empty(),"alternate tick progressed objectives");
  auto forged=s.checkpoint();forged.campaign.mission->objectives[0].complete=true;
  const auto stable=bytes(s);rejects([&]{s.restore(forged);},"forged timer completion accepted");need(bytes(s)==stable,"restore failure published");
  forged=s.checkpoint();forged.campaign.mission->groups.push_back({});
  rejects([&]{s.restore(forged);},"wrong group shape accepted");
  forged=s.checkpoint();forged.campaign.wizards[0].spells.push_back("cure");
  rejects([&]{s.restore(forged);},"forged reward accepted");
  auto wire=g::encodeCampaign(s.progress());need(wire[8]==4,"objective campaign wire version");
  for(std::size_t n=0;n<wire.size();++n) {
    auto shortWire=wire;shortWire.resize(n);rejects([&]{g::decodeCampaign(shortWire);},"truncated campaign accepted");
  }
  auto badWire=wire;badWire[8]=5;rejects([&]{g::decodeCampaign(badWire);},"unknown campaign version accepted");
  badWire=wire;badWire.back()=255;rejects([&]{g::decodeCampaign(badWire);},"oversized objective elapsed accepted");
  badWire=wire;badWire.push_back(0);rejects([&]{g::decodeCampaign(badWire);},"trailing bytes accepted");
  auto plan=*s.missionPlan();
  const auto badPlan=[&](g::MissionPlan p){rejects([&]{g::validateMissionPlan(p,resources->mission());},"invalid objective definition accepted");};
  plan.nodes[0].ticks=0;badPlan(plan);plan=*s.missionPlan();plan.nodes[0].ticks=4097;badPlan(plan);
  plan=*s.missionPlan();plan.nodes[0].after={0};badPlan(plan);
  plan=*s.missionPlan();plan.groups[0].after={1};badPlan(plan); // Cycle through its own eliminate node.
  plan=*s.missionPlan();plan.nodes[1].after={0,0};badPlan(plan);
  plan=*s.missionPlan();plan.nodes[1].group=8;badPlan(plan);
  plan=*s.missionPlan();plan.nodes[0].id=plan.groups[0].id;badPlan(plan);
  plan=*s.missionPlan();plan.groups[0].locations={{0,0,0}};badPlan(plan);
  plan=*s.missionPlan();plan.nodes[0].kind=static_cast<g::ObjectiveKind>(9);badPlan(plan);
  plan=*s.missionPlan();plan.nodes[0].target=g::MissionTarget::player;badPlan(plan);
  plan=*s.missionPlan();plan.nodes[5].target=g::MissionTarget::group;badPlan(plan);
  plan=*s.missionPlan();plan.nodes.resize(17);badPlan(plan);
  plan=*s.missionPlan();plan.nodes[1].text=std::string(513,'x');badPlan(plan);
  // Exercise strict authoring parser separately from geometry/graph validation.
  const auto source=std::filesystem::path(__FILE__).parent_path().parent_path()/"apps/skirmish/plains_survival.json";
  std::ifstream in(source);std::string json((std::istreambuf_iterator<char>(in)),{});need(!json.empty(),"definition input");
  auto doc=QJsonDocument::fromJson(QByteArray::fromStdString(json)).object();
  const auto malformed=[&](QJsonObject o){rejects([&]{auto p=app::decodeObjectiveDefinition(QJsonDocument(o).toJson().toStdString());g::validateMissionPlan(p,resources->mission());},"malformed JSON accepted");};
  auto j=doc;j["unexpected"]=true;malformed(j);j=doc;j["version"]=2.5;malformed(j);
  j=doc;j["groups"]=false;malformed(j);j=doc;j["reward_spell"]=42;malformed(j);
  auto nodes=doc["objectives"].toArray();auto node=nodes[0].toObject();node["ticks"]=1.5;nodes[0]=node;j=doc;j["objectives"]=nodes;malformed(j);
  node=nodes[0].toObject();node["after"]=QJsonArray{"missing"};nodes[0]=node;j=doc;j["objectives"]=nodes;malformed(j);
  rejects([&]{app::decodeObjectiveDefinition("{");},"truncated JSON accepted");
  rejects([&]{app::decodeObjectiveDefinition(std::string(65537,' '));},"JSON byte budget absent");
  // Timers, independent objectives and native success do not require killing the rival.
  unsigned seeded=0;
  for(unsigned ticks:{1U,2U,7U,23U,41U}) {
    auto p=timerPlan(ticks);auto a=fresh(&p);
    for(unsigned i=0;i<ticks;++i) {
      if(i%2==0) a.step({true,true,false});
      auto clone=fresh(&p);clone.restore(a.checkpoint());a.step();clone.step();need(bytes(a)==bytes(clone),"seeded timer continuation");
      need(a.battle() && a.progress().mission->objectives[0].elapsed==i+1,"timer admission");++seeded;
    }
    need(a.progress().mission->objectives[0].complete && !a.progress().mission->objectives[1].complete,"dependency cascade in one tick");
    auto r=a.step();need(r.result && r.result->outcome==g::CampaignOutcome::victory && !a.battle(),"timer/protection success");
    need(a.spellAvailable(g::Spell::cure),"timer reward absent");
  }
  // Protection remains active before dependencies, including after its own node completes.
  auto protect=timerPlan(8);protect.nodes[1].target=g::MissionTarget::rival;protect.nodes[1].after.clear();
  auto failure=fresh(&protect);failure.step();need(failure.progress().mission->objectives[1].complete,"protect completion fixture");
  auto victim=failure.battle()->state().wizards[1].actor;health(failure,{victim},1);bolt(failure,victim);
  for(unsigned i=0;i<16 && failure.battle();++i) failure.step();
  need(!failure.battle() && failure.progress().lastResult->outcome==g::CampaignOutcome::defeat && !failure.spellAvailable(g::Spell::cure),"protection failure lost precedence");
  auto pendingProtect=timerPlan(64);pendingProtect.nodes[1].target=g::MissionTarget::rival;
  auto pendingFailure=fresh(&pendingProtect);const auto pendingVictim=pendingFailure.battle()->state().wizards[1].actor;
  health(pendingFailure,{pendingVictim},1);bolt(pendingFailure,pendingVictim);
  for(unsigned i=0;i<16 && pendingFailure.battle();++i) pendingFailure.step();
  need(!pendingFailure.battle() && pendingFailure.progress().lastResult->outcome==g::CampaignOutcome::defeat,"protection ignored before prerequisites");
  auto noReward=timerPlan(1);noReward.rewardSpell.clear();auto unrewarded=fresh(&noReward);unrewarded.step();unrewarded.step();
  need(!unrewarded.battle() && !unrewarded.spellAvailable(g::Spell::cure),"empty reward changed loadout");
  failure.startMission();need(failure.progress().battle->attempt==3 && !failure.progress().mission->objectives[0].elapsed && !failure.progress().mission->objectives[1].complete,"retry retained events");
  // A partially blocked group must be identical to an ordinary battle tick with no spawn attempt.
  auto blockedPlan=timerPlan(1);blockedPlan.groups={{"wave",{{8,4,7},{8,6,7}},{0}}};
  blockedPlan.nodes.insert(blockedPlan.nodes.begin()+1,{"clear","Clear",g::ObjectiveKind::eliminate,g::MissionTarget::group,0,0,{0}});
  blockedPlan.nodes[2].after={1};auto blocked=fresh(&blockedPlan);blocked.step();
  auto combat=blocked.battle()->combat();auto meta=blocked.battle()->state();g::Entity e;e.type=17;e.owner=0;e.x=8;e.y=6;e.z=7;
  auto blocker=combat.spawn(e,combat.driver()->creatureStats(),false,g::CombatControl::hold);meta.units.push_back({blocker,g::UnitRole::sentinel});
  auto checkpoint=blocked.checkpoint();checkpoint.world=combat.world().state();checkpoint.world.campaign=g::encodeSkirmish(meta);blocked.restore(checkpoint);
  for(unsigned i=0;i<3;++i) {
    auto expected=*blocked.battle();expected.step({},false);blocked.step();
    need(blocked.progress().mission->groups[0].empty() && g::encodeSnapshot(expected.world().state())==g::encodeSnapshot(blocked.battle()->world().state()),"blocked group consumed state/RNG or partially spawned");
  }
  auto battle=*blocked.battle();battle.removeCreature(blocker);checkpoint=blocked.checkpoint();checkpoint.world=battle.world().state();blocked.restore(checkpoint);blocked.step();
  need(blocked.progress().mission->groups[0].size()==2,"blocked group retry");
  const auto admitted=blocked.progress().mission->groups[0];need(admitted[0].slot==blocker.slot && admitted[0].generation!=blocker.generation,"spawn did not bind reused generation");
  checkpoint=blocked.checkpoint();checkpoint.campaign.mission->groups[0][0].generation=blocker.generation;
  const auto beforeReject=bytes(blocked);rejects([&]{blocked.restore(checkpoint);},"stale group generation accepted");need(bytes(blocked)==beforeReject,"stale group restore published");
  checkpoint=blocked.checkpoint();checkpoint.campaign.mission->groups[0].clear();
  rejects([&]{blocked.restore(checkpoint);},"erased admission accepted live hostile actors");
  blocked.step();need(blocked.progress().mission->groups[0]==admitted,"wave replayed");
  // Base simultaneous wizard death wins over completion/reward as a draw.
  auto drawPlan=timerPlan(64);auto draw=fresh(&drawPlan);
  const auto w=draw.battle()->state().wizards;health(draw,{w[0].actor,w[1].actor},1);
  auto c=draw.battle()->combat();auto m=draw.battle()->state();
  const auto point=[&](g::Handle h){const auto* e=draw.battle()->world().find(h);return g::Point{e->x,e->y,e->z};};
  m.projectiles={{1,0,w[0].actor,w[1].actor,point(w[0].actor),point(w[1].actor),1},{2,1,w[1].actor,w[0].actor,point(w[1].actor),point(w[0].actor),1}};m.nextProjectile=3;
  checkpoint=draw.checkpoint();checkpoint.world=c.world().state();checkpoint.world.campaign=g::encodeSkirmish(m);draw.restore(checkpoint);draw.step();
  need(!draw.battle() && draw.progress().lastResult->outcome==g::CampaignOutcome::draw && !draw.spellAvailable(g::Spell::cure),"simultaneous deaths rewarded survival");
  // Full installed health and damage, only ordinary production player orders.
  auto natural=fresh();unsigned ticks=0,boundaries=0;std::string previous;
  auto mark=[&] {
    std::string key;
    if(natural.progress().mission) {for(const auto& o:natural.progress().mission->objectives) key+=o.complete?'1':'0';for(const auto& group:natural.progress().mission->groups) key+=char('0'+group.size());}
    else key="settled";
    // Also retain both sides of each timed spawn boundary.
    if(natural.progress().mission) for(const auto& o:natural.progress().mission->objectives) if(o.elapsed==23) key+='t';
    if(key!=previous) {
      if(emitSnapshots) need(natural.save(std::filesystem::path(argv[3])/("boundary-"+std::to_string(boundaries)+".mnmc")).durable,"boundary save");
      previous=key;++boundaries;
    }
  };
  mark();
  for(;ticks<1024 && natural.battle();++ticks) {pilot(natural);natural.step();mark();}
  need(!natural.battle() && natural.progress().lastResult->outcome==g::CampaignOutcome::victory,"full-health Last Stand strategy failed");
  need(natural.progress().region=="arena-finale" && natural.spellAvailable(g::Spell::cure),"Last Stand campaign reward/transition");
  const auto settled=bytes(natural);need(!natural.step().result && bytes(natural)==settled,"settlement replay");
  auto clone=app::newMapCampaign(resources);clone.restore(natural.checkpoint());clone.startMission();natural.startMission();need(bytes(clone)==bytes(natural),"next mission restart");
  need(!natural.progress().mission && natural.spellAvailable(g::Spell::cure),"finale loadout");
  if(emitSnapshots) need(natural.save(std::filesystem::path(argv[3])/"finale.mnmc").durable,"finale save");
  std::cout<<"{\"success\":true,\"natural_ticks\":"<<ticks<<",\"boundaries\":"<<boundaries<<",\"seeded_continuations\":"<<seeded<<",\"blocked_wave\":true,\"draw\":true,\"protected_actor_failure\":true,\"reward_transition\":true}\n";
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
