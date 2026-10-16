#pragma once
#include "../../game/persistence/snapshot.hpp"
#include "../../reconstruction/casting/cure_presentation.hpp"
#include "../cure-lab/session.hpp"

namespace mnm::cure {
using mnm::reconstruction::casting::CureAnimationDefinition;
using mnm::reconstruction::casting::CurePresentationState;
// World owns identities/positions and State.systems owns the versioned Cure
// continuation plus selected ANI records/cursors. No external asset is needed
// to resume. This consumer exclusively owns systems; AI/campaign composition
// and original real-time tick cadence are separate integration boundaries.
class CureWorldSession {
  game::World world_;

public:
  CureWorldSession(CureSessionState, CureAnimationDefinition,
                   std::uint32_t capacity = 64);
  explicit CureWorldSession(game::World);
  const game::World &world() const { return world_; }
  CureSessionState state() const;
  CurePresentationState presentation() const;
  game::Handle actor(std::uint32_t index) const;
  std::optional<game::Handle> effect(std::uint32_t index) const;
  std::vector<CureSessionEvent> cast(game::Handle source, game::Handle target,
                                     std::uint32_t deliveryMode = 1);
  std::vector<CureSessionEvent>
      advance(game::TickInput = {}, const std::vector<CureAudioInput> & = {});
  // Semantic fixture/host inputs, staged against exact live World identities.
  // Position input is a native relocation policy, not recovered path movement.
  void relocate(game::Handle actor, game::Point);
  void changeHealth(game::Handle actor, std::int32_t delta);
  void enqueue(game::Command);
  game::Bytes checkpoint() const;
  static CureWorldSession restore(const game::Bytes &);
};
} // namespace mnm::cure
