#pragma once
#include "campaign.hpp"
#include "map_navigation.hpp"
#include "sprite_loader.hpp"
#include "sphere_runtime.hpp"
namespace mnm::skirmish {
// Immutable resource context shared by campaign ownership and presentation.
class MapCampaignResources final : public game::CampaignResources {
    bool cure_=false, spheres_=false, creatures_=false, staged_=false, objectives_=false, branches_=false;
    std::shared_ptr<const game::CombatDriver> mapDriver_, arenaDriver_;
    sphere::Environment sphereEnvironment_;
    game::SkirmishMission mission_;
    scene::MapNavigation map_;
    assets::TerrainCatalog catalog_;
    assets::Sprite sprites_;
    std::shared_ptr<const game::Navigation> navigation_;
    game::CampaignDefinition definition_;
    std::array<std::string,3> hashes_;
  public:
    explicit MapCampaignResources(const std::filesystem::path &root, bool enableCure = false, bool enableSphere = false, bool installedCreatures = false, bool mixedActors = false, bool stagedMission = false, bool objectiveMission = false, bool branchingCampaign = false);
    const auto &mission() const {return mission_;}
    const auto &map() const {return map_;}
    const auto &catalog() const {return catalog_;}
    const auto &sprites() const {return sprites_;}
    const auto &definition() const {return definition_;}
    const auto &hashes() const {return hashes_;}
    game::SkirmishSession create(const game::CampaignMission &) const override;
    game::SkirmishSession restore(const game::CampaignMission &,game::State) const override;
};
std::shared_ptr<const MapCampaignResources> mapCampaignResources(const std::filesystem::path &, bool enableCure = false, bool enableSphere = false, bool installedCreatures = false, bool mixedActors = false, bool stagedMission = false, bool objectiveMission = false, bool branchingCampaign = false);
game::CampaignSession newMapCampaign(std::shared_ptr<const MapCampaignResources>);
game::CampaignSession loadMapCampaign(std::shared_ptr<const MapCampaignResources>,const std::filesystem::path &);
game::CampaignSession loadMapCampaign(const std::filesystem::path &root,const std::filesystem::path &save);
} // namespace mnm::skirmish
