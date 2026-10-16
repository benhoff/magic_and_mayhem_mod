#pragma once
#include "simulation/campaign_session.hpp"
namespace mnm::skirmish {
// Two authored duels over the existing arena, with its ordinary-health backend.
game::CampaignSession newCampaign(bool enableCure = true, bool enableSphere = true,
                                  std::shared_ptr<const game::CombatDriver> = {});
game::CampaignSession loadCampaign(const std::filesystem::path&, std::shared_ptr<const game::CombatDriver> = {});
}
