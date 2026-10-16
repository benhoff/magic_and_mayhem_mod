#include "map_campaign.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::game;
namespace {
void need(bool v,const char *why) {if(!v) throw std::runtime_error(why);}
template<class F> void refuses(F f) {bool failed=false;try{f();}catch(const std::exception&){failed=true;}need(failed,"expected refusal");}
Bytes bytes(const CampaignSession &s){return encodeCampaignSnapshot(s.checkpoint(),s.progress().identity);}
}
int main(int argc,char **argv) try {
    if(argc!=2) throw std::invalid_argument("resource root required");
    auto resources=mnm::skirmish::mapCampaignResources(argv[1]);
    auto session=mnm::skirmish::newMapCampaign(resources);session.startMission();
    need(session.battle()->world().state().navigation->dimensions==Point{8,8,23},"unexpected crop geometry");
    need(session.battle()->world().find({0,1})->z==3,"wrong supported spawn layer");
    session.enqueue({SkirmishOrderKind::summon,{0,1},{},Point{2,5,3}});
    for(int i=0;i<3;++i) session.step();
    session.enqueue({SkirmishOrderKind::move,{2,1},{},Point{3,5,3}});
    session.enqueue({SkirmishOrderKind::bolt,{0,1},Handle{1,1},{}});
    session.step();auto checkpoint=session.checkpoint();auto before=bytes(session);
    auto reject=[&](auto edit) {auto bad=checkpoint;edit(bad);refuses([&]{session.restore(bad);});need(bytes(session)==before,"failed restore changed live campaign");};
    reject([](auto &c){auto m=decodeSkirmish(c.world.campaign);std::swap(m.mission->aiSummons[0],m.mission->aiSummons[1]);c.world.campaign=encodeSkirmish(m);});
    reject([](auto &c){auto m=decodeSkirmish(c.world.campaign);m.mission->map="forged";c.world.map="forged";c.world.campaign=encodeSkirmish(m);});
    reject([](auto &c){c.world.navigation->fingerprint^=1;});
    reject([](auto &c){c.campaign.identity.contentRevision+="changed";});
    reject([](auto &c){auto combat=decodeCombat(c.world.systems);combat.healthRulesIdentity^=1;c.world.systems=encodeCombat(combat);});
    auto restored=mnm::skirmish::newMapCampaign(resources);restored.restore(checkpoint);
    for(int i=0;i<6;++i){session.step();restored.step();}
    need(bytes(session)==bytes(restored),"installed split continuation differs");
    std::cout<<"{\"success\":true,\"refused_checkpoints\":5,\"assets\":[";
    bool first=true;for(const auto &h:resources->hashes()){if(!first) std::cout<<',';first=false;std::cout<<'"'<<h<<'"';}
    std::cout<<"]}\n";
} catch(const std::exception &e){std::cerr<<e.what()<<'\n';return 1;}
