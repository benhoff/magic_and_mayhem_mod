#pragma once
#include "persistence/campaign.hpp"
#include "skirmish.hpp"
namespace mnm::game {
enum class ObjectiveKind : std::uint32_t { eliminate, survive, protect };
enum class MissionTarget : std::uint32_t { group, player, rival };
struct MissionGroup {
  std::string id;
  std::vector<Point> locations;
  std::vector<std::uint32_t> after;
};
struct MissionObjective {
  std::string id, text;
  ObjectiveKind kind = ObjectiveKind::eliminate;
  MissionTarget target = MissionTarget::group;
  std::uint32_t group = 0, ticks = 0;
  std::vector<std::uint32_t> after;
};
// Deliberately finite objective language, separate from original mission
// scripts.
struct MissionPlan {
  std::uint32_t version = 1;
  std::string id, briefing;
  std::array<std::string, 3> objectives;
  std::vector<Point> initial, reinforcement;
  std::string rewardSpell;
  std::vector<MissionGroup> groups;
  std::vector<MissionObjective> nodes;
};
void validateMissionPlan(const MissionPlan &, const SkirmishMission &);
bool missionGroupDead(const SkirmishSession &, const std::vector<Handle> &);
void validateMissionProgress(const MissionPlan &, const MissionProgress &,
                             const SkirmishSession &);
// These operate only on the campaign's unpublished transaction candidate.
MissionProgress startObjectives(const MissionPlan &, SkirmishSession &);
void admitMissionGroups(const MissionPlan &, MissionProgress &, SkirmishSession &);
void advanceObjectives(const MissionPlan &, MissionProgress &, const SkirmishSession &);
bool objectivesComplete(const MissionProgress &);
bool protectionFailed(const MissionPlan &, const SkirmishSession &);
} // namespace mnm::game
