#include "source_graph.hpp"
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
int word(unsigned v) { return v <= 0x7fffffffu ? int(v) : -1 - int(~v); }
void ownerDomain(int owner) {
  if (owner < 0 || owner >= 8)
    throw std::invalid_argument("source graph owner");
}
void familyDomain(unsigned family) {
  if (family >= 2)
    throw std::invalid_argument("source graph family");
}
unsigned begin(SourceGraph &g, unsigned id, unsigned depth) {
  if (depth >= g.size())
    throw std::invalid_argument("cyclic source children");
  auto &n = g.at(id);
  unsigned slot = 0;
  while (slot < 8 && !n.free[slot])
    ++slot;
  if (slot == 8) {
    if (n.roundRobin >= 8)
      throw std::invalid_argument("source graph round robin");
    slot = n.roundRobin;
    n.roundRobin = (slot + 1) & 7;
  }
  n.position[slot] = 0;
  n.nested[slot] =
      n.children.empty() ? 0 : begin(g, n.children.front(), depth + 1);
  n.free[slot] = 0;
  return slot;
}
std::optional<unsigned> next(SourceGraph &g, unsigned id, unsigned slot,
                             int mode, unsigned depth) {
  if (depth >= g.size() || slot >= 8)
    throw std::invalid_argument("source graph traversal");
  auto &n = g.at(id);
  if (n.actor) {
    if (word(n.activity) >= mode && !n.nested[slot]) {
      n.nested[slot] = 1;
      return n.actor;
    }
  } else {
    while (n.position[slot] != n.children.size()) {
      const auto child = n.children.at(n.position[slot]);
      auto actor = next(g, child, n.nested[slot], mode, depth + 1);
      if (actor)
        return actor;
      ++n.position[slot];
      if (n.position[slot] != n.children.size())
        n.nested[slot] = begin(g, n.children[n.position[slot]], depth + 1);
    }
  }
  n.free[slot] = 1;
  return {};
}
} // namespace
unsigned beginSourceTraversal(SourceGraph &g, unsigned id) {
  return begin(g, id, 0);
}
std::optional<unsigned> nextSourceActor(SourceGraph &g, unsigned id,
                                        unsigned slot, int mode) {
  return next(g, id, slot, mode, 0);
}
void assignCreatureSourceRoot(int owner, unsigned family,
                              std::optional<unsigned> root,
                              const std::array<unsigned, 8> &flags,
                              std::array<std::optional<unsigned>, 2> &roots) {
  ownerDomain(owner);
  familyDomain(family);
  roots[family] = flags[unsigned(owner)] && root ? std::nullopt : root;
}
std::optional<unsigned> terminalCreatureSourceRoot(
    const SourceGraph &g, unsigned family,
    const std::array<std::optional<unsigned>, 2> &roots) {
  familyDomain(family);
  auto root = roots[family];
  unsigned steps = 0;
  while (root) {
    if (++steps > g.size())
      throw std::invalid_argument("cyclic source ancestry");
    const auto &n = g.at(*root);
    if (!n.parent)
      return root;
    root = n.parent;
  }
  return {};
}
bool creatureControllerAbsent(int owner,
                              const std::array<int, 8> &controllers) {
  ownerDomain(owner);
  return controllers[unsigned(owner)] == -1;
}
} // namespace mnm::reconstruction::combat
