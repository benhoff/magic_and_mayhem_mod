#pragma once
#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>
namespace mnm::reconstruction::combat {
struct Cell {
  std::int32_t x = 0, y = 0, z = 0;
  bool operator==(const Cell &o) const {
    return x == o.x && y == o.y && z == o.z;
  }
};
struct Extent {
  std::int32_t x = 32, y = 32, z = 8;
};
struct ReachActor {
  Cell position;
  std::int32_t fineHeight = 16, width = 1, height = 1;
  // Original uses target route endpoint XY/Z and supplied terrain fine height
  // while motion is active. Its current logical Z is still used for overlap.
  std::optional<Cell> endpoint;
  std::int32_t endpointTerrainHeight = 0;
};
struct ReachResult {
  bool admitted = false, diagonalChecked = false;
};
ReachResult meleeReach(const ReachActor &attacker, const ReachActor &target,
                       Extent, const std::function<bool(Cell, Cell)> &diagonal);
std::int32_t relativeDirection(Cell attacker, Cell target,
                               std::uint32_t targetFacing, Extent);
std::int32_t creatureDistance(Cell, Cell, Extent);
struct StunState {
  std::uint32_t type = 17, health = 360, mode = 0, behavior = 3, pending = 0,
                action = 10, engaged = 0, incoming = 0, marker = 7,
                readiness = 7, animationDirty = 0, animationKind = 0,
                configFlag = 0, form = 0, flight = 0, savedHeight = 0,
                blockDE3 = 7, block962 = 7, block963 = 7, block71c = 0,
                block723 = 0;
  std::int32_t timer = 0, target = 0;
  Cell position;
  bool attachment = false, queueEffects = false, surface = false;
};
enum class ControlEffect {
  reset,
  behavior,
  action,
  releaseAttachment,
  releaseQueue,
  surfaceRepair,
  formRestore,
  colorSet,
  colorClear,
  stunAudio,
  animation,
  knockbackMove
};
struct ControlEvent {
  ControlEffect kind;
  std::int32_t argument = 0;
  bool operator==(const ControlEvent &o) const {
    return kind == o.kind && argument == o.argument;
  }
};
struct StunResult {
  StunState state;
  std::vector<ControlEvent> events;
};
StunResult beginStun(StunState, std::int32_t duration, std::uint32_t mode,
                     bool forced = false);
StunResult tickStun(StunState, std::int32_t animationEvent);
StunResult knockbackResponse(StunState);
struct KnockbackInput {
  StunState actor;
  bool motion = false;
  std::uint32_t facing = 0, response = 0;
  Extent dimensions;
  // First eight offsets from the pinned route-neighbor table (Z unused).
  std::array<Cell, 8> offsets;
};
struct KnockbackResult {
  bool admitted = false;
  std::optional<Cell> requested;
  std::vector<Cell> checks;
};
KnockbackResult admitKnockback(const KnockbackInput &, std::uint32_t randomWord,
                               const std::function<bool(Cell)> &legal,
                               const std::function<bool(Cell)> &move);
struct ReadyResult {
  std::int32_t cooldown = 0;
  bool restartIdle = false;
  unsigned behaviorCalls = 0;
};
ReadyResult readyStep(std::int32_t cooldown, std::int32_t animationEvent,
                      const std::function<void(std::int32_t &)> &behavior);
} // namespace mnm::reconstruction::combat
