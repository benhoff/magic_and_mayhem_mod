#pragma once
#include "../combat/autonomy.hpp"
namespace mnm::reconstruction::combat {
struct CreatureResourceState {
  std::uint32_t current = 0, maximum = 0;
  std::array<std::uint32_t, 8> ownerSnapshot{};
};
struct CreatureResourceServices {
  std::function<void(unsigned)> primaryDisplay, secondaryDisplay;
};
// Whole507f00: current/max are logical E8/EC values. The second display
// callback reads current again after the first; dirty is the owned92e word.
void changeCreatureResource(AutonomousCreature &, CreatureResourceState &,
                            unsigned &dirty, unsigned delta, int localOwner,
                            const CreatureResourceServices &);
// Whole507fa0 has a signed capacity clamp for a foreign-owner update, unlike
// changeCreatureResource's unsigned capacity clamp. Own-owner ignores delta.
void updateCreatureResourceSnapshot(const AutonomousCreature &,
                                    CreatureResourceState &, int owner,
                                    unsigned delta);
struct AssociatedOwnerRecord {
  std::uint32_t enabled = 0, actor = 0;
};
// Whole466870 returns an index even for an inactive/dead actor. Owner-1,
// disabled records and unsigned out-of-capacity indices return no subject.
std::optional<unsigned>
associatedCreature(int owner, const std::array<AssociatedOwnerRecord, 8> &,
                   unsigned capacity);
} // namespace mnm::reconstruction::combat
