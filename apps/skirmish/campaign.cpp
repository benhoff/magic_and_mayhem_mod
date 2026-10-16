#include "campaign.hpp"
#include "arena.hpp"
#include "health_rules.hpp"
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace mnm::skirmish {
namespace {
class Resources final:public game::CampaignResources {
    std::shared_ptr<const game::Navigation> navigation_=arenaNavigation();
    std::shared_ptr<const game::HealthRules> rules_=ordinaryHealthRules();
public:
    game::SkirmishSession create(const game::CampaignMission& mission) const override {
        if(mission.map!="native-skirmish-arena-v1") throw std::invalid_argument("unknown campaign arena");
        return game::SkirmishSession(navigation_,rules_);
    }
    game::SkirmishSession restore(const game::CampaignMission& mission,game::State state) const override {
        if(mission.map!="native-skirmish-arena-v1" || state.map!=mission.map)
            throw std::invalid_argument("unknown campaign arena binding");
        game::World world(0);world.restore(std::move(state));
        game::SkirmishSession restored(std::move(world),navigation_,rules_);
        if(!game::sameMission(restored.mission(),game::arenaMission()))
            throw std::invalid_argument("campaign arena mission mismatch");
        return restored;
    }
};
void append(game::Bytes& bytes,const game::Bytes& value) {
    const auto size=static_cast<std::uint32_t>(value.size());
    for(unsigned i=0;i<4;++i) bytes.push_back(static_cast<std::uint8_t>(size>>(8*i)));
    bytes.insert(bytes.end(),value.begin(),value.end());
}
void append(game::Bytes& bytes,const std::string& text) {append(bytes,game::Bytes(text.begin(),text.end()));}
game::CampaignDefinition definition(const Resources& resources) {
    game::CampaignDefinition content;auto& p=content.initial;
    p.identity={"native-skirmish-campaign","binding-pending"};
    p.player="player";p.realm="arena";p.region="first-duel";
    game::CampaignWizard player;player.id="player";player.spells={"summon-sentinel","arc-bolt"};
    game::CampaignWizard rival;rival.id="rival";p.wizards={player,rival};
    p.realms={{"arena",{{"first-duel","rival",false},{"second-duel","rival",false}}}};
    content.missions={{"arena","first-duel","native-skirmish-arena-v1","rival"},
                      {"arena","second-duel","native-skirmish-arena-v1","rival"}};
    // Pin the manifest, native progression policy and exact initial arena
    // snapshot (navigation/profile fingerprint, rules hash and health identity).
    auto bytes=game::encodeCampaign(p);append(bytes,std::string("linear-missions-v1"));
    for(const auto& m:content.missions) {
        append(bytes,m.realm);append(bytes,m.region);append(bytes,m.map);append(bytes,m.opponent);
    }
    append(bytes,game::encodeSnapshot(resources.create(content.missions.front()).world().state()));
    std::uint64_t hash=14695981039346656037ULL;
    for(auto byte:bytes) {hash^=byte;hash*=1099511628211ULL;}
    std::ostringstream revision;revision<<"arena-v1-"<<std::hex<<std::setfill('0')<<std::setw(16)<<hash;
    p.identity.contentRevision=revision.str();return content;
}
}
game::CampaignSession newCampaign() {
    auto resources=std::make_shared<Resources>();
    return {definition(*resources),std::move(resources)};
}
game::CampaignSession loadCampaign(const std::filesystem::path& path) {
    auto campaign=newCampaign();campaign.restore(path);return campaign;
}
}
