#pragma once
#include "../../reconstruction/casting/admission.hpp"
#include "../../reconstruction/casting/cure_effect.hpp"
#include "../../reconstruction/casting/cure_services.hpp"
#include "../../reconstruction/combat/health.hpp"

namespace mnm::cure {
namespace combat = mnm::reconstruction::combat;
namespace audio = mnm::reconstruction::audio;
using mnm::reconstruction::casting::CurePosition;
using mnm::reconstruction::casting::CureEffect;
using mnm::reconstruction::casting::CastResource;
using mnm::reconstruction::casting::CureStatus;
using mnm::reconstruction::casting::CureStatusAudio;
using mnm::reconstruction::casting::CastLearningState;
struct OwnedCureActor {
  bool active = true;
  CurePosition position;
  combat::State health;
  std::uint32_t maximumHealth = 24, ongoingDamage = 0;
  CastResource resource;
  CureStatus status;
  std::array<std::uint32_t, 8> visible{};
};
struct CureEffectHandle {
  std::uint32_t slot = 0, generation = 0;
};
struct OwnedCureEffect {
  bool active = false, visible = false;
  std::uint32_t generation = 0;
  CureEffect effect;
};
struct CureSessionState {
  std::uint64_t tick = 0;
  std::uint32_t cost = 1, magnitude = 1, localOwner = 0;
  combat::Extent dimensions{32, 32, 4};
  std::vector<OwnedCureActor> actors;
  std::vector<OwnedCureEffect> effects;
  CureStatusAudio audio;
  CastLearningState learning;
  std::array<std::array<std::uint32_t, 8>, 8> observed{};
  std::uint32_t resourceCounter = 0;
};
struct CureSessionEvent {
  enum Kind : std::uint32_t {
    visibility, cue, castCue, resourceFirst, resourceSecond,
    health, feedback, created, transition, released, audioStatus, audioStop,
    audioPosition
  } kind = cue;
  std::uint32_t actor = UINT32_MAX, value = 0;
  CurePosition position;
};
// Animation result is an explicit input, not a guessed timer. Exactly1 completes
// deferred type77; all other results wait. Handles reject reused pool slots.
struct CureAnimationInput {
  CureEffectHandle effect;
  std::uint32_t result = 0;
};
struct CureAudioInput {
  std::int32_t statusResult = 0, stopResult = 0, positionResult = 0;
  std::uint32_t flags = 0;
};
// Headless owned composition, explicitly a native policy. First-free fixed pool,
// no spatial/render registration; explicit animation/backend replies. Reuses
// compared admission, Cure dispatch, ordinary positive health, resource and
// status helpers. Does not implement original creator/setup/release wholesale.
class CureSession {
  CureSessionState state_;
public:
  explicit CureSession(CureSessionState);
  const CureSessionState &state() const { return state_; }
  std::vector<CureSessionEvent> cast(std::uint32_t source,
                                    std::uint32_t target,
                                    std::uint32_t deliveryMode = 1);
  std::vector<CureSessionEvent> advance(const std::vector<CureAnimationInput> &,
                                       const std::vector<CureAudioInput> & = {});
  // Versioned little-endian, bounded, checksummed continuation. Restore validates
  // the complete candidate before publishing; no host pointers or Qt state.
  std::vector<std::uint8_t> checkpoint() const;
  static CureSession restore(const std::vector<std::uint8_t> &);
};
} // namespace mnm::cure
