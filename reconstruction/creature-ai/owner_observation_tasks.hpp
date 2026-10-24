#pragma once
#include "contribution_account.hpp"
#include "owner_knowledge_cache.hpp"
#include "owner_observation_snapshots.hpp"
#include <limits>
namespace mnm::reconstruction::combat {
// NoCD40209ca7 418860..41896b. Checked PE32 source/task storage; observers
// and task recipients are borrowed. Normal bounded acyclic source forests.
// Read-only contribution profiles replace the original global CFG provider.
template <class M>
std::uint32_t creditOwnerSighting(M &m, std::uint32_t seen, unsigned owner,
                                unsigned family,
                                const ContributionProfiles &profiles) {
  using U = std::uint32_t;
  if (owner >= 8 || family > 1)
    throw std::invalid_argument("sighting owner/family");
  U observer = m.get32(seen + 0x48 + 4 * owner);
  if (!observer)
    return 0;
  U root = m.get32(observer + 0x678 + 4 * family);
  if (!root || !OwnerTaskSources<M>(m).drain(root, 3))
    return 0;
  // 522770 follows the selected source's ancestry, rather than searching the
  // eligible actor's source or the observed actor's own task roots.
  U terminal = root;
  for (unsigned depth = 0;; ++depth) {
    if (depth >= 128)
      throw std::invalid_argument("sighting source ancestry");
    U parent = m.get32(terminal + 0x39);
    if (!parent)
      break;
    terminal = parent;
  }
  U task = m.get32(terminal + 5);
  if (!task)
    return 0;
  U health = m.get32(seen + 0xe4), type = m.get32(seen + 0xa8);
  if (!health || health > U(std::numeric_limits<int>::max()) || type >= 28)
    throw std::invalid_argument("sighting account domain");
  U kind = m.get32(m.get32(seen + 0xac));
  U cap = kind == 0 || kind == 24 || kind == 25 || kind == 26 ? 10 : 1;
  U amount = contributionAccount(profiles, type, int(health),
                                int(std::min(health, cap)), 0, 0, 0);
  // Original ADD is modulo2^32, independent of the logical task API's
  // separately documented overflow-refusal policy.
  m.set32(task + 0x2f, m.get32(task + 0x2f) + amount);
  return amount;
}
// Bounded headless composition: one enabled owner, at most one observed actor,
// increasing ticks and non-reentrant byte storage/profile services. Other
// owners' records must be empty. Native ordered snapshot index is a policy;
// this does not reproduce PE tree topology, whole-brain scheduling or World.
// Knowledge matching, owned value lifetime, cache invalidation, source graph
// eligibility, ancestry lookup and numeric valuation are composed here.
template <class M, class Actor>
void updateOwnerKnowledgeWithTasks(
    M &memory, OwnerKnowledge &knowledge, OwnerObservationSnapshots<M> &store,
    OwnerKnowledgeCache<M> &cache, int tick, Extent dimensions,
    const std::array<bool, 8> &enabled,
    const std::array<std::array<std::uint8_t, 8>, 8> &allied,
    const std::vector<OwnerObservation> &actors, unsigned family,
    const ContributionProfiles &profiles, Actor actor) {
  unsigned selected = 8, count = 0;
  for (unsigned i = 0; i < 8; ++i) {
    if (enabled[i]) { selected = i; ++count; }
    else if (!knowledge.records[i].empty())
      throw std::invalid_argument("sighting disabled owner records");
  }
  if (count != 1 || actors.size() > 1 || family > 1)
    throw std::invalid_argument("sighting composition domain");
  int next = knowledge.nextKey[selected];
  cache.invalidate();
  updateOwnerKnowledge(knowledge, tick, dimensions, enabled, allied, actors);
  store.reconcile(knowledge, tick, actor);
  // The admitted non-reentrant domain permits committing the logical update
  // and owned value before applying its new-insertion consequence. Refresh and
  // expiry never notify. No original output feeds this implementation.
  for (const auto &row : knowledge.records[selected])
    if (row.memory.key >= next)
      creditOwnerSighting(memory, actor(row.memory.actor.slot), selected,
                         family, profiles);
}
} // namespace mnm::reconstruction::combat
