#include "owner_task_lifecycle.hpp"
#include <climits>
#include <cmath>
#include <stdexcept>
namespace mnm::reconstruction::combat {
bool admitOwnerTask6(const OwnerTask6Admission &a) {
  return a.enabled && a.brain == 0 && !a.parent && a.allAssigned &&
         (a.allOffSpecialCells || a.singletonWizardReady) && a.ownerAvailable &&
         !a.ownerDistressed;
}
void validateOwnerPreviousTask(const OwnerPreviousTask &a) {
  if (a.started < 0 || a.expires < a.started || !std::isfinite(a.value) ||
      a.value < -1 || a.value > 1 || !std::isfinite(a.baseline) ||
      a.baseline < 0 || a.baseline > 1000000 || a.successes < 0 ||
      a.successes >= INT_MAX || a.attempts < 0 || a.attempts >= INT_MAX)
    throw std::invalid_argument("owner previous task domain");
}
void validateOwnerReplacementPolicy(const OwnerReplacementPolicy &p) {
  if (!std::isfinite(p.improvement) || !std::isfinite(p.expiry) ||
      p.improvement < 0 || p.improvement > 1 || p.expiry < 0 || p.expiry > 1 ||
      p.ownerNumerator < 0 || p.ownerNumerator > 1000000 ||
      p.ownerDenominator < 0 || p.ownerDenominator == INT_MAX)
    throw std::invalid_argument("owner replacement policy domain");
}
long double ownerTaskProgress(const OwnerPreviousTask &a,
                              const OwnerReplacementPolicy &p) {
  validateOwnerPreviousTask(a);
  validateOwnerReplacementPolicy(p);
  const long double current = p.ownerAvailable
                                  ? static_cast<long double>(p.ownerNumerator) /
                                        (p.ownerDenominator + 1)
                                  : 0;
  // Exact pinned binary64 epsilon from5c5378, promoted to x87 precision.
  const long double epsilon = static_cast<long double>(0.001);
  return ((a.baseline + epsilon) *
          (static_cast<long double>(a.successes + 1) / (a.attempts + 1))) /
         (current + epsilon);
}
bool replaceOwnerTask(float candidate, const OwnerPreviousTask *a, int tick,
                      const OwnerReplacementPolicy &p) {
  validateOwnerReplacementPolicy(p);
  if (!std::isfinite(candidate) || candidate < -1 || candidate > 1 || tick < 0)
    throw std::invalid_argument("owner replacement candidate domain");
  if (!a)
    return true;
  validateOwnerPreviousTask(*a);
  if (tick < a->started)
    throw std::invalid_argument("owner replacement time domain");
  if (a->completed)
    return true;
  if (a->weighted && candidate != 1)
    return false;
  const long double denominator =
      1.L - static_cast<long double>(candidate) * a->value;
  if (denominator > 0 &&
      (static_cast<long double>(candidate) - a->value) / denominator >
          p.improvement)
    return true;
  if (a->value >= 1)
    return false;
  // The original pow wrapper stores both operands to binary64 before calling
  // the CRT. Native libm is compared by decision, not claimed bit-identical.
  const double age = double(static_cast<long double>(tick - a->started) /
                            (a->expires - a->started));
  return std::pow(double(ownerTaskProgress(*a, p)), age) < p.expiry &&
         candidate > 0;
}
int ownerTaskDuration(RandomState &r) {
  const unsigned word = nextRandom(r) & 0x7fffffffu;
  if (!word)
    throw std::invalid_argument("owner task zero exponential draw");
  const long double u = static_cast<long double>(word) * 0x1p-31L;
  long double logarithm;
#if defined(__i386__) || defined(__x86_64__)
  asm volatile("fldln2\n\tfldt %1\n\tfyl2x\n\tfstpt %0"
               : "=m"(logarithm)
               : "m"(u)
               : "st");
#else
  logarithm = std::log(u);
#endif
  const auto duration = -100.L * logarithm;
  if (!std::isfinite(duration) || duration >= INT_MAX)
    throw std::invalid_argument("owner task duration overflow");
  return int(duration);
}
bool advanceOwnerTaskReplanning(OwnerTaskSelection &s, int &budget,
                                RandomState &random,
                                const OwnerTaskSelectionServices &services,
                                const OwnerPreviousTask *old, int tick,
                                const OwnerReplacementPolicy &policy,
                                float bias, bool save, bool rejectOne) {
  validateOwnerReplacementPolicy(policy);
  if (old)
    validateOwnerPreviousTask(*old);
  if (!services.next || tick < 0 || (old && tick < old->started))
    throw std::invalid_argument("owner replanning services/time domain");
  auto filtered = services;
  filtered.next = [&](unsigned i, int &b) -> std::optional<float> {
    while (auto v = services.next(i, b))
      if (replaceOwnerTask(*v, old, tick, policy))
        return v;
    return {};
  };
  return advanceOwnerTaskSelection(s, budget, random, filtered, bias, save,
                                   rejectOne);
}
} // namespace mnm::reconstruction::combat
