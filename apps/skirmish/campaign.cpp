#include "campaign.hpp"
#include "arena.hpp"
#include "health_rules.hpp"
#include "cure_runtime.hpp"
#include <exception>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace mnm::skirmish {
namespace {
class Resources final:public game::CampaignResources {
    bool cure_, sphere_;
    std::shared_ptr<const game::CombatDriver> driver_;
    std::shared_ptr<const game::Navigation> navigation_=arenaNavigation();
    std::shared_ptr<const game::HealthRules> rules_=ordinaryHealthRules();
public:
    explicit Resources(bool cure, bool sphere, std::shared_ptr<const game::CombatDriver> driver={}):cure_(cure), sphere_(cure && sphere), driver_(std::move(driver)) {}
    bool hasCreatures() const {return bool(driver_);}
    bool hasCure() const {return cure_;}
    bool hasSphere() const {return sphere_;}
    game::SkirmishSession create(const game::CampaignMission& mission) const override {
        if(mission.map!="native-skirmish-arena-v1") throw std::invalid_argument("unknown campaign arena");
        return game::SkirmishSession(navigation_,rules_,sphere_ ? battleRuntime() : cure_ ? cureRuntime() : nullptr,driver_);
    }
    game::SkirmishSession restore(const game::CampaignMission& mission,game::State state) const override {
        if(mission.map!="native-skirmish-arena-v1" || state.map!=mission.map)
            throw std::invalid_argument("unknown campaign arena binding");
        auto effects=resolveCureRuntime(state);
        if(bool(effects)!=cure_ || (effects && (effects->identity()==sphereBattleIdentity)!=sphere_)) throw std::invalid_argument("campaign effect runtime binding mismatch");
        game::World world(0);world.restore(std::move(state));
        return {std::move(world),navigation_,rules_,std::move(effects),driver_};
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
    game::CampaignWizard player;player.id="player";player.spells={resources.hasCreatures()?"summon-crocodile":"summon-sentinel","arc-bolt"};
    if(resources.hasCure()) player.spells.push_back("cure");
    if(resources.hasSphere()) player.spells.push_back("magic-sphere");
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
game::CampaignSession newCampaign(bool enableCure, bool enableSphere, std::shared_ptr<const game::CombatDriver> driver) {
    auto resources=std::make_shared<Resources>(enableCure,enableSphere,std::move(driver));
    return {definition(*resources),std::move(resources)};
}
game::CampaignSession loadCampaign(const std::filesystem::path& path, std::shared_ptr<const game::CombatDriver> driver) {
    if(driver) {
        auto installed=newCampaign(true,true,std::move(driver));
        try {installed.restore(path);return installed;} catch(const std::exception&) {}
    }
    auto campaign=newCampaign();
    try {campaign.restore(path);return campaign;}
    catch(const std::exception&) {
        auto failure=std::current_exception();
        try {auto cureOnly=newCampaign(true,false);cureOnly.restore(path);return cureOnly;}
        catch(const std::exception&) {}
        try {auto legacy=newCampaign(false);legacy.restore(path);return legacy;}
        catch(const std::exception&) {std::rethrow_exception(failure);}
    }
}
}
