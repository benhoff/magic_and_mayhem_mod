#pragma once
#include "../combat/melee.hpp"
#include <functional>
#include <optional>

namespace mnm::reconstruction::combat {
// Logical projection of 4284e0. Evaluator-specific candidate enumeration,
// preparation and saving stay explicit services, not opaque host pointers.
struct OwnerTaskSelection {
  bool reset = true, weighted = false;
  unsigned evaluator = 0, ties = 0;
  int selected = -1;
  float value = -2, weight = 0;
};
struct OwnerTaskSelectionServices {
  unsigned count = 0;
  bool root = true;
  std::function<void(unsigned)> prepare;
  std::function<std::optional<float>(unsigned, int &budget)> next;
  std::function<void(unsigned)> save;
  std::function<void()> finish;
};
// Taskless old-plan domain, count-based budget (6e1f64=0), finite [-1,1]
// candidate values, optional additive bias and rejection of exact score 1.
// true means this slice saved a candidate; reset indicates completed sweep.
bool advanceOwnerTaskSelection(OwnerTaskSelection &, int &budget, RandomState &,
                               const OwnerTaskSelectionServices &,
                               float bias = 0, bool save = true,
                               bool rejectOne = false);
void validateOwnerTaskSelection(const OwnerTaskSelection &,
                                unsigned evaluators);
} // namespace mnm::reconstruction::combat
