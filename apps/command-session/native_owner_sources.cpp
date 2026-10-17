#include "native_owner_sources.hpp"
#include <algorithm>
#include <set>
#include <stdexcept>
namespace mnm::command_session {
namespace {
void need(bool b, const char *why) {
  if (!b)
    throw std::invalid_argument(why);
}
void retire(NativeOwnerSources &s, unsigned task) {
  auto &row = s.planners.at(task);
  if (row.planner) {
    auto &p = *row.planner;
    p.active.reset();
    p.activeSerial = 0;
    auto &g = *p.grouped;
    g.active.reset();
    g.replanning->previous.reset();
    *g.scheduler = {};
  }
}
void synchronize(Continuation &c) {
  auto &s = *c.native.ownerSources;
  auto &storage = s.storage;
  const auto &base = *c.native.ownerPlanner;
  for (auto &row : s.planners)
    if (row.planner &&
        (std::find(storage.manager.nodes.begin(), storage.manager.nodes.end(),
                   row.source) == storage.manager.nodes.end() ||
         storage.sources.graph.at(row.source).parent)) {
      const unsigned id = unsigned(&row - s.planners.data());
      if (storage.sources.information[row.source].recipient == id)
        model::unbindOwnerSourceTask(storage, id);
      row.planner.reset();
    }
  for (auto root : storage.manager.nodes) {
    if (storage.sources.graph[root].parent)
      continue;
    auto members = model::ownerSourceMembers(storage.sources.graph, root, 3);
    if (members.empty())
      continue;
    auto at = std::find_if(
        s.planners.begin(), s.planners.end(),
        [&](const auto &row) { return row.planner && row.source == root; });
    if (at == s.planners.end()) {
      need(s.planners.size() < 4096, "native source planner capacity");
      s.planners.push_back({root, base});
      at = s.planners.end() - 1;
      at->planner->members = members;
      at->planner->actor = members.front();
    } else if (at->planner->members != members) {
      // Native origin/member-epoch restart. Keep installed task accounting;
      // discard only an incomplete candidate made from a different source.
      auto &p = *at->planner;
      p.members = members;
      p.actor = members.front();
      p.selection = {};
      p.candidate = {};
      p.candidateReady = false;
      p.features.clear();
      p.grouped->candidate.reset();
      p.grouped->best.reset();
      p.grouped->groups = {};
      p.grouped->observations.clear();
      p.grouped->cursor = 0;
      p.grouped->probe = {};
      p.grouped->from = p.grouped->to = -1;
      p.grouped->replanning->prepared = false;
      p.grouped->replanning->eligible = false;
      p.grouped->scheduler->planningPending = false;
    }
  }
}
} // namespace
std::vector<unsigned> nativeOwnerSourceActors(const NativeOwnerPlanner &s) {
  return s.members.empty() ? std::vector<unsigned>{s.actor} : s.members;
}
void prepareNativeOwnerSources(Continuation &c) {
  need(c.native.ownerPlanner && !c.native.ownerSources && !c.native.tick,
       "native source setup requires fresh planner");
  auto staged = c;
  auto &s = staged.native.ownerSources.emplace();
  auto &v = s.storage.sources;
  v.roots.resize(c.world.creatures.size());
  v.actorFlag8e5.assign(c.world.creatures.size(), 1);
  v.controllers.fill(0);
  v.mode = 3;
  v.random = c.native.ownerPlanner->random;
  model::SourceControllerConfiguration cfg;
  for (unsigned i = 0; i < c.world.creatures.size(); ++i)
    if (c.world.creatures[i].ai.owner == int(c.native.ownerPlanner->owner)) {
      model::updateCreatureSourceOwned(i, staged.world, v, s.storage.manager,
                                       s.storage.pending, cfg,
                                       [](unsigned) { return true; });
      const auto leaf = v.roots[i][0];
      need(leaf.has_value(), "native ordinary source creation refused");
      // Source availability producer is an explicit native host policy.
      v.graph[*leaf].activity = 3;
    }
  synchronize(staged);
  validate(staged);
  c = std::move(staged);
}
void validateNativeOwnerSources(const Continuation &c) {
  if (!c.native.ownerSources)
    return;
  const auto &s = *c.native.ownerSources;
  need(c.native.ownerPlanner && c.native.ownerPlanner->grouped &&
           !c.native.ownerPlanner->active &&
           c.native.ownerPlanner->members.empty() &&
           c.native.ownerPlanner->selection.reset && s.nextSerial &&
           s.planners.size() <= 128 && s.storage.sources.graph.size() <= 128 &&
           s.storage.sources.recipientRoot.size() <= 128,
       "native source planner template");
  model::validateOwnerSourceStorage(s.storage, c.world.creatures.size());
  need(!s.storage.manager.kind && s.storage.sources.mode == 3,
       "native ordinary source family");
  const auto &random = s.storage.sources.random;
  need(random.left < 250 && random.right < 250 &&
           (random.right + 250 - random.left) % 250 == 103,
       "native source shared random state");
  for (auto id : s.storage.manager.nodes) {
    if (auto actor = s.storage.sources.graph[id].actor)
      need(c.world.creatures.at(*actor).ai.owner ==
               int(c.native.ownerPlanner->owner),
           "native source foreign actor");
    if (auto task = s.storage.sources.information[id].recipient)
      need(*task < s.planners.size() && s.planners[*task].planner &&
               s.planners[*task].source == id,
           "native source orphan task binding");
  }
  std::set<unsigned> roots, serials;
  for (unsigned i = 0; i < s.planners.size(); ++i) {
    const auto &row = s.planners[i];
    if (!row.planner)
      continue;
    need(std::find(s.storage.manager.nodes.begin(),
                   s.storage.manager.nodes.end(),
                   row.source) != s.storage.manager.nodes.end() &&
             !s.storage.sources.graph.at(row.source).parent &&
             roots.insert(row.source).second,
         "native source planner root");
    const auto &p = *row.planner;
    need(!p.members.empty() && p.actor == p.members.front() &&
             p.owner == c.native.ownerPlanner->owner &&
             p.activeSerial < s.nextSerial && p.grouped &&
             p.grouped->scheduler && p.grouped->replanning &&
             p.grouped->replanning->eventInputs,
         "native source planner identity");
    need(p.members ==
             model::ownerSourceMembers(s.storage.sources.graph, row.source, 3),
         "native source member traversal differs");
    if (p.activeSerial)
      need(serials.insert(p.activeSerial).second,
           "duplicate native source task serial");
    for (auto actor : p.members) {
      const auto &roots = c.actions.segments.at(actor).contributionRoots;
      need(!roots[0] && !roots[1],
           "native source member has duplicate contribution roots");
    }
    const auto recipient = s.storage.sources.information[row.source].recipient;
    need(bool(p.active) == recipient.has_value() &&
             (!recipient || *recipient == i),
         "native source task ownership");
  }
}
void updateNativeOwnerSources(Continuation &c,
                              const NativeCreatureProfile &profile,
                              model::OwnedCreatureCommands &commands) {
  auto &s = *c.native.ownerSources;
  auto &storage = s.storage;
  for (auto id : storage.manager.nodes)
    if (auto actor = storage.sources.graph[id].actor) {
      const auto &a = c.world.creatures.at(*actor).ai;
      storage.sources.graph[id].activity = a.active && a.health ? 3 : 0;
    }
  synchronize(c);
  // Preserve pending source classifications in checkpoints. The native host
  // folds ordinary queue notifications into root work, then visits one root
  // per frame. Original specialized421f70 and World allocation are separate.
  auto enqueue = [&](unsigned source) {
    auto root = std::optional<unsigned>(source);
    while (root && storage.sources.graph[*root].parent)
      root = storage.sources.graph[*root].parent;
    if (root &&
        std::find(storage.pendingRoots.begin(), storage.pendingRoots.end(),
                  *root) == storage.pendingRoots.end())
      storage.pendingRoots.push_back(*root);
  };
  for (auto *q : {&storage.pending.general, &storage.pending.other})
    while (!q->empty()) {
      enqueue(q->back());
      q->pop_back();
    }
  if (storage.pendingRoots.empty())
    for (const auto &row : s.planners)
      if (row.planner)
        enqueue(row.source);
  // Active tasks on empty sources retain their original expiry/progress state;
  // they are retired by the host when no member can execute or plan.
  for (unsigned i = 0; i < s.planners.size(); ++i) {
    auto &row = s.planners[i];
    if (row.planner &&
        model::ownerSourceMembers(storage.sources.graph, row.source, 3)
            .empty()) {
      if (storage.sources.information[row.source].recipient)
        model::unbindOwnerSourceTask(storage, i);
      row.planner.reset();
    }
  }
  const auto removed = model::pruneOwnerSources(storage, c.world);
  (void)removed;
  while (!storage.pendingRoots.empty()) {
    const unsigned root = storage.pendingRoots.front();
    storage.pendingRoots.erase(storage.pendingRoots.begin());
    auto at = std::find_if(
        s.planners.begin(), s.planners.end(),
        [&](const auto &row) { return row.planner && row.source == root; });
    if (at == s.planners.end())
      continue;
    const unsigned task = unsigned(at - s.planners.begin());
    auto base = std::move(*c.native.ownerPlanner);
    *c.native.ownerPlanner = std::move(*at->planner);
    auto &p = *c.native.ownerPlanner;
    p.random = storage.sources.random;
    p.nextSerial = s.nextSerial;
    updateNativeOwnerGroupPlanner(c, profile, commands);
    storage.sources.random = p.random;
    s.nextSerial = p.nextSerial;
    *at->planner = std::move(p);
    *c.native.ownerPlanner = std::move(base);
    const auto old = storage.sources.information[root].recipient;
    if (at->planner->active && !old)
      model::bindOwnerSourceTask(storage, root, task);
    else if (!at->planner->active && old)
      model::unbindOwnerSourceTask(storage, task);
    break;
  }
}
void splitNativeOwnerSource(Continuation &c, unsigned root,
                            const std::vector<unsigned> &leaves) {
  need(c.native.ownerSources.has_value(),
       "native source split without sources");
  validate(c);
  auto staged = c;
  auto &s = *staged.native.ownerSources;
  const auto change =
      model::splitOwnerSource(s.storage, staged.world, root, leaves);
  for (auto task : change.retiredTasks)
    retire(s, task);
  synchronize(staged);
  validate(staged);
  c = std::move(staged);
}
void mergeNativeOwnerSources(Continuation &c, unsigned left, unsigned right) {
  need(c.native.ownerSources.has_value(),
       "native source merge without sources");
  validate(c);
  auto staged = c;
  auto &s = *staged.native.ownerSources;
  const auto change = model::mergeOwnerSources(s.storage, left, right);
  for (auto task : change.retiredTasks)
    retire(s, task);
  synchronize(staged);
  validate(staged);
  c = std::move(staged);
}
} // namespace mnm::command_session
