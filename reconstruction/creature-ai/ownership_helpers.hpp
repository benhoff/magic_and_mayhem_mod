#pragma once
#include "execution.hpp"
namespace mnm::reconstruction::combat {
struct CreatureOwnerVisibility {
  std::array<unsigned, 8> secondary{};
};
using OwnerVisionSharing = std::array<std::array<std::uint8_t, 8>, 8>;
// Whole411e50 and407b90. Activity89 is a finite state word, not a refcount.
bool isWizardKind(unsigned kind);
void clearRootActivityFlag(unsigned &activity);
// Whole51a880 sets own/shared-allied visibility30, otherwise0, then clears
// the separate48..64 words. Original neutral-owner null alias is excluded.
void refreshCreatureOwnerVisibility(AutonomousCreature &,
                                    const AutonomousWorld &,
                                    const OwnerVisionSharing &,
                                    CreatureOwnerVisibility &);
// Whole512340 clears only kind2 marker tokens, loading later tokens after
// releases. Caller keeps order storage/shape stable during reentry.
void clearQueuedMoveMarkers(CreatureExecution &,
                            const std::function<void(unsigned)> &release);
} // namespace mnm::reconstruction::combat
