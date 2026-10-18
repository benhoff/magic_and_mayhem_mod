#pragma once
#include "simulation/campaign_session.hpp"
#include "view.hpp"
namespace mnm::skirmish {
// Application text/selection model; widgets receive no simulation indexes.
void presentCampaign(const game::CampaignSession &, const game::CampaignDefinition *, ArenaView &);
}
