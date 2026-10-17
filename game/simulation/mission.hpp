#pragma once
#include "persistence/campaign.hpp"
#include "skirmish.hpp"
namespace mnm::game {
// Deliberately finite objective language, separate from original mission
// scripts.
struct MissionPlan {
  std::uint32_t version = 1;
  std::string id, briefing;
  std::array<std::string, 3> objectives;
  std::vector<Point> initial, reinforcement;
  std::string rewardSpell;
};
void validateMissionPlan(const MissionPlan &, const SkirmishMission &);
bool missionGroupDead(const SkirmishSession &, const std::vector<Handle> &);
void validateMissionProgress(const MissionPlan &, const MissionProgress &,
                             const SkirmishSession &);
} // namespace mnm::game
