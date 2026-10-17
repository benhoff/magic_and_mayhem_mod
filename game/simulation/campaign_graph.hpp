#pragma once
#include "persistence/campaign.hpp"
namespace mnm::game {
struct CampaignNode {
    std::vector<std::uint32_t> all, any;
    std::string choice;
    std::vector<std::string> rewards;
};
// Definition order is topological; node zero is the sole entry and the last
// node is the terminal mission. Campaign mission IDs live in the owning roster.
struct CampaignGraph {
    std::vector<CampaignNode> nodes;
};
bool campaignSpell(const std::string&);
void validateCampaignGraph(const CampaignGraph&);
bool campaignNodeAvailable(const CampaignGraph&,const CampaignGraphProgress&,std::uint32_t);
}
