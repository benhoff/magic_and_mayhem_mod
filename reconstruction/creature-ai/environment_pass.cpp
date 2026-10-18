#include "environment_pass.hpp"
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
int word(unsigned n) { return n <= 0x7fffffffu ? int(n) : -1 - int(~n); }
int normalized(int n, int size) {
  n %= size;
  return n < 0 ? n + size : n;
}
} // namespace
void creatureEnvironmentPass(AutonomousWorld &world,
                             EnvironmentPassState &state,
                             const EnvironmentPassConfiguration &configuration,
                             const EnvironmentPassServices &s) {
  if (world.dimensions.x <= 0 || world.dimensions.y <= 0 ||
      world.creatures.size() > UINT32_MAX)
    throw std::invalid_argument("environment owned world extent");
  const int waterline = configuration.waterline;
  state.cursor = 0;
  state.filter = EnvironmentFilter::living;
  state.receiverAdjustment = 0;
  while (state.cursor < world.creatures.size()) {
    auto &a = world.creatures[state.cursor++];
    if (!a.ai.active || !a.ai.health)
      continue;
    if (a.movementKind > 3)
      throw std::invalid_argument(
          "undefined original environment latch domain");
    const int top = word(a.height + unsigned(a.ai.position.z) - 1u);
    bool harm = false;
    switch (a.movementKind) {
    case 0:
    case 1:
      harm = top < waterline;
      break;
    case 2: {
      const int edge = word(unsigned(waterline) - 1u);
      if (top < edge)
        harm = true;
      else if (top == edge) {
        const Cell point{normalized(a.ai.position.x, world.dimensions.x),
                         normalized(a.ai.position.y, world.dimensions.y),
                         word(unsigned(a.ai.position.z) + 1u)};
        harm = !s.water(a, point);
      }
      break;
    }
    default:
      break;
    }
    if (harm)
      s.health(a, word(0u - configuration.damage), -1, true);
  }
}
} // namespace mnm::reconstruction::combat
