#pragma once
#include "../combat/autonomy.hpp"
#include "descriptor.hpp"
namespace mnm::reconstruction::casting {
using combat::AutonomousCreature;
using combat::AutonomousWorld;
using combat::Cell;
using CastEffectDescriptor = std::array<std::uint32_t, 63>;
struct CastSpellConfiguration {
  std::uint32_t cost = 0, pathMode = 0, resourceMode = 0, resourceDelta = 0;
  std::int32_t travelKind = 0;
  std::array<std::array<std::uint32_t, 8>, 8> observed{};
};
struct CastLearningState {
  std::array<std::uint8_t, 124> known{};
  std::vector<std::array<std::uint32_t, 2>> notices;
};
struct CastAdmissionState {
  std::vector<CastSpellConfiguration> spells;
  std::vector<CastLearningState> profiles;
  std::vector<std::uint32_t> resource, direction;
  std::array<std::array<Cell, 8>, 28> muzzle{};
  std::uint32_t resourceCounter = 0, specialValue = 0, iteratorCursor = 0;
};
struct CastMovementDescriptor {
  std::uint32_t height = 1, width = 1, flight = 0, movementKind = 0,
                creatureType = 0;
  bool restricted = false;
};
struct CastAdmissionServices {
  std::function<std::optional<unsigned>()> pool, currentProfile, nextActor;
  std::function<void()> initializePath;
  std::function<void(AutonomousCreature &, unsigned, std::uint32_t)> visibility;
  std::function<bool(const AutonomousCreature &, Cell)> containsPoint;
  std::function<bool(const AutonomousCreature &)> chargeable, ownerMessage;
  std::function<void(AutonomousCreature &, std::uint32_t)> resource;
  std::function<void()> insufficientOwnerMessage;
  std::function<void(unsigned, Cell)> cue;
  std::function<void(unsigned, Cell, unsigned)> castCue;
  // FineXY constructor normalization happens before this service. Z is copied.
  // Word9/6/7/8 and flag1 are the original additional path arguments.
  std::function<bool(Cell, Cell, CastEffectDescriptor &)> path;
  std::function<void(unsigned, unsigned, const CastEffectDescriptor &)>
      secondary;
  std::function<bool(unsigned, const CastMovementDescriptor &, Cell &)> adjust;
  std::function<void(AutonomousCreature &, Cell, bool)> relocate;
  std::function<void(AutonomousCreature &, unsigned, unsigned)> areaEffect;
};
// Whole57b710 ordinary control flow with explicit lower services. Incidental
// EAX is not a Boolean result. Caller-staged state owns refusal/rollback.
void admitCast(const CastDescriptor &, AutonomousWorld &, CastAdmissionState &,
               const CastAdmissionServices &);
} // namespace mnm::reconstruction::casting
