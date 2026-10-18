#include "owner_brain_driver.hpp"
#include <algorithm>
#include <climits>
#include <set>
#include <stdexcept>
namespace mnm::reconstruction::combat {
void validateOwnerBrainDriver(const OwnerBrainDriver &s) {
  if (s.clock < 0 || s.phase > 4 || bool(s.phase) != s.current.has_value() ||
      s.execution.size() > 128 ||
      std::set<unsigned>(s.execution.begin(), s.execution.end()).size() !=
          s.execution.size())
    throw std::invalid_argument("owner brain driver continuation");
}
int ownerWorldBrainAllocation(int total) {
  if (total < 0 || total > 1000000)
    throw std::invalid_argument("owner World count allocation");
  return total / 2;
}
OwnerBrainBudget advanceOwnerBrainDriver(OwnerBrainDriver &s,
                                         OwnerSourceStorage &v, int total,
                                         const OwnerBrainDriverServices &p,
                                         bool disabled) {
  validateOwnerBrainDriver(s);
  if (total < 0 || total > 1000000 || s.clock == INT_MAX || !p.cleanup ||
      !p.snapshot || !p.sources || !p.hasTask || !p.plan || !p.tasks ||
      !p.execute)
    throw std::invalid_argument("owner brain driver services/allocation");
  ++s.clock;
  OwnerBrainBudget out{total / 2, total - total / 2};
  if (disabled)
    return out;
  bool fresh = !s.phase;
  if (fresh)
    p.cleanup();
  p.snapshot(fresh);
  if (v.pendingRoots.empty())
    v.pendingRoots = p.sources();
  while (true) {
    if (!s.phase) {
      if (!v.pending.general.empty())
        s.phase = 1;
      else if (!v.pending.wizard.empty())
        s.phase = 2;
      else if (!v.pending.other.empty())
        s.phase = 3;
      else if (!v.pendingRoots.empty())
        s.phase = 4;
      else
        break;
      s.current = s.phase == 1   ? v.pending.general.back()
                  : s.phase == 2 ? v.pending.wizard.back()
                  : s.phase == 3 ? v.pending.other.back()
                                 : v.pendingRoots.back();
    }
    const unsigned root = *s.current;
    const bool planned = s.phase != 1 || !p.hasTask(root);
    if (planned)
      p.plan(root, s.phase == 1, out.planning);
    // An already-installed general-queue source skips both planning and the
    // budget check in415c40; it is removed even when planning has zero units.
    if (planned && out.planning < 1)
      break;
    // A plan may remove its own root from a queue (a merge or421f70 install).
    auto erase = [&](auto &q) {
      q.erase(std::remove(q.begin(), q.end(), root), q.end());
    };
    if (s.phase == 1)
      erase(v.pending.general);
    else if (s.phase == 2)
      erase(v.pending.wizard);
    else if (s.phase == 3)
      erase(v.pending.other);
    else
      erase(v.pendingRoots);
    s.phase = 0;
    s.current.reset();
    if (!fresh) {
      p.snapshot(false);
      p.cleanup();
      p.snapshot(true);
      fresh = true;
    }
  }
  if (!fresh)
    p.snapshot(false);
  if (s.execution.empty())
    s.execution = p.tasks();
  while (out.execution > 0 && !s.execution.empty()) {
    unsigned task = s.execution.front();
    p.execute(task, out.execution);
    s.execution.erase(s.execution.begin());
  }
  validateOwnerBrainDriver(s);
  return out;
}
} // namespace mnm::reconstruction::combat
