// NS89: original-first multi-source decision-to-command capture.
#define main unused_multi_loop_fixture_main
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include "owner-task6-loop-test.cpp"
#pragma GCC diagnostic pop
#undef main
#include "../reconstruction/creature-ai/owner_brain_driver.hpp"
#include "../reconstruction/creature-ai/owner_task_scheduler.hpp"
#include "../reconstruction/creature-ai/source_decision.hpp"
#include <set>
namespace {
constexpr unsigned episodes = 32, frames = 128, capacity = 512;
constexpr std::array<unsigned, 3> actors{{0, 2, 3}};
struct Trace {
  Bytes bytes;
  std::size_t at = 8;
  bool capture;
  Trace(const char *path, bool write) : capture(write) {
    if (write) {
      std::ifstream f(path);
      need(!f, "existing capture");
      bytes = {'M', 'N', 'M', 'M', 'S', '0', '0', '1'};
    } else {
      std::ifstream f(path, std::ios::binary | std::ios::ate);
      need(bool(f) && f.tellg() >= 16 && f.tellg() <= 32 * 1024 * 1024,
           "trace size");
      bytes.resize(std::size_t(f.tellg()));
      f.seekg(0);
      need(bool(f.read(reinterpret_cast<char *>(bytes.data()), bytes.size())),
           "trace read");
      need(std::equal(bytes.begin(), bytes.begin() + 8, "MNMMS001"),
           "trace version");
    }
    check(episodes, "episodes");
    check(frames, "frames");
  }
  unsigned word(unsigned value = 0) {
    if (capture) {
      for (unsigned i = 0; i < 4; ++i)
        bytes.push_back(static_cast<unsigned char>(value >> (i * 8)));
      at += 4;
      return value;
    }
    need(at <= bytes.size() && bytes.size() - at >= 4, "truncated trace");
    unsigned v = 0;
    for (unsigned i = 0; i < 4; ++i)
      v |= unsigned(bytes[at++]) << (i * 8);
    return v;
  }
  bool flag(bool value) {
    auto v = word(value);
    need(v < 2, "trace flag");
    return bool(v);
  }
  void check(unsigned v, const char *why) { record(v, word(v), why); }
  void vector(const std::vector<unsigned> &v, const char *why) {
    const unsigned count = word(v.size());
    if (count != v.size()) {
      std::string detail = std::string(why) +
                           " extent native=" + std::to_string(v.size()) +
                           " original=" + std::to_string(count);
      for (unsigned i = 0; i < std::min<unsigned>(count, v.size()) &&
                           at + 4 * i + 4 <= bytes.size();
           ++i) {
        unsigned value = 0;
        for (unsigned j = 0; j < 4; ++j)
          value |= unsigned(bytes[at + 4 * i + j]) << (8 * j);
        if (v[i] != value) {
          detail += " first[" + std::to_string(i) +
                    "] native=" + std::to_string(v[i]) +
                    " original=" + std::to_string(value);
          break;
        }
      }
      throw std::runtime_error(detail);
    }
    record(v.size(), count, why);
    for (unsigned i = 0; i < v.size(); ++i)
      check(v[i], (std::string(why) + "[" + std::to_string(i) + "]").c_str());
  }
  void finish(const char *path) {
    need(at == bytes.size(), "trailing trace");
    need(bytes.size() <= 32 * 1024 * 1024, "trace output size");
    if (capture) {
      std::ofstream f(path, std::ios::binary);
      need(bool(f.write(reinterpret_cast<const char *>(bytes.data()),
                        bytes.size())),
           "trace write");
    }
  }
};
struct Deck {
  m::AutonomousWorld world;
  m::OwnerKnowledge knowledge;
  std::vector<m::NavigationRegion> graph;
  m::BrainInferenceModel brain;
  m::OwnerSourceStorage storage;
  std::array<int, 3> speeds;
  explicit Deck(unsigned n) : graph(8), brain(model(n)) {
    world.dimensions = {32, 32, 8};
    world.relation[0][0] = 1;
    world.creatures.resize(4);
    for (unsigned i = 0; i < 8; ++i) {
      auto &r = graph[i];
      r.position = {int(2 + i * 2), 2, 2};
      r.parameter = 1;
      r.dangerBase = i < 3 && (n & 2) ? 1u : 0u;
      if (i + 1 < 8) {
        r.connections[0].destination = int(i + 1);
        r.connections[0].cost = 4;
        r.connections[0].clearance.fill(8);
      }
    }
    for (unsigned j = 0; j < 4; ++j) {
      auto &a = world.creatures[j];
      a.ai.slot = int(j);
      a.ai.owner = j == 1;
      a.ai.kind = 1;
      a.ai.type = j == 1 ? 18 : 17;
      a.ai.active = true;
      a.ai.health = 100;
      a.ai.physical = 1;
      a.ai.position =
          j == 1 ? m::Cell{16, 2, 2} : graph[j == 0 ? 0 : j - 1].position;
      a.region = j == 1 ? 7 : int(j == 0 ? 0 : j - 1);
    }
    for (unsigned j = 0; j < 3; ++j)
      speeds[j] = (n & (1u << (j + 2))) ? 48 : 24;
    for (unsigned i = 0; i < 2; ++i) {
      m::OwnerMemory r;
      r.memory.key = 5 + int(i * 3);
      r.memory.observedTick = r.markedTick = 100;
      r.memory.actor.slot = 1;
      r.memory.actor.owner = 1;
      r.memory.actor.health = 100;
      r.memory.actor.region = 6 + int(i);
      r.memory.actor.position = graph[6 + i].position;
      r.planning.emplace();
      r.planning->position = r.memory.actor.position;
      r.planning->speed = 24;
      r.planning->physical = true;
      knowledge.records[0].push_back(r);
    }
    // Real inference on produced friendly spread: sign changes which topology
    // has a better generated score. No candidate scores are supplied.
    for (auto &l : brain.layers) {
      l.outside = 2.5f;
      l.matrix.assign(72 * 72, 0);
      for (auto &node : l.nodes) {
        node[0] = 0;
        node[1] = 16;
        for (unsigned j = 0; j < 10; ++j)
          node[j + 2] = float(j) / 4;
      }
    }
    brain.layers[2].matrix[10 * 72 + 10] = (n & 1) ? -1.f : 1.f;
    auto &v = storage.sources;
    v.graph.resize(4);
    v.information.resize(4);
    v.roots.resize(4);
    v.actorFlag8e5.resize(4);
    v.mode = 3;
    for (unsigned i = 0; i < 4; ++i) {
      v.graph[i].free.fill(1);
      v.graph[i].activity = 3;
      storage.manager.nodes.push_back(i);
    }
    for (unsigned i = 0; i < 3; ++i) {
      v.graph[i].actor = actors[i];
      v.roots[actors[i]][0] = i;
    }
    v.graph[3].children = {0, 1};
    v.graph[0].parent = v.graph[1].parent = 3;
  }
  m::OwnerPlannerActor friendly(unsigned slot) const {
    auto j = std::find(actors.begin(), actors.end(), slot) - actors.begin();
    need(j < 3, "friendly slot");
    m::OwnerPlannerActor a;
    a.position = world.creatures[slot].ai.position;
    a.physical = true;
    a.speed = speeds[j];
    a.danger = int(graph.at(unsigned(world.creatures[slot].region)).dangerBase);
    return a;
  }
};
struct Candidate {
  m::OwnerGroupTask task;
  std::vector<int> features;
  int duration = 0;
};
struct Evaluator {
  m::OwnerMemoryGroups groups;
  m::OwnerTaskProbe probe;
  m::OwnerTaskPathTotals totals;
  std::vector<unsigned> members;
  unsigned cursor = 0, member = 0;
  int deadline = 0;
  std::optional<Candidate> candidate, best;
  void prepare(const std::vector<unsigned> &ids, Deck &d, int tick) {
    members = ids;
    cursor = member = 0;
    probe = {};
    totals = {};
    groups = {};
    deadline = tick + 20 + 2 * int(ids.size());
    m::buildOwnerMemoryGroups(
        groups, d.knowledge, 0, tick, true, {32, 32, 8}, d.world.relation,
        {[&](int a, int b, int &center) {
           return m::regionalDangerCenter(d.graph, a, b, {4, 1, 1, 3, 0, 0, 0},
                                          {32, 32, 8}, center);
         },
         [](auto, auto) { return false; }});
  }
  std::optional<float> next(Deck &d, m::RandomState &random, int tick,
                            int &budget) {
    auto &roots = groups.nodes[0].children;
    while (cursor < roots.size() && tick <= deadline) {
      const unsigned root = roots[cursor];
      const auto &node = groups.nodes[root];
      m::RegionalQuery q;
      q.movement = {1, 1, 0, 3, 0, 17, 0};
      m::RegionalServices lower;
      lower.danger = [&](int id) {
        return int(d.graph.at(unsigned(id)).dangerBase);
      };
      auto result = m::advanceOwnerTaskProbe(
          probe, d.graph, d.world.creatures[members[member]].region,
          node.region, q, {32, 32, 8}, budget, lower);
      if (result == m::OwnerTaskProbeResult::pending)
        return {};
      if (result == m::OwnerTaskProbeResult::failed) {
        ++cursor;
        member = 0;
        probe = {};
        totals = {};
        continue;
      }
      auto at =
          std::find_if(probe.search.records.begin(), probe.search.records.end(),
                       [&](auto &r) { return r.node == node.region; });
      need(at != probe.search.records.end(), "member goal");
      auto path = mnm::reconstruction::neighbor_movement(at->movement);
      int risk = 0;
      for (int id : probe.path)
        risk = std::max(risk, lower.danger(id));
      m::addOwnerTaskPath(totals, path.accumulated_cost,
                          d.friendly(members[member]).speed, path.unknown_18,
                          lower.danger(node.region), risk);
      probe = {};
      if (++member < members.size())
        continue;
      ++cursor;
      member = 0;
      std::vector<m::OwnerPlannerActor> friendly, enemy;
      for (auto id : members)
        friendly.push_back(d.friendly(id));
      for (int key : m::ownerGroupKeys(groups, root)) {
        auto it = std::find_if(d.knowledge.records[0].begin(),
                               d.knowledge.records[0].end(),
                               [&](auto &r) { return r.memory.key == key; });
        need(it != d.knowledge.records[0].end(), "memory key");
        enemy.push_back(*it->planning);
      }
      Candidate c;
      c.task = {groups, root, totals.risk};
      const auto prefix =
          m::hostileTaskGroupFeatures(friendly, enemy, {32, 32, 8});
      c.features.assign(prefix.begin(), prefix.end());
      c.duration = m::ownerTask6Duration(random, totals.travel);
      c.features.insert(
          c.features.end(),
          {int(std::sqrt(static_cast<long double>(node.uncertaintySquared))),
           totals.travel, totals.auxiliary / int(members.size()),
           totals.danger / int(members.size()), 0});
      auto p = m::inferOwnerTask(d.brain, c.features);
      candidate = std::move(c);
      totals = {};
      return m::ownerTaskValue({p[0], p[1], p[2]}, {0, 0, 1}, 100);
    }
    return {};
  }
};
struct Task {
  unsigned root = 0, serial = 0;
  Candidate data;
  m::OwnerPreviousTask previous;
  bool complete = false;
};
struct Native {
  Deck &d;
  m::OwnerBrainDriver driver;
  m::RandomState random;
  std::map<unsigned, Task> tasks;
  unsigned serial = 0;
  m::OwnerSourceDecision decision;
  m::OwnerTaskSelection selection;
  Evaluator evaluator;
  std::optional<unsigned> current, other;
  m::OwnerSourceDecisionInputs input;
  std::vector<unsigned> calls, retired, orders, candidates, visits;
  explicit Native(Deck &deck, unsigned n)
      : d(deck), random(m::seedRandom(17 + n * 137)) {
    driver.clock = 99;
  }
  Task *active(unsigned root) {
    auto recipient = d.storage.sources.information.at(root).recipient;
    return recipient ? &tasks.at(*recipient) : nullptr;
  }
  void retire(unsigned key) {
    auto &t = tasks.at(key);
    retired.push_back(t.serial);
    m::unbindOwnerSourceTask(d.storage, key);
    tasks.erase(key);
    driver.execution.erase(
        std::remove(driver.execution.begin(), driver.execution.end(), key),
        driver.execution.end());
  }
  void plan(unsigned root, int &budget, int tick, int net) {
    auto &g = d.storage.sources.graph;
    auto ids = m::ownerSourceMembers(g, root, 3);
    // Empty committed groups may survive the supplied no-op maintenance.
    // Whole423590 rejects them before entering a task selector.
    if (ids.empty())
      return;
    if (g.at(root).parent) {
      // Whole423590 still visits an inadmissible nested leaf. Its root-free
      // selector calls next once, leaves budget/RNG untouched and clears the
      // shared score. The parent gate prevents any task6 candidate production.
      need(!current || *current == root,
           "nested source during pending root search");
      selection = {};
      decision = {};
      const auto members = m::ownerSourceMembers(g, root, 3);
      m::OwnerTaskSelectionServices lower{
          1,
          false,
          [&](unsigned) {
            need(!m::admitOwnerTask6(
                     {true, 0, true, true, true, false, true, false}),
                 "nested task admission");
          },
          [&](unsigned, int &b) -> std::optional<float> {
            candidates.push_back(members.size());
            candidates.insert(candidates.end(), members.begin(), members.end());
            candidates.insert(candidates.end(), {unsigned(b), unsigned(b), 0});
            return {};
          },
          [](unsigned) { throw std::runtime_error("nested candidate saved"); },
          [] {}};
      m::advanceOwnerTaskSelection(selection, budget, random, lower);
      if (budget < 1) {
        current = root;
        decision.reset = false;
      } else {
        current.reset();
      }
      return;
    }
    if (!current) {
      current = root;
      decision = {};
      selection = {};
      evaluator = {};
      input = {};
      input.members = ids;
      for (auto id : ids)
        input.speeds.push_back(d.friendly(id).speed);
      const auto *old = active(root);
      bool largest = true;
      for (auto id : d.storage.manager.nodes)
        if (!g[id].parent &&
            m::ownerSourceMembers(g, id, 3).size() > ids.size())
          largest = false;
      input.splitFirst = ids.size() >= 2 && (!old || old->previous.completed) &&
                         largest && net <= 0;
    }
    need(current == root, "pending source identity");
    m::OwnerSourceDecisionServices services;
    services.chooseOther = [&](m::RandomState &rng) {
      std::vector<m::OwnerMergeCandidate> choices;
      auto points = [&](const auto &members) {
        std::vector<m::Cell> p;
        for (auto id : members)
          p.push_back(d.world.creatures[id].ai.position);
        return p;
      };
      for (auto id : d.storage.manager.nodes)
        if (id != root && !g[id].parent) {
          auto members = m::ownerSourceMembers(g, id, 3);
          if (!members.empty())
            choices.push_back(
                {id, m::ownerSourceDistance(points(input.members),
                                            points(members), {32, 32, 8})});
        }
      other = m::chooseOwnerMergeCandidate(choices, rng);
      input.otherMembers.clear();
      input.otherTask.reset();
      if (other) {
        input.otherMembers = m::ownerSourceMembers(g, *other, 3);
        if (auto *t = active(*other))
          input.otherTask = t->previous;
      }
    };
    services.select = [&](const auto &members, bool isRoot, float bias,
                          const auto *old, bool reject, int &b) {
      m::OwnerTaskSelectionServices lower{
          1,
          isRoot,
          [&](unsigned) { evaluator.prepare(members, d, tick); },
          [&](unsigned, int &left) {
            const int before = left;
            auto value = evaluator.next(d, random, tick, left);
            candidates.push_back(members.size());
            candidates.insert(candidates.end(), members.begin(), members.end());
            candidates.insert(candidates.end(),
                              {unsigned(before), unsigned(left),
                               unsigned(value.has_value())});
            if (value) {
              const auto &c = *evaluator.candidate;
              candidates.insert(
                  candidates.end(),
                  {bits(*value), unsigned(c.duration), unsigned(c.task.risk)});
              for (int feature : c.features)
                candidates.push_back(unsigned(feature));
            }
            return value;
          },
          [&](unsigned) { evaluator.best = evaluator.candidate; },
          [] {}};
      return m::advanceOwnerTaskReplanning(selection, b, random, lower, old,
                                           tick, {}, bias, true, reject);
    };
    if (!m::advanceOwnerSourceDecision(decision, selection, input, budget,
                                       random, services))
      return;
    const float value = selection.value - decision.winningBias;
    auto *t = active(root);
    if (decision.winner >= 0 && evaluator.best &&
        m::replaceOwnerSourceTask(value, t ? &t->previous : nullptr, tick,
                                  {})) {
      unsigned destination = root;
      if (decision.winner == 2) {
        auto change = m::mergeOwnerSources(d.storage, root, *other);
        destination = change.root;
        for (auto key : change.retiredTasks) {
          retired.push_back(tasks.at(key).serial);
          tasks.erase(key);
          driver.execution.erase(std::remove(driver.execution.begin(),
                                             driver.execution.end(), key),
                                 driver.execution.end());
        }
      } else if (decision.winner == 4) {
        auto kept = m::ownerSourceSplitMembers(input);
        std::vector<unsigned> moved;
        for (auto id : d.storage.manager.nodes)
          if (g[id].actor &&
              std::find(input.members.begin(), input.members.end(),
                        *g[id].actor) != input.members.end() &&
              std::find(kept.begin(), kept.end(), *g[id].actor) == kept.end())
            moved.push_back(id);
        auto change = m::splitOwnerSource(d.storage, d.world, root, moved);
        for (auto key : change.retiredTasks) {
          retired.push_back(tasks.at(key).serial);
          tasks.erase(key);
          driver.execution.erase(std::remove(driver.execution.begin(),
                                             driver.execution.end(), key),
                                 driver.execution.end());
        }
        d.storage.pending = {}; // supplied registration service is a no-op
      } else if (t)
        retire(t->serial);
      Task task;
      task.root = destination;
      task.serial = serial++;
      task.data = *evaluator.best;
      task.previous.started = tick;
      task.previous.expires = tick + task.data.duration;
      task.previous.value = value;
      const unsigned key = task.serial;
      tasks.emplace(key, std::move(task));
      m::bindOwnerSourceTask(d.storage, destination, key);
    }
    current.reset();
  }
  void advance(int total, bool disabled, int net, int raw) {
    calls.clear();
    retired.clear();
    orders.clear();
    candidates.clear();
    visits.clear();
    const int tick = driver.clock + 1;
    for (unsigned i = 0; i < d.graph.size(); ++i)
      d.graph[i].dangerBase = i < 3 ? unsigned(net + raw) : 0;
    for (auto &r : d.knowledge.records[0])
      r.markedTick = tick;
    m::OwnerBrainDriverServices svc;
    svc.cleanup = [&] {
      std::vector<unsigned> keys;
      for (auto &[key, t] : tasks)
        if (m::retireOwnerTask(t.previous, t.complete, tick, {}))
          keys.push_back(key);
      for (auto key : keys)
        retire(key);
    };
    svc.snapshot = [](bool) {};
    svc.sources = [&] { return d.storage.manager.nodes; };
    svc.hasTask = [&](unsigned id) { return active(id) != nullptr; };
    svc.plan = [&](unsigned id, bool specialized, int &b) {
      if (!specialized)
        plan(id, b, tick, net + raw);
    };
    svc.tasks = [&] {
      std::vector<unsigned> keys;
      for (auto &[key, t] : tasks) {
        (void)t;
        keys.push_back(key);
      }
      return keys;
    };
    svc.execute = [&](unsigned key, int &b) {
      auto it = tasks.find(key);
      if (it == tasks.end())
        return;
      auto &t = it->second;
      if (t.complete || tick >= t.previous.expires)
        return;
      calls.push_back(key);
      auto ids = m::ownerSourceMembers(d.storage.sources.graph, t.root, 3);
      std::vector<int> danger(ids.size(), net + raw);
      const int beforeBudget = b;
      auto out = m::executeOwnerSourceGroupTask(t.data.task, d.knowledge, tick,
                                                0, ids, d.world, danger, danger,
                                                0, std::vector<int>(4), b);
      t.complete = !out.continuing;
      visits.insert(visits.end(), {key, unsigned(beforeBudget), unsigned(b),
                                   unsigned(out.continuing)});
      for (unsigned j = 0; j < ids.size(); ++j) {
        auto &o = out.orders[j];
        if (o.stop)
          orders.insert(orders.end(), {ids[j], 3});
        else if (o.kind == m::OwnerHostileOrderKind::target)
          orders.insert(orders.end(), {ids[j], 1, unsigned(o.target)});
        else if (o.kind == m::OwnerHostileOrderKind::move)
          orders.insert(orders.end(),
                        {ids[j], 2, unsigned(o.position.x),
                         unsigned(o.position.y), unsigned(o.position.z)});
      }
    };
    m::advanceOwnerBrainDriver(driver, d.storage, total, svc, disabled);
  }
};
#ifdef MNM_BRAIN_REFERENCE
struct Original;
Original *currentOriginal = nullptr;
std::array<unsigned, 7> multiTable;
void *__attribute__((cdecl)) multiAllocate(unsigned);
void __attribute__((cdecl)) multiRelease(void *);
void *__attribute__((thiscall)) multiClone(void *) {
  rawTasks.push_back(std::make_unique<RawTask>(multiTable.data()));
  return rawTasks.back()->data.data();
}
void __attribute__((thiscall)) multiNoop(void *) {}
void __attribute__((thiscall)) multiRegister(void *, void *) {}
void __attribute__((thiscall)) multiSight(void *, unsigned) {}
void __attribute__((thiscall)) multiPlan(void *, void *, int *) {}
struct Original {
  OriginalLoop old;
  std::vector<Bytes> nodes = std::vector<Bytes>(4, Bytes(0x8d));
  std::vector<unsigned char *> addresses;
  std::vector<Bytes> children = std::vector<Bytes>(4, Bytes(256));
  Bytes configs = Bytes(3 * 0x5c9), manager = Bytes(capacity * 4);
  unsigned count = 4;
  std::vector<unsigned> calls, retired, orders, candidates, visits;
  Original(Deck &d, unsigned n)
      : old(d.graph, d.knowledge, m::seedRandom(17 + n * 137), false) {
    currentOriginal = this;
    put(rawTasks.back()->data.data(), 0, address(multiTable.data()));
    for (auto &buffer : nodes)
      addresses.push_back(buffer.data());
    for (unsigned i = 0; i < 4; ++i) {
      auto *p = node(i);
      p[4] = 1;
      put(p, 0x89, 3);
      for (unsigned j = 0; j < 8; ++j)
        p[0x41 + j] = 1;
      put(p, 0x29, address(children[i].data()));
      put(p, 0x2d, address(children[i].data()));
      put(p, 0x31, address(children[i].data() + children[i].size()));
      put(manager.data(), i * 4, address(p));
    }
    for (unsigned i = 0; i < 3; ++i) {
      unsigned slot = actors[i];
      auto *a = old.world.pool.data() + slot * 0xe4b;
      if (slot)
        std::memcpy(a, old.world.pool.data(), 0xe4b);
      put(a, 0, slot);
      put(a, 8, d.world.creatures[slot].ai.position.x);
      put(a, 0xde7, i);
      put(a, 0x678, address(node(i)));
      auto *cfg = configs.data() + i * 0x5c9;
      std::memcpy(cfg, old.world.configs.data(), 0x5c9);
      put(a, 0xac, address(cfg));
      put(a, 0x778, d.speeds[i] == 48);
      put(node(i), 0x35, address(a));
    }
    put(old.world.pool.data() + 0xe4b, 8, 16);
    put(node(0), 0x39, address(node(3)));
    put(node(1), 0x39, address(node(3)));
    put(children[3].data(), 0, address(node(0)));
    put(children[3].data(), 4, address(node(1)));
    put(node(3), 0x2d, address(children[3].data() + 8));
    auto *b = old.brain.data();
    put(b, 0x1e, address(manager.data()));
    put(b, 0x22, address(manager.data() + 16));
    put(b, 0x26, address(manager.data() + manager.size()));
    put(b, 0x14, 99);
    put(b, 0x11ddd, bits(-2.f));
    put(b, 0x11dd8, UINT32_MAX);
    for (unsigned off : {0x11ea6u, 0x11f33u})
      for (unsigned j = 0; j < 8; ++j)
        b[off + 0x41 + j] = 1;
    global(0x5faf54, address(old.queueNil.data()));
    global(0x5faf58, address(old.queueNil.data()));
    global(0x5d865c, 20);
    global(0x6e1f58, 0);
  }
  unsigned char *node(unsigned i) const { return addresses.at(i); }
  unsigned id(unsigned ptr) const {
    if (!ptr)
      return UINT32_MAX;
    for (unsigned i = 0; i < count; ++i)
      if (address(node(i)) == ptr)
        return i;
    std::string detail = "unknown source pointer " + std::to_string(ptr) +
                         " brain=" + std::to_string(address(old.brain.data())) +
                         " count=" + std::to_string(count);
    for (unsigned i = 0; i < count; ++i)
      if (get(node(i), 0x39) == ptr)
        detail += " child=" + std::to_string(i);
    throw std::runtime_error(detail);
  }
  std::vector<unsigned> list(unsigned begin, unsigned end) const {
    need(end >= begin && (end - begin) % 4 == 0 && end - begin <= capacity * 4,
         "source list bound");
    std::vector<unsigned> v;
    for (auto p = begin; p != end; p += 4)
      v.push_back(id(get(reinterpret_cast<void *>(p))));
    return v;
  }
  std::vector<unsigned> parents() const {
    std::vector<unsigned> out(count, UINT32_MAX);
    for (unsigned i = 0; i < count; ++i)
      for (auto child : list(get(node(i), 0x29), get(node(i), 0x2d))) {
        need(out[child] == UINT32_MAX, "duplicate committed source parent");
        out[child] = i;
      }
    for (unsigned i = 0; i < count; ++i) {
      const auto ptr = get(node(i), 0x39);
      const auto begin = address(old.brain.data());
      const bool scratch = ptr == begin + 0x11ea6 || ptr == begin + 0x11f33;
      if (!scratch)
        need(id(ptr) == out[i], "committed source parent projection");
    }
    return out;
  }
  std::vector<unsigned> mapKeys(unsigned offset) const {
    const auto head = get(old.brain.data(), offset),
               nil = address(old.queueNil.data());
    std::vector<unsigned> out;
    std::set<unsigned> seen;
    std::function<void(unsigned)> walk = [&](unsigned node) {
      if (node == nil)
        return;
      need(node && node != head && seen.size() < 128 &&
               seen.insert(node).second,
           "task map traversal");
      auto *p = reinterpret_cast<void *>(node);
      walk(get(p));
      out.push_back(get(p, 12));
      walk(get(p, 8));
    };
    walk(get(reinterpret_cast<void *>(head), 4));
    return out;
  }
  std::vector<std::pair<unsigned, void *>> tasks() const {
    std::vector<std::pair<unsigned, void *>> out;
    for (unsigned i = 0; i < count; ++i)
      if (auto p = get(node(i), 5))
        out.emplace_back(get(reinterpret_cast<void *>(p), 0x1f),
                         reinterpret_cast<void *>(p));
    std::sort(out.begin(), out.end());
    return out;
  }
  void advance(int total, bool disabled, int net, int raw) {
    calls.clear();
    retired.clear();
    orders.clear();
    candidates.clear();
    visits.clear();
    global(0x6e1f54, disabled);
    const int tick = int(get(old.brain.data(), 0x14)) + 1;
    for (auto &r : old.memories)
      put(r.data(), 20, tick);
    for (unsigned i = 0; i < 8; ++i)
      old.world.regions[i * 660 + 644] =
          static_cast<unsigned char>(i < 3 ? net + raw : 0);
    reinterpret_cast<void(__attribute__((thiscall)) *)(void *, int)>(0x415c40)(
        old.brain.data(), total);
    const auto begin = get(old.brain.data(), 0x1e),
               end = get(old.brain.data(), 0x22);
    need(end >= begin && end - begin <= capacity * 4, "manager bound");
    for (auto at = begin; at != end; at += 4) {
      auto *p =
          reinterpret_cast<unsigned char *>(get(reinterpret_cast<void *>(at)));
      if (std::find(addresses.begin(), addresses.end(), p) == addresses.end()) {
        need(addresses.size() < capacity, "committed source capacity");
        addresses.push_back(p);
      }
    }
    count = addresses.size();
    guardAllocations();
  }
};
void *__attribute__((cdecl)) multiAllocate(unsigned n) {
  return guardedAllocate(n);
}

void __attribute__((cdecl)) multiRelease(void *p) {
  if (!p)
    return;
  auto &r = *currentOriginal;
  for (auto *node : r.addresses)
    if (p == node)
      return;
  for (auto &v : r.children)
    if (p == v.data())
      return;
  if (p == r.manager.data())
    return;
  loopRelease(p);
}
void __attribute__((thiscall)) multiAppend(void *p, void *where, unsigned count,
                                           void **item) {
  need(count == 1 && get(p, 8) == address(where), "append ABI");
  if (!get(p, 4) || get(p, 8) + 4 > get(p, 12)) {
    const unsigned begin = get(p, 4), end = get(p, 8);
    need(end >= begin && end - begin <= 16384, "vector growth bound");
    const unsigned used = end - begin, size = std::max(256u, 2 * (used + 4));
    auto *v = guardedAllocate(size);
    if (used)
      std::memcpy(v, reinterpret_cast<void *>(begin), used);
    put(p, 4, address(v));
    put(p, 8, address(v) + used);
    put(p, 12, address(v) + size);
    // Old fixture buffers remain owned until episode teardown.
  }
  put(reinterpret_cast<void *>(get(p, 8)), 0, address(*item));
  put(p, 8, get(p, 8) + 4);
}
void __attribute__((thiscall)) multiTeardown(void *p) {
  for (const auto &task : rawTasks)
    if (p == task->data.data()) {
      if (get(p, 0xc) >= 100)
        currentOriginal->retired.push_back(get(p, 0x1f));
      return;
    }
}

unsigned __attribute__((thiscall)) multiExecute(void *p, void *k, int tick,
                                                int *budget) {
  currentOriginal->calls.push_back(get(p, 0x1f));
  const int before = budget[2];
  const auto value = reinterpret_cast<unsigned(__attribute__((thiscall)) *)(
      void *, void *, int, int *)>(0x432900)(p, k, tick, budget);
  currentOriginal->visits.insert(
      currentOriginal->visits.end(),
      {get(p, 0x1f), unsigned(before), unsigned(budget[2]), value & 255u});
  return value;
}
unsigned __attribute__((thiscall)) multiNext(void *task, float *out,
                                             int *budget, int tick) {
  auto &log = currentOriginal->candidates;
  std::vector<unsigned> members;
  std::function<void(unsigned)> walk = [&](unsigned source) {
    auto *p = reinterpret_cast<void *>(source);
    if (auto actor = get(p, 0x35))
      members.push_back(get(reinterpret_cast<void *>(actor)));
    else
      for (auto at = get(p, 0x29); at != get(p, 0x2d); at += 4)
        walk(get(reinterpret_cast<void *>(at)));
  };
  walk(get(task, 8));
  const int before = budget[2];
  const auto result = reinterpret_cast<unsigned(__attribute__((thiscall)) *)(
      void *, float *, int *, int)>(0x431ce0)(task, out, budget, tick);
  log.push_back(members.size());
  log.insert(log.end(), members.begin(), members.end());
  log.insert(log.end(), {unsigned(before), unsigned(budget[2]), result & 255u});
  if (result & 255u) {
    log.insert(log.end(), {bits(*out), get(task, 0x15b), get(task, 0x157)});
    need(get(task, 0x167) - get(task, 0x163) == 72 * 4,
         "candidate feature count");
    for (unsigned i = 0; i < 72; ++i)
      log.push_back(reinterpret_cast<unsigned *>(get(task, 0x163))[i]);
  }
  return result;
}
void __attribute__((thiscall)) multiTarget(void *a, void *b) {
  currentOriginal->orders.insert(currentOriginal->orders.end(),
                                 {get(a), 1, get(b)});
}
void __attribute__((thiscall)) multiMove(void *a, int x, int y, int z,
                                         unsigned p, unsigned q, unsigned r) {
  need(!p && !q && !r, "move options");
  currentOriginal->orders.insert(
      currentOriginal->orders.end(),
      {get(a), 2, unsigned(x), unsigned(y), unsigned(z)});
}
void __attribute__((thiscall)) multiStop(void *a, unsigned) {
  currentOriginal->orders.insert(currentOriginal->orders.end(), {get(a), 3});
}
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_BRAIN_REFERENCE
  need(argc == 3, "PE NEW_CAPTURE");
  const char *path = argv[2];
  map_image(load(argv[1]));
  std::array<unsigned char, 4096> thread{};
  user_desc segment{};
  segment.entry_number = unsigned(-1);
  segment.base_addr = address(thread.data());
  segment.limit = 0xfffff;
  segment.seg_32bit = segment.limit_in_pages = segment.useable = 1;
  need(!syscall(SYS_set_thread_area, &segment), "FS");
  unsigned short selector = (segment.entry_number << 3) | 3u;
  asm volatile("movw %0, %%fs" ::"rm"(selector));
  redirect(0x597850, reinterpret_cast<void *>(&multiAllocate));
  redirect(0x597870, reinterpret_cast<void *>(&multiRelease));
  redirect(0x42a340, reinterpret_cast<void *>(&multiAppend));
  redirect(0x418050, reinterpret_cast<void *>(&multiTeardown));
  redirect(0x476e30, reinterpret_cast<void *>(&training));
  redirect(0x416fe0, reinterpret_cast<void *>(&multiRegister));
  redirect(0x4181d0, reinterpret_cast<void *>(&multiSight));
  redirect(0x421f70, reinterpret_cast<void *>(&multiPlan));
  for (unsigned a : {0x417580u, 0x428e40u, 0x428f80u})
    redirect(a, reinterpret_cast<void *>(&multiNoop));
  redirect(0x50af10, reinterpret_cast<void *>(&multiTarget));
  redirect(0x50fd50, reinterpret_cast<void *>(&multiMove));
  redirect(0x50fef0, reinterpret_cast<void *>(&multiStop));
  multiTable = {0x431be0,
                address(reinterpret_cast<void *>(&multiClone)),
                address(reinterpret_cast<void *>(&multiNext)),
                0x4261b0,
                0x4328d0,
                address(reinterpret_cast<void *>(&multiExecute)),
                0x432fb0};
  taskVtable = {multiTable[0], multiTable[1], multiTable[2],
                multiTable[3], multiTable[4], multiTable[5]};
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "RX");
  Trace trace(path, true);
#else
  need(argc == 2, "CAPTURE");
  const char *path = argv[1];
  Trace trace(path, false);
#endif
  unsigned installs = 0, executions = 0, pending = 0, splits = 0, merges = 0,
           retirements = 0;
  for (unsigned n = 0; n < episodes; ++n) {
    trace.check(n, "fixture");
    Deck d(n);
#ifdef MNM_BRAIN_REFERENCE
    using RCtor = void *(__attribute__((thiscall)) *)(void *, unsigned);
    reinterpret_cast<RCtor>(0x53a660)(reinterpret_cast<void *>(0x6eaf08), 512);
    RawModel rm(d.brain);
    Original state(d, n);
#else
    Native state(d, n);
#endif
    unsigned lastSerial = 0, lastCount = 4;
    for (unsigned frame = 0; frame < frames; ++frame) {
      trace.check(frame, "frame");
      int tick = 100 + int(frame);
#ifdef MNM_BRAIN_REFERENCE
      tick = int(get(state.old.brain.data(), 0x14)) + 1;
      if (frame == 64) {
        auto tasks = state.tasks();
        if (!tasks.empty())
          tick = int(get(tasks.front().second, 0x10));
      }
#endif
      tick = int(trace.word(unsigned(tick)));
      const int total =
          int(trace.word(frame < 4        ? frame
                         : frame % 9 == 0 ? 0
                                          : 32 + (n + frame) % 9));
      const bool disabled = trace.flag(frame % 31 == 0);
      const int net = int(trace.word((n & 2) ? 1 : 0));
      const int raw = int(trace.word(frame > 90 && frame % 11 == 0 ? 50 : 0));
      need(tick >= 100 && tick < 1000000 && total >= 0 && total <= 40 &&
               (net == 0 || net == 1) && (raw == 0 || raw == 50),
           "stimulus domain");
#ifdef MNM_BRAIN_REFERENCE
      put(state.old.brain.data(), 0x14, tick - 1);
      state.advance(total, disabled, net, raw);
#else
      state.driver.clock = tick - 1;
      state.advance(total, disabled, net, raw);
#endif
      std::vector<unsigned> v;
      auto add = [&](unsigned x) { v.push_back(x); };
      auto append = [&](const auto &list) {
        add(list.size());
        v.insert(v.end(), list.begin(), list.end());
      };
#ifdef MNM_BRAIN_REFERENCE
      auto *b = state.old.brain.data();
      add(get(b, 0x14));
      add(get(b, 0x11dab));
      add(state.id(get(b, 0x11daf)));
      append(state.list(get(b, 0x11db7), get(b, 0x11dbb)));
      add(b[0x11dd3]);
      add(get(b, 0x11ddd));
      add(b[0x11dd3] ? UINT32_MAX : get(b, 0x11dd4));
      add(b[0x11dd3] ? UINT32_MAX : get(b, 0x11dd8));
      add(*reinterpret_cast<unsigned *>(0x6a29d8));
      add(*reinterpret_cast<unsigned *>(0x6a29dc));
      for (unsigned i = 0; i < 250; ++i)
        add(reinterpret_cast<unsigned *>(0x6a29e0)[i]);
      auto mapKeys = state.mapKeys(0xbe), pendingKeys = state.mapKeys(0x11dc7);
      need(pendingKeys.size() == get(b, 0x11dcf), "pending task map count");
      std::vector<unsigned> boundKeys;
      for (auto [key, p] : state.tasks()) {
        (void)p;
        boundKeys.push_back(key);
      }
      need(boundKeys == mapKeys, "task map/source binding conservation");
      append(mapKeys);
      append(pendingKeys);
      auto live = state.list(get(b, 0x1e), get(b, 0x22));
      append(live);
      add(state.count);
      const auto parents = state.parents();
      for (unsigned i = 0; i < state.count; ++i) {
        add(parents[i]);
        append(state.list(get(state.node(i), 0x29), get(state.node(i), 0x2d)));
      }
      auto tasks = state.tasks();
      add(get(b, 0xca));
      add(tasks.size());
      for (auto [key, p] : tasks) {
        add(key);
        add(state.id(get(p, 8)));
        for (unsigned off : {0xcu, 0x10u, 0x1bu, 0x2bu, 0x2fu, 0x33u})
          add(get(p, off));
        add(*(static_cast<unsigned char *>(p) + 0x14));
        add(*(static_cast<unsigned char *>(p) + 0x15));
        auto *target = reinterpret_cast<void *>(get(p, 0x16f));
        add(get(target, 13));
        add(get(target, 25));
        add(get(p, 0x173));
        unsigned count = (get(p, 0x183) - get(p, 0x17f)) / 4;
        need(count == 72, "saved features");
        add(count);
        for (unsigned j = 0; j < count; ++j)
          add(reinterpret_cast<unsigned *>(get(p, 0x17f))[j]);
      }
      append(state.calls);
      append(state.retired);
      append(state.orders);
      append(state.candidates);
      append(state.visits);
      installs += get(b, 0xca) - lastSerial;
      lastSerial = get(b, 0xca);
      executions += state.calls.size();
      pending += get(b, 0x11dab) != 0;
      retirements += state.retired.size();
      splits += state.count > lastCount;
      lastCount = state.count;
#else
      add(state.driver.clock);
      add(state.driver.phase);
      add(state.driver.current.value_or(UINT32_MAX));
      append(d.storage.pendingRoots);
      add(state.decision.reset || !state.current);
      add(bits(state.selection.value -
               (state.current ? 0 : state.decision.winningBias)));
      add(state.decision.reset || !state.current ? UINT32_MAX
                                                 : state.decision.stage);
      add(state.decision.reset || !state.current
              ? UINT32_MAX
              : unsigned(state.decision.winner));
      add(state.random.left);
      add(state.random.right);
      for (auto x : state.random.words)
        add(x);
      std::vector<unsigned> mapKeys;
      for (auto &[key, t] : state.tasks) {
        (void)t;
        mapKeys.push_back(key);
      }
      append(mapKeys);
      append(state.driver.execution);
      append(d.storage.manager.nodes);
      add(d.storage.sources.graph.size());
      for (auto &g : d.storage.sources.graph) {
        add(g.parent.value_or(UINT32_MAX));
        append(g.children);
      }
      add(state.serial);
      add(state.tasks.size());
      for (auto &[key, t] : state.tasks) {
        add(key);
        add(t.root);
        for (auto x :
             {unsigned(t.previous.started), unsigned(t.previous.expires),
              bits(t.previous.value), bits(t.previous.baseline),
              unsigned(t.previous.successes), unsigned(t.previous.attempts)})
          add(x);
        add(t.complete);
        add(t.previous.completed);
        auto &target = t.data.task.groups.nodes[t.data.task.root];
        add(target.position.x);
        add(target.region);
        add(t.data.task.risk);
        add(t.data.features.size());
        for (int x : t.data.features)
          add(unsigned(x));
      }
      append(state.calls);
      append(state.retired);
      append(state.orders);
      append(state.candidates);
      append(state.visits);
      installs += state.serial - lastSerial;
      lastSerial = state.serial;
      executions += state.calls.size();
      pending += state.driver.phase != 0;
      retirements += state.retired.size();
      splits += d.storage.sources.graph.size() > lastCount;
      lastCount = d.storage.sources.graph.size();
#endif
      try {
        trace.vector(v, "state");
      } catch (const std::exception &e) {
        throw std::runtime_error(std::string(e.what()) +
                                 " episode=" + std::to_string(n) +
                                 " frame=" + std::to_string(frame));
      }
      // Count actual parented original roots as a topology witness, independent
      // of the selected-score and task-state comparison above.
#ifdef MNM_BRAIN_REFERENCE
      merges += state.parents()[2] != UINT32_MAX;
#else
      merges += d.storage.sources.graph[2].parent.has_value();
#endif
    }
#ifdef MNM_BRAIN_REFERENCE
    rawTasks.clear();
#endif
  }
#ifdef MNM_BRAIN_REFERENCE
  need(std::equal(text.begin(), text.end(),
                  reinterpret_cast<unsigned char *>(0x401000)),
       "private text changed");
#endif
  need(installs && executions && pending && splits && merges && retirements,
       "pipeline witnesses");
  trace.finish(path);
  std::cout << "{\"success\":true,\"cases\":" << episodes
            << ",\"slices\":" << episodes * frames << ",\"checks\":" << checks
            << ",\"installed\":" << installs << ",\"executed\":" << executions
            << ",\"pending\":" << pending << ",\"split_slices\":" << splits
            << ",\"merged_slices\":" << merges
            << ",\"retirements\":" << retirements << ",\"digest\":\"" << digest
            << "\"}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
