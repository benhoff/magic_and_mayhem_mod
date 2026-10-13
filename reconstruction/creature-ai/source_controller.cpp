#include "source_controller.hpp"
#include <limits>
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
int normalize(int v, int size) {
  if (size <= 0)
    throw std::invalid_argument("source coordinate extent");
  v %= size;
  return v < 0 ? v + size : v;
}
unsigned distance(Cell a, Cell b, Extent size) {
  auto axis = [](int x, int y, int n) {
    x = normalize(x, n);
    y = normalize(y, n);
    const int d = x >= y ? x - y : y - x;
    return unsigned(d > n / 2 ? n - d : d);
  };
  unsigned x = axis(a.x, b.x, size.x) << 5, y = axis(a.y, b.y, size.y) << 5;
  unsigned z = (unsigned(a.z) - unsigned(b.z)) << 4;
  return (x * x + y * y + z * z) >> 10;
}
} // namespace
void updateCreatureSourceController(unsigned slot, AutonomousWorld &world,
                                    SourceControllerState &state,
                                    SourceManager &manager,
                                    const SourceControllerConfiguration &cfg,
                                    const SourceControllerServices &s) {
  auto &actor = world.creatures.at(slot);
  if (actor.ai.owner < 0 || actor.ai.owner >= 8 || manager.kind >= 2 ||
      state.graph.size() != state.information.size())
    throw std::invalid_argument("source controller ownership domain");
  const unsigned owner = unsigned(actor.ai.owner);
  if ((state.controllers[owner] == -1 && !state.actorFlag8e5.at(slot)) ||
      state.ownerFlag4[owner] || actor.ai.flag704 || actor.ai.type == 22 ||
      (cfg.players > 1 && actor.ai.owner != cfg.networkOwner))
    return;
  std::uint8_t hint = cfg.initialHint25;
  auto active = [&](unsigned id) {
    const int before = state.mode;
    state.mode = 3;
    auto cursor = beginSourceTraversal(state.graph, id);
    auto first = nextSourceActor(state.graph, id, cursor, state.mode);
    hint = bool(first);
    // Preserve both the immediate second call and the drain, including their
    // cursor/free-mask writes. The later mode-restored lookup starts anew.
    while (nextSourceActor(state.graph, id, cursor, state.mode)) {
    }
    state.mode = before;
    return bool(first);
  };
  unsigned owned = 0;
  for (unsigned id : manager.nodes)
    if (!state.graph.at(id).parent && active(id)) {
      auto cursor = beginSourceTraversal(state.graph, id);
      auto source = nextSourceActor(state.graph, id, cursor, state.mode);
      if (!source)
        throw std::invalid_argument("source controller restored-mode lookup");
      if (world.creatures.at(*source).ai.owner == actor.ai.owner)
        ++owned;
    }
  const bool absent =
      creatureControllerAbsent(actor.ai.owner, state.controllers);
  bool fresh = absent;
  if (!absent && int(owned) < cfg.rootThreshold)
    fresh = (nextRandom(state.random) & 0x7fffffffu) % 2 == 0;
  unsigned minimum = 0x7fffffffu;
  std::optional<unsigned> selected;
  if (!fresh)
    for (unsigned id : manager.nodes)
      if (state.graph.at(id).actor && active(id)) {
        const unsigned peer = *state.graph[id].actor;
        if (world.creatures.at(peer).ai.owner == actor.ai.owner) {
          unsigned value = distance(world.creatures[peer].ai.position,
                                    actor.ai.position, world.dimensions);
          hint = cfg.coordinateHint25;
          auto root = terminalCreatureSourceRoot(state.graph, manager.kind,
                                                 state.roots.at(peer));
          if (value < minimum) {
            minimum = value;
            selected = root;
          }
        }
      }
  auto allocate = [&](std::optional<unsigned> source) {
    if (!s.allocate(0x8d))
      throw std::invalid_argument("source controller allocation failure");
    if (state.graph.size() >= UINT32_MAX)
      throw std::invalid_argument("source controller node capacity");
    unsigned id = state.graph.size();
    SourceGraphNode node;
    node.actor = source;
    node.free.fill(1);
    SourceNodeInformation info;
    info.kind = manager.kind;
    info.hint25 = hint;
    info.tag9 = source ? 0xffffffffu : 0;
    if (source)
      node.activity = 3;
    state.graph.push_back(std::move(node));
    state.information.push_back(info);
    manager.nodes.push_back(id);
    return id;
  };
  unsigned leaf = allocate(slot);
  assignCreatureSourceRoot(actor.ai.owner, manager.kind, leaf, state.ownerFlag4,
                           state.roots.at(slot));
  state.graph[leaf].activity = 1;
  unsigned chosen = leaf;
  if (!fresh && minimum != 0x7fffffffu) {
    if (!selected)
      throw std::invalid_argument("source controller selected null root");
    if (state.graph.at(*selected).actor) {
      if (manager.excluded[0] != selected && manager.excluded[1] != selected) {
        unsigned group = allocate({});
        auto attach = [&](unsigned child) {
          if (!state.graph[group].actor && !state.graph[child].parent) {
            state.graph[group].children.push_back(child);
            state.graph[child].parent = group;
          }
        };
        attach(*selected);
        attach(leaf);
        state.information[group].recipient =
            state.information[*selected].recipient;
        state.information[*selected].recipient.reset();
        if (state.information[group].recipient)
          state.recipientRoot.at(*state.information[group].recipient) = group;
        chosen = group;
      }
    } else {
      if (!state.graph[leaf].parent) {
        state.graph[*selected].children.push_back(leaf);
        state.graph[leaf].parent = *selected;
      }
      chosen = *selected;
    }
  }
  if (!state.information[chosen].recipient)
    s.registerRoot(chosen);
}
} // namespace mnm::reconstruction::combat
