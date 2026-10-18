#include "owner_planner.hpp"
#include <cmath>
#include <stdexcept>

namespace mnm::reconstruction::combat {
void validateOwnerTaskSelection(const OwnerTaskSelection &s, unsigned count) {
  if (count > 256 || s.evaluator > count || s.selected < -1 ||
      (s.selected >= 0 && unsigned(s.selected) >= count) ||
      !std::isfinite(s.value) || !std::isfinite(s.weight) || s.weight < 0 ||
      s.ties > 1000000)
    throw std::invalid_argument("owner task selection domain");
}
bool advanceOwnerTaskSelection(OwnerTaskSelection &s, int &budget,
                               RandomState &random,
                               const OwnerTaskSelectionServices &p, float bias,
                               bool save, bool rejectOne) {
  validateOwnerTaskSelection(s, p.count);
  if (!std::isfinite(bias) || !p.prepare || !p.next || !p.save || !p.finish)
    throw std::invalid_argument("owner task selection services");
  bool changed = false;
  auto choose = [&](unsigned i) {
    if (save) {
      p.save(i);
      s.selected = int(i);
      changed = true;
    }
  };
  if (s.reset) {
    s.reset = false;
    s.evaluator = 0;
    s.weight = 0;
    if (p.count)
      p.prepare(0);
    if (p.root && --budget < 1)
      return false;
  }
  unsigned i = s.evaluator;
  for (; i < p.count; ++i) {
    if (i != s.evaluator)
      p.prepare(i);
    for (auto candidate = p.next(i, budget); candidate;
         candidate = p.next(i, budget)) {
      if (!std::isfinite(*candidate) || *candidate < -1 || *candidate > 1)
        throw std::invalid_argument("owner task candidate value");
      if (rejectOne && *candidate == 1)
        continue;
      // The original stores the biased result back to a 32-bit float.
      const float value = *candidate + bias;
      if (!s.weighted || value == 1 || s.value == 1) {
        if (s.value <= value) {
          if (s.value < value)
            s.ties = 0;
          if (s.ties == 1000000)
            throw std::invalid_argument("owner task tie admission bound");
          ++s.ties;
          if ((nextRandom(random) & 0x7fffffffu) % s.ties == 0) {
            s.value = value;
            choose(i);
          }
        }
      } else {
        const float weight = value > -1 ? 1.f : 0.f;
        s.weight += weight;
        if (s.weight == 0 ||
            static_cast<long double>(nextRandom(random) & 0x7fffffffu) *
                    0x1p-31L <
                static_cast<long double>(weight) / s.weight) {
          if (save) {
            choose(i);
            if (s.value < value)
              s.value = value;
          }
        }
      }
    }
    if (budget < 1) {
      s.evaluator = i;
      return changed;
    }
  }
  s.evaluator = p.count;
  p.finish();
  s.reset = true;
  return changed;
}
} // namespace mnm::reconstruction::combat
