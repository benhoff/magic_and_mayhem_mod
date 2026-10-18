#include "source_lifecycle.hpp"
#include <algorithm>
#include <limits>
#include <set>
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
constexpr unsigned retired = UINT32_MAX;
void need(bool value, const char *reason) {
  if (!value)
    throw std::invalid_argument(reason);
}
template <class C> void removeFrom(C &items, unsigned id) {
  items.erase(std::remove(items.begin(), items.end(), id), items.end());
}
void unqueue(OwnerSourceStorage &s, unsigned id) {
  removeFrom(s.pending.general, id);
  removeFrom(s.pending.wizard, id);
  removeFrom(s.pending.other, id);
  removeFrom(s.pendingRoots, id);
}
void attach(SourceGraph &g, unsigned parent, unsigned child) {
  need(!g.at(parent).actor && !g.at(child).parent, "source attachment domain");
  g[parent].children.push_back(child);
  g[child].parent = parent;
}
unsigned group(OwnerSourceStorage &s) {
  need(s.sources.graph.size() < 4096, "source storage capacity");
  const unsigned id = s.sources.graph.size();
  SourceGraphNode n;
  n.free.fill(1);
  s.sources.graph.push_back(n);
  SourceNodeInformation info;
  info.kind = s.manager.kind;
  s.sources.information.push_back(info);
  s.manager.nodes.push_back(id);
  return id;
}
void retireTask(OwnerSourceStorage &s, unsigned root,
                std::vector<unsigned> &out) {
  auto task = s.sources.information.at(root).recipient;
  if (task) {
    out.push_back(*task);
    unbindOwnerSourceTask(s, *task);
  }
}
} // namespace
void detachSourceChild(SourceGraph &g, unsigned parent, unsigned child) {
  auto &p = g.at(parent);
  (void)g.at(child);
  if (p.actor)
    return;
  const auto at = std::find(p.children.begin(), p.children.end(), child);
  if (at == p.children.end())
    return;
  const unsigned index = unsigned(at - p.children.begin());
  p.children.erase(at);
  for (unsigned i = 0; i < 8; ++i)
    if (!p.free[i] && index < p.position[i])
      --p.position[i];
  g[child].parent.reset();
}
void clearSourceChildren(SourceGraph &g, unsigned parent) {
  auto &p = g.at(parent);
  while (!p.children.empty()) {
    g.at(p.children.back()).parent.reset();
    p.children.pop_back();
  }
}
std::vector<unsigned> ownerSourceMembers(const SourceGraph &g, unsigned root,
                                         int mode) {
  std::vector<unsigned> out;
  std::set<unsigned> seen;
  std::vector<unsigned> stack{root};
  while (!stack.empty()) {
    const unsigned id = stack.back();
    stack.pop_back();
    need(id < g.size() && seen.insert(id).second, "source member graph");
    const auto &n = g[id];
    if (n.actor) {
      const auto activity =
          n.activity <= INT32_MAX ? int(n.activity) : -1 - int(~n.activity);
      if (activity >= mode)
        out.push_back(*n.actor);
    } else
      stack.insert(stack.end(), n.children.rbegin(), n.children.rend());
  }
  return out;
}
void validateOwnerSourceStorage(const OwnerSourceStorage &s, unsigned actors) {
  const auto &v = s.sources;
  need(s.manager.kind < 2 && v.graph.size() == v.information.size() &&
           v.graph.size() <= 4096 && v.roots.size() == actors &&
           v.actorFlag8e5.size() == actors,
       "source storage shape");
  std::set<unsigned> live(s.manager.nodes.begin(), s.manager.nodes.end());
  need(live.size() == s.manager.nodes.size(), "duplicate source storage node");
  std::set<unsigned> leaves, tasks;
  for (auto id : s.manager.nodes) {
    need(id < v.graph.size(), "source storage node identity");
    const auto &n = v.graph[id];
    need(v.information[id].kind == s.manager.kind && n.roundRobin < 8,
         "source storage node metadata");
    if (n.actor)
      need(*n.actor < actors && leaves.insert(*n.actor).second &&
               n.children.empty(),
           "source storage actor identity");
    if (n.parent) {
      need(live.count(*n.parent) && !v.graph[*n.parent].actor,
           "source storage parent");
      const auto &children = v.graph[*n.parent].children;
      need(std::count(children.begin(), children.end(), id) == 1,
           "source storage parent linkage");
    }
    for (auto child : n.children)
      need(live.count(child) && v.graph.at(child).parent == id,
           "source storage child linkage");
    for (unsigned i = 0; i < 8; ++i)
      need(n.free[i] <= 1 &&
               (n.actor || n.free[i] || n.position[i] <= n.children.size()),
           "source storage cursor");
    (void)ownerSourceMembers(v.graph, id, 0);
    if (auto task = v.information[id].recipient) {
      need(*task < v.recipientRoot.size() && v.recipientRoot[*task] == id &&
               tasks.insert(*task).second,
           "source storage task linkage");
    }
  }
  for (unsigned i = 0; i < actors; ++i)
    if (auto root = v.roots[i][s.manager.kind])
      need(live.count(*root) && v.graph[*root].actor == i,
           "source storage actor root");
  for (unsigned i = 0; i < v.recipientRoot.size(); ++i)
    need(v.recipientRoot[i] == retired || tasks.count(i), "orphan source task");
  auto queue = [&](const auto &q) {
    std::set<unsigned> seen;
    for (auto id : q)
      need(live.count(id) && seen.insert(id).second, "source queue identity");
  };
  queue(s.pending.general);
  queue(s.pending.wizard);
  queue(s.pending.other);
  queue(s.pendingRoots);
}
void bindOwnerSourceTask(OwnerSourceStorage &s, unsigned root, unsigned task) {
  need(task < 4096 && std::find(s.manager.nodes.begin(), s.manager.nodes.end(),
                                root) != s.manager.nodes.end(),
       "source task binding domain");
  auto &recipient = s.sources.information.at(root).recipient;
  need(!recipient, "source task already owned");
  if (task >= s.sources.recipientRoot.size())
    s.sources.recipientRoot.resize(task + 1, retired);
  need(s.sources.recipientRoot[task] == retired, "task already bound");
  recipient = task;
  s.sources.recipientRoot[task] = root;
}
void unbindOwnerSourceTask(OwnerSourceStorage &s, unsigned task) {
  need(task < s.sources.recipientRoot.size() &&
           s.sources.recipientRoot[task] != retired,
       "source task retirement identity");
  const unsigned root = s.sources.recipientRoot[task];
  need(s.sources.information.at(root).recipient == task,
       "source task retirement linkage");
  s.sources.information[root].recipient.reset();
  s.sources.recipientRoot[task] = retired;
}
OwnerSourceTopologyChange mergeOwnerSources(OwnerSourceStorage &s,
                                            unsigned left, unsigned right) {
  need(left != right && !s.sources.graph.at(left).parent &&
           !s.sources.graph.at(right).parent,
       "source merge roots");
  OwnerSourceTopologyChange out;
  retireTask(s, left, out.retiredTasks);
  retireTask(s, right, out.retiredTasks);
  unqueue(s, right);
  auto &g = s.sources.graph;
  if (!g[left].actor) {
    attach(g, left, right);
    out.root = left;
  } else if (!g[right].actor) {
    attach(g, right, left);
    out.root = right;
  } else {
    out.root = group(s);
    attach(g, out.root, left);
    attach(g, out.root, right);
  }
  return out;
}
OwnerSourceTopologyChange splitOwnerSource(OwnerSourceStorage &s,
                                           AutonomousWorld &world,
                                           unsigned root,
                                           const std::vector<unsigned> &moved) {
  const auto all = ownerSourceMembers(s.sources.graph, root, 2);
  need(!s.sources.graph.at(root).actor && !s.sources.graph[root].parent &&
           !moved.empty() && moved.size() < all.size(),
       "source split domain");
  std::set<unsigned> seen;
  for (auto leaf : moved) {
    need(leaf < s.sources.graph.size() && s.sources.graph[leaf].actor &&
             std::find(all.begin(), all.end(), *s.sources.graph[leaf].actor) !=
                 all.end() &&
             seen.insert(leaf).second,
         "source split leaf");
  }
  OwnerSourceTopologyChange out;
  retireTask(s, root, out.retiredTasks);
  out.root = group(s);
  for (auto leaf : moved) {
    const auto parent = s.sources.graph[leaf].parent;
    need(parent.has_value(), "source split unattached leaf");
    detachSourceChild(s.sources.graph, *parent, leaf);
    attach(s.sources.graph, out.root, leaf);
  }
  registerSourceRoot(world, s.sources, s.pending, out.root);
  return out;
}
std::vector<unsigned> pruneOwnerSources(OwnerSourceStorage &s,
                                        AutonomousWorld &world,
                                        std::optional<unsigned> current,
                                        std::optional<unsigned> other) {
  validateOwnerSourceStorage(s, world.creatures.size());
  std::vector<unsigned> removed;
  auto &v = s.sources;
  const int before = v.mode;
  v.mode = 0;
  for (unsigned index = 0; index < s.manager.nodes.size();) {
    const unsigned id = s.manager.nodes[index];
    bool protectedNode = false;
    auto ancestor = std::optional<unsigned>(id);
    while (ancestor) {
      protectedNode |= v.information.at(*ancestor).recipient.has_value();
      ancestor = v.graph.at(*ancestor).parent;
    }
    if (!protectedNode) {
      v.mode = 1;
      const unsigned cursor = beginSourceTraversal(v.graph, id);
      const bool populated =
          nextSourceActor(v.graph, id, cursor, v.mode).has_value();
      while (nextSourceActor(v.graph, id, cursor, v.mode)) {
      }
      v.mode = 0;
      protectedNode = populated;
      ancestor = id;
      while (ancestor) {
        protectedNode |= ancestor == current || ancestor == other;
        ancestor = v.graph.at(*ancestor).parent;
      }
    }
    if (protectedNode) {
      ++index;
      continue;
    }
    s.manager.nodes[index] = s.manager.nodes.back();
    s.manager.nodes.pop_back();
    unqueue(s, id);
    const unsigned cursor = beginSourceTraversal(v.graph, id);
    while (auto actor = nextSourceActor(v.graph, id, cursor, v.mode))
      if (v.roots.at(*actor)[s.manager.kind] == id)
        assignCreatureSourceRoot(world.creatures.at(*actor).ai.owner,
                                 s.manager.kind, {}, v.ownerFlag4,
                                 v.roots[*actor]);
    // 407650 initializes its child traversal, walks immediate children without
    // yielding their actors, then frees that cursor before detaching itself.
    const unsigned destruction = beginSourceTraversal(v.graph, id);
    v.graph[id].position[destruction] = v.graph[id].children.size();
    v.graph[id].free[destruction] = 1;
    if (auto parent = v.graph[id].parent)
      detachSourceChild(v.graph, *parent, id);
    clearSourceChildren(v.graph, id);
    removed.push_back(id);
  }
  v.mode = before;
  return removed;
}
} // namespace mnm::reconstruction::combat
