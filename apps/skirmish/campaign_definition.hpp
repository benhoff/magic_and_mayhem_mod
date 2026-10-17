#pragma once
#include "simulation/campaign_session.hpp"
#include <string_view>
namespace mnm::skirmish {
// Strict authoring adapter for one compiled native chapter; no Qt in the core.
game::CampaignDefinition decodeCampaignDefinition(std::string_view,const game::SkirmishMission&,const std::string& arenaMap);
}
