// Full ordinary task6 generation is called by the original423590 driver.
// Supplied world/memory/model construction and object clone/teardown are
// explicit.
#pragma push_macro("main")
#undef main
#define main unused_loop_journey_main
#include "owner-task-probe-test.cpp"
#undef main
#pragma pop_macro("main")
#include "../reconstruction/creature-ai/owner_group_features.hpp"
#include "../reconstruction/creature-ai/owner_group_task.hpp"
#include "../reconstruction/creature-ai/owner_task_inputs.hpp"
#include "../reconstruction/creature-ai/owner_task_lifecycle.hpp"
#include <memory>
namespace {
m::BrainInferenceModel model(unsigned n) {
  m::BrainInferenceModel out;
  out.key = 6;
  out.dimension = 72;
  for (unsigned k = 0; k < 3; ++k) {
    m::BrainInferenceLayer l;
    l.observations = 1;
    l.outside = float(k + 1);
    l.nodes.resize(72);
    l.matrix.assign(72 * 72, 0);
    for (auto &v : l.nodes) {
      v[1] = 100;
      for (unsigned j = 2; j < 12; ++j)
        v[j] = float(k + 1) * float(j) / 8;
    }
    // Real nonuniform energy, dependent on produced enemy count/distance/path.
    for (unsigned j : {8u, 35u, 68u})
      l.matrix[j * 72 + j] = float((n + k) % 5 + 1) / 16;
    out.layers.push_back(l);
  }
  return out;
}
struct NativeLoop {
  m::OwnerTaskSelection selection;
  m::OwnerMemoryGroups groups;
  m::OwnerTaskProbe probe;
  m::RandomState random;
  std::optional<m::OwnerPreviousTask> previous;
  std::optional<m::OwnerGroupTask> candidate, best, active;
  std::vector<int> features, bestFeatures, activeFeatures;
  unsigned cursor = 0, serial = 0;
  int duration = 0, bestDuration = 0, deadline = 0;
  bool eligible = false;
  NativeLoop(unsigned seed) : random(m::seedRandom(seed)) {}
  bool advance(m::OwnerKnowledge &knowledge,
               std::vector<m::NavigationRegion> &graph,
               const m::BrainInferenceModel &brain, int tick, int &budget,
               bool enabled) {
    if (selection.reset) {
      selection.value = -2;
      selection.selected = -1;
      selection.ties = 0;
      best.reset();
      bestFeatures.clear();
    }
    m::OwnerTaskSelectionServices services;
    services.count = 1;
    services.root = true;
    services.prepare = [&](unsigned) {
      deadline = tick + 22;
      eligible = m::admitOwnerTask6(
          {enabled, 0, false, true, true, false, true, false});
      groups = {};
      cursor = 0;
      probe = {};
      if (eligible) {
        std::array<std::array<std::uint8_t, 8>, 8> relation{};
        relation[0][0] = 1;
        m::buildOwnerMemoryGroups(
            groups, knowledge, 0, tick, true, {32, 32, 8}, relation,
            {[&](int a, int b, int &v) {
               return m::regionalDangerCenter(
                   graph, a, b, {4, 1, 1, 3, 0, 0, 0}, {32, 32, 8}, v);
             },
             [](auto, auto) { return false; }});
      }
    };
    services.next = [&](unsigned, int &b) -> std::optional<float> {
      if (!eligible)
        return {};
      auto &roots = groups.nodes[0].children;
      while (cursor < roots.size() && tick <= deadline) {
        auto root = roots[cursor];
        auto &node = groups.nodes[root];
        m::RegionalQuery q;
        q.movement = {1, 1, 0, 3, 0, 17, 0};
        m::RegionalServices lower;
        lower.danger = [&](int id) {
          return int(graph.at(unsigned(id)).dangerBase);
        };
        auto result = m::advanceOwnerTaskProbe(probe, graph, 0, node.region, q,
                                               {32, 32, 8}, b, lower);
        if (result == m::OwnerTaskProbeResult::pending)
          return {};
        ++cursor;
        if (result == m::OwnerTaskProbeResult::failed) {
          probe = {};
          continue;
        }
        m::OwnerPlannerActor friendly;
        friendly.position = {2, 2, 2};
        friendly.speed = 24;
        friendly.physical = true;
        std::vector<m::OwnerPlannerActor> enemies;
        for (int key : m::ownerGroupKeys(groups, root)) {
          auto it = std::find_if(knowledge.records[0].begin(),
                                 knowledge.records[0].end(),
                                 [&](auto &r) { return r.memory.key == key; });
          enemies.push_back(*it->planning);
        }
        auto prefix =
            m::hostileTaskGroupFeatures({friendly}, enemies, {32, 32, 8});
        features.assign(prefix.begin(), prefix.end());
        auto at = std::find_if(probe.search.records.begin(),
                               probe.search.records.end(),
                               [&](auto &r) { return r.node == node.region; });
        need(at != probe.search.records.end(), "loop probe record");
        auto payload = mnm::reconstruction::neighbor_movement(at->movement);
        duration = m::ownerTask6Duration(
            random, m::ownerTaskTravelTime(payload.accumulated_cost, 24));
        features.insert(
            features.end(),
            {int(std::sqrt(static_cast<long double>(node.uncertaintySquared))),
             m::ownerTaskTravelTime(payload.accumulated_cost, 24),
             payload.unknown_18, lower.danger(node.region), 0});
        auto p = m::inferOwnerTask(brain, features);
        int risk = 0;
        for (int id : probe.path)
          risk = std::max(risk, lower.danger(id));
        candidate = m::OwnerGroupTask{groups, root, risk};
        probe = {};
        return m::ownerTaskValue({p[0], p[1], p[2]}, {0, 0, 1}, 100);
      }
      return {};
    };
    services.save = [&](unsigned) {
      best = candidate;
      bestDuration = duration;
      bestFeatures = features;
    };
    services.finish = [] {};
    m::advanceOwnerTaskSelection(selection, budget, random, services);
    if (selection.reset && best &&
        m::replaceOwnerTask(selection.value, previous ? &*previous : nullptr,
                            tick, {})) {
      active = best;
      activeFeatures = bestFeatures;
      previous = m::OwnerPreviousTask{};
      previous->started = tick;
      previous->expires = tick + bestDuration;
      previous->value = selection.value;
      previous->weighted = selection.weighted;
      ++serial;
      return true;
    }
    return false;
  }
};
#ifdef MNM_BRAIN_REFERENCE
std::vector<unsigned> commands;
void __attribute__((thiscall)) targetCommand(void *, void *target) {
  commands = {1, *reinterpret_cast<unsigned *>(target)};
}
void __attribute__((thiscall)) moveCommand(void *, int x, int y, int z,
                                           unsigned adjust, unsigned option,
                                           unsigned other) {
  need(!adjust && !option && !other, "hostile movement options");
  commands = {2, unsigned(x), unsigned(y), unsigned(z)};
}
void __attribute__((thiscall)) stopCommand(void *, unsigned) { commands = {3}; }
void guardAllocations() {
  for (auto item : allocated) {
    auto *p = static_cast<unsigned char *>(item.first);
    auto n = item.second;
    for (unsigned i = 0; i < 64; ++i)
      need(p[i] == 0xa5 && p[n - 64 + i] == 0xa5,
           "original fixture allocation redzone");
  }
}
void *__attribute__((cdecl)) guardedAllocate(unsigned n) {
  guardAllocations();
  auto *p = static_cast<unsigned char *>(allocate(n + 128));
  std::fill(p, p + n + 128, 0xa5);
  return p + 64;
}
void __attribute__((cdecl)) guardedRelease(void *p) {
  if (!p)
    return;
  guardAllocations();
  release(static_cast<unsigned char *>(p) - 64);
}

struct RawModel {
  Bytes head = Bytes(64), node = Bytes(64), nil = Bytes(64), layers;
  std::vector<Bytes> rows;
  std::vector<std::vector<Bytes>> matrices;
  RawModel(const m::BrainInferenceModel &m)
      : layers(m.layers.size() * 40), rows(m.layers.size()),
        matrices(m.layers.size()) {
    void *manager = reinterpret_cast<void *>(0x6c3898);
    std::memset(manager, 0, 32);
    put(manager, 4, address(head.data()));
    put(manager, 12, 1);
    for (unsigned o : {0u, 4u, 8u})
      put(head.data(), o, address(node.data()));
    head[16] = nil[16] = 1;
    put(node.data(), 0, address(nil.data()));
    put(node.data(), 4, address(head.data()));
    put(node.data(), 8, address(nil.data()));
    put(node.data(), 12, 6);
    put(node.data(), 16, 72);
    put(node.data(), 28, address(layers.data()));
    put(node.data(), 32, address(layers.data() + layers.size()));
    put(node.data(), 36, address(layers.data() + layers.size()));
    global(0x5fce98, address(nil.data()));
    for (unsigned i = 0; i < m.layers.size(); ++i) {
      auto &l = m.layers[i];
      auto *p = layers.data() + 40 * i;
      put(p, 0, bits(l.observations));
      put(p, 20, bits(l.outside));
      put(p, 8, address(l.nodes.data()));
      put(p, 12, address(l.nodes.data() + 72));
      put(p, 16, address(l.nodes.data() + 72));
      rows[i].resize(72 * 16);
      matrices[i].resize(72);
      for (unsigned j = 0; j < 72; ++j) {
        matrices[i][j].resize(72 * 4);
        std::memcpy(matrices[i][j].data(), l.matrix.data() + j * 72, 72 * 4);
        auto *r = rows[i].data() + j * 16;
        put(r, 4, address(matrices[i][j].data()));
        put(r, 8, address(matrices[i][j].data() + 72 * 4));
        put(r, 12, address(matrices[i][j].data() + 72 * 4));
      }
      put(p, 28, address(rows[i].data()));
      put(p, 32, address(rows[i].data() + rows[i].size()));
      put(p, 36, address(rows[i].data() + rows[i].size()));
    }
  }
};
struct RawTask {
  Bytes data = Bytes(0x1b4);
  std::array<int, 72> candidate{}, saved{};
  RawTask(const unsigned *vt) {
    using Ctor = void *(__attribute__((thiscall)) *)(void *);
    reinterpret_cast<Ctor>(0x41b010)(data.data() + 0xec);
    put(data.data(), 0, address(vt));
    put(data.data(), 4, 6);
    put(data.data(), 0x23, unsigned(-1));
    put(data.data(), 0x163, address(candidate.data()));
    put(data.data(), 0x167, address(candidate.data()));
    put(data.data(), 0x16b, address(candidate.data() + 72));
    put(data.data(), 0x17f, address(saved.data()));
    put(data.data(), 0x183, address(saved.data()));
    put(data.data(), 0x187, address(saved.data() + 72));
  }
};
std::vector<std::unique_ptr<RawTask>> rawTasks;
std::array<unsigned, 6> taskVtable;
unsigned trainingCalls = 0;
unsigned __attribute__((thiscall)) account(void *, unsigned, float, float,
                                           float) {
  return 34;
}
void *__attribute__((thiscall)) cloneTask(void *) {
  rawTasks.push_back(std::make_unique<RawTask>(taskVtable.data()));
  return rawTasks.back()->data.data();
}
unsigned __attribute__((thiscall)) training(void *, unsigned, void *) {
  ++trainingCalls;
  return unsigned(-1);
}
void __attribute__((thiscall)) teardown(void *) {}
void __attribute__((cdecl)) loopRelease(void *p) {
  for (auto &t : rawTasks)
    if (p == t->data.data())
      return;
  guardedRelease(p);
}
struct OriginalLoop {
  OriginalProbe world;
  Bytes brain = Bytes(0x12100), queueHead = Bytes(32), pendingHead = Bytes(32),
        queueNil = Bytes(32), head = Bytes(0xe68), nil = Bytes(0xe68);
  Bytes sourcesHead = Bytes(32), partitionHead = Bytes(32);
  std::vector<Bytes> memories;
  std::array<unsigned, 1> list{};
  OriginalLoop(const std::vector<m::NavigationRegion> &graph,
               const m::OwnerKnowledge &k, const m::RandomState &random,
               bool weighted)
      : world(graph, 0, 7), memories(k.records[0].size(), Bytes(0xe68)) {
    put(world.pool.data(), 8, 2);
    put(world.pool.data() + 0xe4b, 8, 14);
    // Ordinary associated owner actor satisfies preparation without introducing
    // a wizard source.
    for (unsigned i = 0; i < 3; ++i)
      global(0x6c48fc + 6 * 12 + i * 4, bits(i == 2 ? 1.f : 0.f));
    for (unsigned i = 0; i < 8; ++i)
      *reinterpret_cast<unsigned char *>(0x643a80 + 24 + i) = i == 0;
    for (unsigned o : {0u, 4u, 8u})
      put(queueNil.data(), o, address(queueNil.data()));
    queueNil[16] = 1;
    global(0x5faf50, address(queueNil.data()));
    for (auto *h : {queueHead.data(), pendingHead.data()}) {
      put(h, 0, address(h));
      put(h, 4, address(queueNil.data()));
      put(h, 8, address(h));
      h[16] = 1;
    }
    put(brain.data(), 0xbe, address(queueHead.data()));
    put(brain.data(), 0x11dc7, address(pendingHead.data()));
    for (auto *h : {sourcesHead.data(), partitionHead.data()}) {
      put(h, 0, address(h));
      put(h, 4, address(queueNil.data()));
      put(h, 8, address(h));
      h[16] = 1;
    }
    put(brain.data(), 0x11df9, address(sourcesHead.data()));
    put(brain.data(), 0x11e19, address(partitionHead.data()));
    cloneTask(nullptr);
    list[0] = address(rawTasks.back()->data.data());
    put(brain.data(), 8, address(list.data()));
    put(brain.data(), 12, address(list.data() + 1));
    brain[0x19] = brain[0x11dd3] = brain[0x11fe0] = 1;
    put(brain.data(), 0x11c06, address(head.data()));
    nil[0xe64] = 1;
    global(0x5faf4c, address(nil.data()));
    const auto h = address(head.data()), n = address(nil.data());
    put(head.data(), 0,
        memories.empty() ? h : address(memories.front().data()));
    put(head.data(), 4,
        memories.empty() ? n : address(memories.front().data()));
    put(head.data(), 8, memories.empty() ? h : address(memories.back().data()));
    for (unsigned i = 0; i < memories.size(); ++i) {
      auto *p = memories[i].data();
      auto &r = k.records[0][i];
      std::memcpy(p + 24, world.pool.data() + 0xe4b, 0xe4b);
      put(p, 0, n);
      put(p, 4, i ? address(memories[i - 1].data()) : h);
      put(p, 8, i + 1 < memories.size() ? address(memories[i + 1].data()) : n);
      put(p, 12, r.memory.key);
      put(p, 16, r.memory.observedTick);
      put(p, 20, r.markedTick);
      put(p, 32, r.memory.actor.position.x);
      put(p, 36, r.memory.actor.position.y);
      put(p, 40, r.memory.actor.position.z);
      put(p, 0x11c, r.memory.actor.flag104);
      put(p, 0xdff, r.memory.actor.region);
      put(p, 0xb7b, r.planning->motionB63);
      p[0xe64] = 1;
    }
    global(0x5d8650, weighted ? 2 : 0);
    global(0x5d8654, bits(m::OwnerReplacementPolicy{}.improvement));
    global(0x5d8658, bits(m::OwnerReplacementPolicy{}.expiry));
    global(0x6a29d8, random.left);
    global(0x6a29dc, random.right);
    std::memcpy(reinterpret_cast<void *>(0x6a29e0), random.words.data(), 1000);
  }
  unsigned active() { return get(world.source.data(), 5); }
  void advance(int tick, int &budget, bool enabled) {
    put(brain.data(), 0x14, tick);
    put(world.pool.data(), 0x936, enabled ? 100 : 0);
    int raw[3] = {0, 0, budget};
    using Plan = void(__attribute__((thiscall)) *)(void *, void *, int *);
    reinterpret_cast<Plan>(0x423590)(brain.data(), world.source.data(), raw);
    budget = raw[2];
    guardAllocations();
  }
  void verify(const NativeLoop &s, int budget, int other) {
    record(unsigned(budget), unsigned(other), "loop budget");
    record(s.selection.reset, brain[0x11dd3], "loop sweep completed");
    record(bits(s.selection.value), get(brain.data(), 0x11ddd),
           "loop selected value");
    record(s.random.left, *reinterpret_cast<unsigned *>(0x6a29d8),
           "loop RNG left");
    record(s.random.right, *reinterpret_cast<unsigned *>(0x6a29dc),
           "loop RNG right");
    for (unsigned i = 0; i < 250; ++i)
      record(s.random.words[i], reinterpret_cast<unsigned *>(0x6a29e0)[i],
             "loop RNG word");
    record(bool(s.active), bool(active()), "loop task presence");
    record(s.serial, get(brain.data(), 0xca), "loop install serial");
    if (!active())
      return;
    auto *p = reinterpret_cast<void *>(active());
    auto saved = reinterpret_cast<int *>(get(p, 0x17f));
    record(unsigned(s.activeFeatures.size()),
           (get(p, 0x183) - get(p, 0x17f)) / 4, "loop saved feature count");
    for (unsigned i = 0; i < s.activeFeatures.size(); ++i)
      record(unsigned(s.activeFeatures[i]), unsigned(saved[i]),
             ("loop saved feature " + std::to_string(i)).c_str());
    auto &old = *s.previous;
    for (auto pair : std::vector<std::pair<unsigned, unsigned>>{
             {unsigned(old.started), 0xc},
             {unsigned(old.expires), 0x10},
             {bits(old.value), 0x1b},
             {bits(old.baseline), 0x2b},
             {unsigned(old.successes), 0x2f},
             {unsigned(old.attempts), 0x33}})
      record(pair.first, get(p, pair.second),
             ("loop installed field " + std::to_string(pair.second) +
              " candidateDuration=" + std::to_string(get(p, 0x15b)) +
              " savedDuration=" + std::to_string(get(p, 0x177)))
                 .c_str());
    record(old.completed, *(static_cast<unsigned char *>(p) + 0x15),
           "loop completed flag");
    auto target = reinterpret_cast<void *>(get(p, 0x16f));
    auto &node = s.active->groups.nodes[s.active->root];
    record(unsigned(node.position.x), get(target, 13), "loop chosen target x");
    record(unsigned(node.region), get(target, 25), "loop chosen target region");
    record(unsigned(s.active->risk), get(p, 0x173), "loop selected risk");
  }
};
#endif
} // namespace
int main(int argc, char **argv) try {
#ifdef MNM_BRAIN_REFERENCE
  need(argc == 2, "PE");
  map_image(load(argv[1]));
  std::array<unsigned char, 4096> thread{};
  user_desc segment{};
  segment.entry_number = unsigned(-1);
  segment.base_addr = address(thread.data());
  segment.limit = 0xfffff;
  segment.seg_32bit = segment.limit_in_pages = segment.useable = 1;
  need(!syscall(SYS_set_thread_area, &segment), "loop FS");
  unsigned short selector =
      static_cast<unsigned short>((segment.entry_number << 3) | 3u);
  asm volatile("movw %0, %%fs" ::"rm"(selector));
  redirect(0x597850, reinterpret_cast<void *>(&guardedAllocate));
  redirect(0x597870, reinterpret_cast<void *>(&loopRelease));
  redirect(0x416d80, reinterpret_cast<void *>(&account));
  redirect(0x418050, reinterpret_cast<void *>(&teardown));
  redirect(0x476e30, reinterpret_cast<void *>(&training));
  redirect(0x50af10, reinterpret_cast<void *>(&targetCommand));
  redirect(0x50fd50, reinterpret_cast<void *>(&moveCommand));
  redirect(0x50fef0, reinterpret_cast<void *>(&stopCommand));
  taskVtable = {0x431be0, address(reinterpret_cast<void *>(&cloneTask)),
                0x431ce0, 0x4261b0,
                0x4328d0, 0x432900};
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "loop RX");
#else
  (void)argc;
  (void)argv;
#endif
  unsigned slices = 0, installed = 0, retained = 0, replaced = 0, orders = 0,
           stops = 0;
  unsigned refusals = 0;
  for (int travel : {-1, INT32_MAX}) {
    auto r = m::seedRandom(17);
    try {
      (void)m::ownerTask6Duration(r, travel);
    } catch (const std::invalid_argument &) {
      ++refusals;
    }
  }
  auto zero = m::seedRandom(17);
  zero.words.fill(0);
  try {
    (void)m::ownerTask6Duration(zero, 0);
  } catch (const std::invalid_argument &) {
    ++refusals;
  }
  need(refusals == 3, "task6 lifetime refusals");
  for (unsigned n = 0; n < 128; ++n) {
    auto brain = model(n);
    std::vector<m::NavigationRegion> graph(8);
    for (unsigned i = 0; i < 8; ++i) {
      auto &r = graph[i];
      r.position = {int(i * 2), 2, 2};
      r.parameter = 1;
      r.dangerBase = i % 3;
      if (i + 1 < 8) {
        auto &e = r.connections[0];
        e.destination = int(i + 1);
        e.cost = 8;
        e.clearance.fill(8);
      }
    }
    m::OwnerKnowledge knowledge;
    for (unsigned i = 0; i < 2; ++i) {
      m::OwnerMemory r;
      r.memory.key = 5 + int(i * 3);
      r.memory.observedTick = r.markedTick = 100;
      r.memory.actor.slot = 1;
      r.memory.actor.owner = 1;
      r.memory.actor.health = 100;
      r.memory.actor.flag104 = (n % 2 && i == 1);
      r.memory.actor.region = 6 + int(i);
      r.memory.actor.position = graph[6 + i].position;
      r.planning.emplace();
      r.planning->position = r.memory.actor.position;
      r.planning->speed = 24;
      r.planning->physical = true;
      r.planning->flag104 = r.memory.actor.flag104;
      r.planning->danger = graph[6 + i].dangerBase;
      knowledge.records[0].push_back(r);
    }
    NativeLoop native(n * 137 + 17);
    native.selection.weighted = n & 2;
#ifdef MNM_BRAIN_REFERENCE
    using RCtor = void *(__attribute__((thiscall)) *)(void *, unsigned);
    reinterpret_cast<RCtor>(0x53a660)(reinterpret_cast<void *>(0x6eaf08), 512);
    RawModel rawModel(brain);
    OriginalLoop old(graph, knowledge, native.random,
                     native.selection.weighted);
#endif
    do {
      int budget = 4, other = 4;
      native.advance(knowledge, graph, brain, 100, budget, false);
      ++slices;
#ifdef MNM_BRAIN_REFERENCE
      old.advance(100, other, false);
      old.verify(native, budget, other);
#else
      (void)other;
      for (unsigned v : {unsigned(budget), unsigned(native.selection.reset),
                         bits(native.selection.value), native.random.left,
                         native.random.right})
        record(v, v, "disabled native");
      for (unsigned v : native.random.words)
        record(v, v, "disabled RNG");
      record(0, 0, "disabled task");
      record(0, 0, "disabled serial");
#endif
    } while (!native.selection.reset);
    int tick = 100;
    for (unsigned sweep = 0; sweep < 5; ++sweep) {
      // Same generated proposal is initially installed, retained, then charged
      // and considered after expiry. No selected value or duration is authored.
      if (sweep == 2 && native.previous) {
        m::addOwnerTaskDamage(nullptr, &*native.previous, 1, 34);
#ifdef MNM_BRAIN_REFERENCE
        put(old.world.pool.data(), 0x660, 1);
        put(old.world.pool.data(), 0x678, address(old.world.source.data()));
        using Damage =
            unsigned(__attribute__((thiscall)) *)(void *, unsigned, void *);
        need((reinterpret_cast<Damage>(0x514990)(old.world.pool.data(), 1,
                                                 nullptr) &
              255) == 1,
             "loop nonlethal damage");
#endif
        tick = std::max(tick, native.previous->expires +
                                  3 * (native.previous->expires -
                                       native.previous->started) +
                                  100);
      }
      const unsigned before = native.serial;
      do {
        int budget = 1 + int((n + slices) % 5), other = budget;
        native.advance(knowledge, graph, brain, tick, budget, true);
        ++slices;
#ifdef MNM_BRAIN_REFERENCE
        old.advance(tick, other, true);
        old.verify(native, budget, other);
#else
        (void)other;
        for (unsigned v : {unsigned(budget), unsigned(native.selection.reset),
                           bits(native.selection.value), native.random.left,
                           native.random.right})
          record(v, v, "native loop");
        for (unsigned v : native.random.words)
          record(v, v, "native RNG word");
        record(bool(native.active), bool(native.active), "native active");
        record(native.serial, native.serial, "native install serial");
        if (native.active) {
          record(unsigned(native.activeFeatures.size()),
                 unsigned(native.activeFeatures.size()),
                 "native feature count");
          for (int f : native.activeFeatures)
            record(unsigned(f), unsigned(f), "native saved feature");
          auto &p = *native.previous;
          for (unsigned v :
               {unsigned(p.started), unsigned(p.expires), bits(p.value),
                bits(p.baseline), unsigned(p.successes), unsigned(p.attempts),
                unsigned(p.completed),
                unsigned(native.active->groups.nodes[native.active->root]
                             .position.x),
                unsigned(
                    native.active->groups.nodes[native.active->root].region),
                unsigned(native.active->risk)})
            record(v, v, "native installed");
        }
#endif
        need(slices < 100000, "loop never completed");
      } while (!native.selection.reset);
      if ((sweep == 0 || sweep == 3) && native.active) {
        m::AutonomousWorld live;
        live.dimensions = {32, 32, 8};
        live.relation[0][0] = 1;
        live.creatures.resize(2);
        for (unsigned i = 0; i < 2; ++i) {
          auto &a = live.creatures[i];
          a.ai.slot = int(i);
          a.ai.kind = 1;
          a.ai.owner = int(i);
          a.ai.type = 17 + int(i);
          a.ai.active = true;
          a.ai.health = 100;
          a.ai.physical = 1;
          a.ai.position = {i ? 14 : 2, 2, 2};
        }
        for (unsigned phase = 0; phase < (sweep == 3 ? 4u : 3u); ++phase) {
          for (auto &r : knowledge.records[0])
            r.markedTick = phase == 1 ? tick - 1 : tick;
          int raw = phase == 3 ? 50 : 0, budget = 8, other = 8;
          auto out =
              m::executeOwnerGroupTask(*native.active, knowledge, tick, 0, 0,
                                       live, raw, raw, 0, {{0, 0}}, budget);
          std::vector<unsigned> order;
          order.reserve(5);
          order.push_back(unsigned(out.continuing));
          if (out.stop) {
            order.push_back(3);
            ++stops;
          } else if (out.kind == m::OwnerHostileOrderKind::target)
            order.insert(order.end(), {1, unsigned(out.target)});
          else if (out.kind == m::OwnerHostileOrderKind::move)
            order.insert(order.end(),
                         {2, unsigned(out.position.x), unsigned(out.position.y),
                          unsigned(out.position.z)});
#ifdef MNM_BRAIN_REFERENCE
          for (auto &row : old.memories)
            put(row.data(), 20, phase == 1 ? tick - 1 : tick);
          old.world.regions[644] = unsigned(raw);
          commands.clear();
          int rawBudget[3] = {0, 0, other};
          using Execute =
              unsigned(__attribute__((thiscall)) *)(void *, void *, int, int *);
          auto result = reinterpret_cast<Execute>(0x432900)(
                            reinterpret_cast<void *>(old.active()),
                            old.brain.data() + 0x11c02, tick, rawBudget) &
                        255;
          auto expected = commands;
          expected.insert(expected.begin(), result);
          need(order == expected, "installed task execution order");
          other = rawBudget[2];
          old.world.regions[644] = 0;
          if (!out.continuing)
            reinterpret_cast<unsigned char *>(old.active())[0x15] = 1;
#else
          other = budget;
#endif
          if (!out.continuing)
            native.previous->completed = true;
          record(unsigned(budget), unsigned(other),
                 "installed task execution budget");
          for (unsigned v : order)
            record(v, v, "installed task order");
          ++orders;
        }
      }
      if (!sweep)
        installed += native.serial > before;
      else if (native.serial == before)
        ++retained;
      else
        ++replaced;
    }
#ifdef MNM_BRAIN_REFERENCE
    guardAllocations();
    rawTasks.clear();
#endif
  }
  need(installed && retained && replaced, "loop decision witnesses");
#ifdef MNM_BRAIN_REFERENCE
  need(std::equal(text.begin(), text.end(),
                  reinterpret_cast<unsigned char *>(0x401000)),
       "loop target text changed");
#endif
  std::cout << "{\"success\":true,\"cases\":128,\"checks\":" << checks
            << ",\"slices\":" << slices << ",\"installed\":" << installed
            << ",\"retained\":" << retained << ",\"replaced\":" << replaced
            << ",\"execution_slices\":" << orders << ",\"risk_stops\":" << stops
            << ",\"refusals\":" << refusals << ",\"digest\":\"" << digest
            << "\"}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
