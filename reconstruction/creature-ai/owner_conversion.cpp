#include "owner_conversion.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
int axis(int a, int b, int size) {
  if (size <= 0 || a < 0 || a >= size || b < 0 || b >= size)
    throw std::invalid_argument("conversion coordinate domain");
  int difference = a >= b ? a - b : b - a;
  return difference > size / 2 ? size - difference : difference;
}
} // namespace
void scanOwnerConversion(AutonomousCreature &source, AutonomousWorld &world,
                         OwnerConversionState &state,
                         const OwnerConversionServices &s) {
  const auto subject = s.subject(source.ai.owner);
  auto full = [&] {
    if (!subject)
      return false;
    const auto &limit = state.limits.at(*subject);
    return limit.current >= limit.maximum;
  };
  if (full())
    return;
  int cursor = state.cursor.at(source.ai.slot);
  if (cursor < 0 || cursor >= 125)
    throw std::invalid_argument("conversion scan cursor");
  const int stop = int(std::min(unsigned(cursor) + 10u, 125u));
  while (cursor < stop) {
    auto &candidate = world.creatures.at(unsigned(cursor));
    if (candidate.ai.active && candidate.ai.health) {
      if (source.ai.owner < 0 || source.ai.owner >= 8 ||
          candidate.ai.owner < 0 || candidate.ai.owner >= 8)
        throw std::invalid_argument("conversion owner domain");
      if (!world.relation[unsigned(source.ai.owner)]
                         [unsigned(candidate.ai.owner)] &&
          candidate.ai.type == source.ai.type) {
        const int x = axis(source.ai.position.x, candidate.ai.position.x,
                           world.dimensions.x);
        const int y = axis(source.ai.position.y, candidate.ai.position.y,
                           world.dimensions.y);
        const int distance = std::max(x, y) + (x >= y ? x - y : y - x) / 2;
        if (distance <= 2) {
          s.notify(candidate.ai.slot);
          s.transfer(candidate, source.ai.owner);
          if (full())
            break;
        }
      }
    }
    ++cursor;
  }
  state.cursor.at(source.ai.slot) = unsigned(cursor) >= 125 ? 0 : cursor;
}
} // namespace mnm::reconstruction::combat
