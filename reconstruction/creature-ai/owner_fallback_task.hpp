#pragma once
#include <optional>
namespace mnm::reconstruction::combat {
// Original kind0:415640,415670,4156a0,419900. Count budget is untouched.
struct OwnerFallbackCandidate {
  bool eligible = false, emitted = false;
};
inline void prepareOwnerFallback(OwnerFallbackCandidate &s, bool parent) {
  s.eligible = !parent;
  if (s.eligible)
    s.emitted = false;
}
inline std::optional<float> nextOwnerFallback(OwnerFallbackCandidate &s) {
  if (!s.eligible || s.emitted)
    return {};
  s.emitted = true;
  return -1.f;
}
constexpr int ownerFallbackDuration() { return 8; }
constexpr bool executeOwnerFallback() { return true; }
} // namespace mnm::reconstruction::combat
