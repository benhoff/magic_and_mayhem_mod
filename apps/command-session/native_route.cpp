#include "native_route.hpp"
#include "../../reconstruction/pathfinding/route_world.hpp"
#include <algorithm>
#include <cstring>
#include <stdexcept>
namespace mnm::command_session {
bool immediateNativeRoute(Continuation &c, const NativeCreatureProfile &p,
                          model::AutonomousCreature &a, model::Cell target,
                          unsigned mode) {
  auto &m = c.actions.motion.at(a.ai.slot);
  auto &r = c.actions.segments.at(a.ai.slot);
  auto &e = c.execution.at(a.ai.slot);
  std::array<unsigned, 131> words{};
  words[0] = unsigned(a.destination.x);
  words[1] = unsigned(a.destination.y);
  words[2] = unsigned(a.destination.z);
  words[3] = r.cursor;
  words[4] = r.count;
  for (unsigned i = 0; i < 16; ++i) {
    const auto &q = r.route[i];
    auto at = 5 + 7 * i;
    words[at] = unsigned(q.position.x);
    words[at + 1] = unsigned(q.position.y);
    words[at + 2] = unsigned(q.position.z);
    for (unsigned j = 0; j < 4; ++j)
      words[at + 3 + j] = unsigned(q.remainder[j]);
  }
  const std::array<unsigned, 14> motion{
      unsigned(m.progress),     unsigned(m.step),
      unsigned(m.travelX),      unsigned(m.travelY),
      unsigned(m.deltaX),       unsigned(m.deltaY),
      unsigned(m.heightDelta),  unsigned(m.heightStep),
      unsigned(m.heightOrigin), e.moveVariant,
      unsigned(m.duration),     unsigned(m.accumulator),
      e.routeChanged,           e.markerB73};
  std::copy(motion.begin(), motion.end(), words.begin() + 117);
  NativeCreatureGeometry geometry(c, p);
  auto snapshot = geometry.snapshot(a, target);
  std::memcpy(&snapshot->object.route, words.data(),
              sizeof(snapshot->object.route));
  auto neighbors = reconstruction::snapshot_neighbors(snapshot);
  auto world = reconstruction::neighbor_search_world(
      neighbors, {target.x, target.y, target.z}, a.ai.slot + 1);
  reconstruction::RouteContextPrefix context{};
  context.unknown_route_flag = 1;
  reconstruction::SearchState search;
  int budget = 250;
  reconstruction::route_search(context, snapshot->object, mode,
                               {target.x, target.y, target.z}, budget, search,
                               world);
  std::memcpy(words.data(), &context.route, sizeof(context.route));
  auto bits = [](unsigned v) {
    return v <= 0x7fffffffu ? int(v) : -1 - int(~v);
  };
  a.destination = {bits(words[0]), bits(words[1]), bits(words[2])};
  r.cursor = words[3];
  r.count = words[4];
  if (r.count > 16 || r.cursor > r.count)
    throw std::invalid_argument("immediate route extent");
  for (unsigned i = 0; i < 16; ++i) {
    auto &q = r.route[i];
    auto at = 5 + 7 * i;
    q.position = {bits(words[at]), bits(words[at + 1]), bits(words[at + 2])};
    for (unsigned j = 0; j < 4; ++j)
      q.remainder[j] = bits(words[at + 3 + j]);
  }
  m.progress = bits(words[117]);
  m.step = bits(words[118]);
  m.travelX = bits(words[119]);
  m.travelY = bits(words[120]);
  m.deltaX = bits(words[121]);
  m.deltaY = bits(words[122]);
  m.heightDelta = bits(words[123]);
  m.heightStep = bits(words[124]);
  m.heightOrigin = bits(words[125]);
  e.moveVariant = words[126];
  m.duration = bits(words[127]);
  m.accumulator = bits(words[128]);
  e.routeChanged = words[129];
  e.markerB73 = words[130];
  a.ai.marker = r.count ? 1 : 0;
  r.regionPending = false;
  return r.count != 0;
}
} // namespace mnm::command_session
