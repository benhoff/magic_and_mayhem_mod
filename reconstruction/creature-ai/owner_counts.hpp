#pragma once
#include "owner_conversion.hpp"
#include "resource.hpp"
namespace mnm::reconstruction::combat {
struct OwnerCountServices {
  std::function<void(AutonomousCreature &, unsigned)> display;
  std::function<void(AutonomousCreature &)> belowLimit, atLimit;
};
// Whole522340 uses whole466870 for lookup. The associated actor is cached;
// it counts active/living same-owner actors excluding its own slot and type27.
// UI callbacks may change its count/limit before the signed threshold reload.
void recountOwnerCreatures(const AutonomousCreature &source, AutonomousWorld &,
                           const std::array<AssociatedOwnerRecord, 8> &,
                           std::vector<CreatureControlLimit> &, int localOwner,
                           const OwnerCountServices &);
} // namespace mnm::reconstruction::combat
