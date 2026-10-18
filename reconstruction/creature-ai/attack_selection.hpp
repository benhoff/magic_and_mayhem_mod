#pragma once
#include "../combat/control.hpp"
#include <optional>
namespace mnm::reconstruction::combat {
struct AttackSelectionState {
  Cell position;
  unsigned width=1, direction=0, facing=0, animationKind=0, offset=0;
  bool reverse=false, attached=false, secondary=false;
};
struct AttackSelectionTarget { Cell position; unsigned width=1; };
// Position is the queried route cell; the object record supplies only kind.
struct AttackSelectionObject { Cell position; unsigned kind=26; };
enum class AttackSelectionEffect { visualRepair, attachmentFacing, secondarySwitch, play };
struct AttackSelectionEvent {
  AttackSelectionEffect kind;
  unsigned argument=0;
  bool operator==(const AttackSelectionEvent &other) const {
    return kind==other.kind && argument==other.argument;
  }
};
struct AttackSelectionResult {
  AttackSelectionState state;
  std::vector<AttackSelectionEvent> events;
};
// Whole50e400 within canonical dimensions and typed object lookup. Object pool
// lifetime, controller effects and invalid original pointers are separate.
AttackSelectionResult selectAttackAnimation(AttackSelectionState, Extent,
    unsigned cameraTurn, std::optional<AttackSelectionTarget>,
    std::optional<AttackSelectionObject> = {});
// Whole510480 boolean projection; no world or controller effects.
bool corpseFlightAnimation(unsigned health,unsigned corpseA,unsigned corpseB);
} // namespace mnm::reconstruction::combat
