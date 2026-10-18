#pragma once
#include "../combat/autonomy.hpp"
#include <climits>

namespace mnm::reconstruction::combat {
struct WizardMana {
  unsigned kind = 0, owner = 0, value = 0, maximum = 0;
  std::array<InventoryEntry, 42> inventory{};
};
struct WizardManaProducer {
  unsigned owner = 0;
  bool active = false, eligible = false, producing = false;
};
// Whole 51e530 count/resource arithmetic. Producer eligibility (412aa0) and
// the rate/period configuration are explicit inputs, not guessed regeneration.
// Mana is fixed point (8 fractional bits); maximum and cost are whole units.
int wizardSpellWait(const WizardMana &, unsigned cost, unsigned count,
                    const std::vector<WizardManaProducer> &, int rate,
                    int period);
struct WizardSpellPeer {
  bool observed = false;
  int wait = INT_MAX;
};
// 40a950's present first-wizard/source and observed-peer minimum projection.
// An observed spell with an infinite wait differs from an unknown spell.
std::optional<int>
wizardSpellAvailability(bool present, unsigned firstKind,
                        const std::vector<WizardSpellPeer> &);
} // namespace mnm::reconstruction::combat
