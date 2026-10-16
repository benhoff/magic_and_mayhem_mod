#include "map_campaign.hpp"
#include "cure_runtime.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
using namespace mnm::game;
namespace app=mnm::skirmish;
namespace {
void need(bool ok,const char *why){if(!ok)throw std::runtime_error(why);}
Bytes bytes(const CampaignSession &s){return encodeCampaignSnapshot(s.checkpoint(),s.progress().identity);}
unsigned health(const CampaignSession &s,unsigned slot){for(const auto &f:s.battle()->combat().combat().fighters)if(f.actor.slot==slot)return f.health;throw std::runtime_error("missing fighter");}
CampaignSession ready(const char *root,bool evade=false){
 auto s=app::newMapCampaign(app::mapCampaignResources(root,true,true));s.startMission();auto combat=s.battle()->combat();
 auto meta=s.battle()->state();meta.aiWait=8;meta.wizards[1].mana=0;
 auto valid=[&](Point p){if(!missionCell(s.battle()->mission(),p)){std::cerr<<"Fixture cell invalid: "<<p.x<<","<<p.y<<","<<p.z<<"; standing:";for(auto q:s.battle()->mission().standing)std::cerr<<" "<<q.x<<","<<q.y<<","<<q.z;std::cerr<<"\n";throw std::runtime_error("fixture outside mission standing cells");}};
 Entity enemy;enemy.type=s.battle()->world().find({0,1})->type;enemy.owner=1;enemy.x=4;enemy.y=evade?6:5;enemy.z=3;
 valid({enemy.x,enemy.y,enemy.z});
 auto foe=combat.spawn(enemy,{24,6,2,3},true,CombatControl::automatic);meta.units.push_back({foe,UnitRole::sentinel});
 Entity friendly=enemy;friendly.owner=0;friendly.x=1;friendly.y=5;
 valid({friendly.x,friendly.y,friendly.z});
 auto friendUnit=combat.spawn(friendly,{24,6,2,3},true,CombatControl::hold);meta.units.push_back({friendUnit,UnitRole::sentinel});combat.damage(friendUnit,15,{1,1});
 auto cp=s.checkpoint();cp.world=combat.world().state();cp.world.campaign=encodeSkirmish(meta);s.restore(cp);return s;
}
std::size_t sphereAt(const Bytes &data){const Bytes magic{'S','H','P','2'};auto it=std::search(data.begin(),data.end(),magic.begin(),magic.end());need(it!=data.end(),"missing bound Sphere payload");return std::size_t(it-data.begin());}
CampaignCheckpoint forgedEnvironment(CampaignCheckpoint cp){
 auto sections=decodeBattleSystems(cp.world.systems);auto at=sphereAt(sections.effects.data);
 Bytes tail(sections.effects.data.begin()+at,sections.effects.data.end());auto env=app::sphere::environment(tail);env.cells[0].flags^=0x20000000u;
 auto replacement=app::sphere::initial(env);need(replacement.size()==tail.size(),"fixture shape changed");std::copy(replacement.begin(),replacement.end(),sections.effects.data.begin()+at);cp.world.systems=encodeBattleSystems(sections);return cp;
}
void cast(CampaignSession &s,bool cure=true){s.enqueue({SkirmishOrderKind::sphere,{0,1},Handle{2,1},{}});if(cure)s.enqueue({SkirmishOrderKind::cure,{0,1},Handle{3,1},{}});}

}
int main(int argc,char **argv)try{
 need(argc>=2,"installed asset root required");
 if(argc==4 && std::string(argv[2])=="seed"){auto s=ready(argv[1]);need(s.save(argv[3]).durable,"fixture publication");return 0;}
 if(argc==5 && (std::string(argv[2])=="forge-environment" || std::string(argv[2])=="forge-profile")){
  auto s=app::loadMapCampaign(argv[1],argv[3]);auto cp=s.checkpoint();
  if(std::string(argv[2])=="forge-environment") cp=forgedEnvironment(cp);
  else for(auto &w:cp.campaign.wizards) if(w.id==cp.campaign.player) w.spells.erase(std::remove(w.spells.begin(),w.spells.end(),"magic-sphere"),w.spells.end());
  need(writeCampaignSnapshot(argv[4],cp,cp.campaign.identity).durable,"forged fixture publication");return 0;
 }
 if(argc==5 && std::string(argv[2])=="resume"){
  auto s=app::loadMapCampaign(argv[1],argv[3]);unsigned hits=0,damage=0,healed=0;
  for(unsigned n=0;n<16;++n){auto report=s.step();need(bool(report.battle),"fixture battle ended");for(const auto &e:report.battle->combat.events){if(e.kind==CombatEventKind::hit && e.actor==Handle{0,1} && e.target==Handle{2,1}){++hits;damage+=e.amount;}if(e.kind==CombatEventKind::healed && e.target==Handle{3,1})healed+=e.amount;}}
  need(hits==1 && damage==20 && healed==8 && health(s,2)<=4 && health(s,3)==17,"resumed single hit/heal");need(s.save(argv[4]).durable,"resumed publication");std::cout<<"{\"success\":true,\"hits\":1,\"damage\":20,\"healed\":8}\n";return 0;
 }
 need(argc==2,"invalid arguments");
 auto resources=app::mapCampaignResources(argv[1],true,true);auto s=ready(argv[1]);auto before=bytes(s);auto bad=forgedEnvironment(s.checkpoint());bool refused=false;
 try{s.restore(bad);}catch(const std::exception&){refused=true;}need(refused && bytes(s)==before,"forged internally valid environment accepted or altered campaign");
 cast(s);auto first=s.step();(void)first;
 need(s.battle()->state().wizards[0].mana==9,"Sphere/Cure single authoritative debit");
 need(app::sphereViews(s.battle()->combat()).size()==1 && app::cureViews(s.battle()->combat()).size()==1,"simultaneous effects missing");
 auto sections=decodeBattleSystems(s.battle()->combat().world().state().systems);
 auto at=sphereAt(sections.effects.data);auto terrain=app::sphere::environment(Bytes(sections.effects.data.begin()+at,sections.effects.data.end()));
 need(terrain.occupancy.size()>4,"installed fixture did not exercise expanded terrain palette");
 auto effect=app::sphereViews(s.battle()->combat())[0].effect;
 auto restored=app::newMapCampaign(resources);restored.restore(s.checkpoint());
 unsigned hits=0,damage=0,healed=0;bool moved=false;
 for(unsigned n=0;n<20;++n){auto report=s.step();restored.step();need(bytes(s)==bytes(restored),"mid-flight per-tick restart differs");restored.restore(restored.checkpoint());auto foe=s.battle()->world().find({2,1});moved=moved || foe->x!=4 || foe->y!=5;for(const auto &e:report.battle->combat.events){if(e.kind==CombatEventKind::hit && e.actor==Handle{0,1} && e.target==Handle{2,1}){++hits;damage+=e.amount;}if(e.kind==CombatEventKind::healed && e.target==Handle{3,1})healed+=e.amount;}}
 need(moved && hits==1 && damage==20 && healed==8 && health(s,2)==4 && health(s,3)==17,"moving map target single hit/heal");
 need(app::sphereViews(s.battle()->combat()).empty() && !s.battle()->world().find(effect),"sealed map boundary did not retire Sphere");
 auto miss=ready(argv[1],true);cast(miss,false);hits=0;
 for(unsigned n=0;n<20;++n){auto report=miss.step();for(const auto &e:report.battle->combat.events)if(e.kind==CombatEventKind::hit && e.actor==Handle{0,1} && e.target==Handle{2,1})++hits;}
 need(hits==0 && health(miss,2)==24 && app::sphereViews(miss.battle()->combat()).empty(),"evading map target or boundary retirement");
 auto removed=ready(argv[1]);cast(removed);removed.step();auto battle=*removed.battle();auto tick=battle.world().state().tick;auto paid=battle.state().wizards[0].mana;auto handle=app::sphereViews(battle.combat())[0].effect;battle.removeCreature({2,1});need(!battle.world().find(handle) && app::sphereViews(battle.combat()).empty() && battle.world().state().tick==tick && battle.state().wizards[0].mana==paid,"generation removal/cancellation/refund/clock");auto cp=removed.checkpoint();cp.world=battle.world().state();removed.restore(cp);
 auto legacy=app::newMapCampaign(app::mapCampaignResources(argv[1]));legacy.startMission();auto cure=app::newMapCampaign(app::mapCampaignResources(argv[1],true));cure.startMission();need(!legacy.battle()->effectsRuntime() && cure.battle()->effectsRuntime()->identity()==app::cureBattleIdentity && s.battle()->effectsRuntime()->identity()==app::sphereBattleIdentity,"legacy map profiles");
 std::cout<<"{\"success\":true,\"layers\":23,\"per_tick_equal\":20,\"moving_hit\":1,\"damage\":20,\"healed\":8,\"evading_miss\":true,\"terrain_retirement\":true,\"forged_environment_refused\":true,\"removed_generation_cancelled\":true,\"legacy_profiles\":2}\n";
}catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
