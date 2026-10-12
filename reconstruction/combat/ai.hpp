#pragma once
#include "control.hpp"
#include <array>
#include <optional>
namespace mnm::reconstruction::combat {
struct AiCreature {
  std::uint32_t slot = 0, type = 17, kind = 1, health = 360,
                maximumHealth = 360, behavior = 1, pending = 0, action = 15,
                history = 0, recent = 0, engaged = 0, incoming = 0, flying = 0,
                ranged = 0, physical = 1, corpseA = 0, corpseB = 0,
                corpseClaim = 0, gateFC = 0, flag704 = 0, countdown = 0,
                flag815 = 0, flag80d = 0, dirty932 = 0, readiness = 0,
                marker = 0;
  std::int32_t owner = 0, age = 0, bravery = 0, target = -1;
  bool active = true, motion = false, bloodLust = false;
  Cell position, home;
  std::array<std::uint32_t, 8> visible{{1, 1, 1, 1, 1, 1, 1, 1}};
  std::array<std::int32_t, 28> hth{}, enemyRadius{}, allyRadius{},
      retaliateThreshold{};
};
struct AiWorld {
  Extent dimensions;
  std::vector<AiCreature> creatures;
  std::array<std::array<std::uint8_t, 8>, 8> relation{};
  std::array<bool, 8> human{};
};
enum class Selector {
  priority,
  corpse21,
  corpse10,
  assist,
  score,
  home,
  escape,
  interaction,
  spell
};
struct Selection {
  std::optional<std::uint32_t> target;
  std::int32_t value = 0, range = 0;
  std::uint32_t substate = 0;
};
Selection selectEnemy(const AiCreature &, const AiWorld &, bool homeRadius,
                      const std::function<bool(std::uint32_t)> &admit);
Selection selectCorpse(const AiCreature &, const AiWorld &, bool troll,
                       const std::function<bool(std::uint32_t)> &admit);
Selection
selectAssist(const AiCreature &, const AiWorld &,
             const std::function<bool(std::uint32_t, std::uint32_t)> &assist,
             const std::function<bool(std::uint32_t)> &admit);
Selection selectRetaliation(const AiCreature &, const AiWorld &,
                            const std::function<bool(std::uint32_t)> &admit);
struct DecisionInputs {
  std::uint16_t cellFlags = 0;
  bool immediate = false, interaction = false, spell = false, priority = false,
       corpse21 = false, corpse10 = false, assist = false, score = false,
       home = false;
  std::int32_t hazard = 0;
  bool ownerEligible = true, humanOwner = false;
};
enum class AiEffect {
  immediate,
  escapePreparation,
  selector,
  hazard,
  substate,
  reset,
  clearReadiness,
  prepareInteraction,
  behavior,
  tailUpdate,
  attackResponse
};
struct AiEvent {
  AiEffect kind;
  std::int32_t value = 0;
  bool operator==(const AiEvent &o) const {
    return kind == o.kind && value == o.value;
  }
};
struct DecisionResult {
  AiCreature actor;
  std::vector<AiEvent> events;
  std::vector<std::uint32_t> draws;
};
// Whole 518420 orchestration with explicit selector/helper returns. Selectors
// below are independently recovered; interaction/spell/hazard effects are APIs.
DecisionResult decide(AiCreature, const DecisionInputs &,
                      const std::function<std::uint32_t()> &);
} // namespace mnm::reconstruction::combat
