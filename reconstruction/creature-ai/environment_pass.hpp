#pragma once
#include "../combat/autonomy.hpp"
namespace mnm::reconstruction::combat {
enum class EnvironmentFilter : unsigned { none, living };
struct EnvironmentPassState {
  std::uint32_t cursor = 0;
  EnvironmentFilter filter = EnvironmentFilter::none;
  std::int32_t receiverAdjustment = 0;
};
struct EnvironmentPassConfiguration {
  std::int32_t waterline = 0;
  std::uint32_t damage = 0;
};
struct EnvironmentPassServices {
  std::function<bool(const AutonomousCreature &, Cell)> water;
  std::function<void(AutonomousCreature &, int, int, bool)> health;
};
// 52a9f0 defined movement kinds0..3. Other kinds retain an uninitialized/prior
// original latch and are refused. Actor storage remains stable during
// callbacks.
void creatureEnvironmentPass(AutonomousWorld &, EnvironmentPassState &,
                             const EnvironmentPassConfiguration &,
                             const EnvironmentPassServices &);
} // namespace mnm::reconstruction::combat
