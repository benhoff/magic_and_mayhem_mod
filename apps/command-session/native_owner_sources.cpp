#include "native_owner_sources.hpp"
#include <algorithm>
#include <limits>
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
void automatic(Continuation &c, const NativeCreatureProfile &profile,
               model::OwnedCreatureCommands &commands, unsigned root) {
  auto &s = *c.native.ownerSources;
  auto context = [&](unsigned source) -> NativeOwnerPlanner * {
    for (auto &row : s.planners)
      if (row.planner && row.source == source)
        return &*row.planner;
    return nullptr;
  };
  if (s.search) {
    auto &v = *s.search;
    auto *current = context(v.source);
    auto *other = v.hasOther ? context(v.other) : nullptr;
    if (!current || current->members != v.inputs.members ||
        current->activeSerial != v.currentSerial ||
        (v.hasOther && (!other || other->members != v.inputs.otherMembers ||
                        other->activeSerial != v.otherSerial)))
      s.search.reset();
  }
  if (!s.search) {
    auto *current = context(root);
    if (!current)
      return;
    NativeOwnerSourceSearch v;
    v.source = root;
    v.currentSerial = current->activeSerial;
    v.inputs.members = current->members;
    for (auto actor : current->members)
      v.inputs.speeds.push_back(
          nativeOwnerPlannerActor(c, profile, actor).speed);
    for (auto &row : s.planners)
      if (row.planner && row.source != root) {
        need(!v.hasOther, "automatic search supports two friendly members");
        v.hasOther = true;
        v.other = row.source;
        v.otherSerial = row.planner->activeSerial;
        v.inputs.otherMembers = row.planner->members;
        v.inputs.otherTask = row.planner->grouped->replanning->previous;
      }
    // Pinned ordinary reset admission: largest available two-member root,
    // no healthy task, no net regional danger, owner roots<20 and both
    // specialized pending queues below3. The bounded domain has no competing
    // larger same-owner root or parent task.
    const auto &previous = current->grouped->replanning->previous;
    long long netDanger = 0;
    bool assigned = true;
    for (auto actor : current->members) {
      const auto &a = c.world.creatures.at(actor);
      assigned &= a.region >= 0;
      if (a.region >= 0)
        netDanger +=
            model::regionDanger(a, c.native.regions.at(unsigned(a.region)),
                                c.native.regionalRiskPowers, true);
    }
    v.inputs.splitFirst =
        current->members.size() == 2 && (!previous || previous->completed) &&
        assigned && netDanger <= 0 &&
        std::count_if(s.planners.begin(), s.planners.end(),
                      [](const auto &row) { return row.planner.has_value(); }) <
            20 &&
        s.storage.pending.wizard.size() < 3 &&
        s.storage.pending.other.size() < 3;
    v.inputs.weighted = current->selection.weighted;
    v.scratch = *c.native.ownerPlanner;
    v.scratch.members = v.inputs.members;
    v.scratch.actor = v.inputs.members.front();
    v.scratch.grouped->perMember = true;
    s.search = std::move(v);
  }
  auto &v = *s.search;
  if (v.hasOther)
    v.inputs.otherTask = context(v.other)->grouped->replanning->previous;
  // Explicit native planning allocation, independent of execution.
  int budget = int(s.planningAllocation);
  model::OwnerSourceDecisionServices lower;
  lower.select = [&](const auto &members, bool rootSource, float bias,
                     const auto *old, bool rejectOne, int &b) {
    auto base = std::move(*c.native.ownerPlanner);
    v.scratch.members = members;
    v.scratch.actor = members.front();
    v.scratch.random = s.storage.sources.random;
    *c.native.ownerPlanner = std::move(v.scratch);
    NativeOwnerProposalEvaluation request{&b,         bias,      old,
                                          rootSource, rejectOne, false};
    updateNativeOwnerGroupPlanner(c, profile, commands, &request);
    v.scratch = std::move(*c.native.ownerPlanner);
    *c.native.ownerPlanner = std::move(base);
    s.storage.sources.random = v.scratch.random;
    return request.changed;
  };
  const bool done = model::advanceOwnerSourceDecision(
      v.decision, v.scratch.selection, v.inputs, budget,
      s.storage.sources.random, lower);
  if (!done)
    return;
  need(s.decisions < std::numeric_limits<unsigned>::max(),
       "source decision overflow");
  ++s.decisions;
  const auto winner = v.decision.winner;
  auto *current = context(v.source);
  const auto value = v.scratch.selection.value - v.decision.winningBias;
  const auto policy = v.scratch.grouped->replanning->policy;
  const auto &previous = current->grouped->replanning->previous;
  // Selection carries biased scores; original commit compares the unbiased
  // winner against the task currently on this source.
  if (winner < 0 ||
      !model::replaceOwnerTask(value, previous ? &*previous : nullptr,
                               int(c.native.tick), policy)) {
    s.search.reset();
    return;
  }
  const auto selected = v.scratch.grouped->best;
  const int duration = v.scratch.grouped->replanning->bestDuration;
  const auto decision = v.decision;
  const auto input = v.inputs;
  unsigned destination = v.source;
  const auto other = v.other;
  s.search.reset();
  if (winner == 2) {
    const auto change = model::mergeOwnerSources(s.storage, destination, other);
    destination = change.root;
    for (auto task : change.retiredTasks)
      retire(s, task);
    ++s.merges;
  } else if (winner == 4) {
    const auto retained = model::ownerSourceSplitMembers(input);
    std::vector<unsigned> moved;
    for (auto id : s.storage.manager.nodes) {
      const auto actor = s.storage.sources.graph[id].actor;
      if (actor &&
          std::find(input.members.begin(), input.members.end(), *actor) !=
              input.members.end() &&
          std::find(retained.begin(), retained.end(), *actor) == retained.end())
        moved.push_back(id);
    }
    const auto change =
        model::splitOwnerSource(s.storage, c.world, destination, moved);
    for (auto task : change.retiredTasks)
      retire(s, task);
    ++s.splits;
  } else if (auto task = s.storage.sources.information[destination].recipient) {
    retire(s, *task);
    model::unbindOwnerSourceTask(s.storage, *task);
  }
  synchronize(c);
  auto *target = context(destination);
  need(target, "automatic source destination missing");
  if (!selected)
    return; // The fallback evaluator owns no installed task.
  auto &g = *target->grouped;
  g.perMember = true;
  g.active = selected;
  target->active = model::OwnerHostileTask{
      model::ownerGroupKeys(selected->groups, selected->root).front(),
      selected->groups.nodes[selected->root].position};
  need(s.nextSerial < std::numeric_limits<unsigned>::max() && duration >= 0 &&
           unsigned(duration) <=
               unsigned(std::numeric_limits<int>::max()) - c.native.tick,
       "automatic task lifetime/serial overflow");
  target->activeSerial = s.nextSerial++;
  target->nextSerial = s.nextSerial;
  model::OwnerPreviousTask task;
  task.started = int(c.native.tick);
  task.expires = task.started + duration;
  task.value = value;
  task.weighted = decision.winner >= 0 && input.weighted;
  task.baseline = policy.ownerAvailable
                      ? float(static_cast<long double>(policy.ownerNumerator) /
                              (policy.ownerDenominator + 1))
                      : 0;
  g.replanning->previous = task;
  *g.scheduler = {};
  const unsigned handle =
      unsigned(std::find_if(s.planners.begin(), s.planners.end(),
                            [&](const auto &row) {
                              return row.planner && row.source == destination;
                            }) -
               s.planners.begin());
  model::bindOwnerSourceTask(s.storage, destination, handle);
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
  need(s.planningAllocation >= 1 && s.planningAllocation <= 64,
       "native source planning allocation");
  need(s.automatic || s.planningAllocation == 4, "inactive source allocation");
  need(s.splits <= s.decisions && s.merges <= s.decisions - s.splits,
       "automatic source decision counters");
  need(s.automatic || (!s.search && !s.decisions && !s.splits && !s.merges),
       "inactive automatic source continuation");
  if (s.automatic) {
    unsigned count = 0;
    for (const auto &row : s.planners)
      if (row.planner)
        count += row.planner->members.size();
    need(count <= 2, "automatic source member bound");
  }
  if (s.search) {
    const auto &v = *s.search;
    model::validateOwnerSourceDecision(v.decision);
    model::validateOwnerSourceDecisionInputs(v.inputs);
    auto expected = v.inputs.members;
    if (v.decision.stage == 2)
      expected.insert(expected.end(), v.inputs.otherMembers.begin(),
                      v.inputs.otherMembers.end());
    if (v.decision.stage == 4)
      expected = model::ownerSourceSplitMembers(v.inputs);
    need((v.decision.stage == 0 || v.decision.stage == 2 ||
          v.decision.stage == 4) &&
             v.scratch.members == expected && v.scratch.random == random &&
             v.scratch.selection.weighted == v.inputs.weighted,
         "automatic source proposal continuation");
    need(!v.decision.reset && v.decision.stage < 5 &&
             !v.inputs.members.empty() &&
             v.inputs.members.size() + v.inputs.otherMembers.size() <= 2 &&
             v.inputs.speeds.size() == v.inputs.members.size() &&
             v.inputs.members == model::ownerSourceMembers(
                                     s.storage.sources.graph, v.source, 3) &&
             v.hasOther == !v.inputs.otherMembers.empty() &&
             (!v.hasOther || (v.other != v.source &&
                              v.inputs.otherMembers ==
                                  model::ownerSourceMembers(
                                      s.storage.sources.graph, v.other, 3))) &&
             v.scratch.grouped && v.scratch.grouped->perMember &&
             !v.scratch.active && !v.scratch.grouped->active &&
             !v.scratch.grouped->replanning->previous &&
             v.scratch.owner == c.native.ownerPlanner->owner,
         "automatic source search identity");
    model::validateOwnerTaskSelection(v.scratch.selection, 2);
    validateNativeOwnerGroupPlanner(c, &v.scratch);
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
    updateNativeOwnerGroupPlanner(c, profile, commands, nullptr, s.automatic);
    storage.sources.random = p.random;
    s.nextSerial = p.nextSerial;
    *at->planner = std::move(p);
    *c.native.ownerPlanner = std::move(base);
    const auto old = storage.sources.information[root].recipient;
    if (at->planner->active && !old)
      model::bindOwnerSourceTask(storage, root, task);
    else if (!at->planner->active && old)
      model::unbindOwnerSourceTask(storage, task);
    if (s.automatic)
      automatic(c, profile, commands, root);
    break;
  }
}
void enableAutomaticOwnerSources(Continuation &c) {
  validate(c);
  need(c.native.ownerSources && !c.native.ownerSources->automatic,
       "automatic source activation requires existing sources");
  auto staged = c;
  auto &s = *staged.native.ownerSources;
  unsigned members = 0;
  for (auto &row : s.planners)
    if (row.planner) {
      members += row.planner->members.size();
      row.planner->grouped->perMember = true;
    }
  need(members == 2, "automatic source scope requires two friendlies");
  s.automatic = true;
  validate(staged);
  c = std::move(staged);
}
void splitNativeOwnerSource(Continuation &c, unsigned root,
                            const std::vector<unsigned> &leaves) {
  need(c.native.ownerSources.has_value(),
       "native source split without sources");
  validate(c);
  auto staged = c;
  auto &s = *staged.native.ownerSources;
  s.search.reset();
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
  s.search.reset();
  const auto change = model::mergeOwnerSources(s.storage, left, right);
  for (auto task : change.retiredTasks)
    retire(s, task);
  synchronize(staged);
  validate(staged);
  c = std::move(staged);
}
} // namespace mnm::command_session
