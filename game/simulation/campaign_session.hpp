#pragma once
#include "mission.hpp"
#include "campaign_graph.hpp"
#include "persistence/campaign.hpp"

namespace mnm::game {
struct CampaignMission {
    std::string realm, region, map, opponent;
};
// Explicit authored, linear mission policy. No original campaign/reward rules.
struct CampaignDefinition {
    CampaignState initial;
    std::vector<CampaignMission> missions;
    std::vector<std::optional<MissionPlan>> plans;
    bool spellLoadout=false;
    std::optional<CampaignGraph> graph = {};
};
// App adapters own content/navigation/health loading. Returned sessions must
// validate their resources normally; restore must retain the exact saved world.
class CampaignResources {
public:
    virtual ~CampaignResources()=default;
    virtual SkirmishSession create(const CampaignMission&) const=0;
    virtual SkirmishSession restore(const CampaignMission&,State) const=0;
};
struct CampaignStepReport {
    std::optional<SkirmishReport> battle;
    std::optional<CampaignResult> result; // Emitted once when an attempt settles.
};
class CampaignSession {
    struct Owned {
        CampaignState progress;
        std::optional<SkirmishSession> battle;
    };
    CampaignDefinition definition_;
    std::shared_ptr<const CampaignResources> resources_;
    Owned owned_;
    bool changing_=false;
    std::size_t missionIndex(const CampaignState&) const;
    std::size_t missionIndex(const std::string&,const std::string&) const;
    void validateGraphProgress(const CampaignState&) const;
    void validateProgress(const CampaignState&) const;
    void validate(const Owned&) const;
public:
    CampaignSession(CampaignDefinition,std::shared_ptr<const CampaignResources>);
    CampaignSession(const CampaignSession&)=delete;
    CampaignSession& operator=(const CampaignSession&)=delete;
    CampaignSession(CampaignSession&&)=default;
    CampaignSession& operator=(CampaignSession&&)=default;
    const CampaignState& progress() const {return owned_.progress;}
    const SkirmishSession* battle() const {return owned_.battle ? &*owned_.battle : nullptr;}
    const CampaignMission& currentMission() const {return definition_.missions.at(missionIndex(progress()));}
    const MissionPlan* missionPlan() const;
    bool spellAvailable(Spell) const;
    std::vector<CampaignMission> availableMissions() const;
    void selectMission(const std::string& realm,const std::string& region);
    void startMission();
    void enqueue(SkirmishOrder);
    CampaignStepReport step(TickInput={});
    CampaignCheckpoint checkpoint() const;
    CommitResult save(const std::filesystem::path&,bool overwrite=false) const;
    void restore(CampaignCheckpoint);
    void restore(const std::filesystem::path&);
};
}
