#pragma once
#include "simulation/campaign_session.hpp"
namespace mnm::skirmish {
// Two authored duels over the existing arena, with its ordinary-health backend.
game::CampaignSession newCampaign(bool enableCure = true);
game::CampaignSession loadCampaign(const std::filesystem::path&);
}
