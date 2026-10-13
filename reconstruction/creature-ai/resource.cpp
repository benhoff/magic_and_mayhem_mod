#include "resource.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
int signedWord(unsigned v) { return v <= 0x7fffffffu ? int(v) : -1 - int(~v); }
bool wizardKind(unsigned v) { return v == 0 || v == 24 || v == 25 || v == 26; }
} // namespace
void changeCreatureResource(AutonomousCreature &a, CreatureResourceState &state,
                            unsigned &dirty, unsigned delta, int localOwner,
                            const CreatureResourceServices &s) {
  const unsigned before = state.current, sum = before + delta;
  state.current = std::min(signedWord(sum) < 0 ? 0u : sum, state.maximum << 8);
  if (wizardKind(a.ai.kind) && !a.block7c && a.ai.owner == localOwner &&
      state.current != before) {
    s.primaryDisplay(state.current >> 8);
    s.secondaryDisplay(state.current >> 8);
  }
  dirty = 1;
}
void updateCreatureResourceSnapshot(const AutonomousCreature &a,
                                    CreatureResourceState &state, int owner,
                                    unsigned delta) {
  if (owner < 0 || owner >= 8)
    throw std::invalid_argument("resource snapshot owner domain");
  auto &snapshot = state.ownerSnapshot[unsigned(owner)];
  if (owner != a.ai.owner) {
    const unsigned sum = snapshot + delta;
    const unsigned positive = signedWord(sum) < 0 ? 0 : sum;
    const unsigned limit = state.maximum << 8;
    snapshot = signedWord(limit) < signedWord(positive) ? limit : positive;
  } else {
    unsigned aggregate = state.current;
    for (const auto &item : a.inventory)
      if (item.available && item.kind == 19)
        aggregate += item.magnitude << 8;
    snapshot = aggregate;
  }
}
std::optional<unsigned>
associatedCreature(int owner,
                   const std::array<AssociatedOwnerRecord, 8> &records,
                   unsigned capacity) {
  if (owner == -1)
    return {};
  if (owner < 0 || owner >= 8)
    throw std::invalid_argument("associated actor owner domain");
  const auto &record = records[unsigned(owner)];
  return record.enabled && record.actor < capacity
             ? std::optional<unsigned>(record.actor)
             : std::nullopt;
}
} // namespace mnm::reconstruction::combat
