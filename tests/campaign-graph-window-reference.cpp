#include "map_campaign.hpp"
#include <iostream>
#include <stdexcept>
int main(int argc,char** argv) try {
    if(argc!=4 && argc!=5) throw std::invalid_argument("campaign-graph-window-reference ROOT INPUT OUTPUT");
    auto s=mnm::skirmish::loadMapCampaign(argv[1],argv[2]);
    if(!s.battle() || !s.progress().graph) throw std::invalid_argument("active graph fixture required");
    auto c=s.battle()->combat();auto m=s.battle()->state();
    if(argc==5 && std::string(argv[4])=="reward") {
        m.pending.clear();auto checkpoint=s.checkpoint();checkpoint.world.campaign=mnm::game::encodeSkirmish(m);s.restore(checkpoint);
        if(!s.save(argv[3]).durable) throw std::runtime_error("reward fixture publication failed");
        std::cout<<"{\"success\":true,\"fixture\":\"earned spell target before queue\"}\n";return 0;
    }
    const auto actor=m.wizards[0].actor,enemy=m.wizards[1].actor;
    unsigned hp=0;for(const auto &f:c.combat().fighters) if(f.actor==actor) hp=f.health;
    if(!hp) throw std::invalid_argument("living fixture player required");
    c.damage(actor,hp-1,enemy);
    const auto *source=c.world().find(enemy),*target=c.world().find(actor);
    m.projectiles={{m.nextProjectile++,1,enemy,actor,{source->x,source->y,source->z},{target->x,target->y,target->z},1}};
    auto checkpoint=s.checkpoint();checkpoint.world=c.world().state();checkpoint.world.campaign=mnm::game::encodeSkirmish(m);
    s.restore(checkpoint);
    if(!s.save(argv[3]).durable) throw std::runtime_error("defeat fixture publication failed");
    s.step();if(s.battle() || s.progress().lastResult->outcome!=mnm::game::CampaignOutcome::defeat) throw std::runtime_error("fixture did not cause defeat");
    std::cout<<"{\"success\":true,\"fixture\":\"one-health pending hostile projectile\"}\n";
} catch(const std::exception &e) {std::cerr<<e.what()<<'\n';return 1;}
