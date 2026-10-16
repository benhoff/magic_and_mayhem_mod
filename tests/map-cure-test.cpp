#include "map_campaign.hpp"
#include "cure_runtime.hpp"
#include "health_rules.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::game;
namespace app = mnm::skirmish;
namespace {
void need(bool ok,const char *why) { if(!ok) throw std::runtime_error(why); }
Bytes bytes(const CampaignSession &s) { return encodeCampaignSnapshot(s.checkpoint(),s.progress().identity); }
unsigned health(const CampaignSession &s) {
  for(const auto &f:s.battle()->combat().combat().fighters) if(f.actor==Handle{2,1}) return f.health;
  throw std::runtime_error("missing Sentinel");
}
CampaignSession ready(const char *root) {
  auto s=app::newMapCampaign(app::mapCampaignResources(root,true));s.startMission();
  auto cp=s.checkpoint();auto meta=s.battle()->state();meta.aiWait=8;meta.wizards[1].mana=0;
  cp.world.campaign=encodeSkirmish(meta);s.restore(cp);
  s.enqueue({SkirmishOrderKind::summon,{0,1},{},Point{2,5,3}});
  for(unsigned i=0;i<3;++i) s.step();
  auto combat=s.battle()->combat();combat.damage({2,1},15,{1,1});
  cp=s.checkpoint();cp.world=combat.world().state();cp.world.campaign=encodeSkirmish(s.battle()->state());s.restore(cp);
  need(health(s)==9,"fixture health");return s;
}
unsigned finish(CampaignSession &s) {
  unsigned healed=0;
  for(unsigned i=0;i<64 && !app::cureViews(s.battle()->combat()).empty();++i) {
    auto report=s.step();need(bool(report.battle),"battle ended during Cure");
    for(const auto &e:report.battle->combat.events) if(e.kind==CombatEventKind::healed) healed+=e.amount;
  }
  need(app::cureViews(s.battle()->combat()).empty(),"Cure did not finish");return healed;
}
}
int main(int argc,char **argv) try {
  need(argc>=2,"installed asset root required");
  if(argc==4 && std::string(argv[2])=="seed") {auto s=ready(argv[1]);need(s.save(argv[3]).durable,"fixture save");return 0;}
  if(argc==5 && std::string(argv[2])=="resume") {
    auto s=app::loadMapCampaign(argv[1],argv[3]);auto healed=finish(s);
    need(healed==8 && health(s)==17,"single resumed heal");need(s.save(argv[4]).durable,"resumed save");
    std::cout<<"{\"success\":true,\"healed\":8,\"health\":17}\n";return 0;
  }
  need(argc==2,"invalid fixture arguments");
  auto s=ready(argv[1]);const auto mana=s.battle()->state().wizards[0].mana;
  s.enqueue({SkirmishOrderKind::move,{2,1},{},Point{3,5,3}});
  s.enqueue({SkirmishOrderKind::cure,{0,1},Handle{2,1},{}});s.step();
  need(s.battle()->state().wizards[0].mana==mana-app::cureCost,"single mana debit");s.step();
  auto views=app::cureViews(s.battle()->combat());auto actor=s.battle()->world().find({2,1});
  need(views.size()==1 && views[0].position==Point{actor->x,actor->y,actor->z} && actor->x!=2 && health(s)==9,"moving target effect");
  auto cp=s.checkpoint();auto restored=app::newMapCampaign(app::mapCampaignResources(argv[1],true));restored.restore(cp);
  unsigned healed=0;
  for(unsigned i=0;i<16;++i) {
    auto report=s.step();restored.step();need(bytes(s)==bytes(restored),"per-tick campaign continuation differs");
    for(const auto &e:report.battle->combat.events) if(e.kind==CombatEventKind::healed) healed+=e.amount;
    restored.restore(restored.checkpoint());
  }
  need(healed==8 && health(s)==17 && app::cureViews(s.battle()->combat()).empty(),"heal completion/cap");
  need(finish(s)==0 && health(s)==17,"duplicate heal after completion");
  auto before=bytes(restored);bool refused=false;
  try {auto bad=cp;bad.campaign.identity.contentRevision+="forged";restored.restore(bad);} catch(const std::exception &) {refused=true;}
  need(refused && before==bytes(restored),"failed restore altered campaign");
  refused=false;try {app::cureRuntime()->validate(s.battle()->combat(),s.battle()->state());} catch(const std::exception &) {refused=true;}
  need(refused,"arena Cure dimensions accepted by Plains");
  auto legacy=app::newMapCampaign(app::mapCampaignResources(argv[1]));legacy.startMission();
  need(!legacy.battle()->effectsRuntime(),"legacy map profile acquired Cure");
  auto legacyCopy=app::newMapCampaign(app::mapCampaignResources(argv[1]));legacyCopy.restore(legacy.checkpoint());
  need(bytes(legacy)==bytes(legacyCopy) && legacy.progress().identity.contentRevision!=s.progress().identity.contentRevision,"legacy profile identity");
  std::cout<<"{\"success\":true,\"layers\":23,\"moving_target\":true,\"single_heal\":8,\"per_tick_equal\":16,\"refused\":2,\"legacy_profile\":true}\n";
} catch(const std::exception &e) {std::cerr<<e.what()<<'\n';return 1;}
