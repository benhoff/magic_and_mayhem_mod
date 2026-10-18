#include "wizard_spell_control.hpp"
#include "ownership_helpers.hpp"
#include <algorithm>
#include <stdexcept>

namespace mnm::reconstruction::combat {
namespace {
int signedWord(unsigned n) {
  return n <= unsigned(INT_MAX) ? int(n) : -1 - int(~n);
}
} // namespace
int wizardSpellWait(const WizardMana &a, unsigned cost, unsigned count,
                    const std::vector<WizardManaProducer> &producers, int rate,
                    int period) {
  if (a.owner >= 8 || producers.size() > 65536)
    throw std::invalid_argument("wizard mana projection extent");
  if (cost > a.maximum)
    return INT_MAX;
  const unsigned required = cost * 256u * count;
  unsigned available = a.value;
  if (signedWord(available) < signedWord(required))
    for (const auto &item : a.inventory)
      if (item.available && item.kind == 0x13)
        available += item.magnitude * 256u;
  const int deficit = signedWord(required - available);
  if (deficit <= 0)
    return 0;
  unsigned income = 0;
  if (isWizardKind(a.kind) && rate > 0) {
    for (const auto &p : producers) {
      if (p.owner >= 8)
        throw std::invalid_argument("wizard mana producer owner");
      if (p.active && p.eligible && p.producing && p.owner == a.owner)
        income += unsigned(rate) * 256u;
    }
    if (signedWord(income) > 0) {
      const int numerator = signedWord(income - 1u + unsigned(deficit));
      return signedWord(unsigned(numerator / signedWord(income)) *
                        unsigned(period));
    }
  }
  return INT_MAX;
}
std::optional<int>
wizardSpellAvailability(bool present, unsigned firstKind,
                        const std::vector<WizardSpellPeer> &peers) {
  if (!present || !isWizardKind(firstKind))
    return std::nullopt;
  std::optional<int> out;
  for (const auto &p : peers)
    if (p.observed)
      out = std::min(out.value_or(INT_MAX), p.wait);
  return out;
}
} // namespace mnm::reconstruction::combat
