#include "campaign.hpp"
#include "health_rules.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::game;
namespace {
void need(bool value,const char* why) {if(!value) throw std::runtime_error(why);}
template<class F> void refuses(F action) {
    bool failed=false;try {action();} catch(const std::exception&) {failed=true;}
    need(failed,"expected campaign arena refusal");
}
Bytes bytes(const CampaignSession& s) {return encodeCampaignSnapshot(s.checkpoint(),s.progress().identity);}
void mutate(CampaignCheckpoint& cp,const std::string& kind) {
    if(kind=="navigation") {need(bool(cp.world.navigation),"active world required");cp.world.navigation->fingerprint^=1;}
    else if(kind=="health") {auto c=decodeCombat(cp.world.systems);c.healthRulesIdentity^=1;cp.world.systems=encodeCombat(c);}
    else if(kind=="revision") cp.campaign.identity.contentRevision="foreign-content";
    else if(kind=="participants") cp.campaign.battle->participants={"rival","player"};
    else if(kind=="replay") ++cp.campaign.transition;
    else if(kind=="owner") cp.campaign.realms[0].regions[0].owner="rival";
    else throw std::invalid_argument("unknown mutation");
}
}
int main(int argc,char** argv) try {
    auto session=mnm::skirmish::newCampaign();
    if(argc==5 && std::string(argv[1])=="rewrite") {
        auto cp=readCampaignSnapshot(argv[2],session.progress().identity);mutate(cp,argv[4]);
        need(writeCampaignSnapshot(argv[3],cp,cp.campaign.identity).durable,"mutation publication was not durable");
        return 0;
    }
    need(argc==1,"invalid test arguments");
    const auto initial=session.progress().identity;
    need(initial.id=="native-skirmish-campaign" && initial.contentRevision.rfind("arena-v1-",0)==0,"missing content binding");
    need(mnm::skirmish::newCampaign().progress().identity.contentRevision==initial.contentRevision,"content identity is unstable");
    session.startMission();session.enqueue({SkirmishOrderKind::bolt,session.battle()->state().wizards[0].actor,
                                          session.battle()->state().wizards[1].actor,{}});
    session.step();session.step();session.step();
    need(!session.battle()->state().projectiles.empty(),"no real arena projectile");
    need(session.battle()->combat().healthRules()->identity()==mnm::skirmish::ordinaryHealthIdentity,"wrong native health binding");
    const auto active=bytes(session);auto cp=session.checkpoint();
    for(const auto& kind:{"navigation","health","revision","participants"}) {
        auto invalid=cp;mutate(invalid,kind);refuses([&]{session.restore(invalid);});
        need(bytes(session)==active,"failed arena restore changed campaign");
    }
    auto restored=mnm::skirmish::newCampaign();restored.restore(cp);
    for(unsigned i=0;i<100 && session.battle();++i) {
        session.step();restored.step();need(bytes(session)==bytes(restored),"rebound arena continuation differs");
    }
    need(session.progress().lastResult && !session.battle(),"real arena did not finish");
    const auto settled=bytes(session);restored.restore(session.checkpoint());
    need(!restored.step().result && bytes(restored)==settled,"arena result replayed after restoration");
    auto invalid=session.checkpoint();mutate(invalid,"replay");refuses([&]{session.restore(invalid);});
    need(bytes(session)==settled,"invalid result ordering changed progress");
    std::cout<<"Campaign arena: exact content/navigation/health binding, rollback and real skirmish continuation passed.\n";
    return 0;
} catch(const std::exception& error) {std::cerr<<error.what()<<'\n';return 1;}
