#define main unused_source_brain_fixture_main
#include "owner-brain-inference-test.cpp"
#undef main
#include "../reconstruction/creature-ai/source_lifecycle.hpp"
#include <set>
namespace {
m::OwnerSourceStorage fixture(unsigned n, m::AutonomousWorld &world) {
  world.dimensions = {32, 32, 8};
  world.creatures.resize(3);
  auto &g = world.creatures;
  for (unsigned i = 0; i < 3; ++i) {
    g[i].ai.slot = i;
    g[i].ai.owner = i == 2;
    g[i].ai.kind = 1;
    g[i].ai.type = 17;
  }
  m::OwnerSourceStorage s;
  s.sources.graph.resize(5);
  s.sources.information.resize(5);
  s.sources.roots.resize(3);
  s.sources.actorFlag8e5.resize(3);
  s.sources.mode = n % 2 ? 1 : 3;
  for (unsigned i = 0; i < 5; ++i) {
    auto &node = s.sources.graph[i];
    node.free.fill(1);
    if (i < 3) {
      node.actor = i;
      node.activity = (n >> (i * 2)) & 3;
      s.sources.roots[i][0] = i;
    }
    s.manager.nodes.push_back(i);
  }
  s.sources.graph[3].children = {0, 1};
  s.sources.graph[4].children = {3, 2};
  s.sources.graph[0].parent = s.sources.graph[1].parent = 3;
  s.sources.graph[3].parent = s.sources.graph[2].parent = 4;
  for (auto *q : {&s.pending.general, &s.pending.wizard, &s.pending.other})
    for (unsigned i = 0; i < 5; ++i)
      if ((n + i) % 3)
        q->push_back(i);
  s.pendingRoots = {4, 0, 2};
  if (n & 64)
    m::bindOwnerSourceTask(s, (n / 128) % 5, 0);
  return s;
}
[[maybe_unused]] void traceGraph(const m::SourceGraph &a,
                                 const m::SourceGraph &b,
                                 const std::vector<unsigned> &ids,
                                 bool cursors) {
  for (auto id : ids) {
    record(a[id].parent.value_or(UINT32_MAX), b[id].parent.value_or(UINT32_MAX),
           "source parent");
    record(a[id].children.size(), b[id].children.size(), "source child count");
    for (unsigned i = 0; i < a[id].children.size(); ++i)
      record(a[id].children[i], b[id].children[i], "source child order");
    if (cursors)
      for (unsigned i = 0; i < 8; ++i) {
        record(a[id].free[i], b[id].free[i], "source free cursor");
        record(a[id].position[i], b[id].position[i], "source cursor position");
        record(a[id].nested[i], b[id].nested[i], "source nested cursor");
      }
  }
}
#ifdef MNM_BRAIN_REFERENCE
unsigned get(const void *p, unsigned off) {
  unsigned v;
  std::memcpy(&v, static_cast<const unsigned char *>(p) + off, 4);
  return v;
}
struct RawSources;
RawSources *currentSources;
void __attribute__((cdecl)) sourceRelease(void *);
void *__attribute__((cdecl)) sourceAllocate(unsigned);
void __attribute__((thiscall)) sourceAppend(void *, void *, unsigned, void **);
void __attribute__((thiscall)) sourceInstall(void *, void *);
void __attribute__((thiscall)) sourceRegister(void *, void *);
void __attribute__((thiscall)) sourceErase(void *, unsigned) {}
void __attribute__((thiscall)) sourceTeardown(void *) {}
struct RawSources {
  Bytes brain = Bytes(0x12000), actors = Bytes(3 * 0xe4b),
        owners = Bytes(8 * 340), sourceList = Bytes(16 * 4),
        rootWork = Bytes(16 * 4), tasks = Bytes(4 * 64), nil = Bytes(64);
  std::vector<Bytes> nodes = std::vector<Bytes>(16, Bytes(0x8d)),
                     children = std::vector<Bytes>(16, Bytes(16 * 4)),
                     heads = std::vector<Bytes>(4, Bytes(64)),
                     blocks = std::vector<Bytes>(3, Bytes(4096)),
                     maps = std::vector<Bytes>(3, Bytes(4));
  unsigned allocated = 0, installed = UINT32_MAX, registered = UINT32_MAX;
  std::vector<unsigned> released, retiredTasks;
  std::vector<Bytes> extras = std::vector<Bytes>(16, Bytes(64));
  unsigned extraCount = 0;
  unsigned pointer(unsigned id) const { return address(nodes.at(id).data()); }
  unsigned id(unsigned ptr) const {
    if (!ptr)
      return UINT32_MAX;
    for (unsigned i = 0; i < nodes.size(); ++i)
      if (pointer(i) == ptr)
        return i;
    throw std::runtime_error("unknown original source pointer");
  }
  void iterator(void *p, unsigned i, unsigned pos) {
    put(p, 0, address(blocks[i].data()));
    put(p, 4, address(blocks[i].data() + 4096));
    put(p, 8, address(blocks[i].data() + pos));
    put(p, 12, address(maps[i].data()));
  }
  RawSources(const m::OwnerSourceStorage &s) {
    currentSources = this;
    allocated = s.sources.graph.size();
    for (unsigned i = 0; i < allocated; ++i) {
      auto *p = nodes[i].data();
      const auto &n = s.sources.graph[i];
      const auto &info = s.sources.information[i];
      put(p, 0, info.kind);
      p[4] = info.kind == 0;
      if (info.recipient) {
        put(p, 5, address(tasks.data() + *info.recipient * 64));
        put(tasks.data() + *info.recipient * 64, 8, pointer(i));
        put(tasks.data() + *info.recipient * 64, 0x23, UINT32_MAX);
        *(tasks.data() + *info.recipient * 64 + 0x15) = 1;
      }
      put(p, 0x35, n.actor ? address(actors.data() + *n.actor * 0xe4b) : 0);
      put(p, 0x39, n.parent ? pointer(*n.parent) : 0);
      put(p, 0x29, address(children[i].data()));
      put(p, 0x2d, address(children[i].data() + n.children.size() * 4));
      put(p, 0x31, address(children[i].data() + children[i].size()));
      for (unsigned j = 0; j < n.children.size(); ++j)
        put(children[i].data(), j * 4, pointer(n.children[j]));
      put(p, 0x89, n.activity);
      put(p, 0x3d, n.roundRobin);
      for (unsigned j = 0; j < 8; ++j) {
        p[0x41 + j] = n.free[j];
        put(p, 0x49 + j * 4, address(children[i].data() + n.position[j] * 4));
        put(p, 0x69 + j * 4, n.nested[j]);
      }
    }
    for (unsigned i = 0; i < 3; ++i) {
      auto *p = actors.data() + i * 0xe4b;
      put(p, 0, i);
      put(p, 0x174, i == 2);
      for (unsigned f = 0; f < 2; ++f)
        if (auto root = s.sources.roots[i][f])
          put(p, 0x678 + f * 4, pointer(*root));
    }
    for (unsigned i = 0; i < s.manager.nodes.size(); ++i)
      put(sourceList.data(), i * 4, pointer(s.manager.nodes[i]));
    put(brain.data(), 0x1e, address(sourceList.data()));
    put(brain.data(), 0x22,
        address(sourceList.data() + s.manager.nodes.size() * 4));
    put(brain.data(), 0x26, address(sourceList.data() + sourceList.size()));
    for (unsigned i = 0; i < s.pendingRoots.size(); ++i)
      put(rootWork.data(), i * 4, pointer(s.pendingRoots[i]));
    put(brain.data(), 0x11db7, address(rootWork.data()));
    put(brain.data(), 0x11dbb,
        address(rootWork.data() + s.pendingRoots.size() * 4));
    put(brain.data(), 0x11dbf, address(rootWork.data() + rootWork.size()));
    for (unsigned i = 0; i < 3; ++i) {
      auto *q = brain.data() + 0x2a + i * 0x30;
      const auto &list = i == 0   ? s.pending.general
                         : i == 1 ? s.pending.wizard
                                  : s.pending.other;
      put(maps[i].data(), 0, address(blocks[i].data()));
      iterator(q + 4, i, 128);
      iterator(q + 20, i, 128 + list.size() * 4);
      put(q, 0x2c, list.size());
      for (unsigned j = 0; j < list.size(); ++j)
        put(blocks[i].data(), 128 + j * 4, pointer(list[j]));
    }
    nil[16] = 1;
    for (unsigned o : {0u, 4u, 8u})
      put(nil.data(), o, address(nil.data()));
    global(0x5faf50, address(nil.data()));
    for (unsigned i = 0; i < 4; ++i) {
      auto *h = heads[i].data();
      h[16] = 1;
      put(h, 0, address(h));
      put(h, 4, address(nil.data()));
      put(h, 8, address(h));
      put(brain.data(),
          std::array<unsigned, 4>{{0xbe, 0x11dc7, 0x11df9, 0x11e19}}[i],
          address(h));
    }
    std::memcpy(reinterpret_cast<void *>(0x643a80), owners.data(),
                owners.size());
    global(0x5d854c, s.sources.mode);
  }
  m::SourceGraph graph(const m::SourceGraph &before) const {
    auto out = before;
    out.resize(allocated);
    for (unsigned i = 0; i < allocated; ++i) {
      auto *p = nodes[i].data();
      auto &n = out[i];
      const auto parent = id(get(p, 0x39));
      n.parent = parent == UINT32_MAX ? std::optional<unsigned>{} : parent;
      n.children.clear();
      for (auto at = get(p, 0x29); at && at != get(p, 0x2d); at += 4)
        n.children.push_back(id(get(reinterpret_cast<void *>(at), 0)));
      for (unsigned j = 0; j < 8; ++j) {
        n.free[j] = p[0x41 + j];
        n.nested[j] = get(p, 0x69 + j * 4);
        n.position[j] =
            get(p, 0x29) ? (get(p, 0x49 + j * 4) - get(p, 0x29)) / 4 : 0;
      }
    }
    return out;
  }
  std::vector<unsigned> list(unsigned begin, unsigned end) const {
    std::vector<unsigned> out;
    for (auto at = begin; at != end; at += 4)
      out.push_back(id(get(reinterpret_cast<void *>(at), 0)));
    return out;
  }
};
void __attribute__((cdecl)) sourceRelease(void *p) {
  if (!p)
    return;
  auto &r = *currentSources;
  for (unsigned i = 0; i < r.allocated; ++i)
    if (p == r.nodes[i].data()) {
      r.released.push_back(i);
      return;
    }
  for (const auto &v : r.children)
    if (p == v.data())
      return;
  for (const auto &v : r.blocks)
    if (p == v.data())
      return;
  for (const auto &v : r.maps)
    if (p == v.data())
      return;
  for (const auto &v : r.extras)
    if (p == v.data())
      return;
  if (p >= r.tasks.data() && p < r.tasks.data() + r.tasks.size()) {
    r.retiredTasks.push_back(
        unsigned(static_cast<unsigned char *>(p) - r.tasks.data()) / 64);
    return;
  }
  throw std::runtime_error("source release storage boundary pointer=" +
                           std::to_string(address(p)));
}
void *__attribute__((cdecl)) sourceAllocate(unsigned bytes) {
  auto &r = *currentSources;
  if (bytes == 0x8d) {
    need(r.allocated < 16, "source node capacity");
    return r.nodes[r.allocated++].data();
  }
  need(bytes <= 4096 && r.extraCount < r.extras.size(),
       "source allocation storage capacity");
  auto &extra = r.extras[r.extraCount++];
  extra.resize(4096);
  return extra.data();
}
void __attribute__((thiscall)) sourceAppend(void *p, void *where,
                                            unsigned count, void **item) {
  auto &r = *currentSources;
  need(count == 1 && get(p, 8) == address(where), "source append boundary");
  if (!get(p, 4)) {
    bool found = false;
    for (unsigned i = 0; i < r.allocated; ++i)
      if (p == r.nodes[i].data() + 0x25) {
        put(p, 4, address(r.children[i].data()));
        put(p, 8, get(p, 4));
        put(p, 12, address(r.children[i].data() + r.children[i].size()));
        found = true;
        break;
      }
    if (!found) {
      need(r.extraCount < r.extras.size(), "source temporary vector capacity");
      auto &extra = r.extras[r.extraCount++];
      put(p, 4, address(extra.data()));
      put(p, 8, get(p, 4));
      put(p, 12, address(extra.data() + extra.size()));
    }
  }
  need(get(p, 8) + 4 <= get(p, 12), "source append capacity");
  put(reinterpret_cast<void *>(get(p, 8)), 0, address(*item));
  put(p, 8, get(p, 8) + 4);
}
void __attribute__((thiscall)) sourceInstall(void *, void *source) {
  currentSources->installed = currentSources->id(address(source));
}
void __attribute__((thiscall)) sourceRegister(void *, void *source) {
  currentSources->registered = currentSources->id(address(source));
}
#endif
} // namespace

#include "../reconstruction/creature-ai/source_decision.hpp"
namespace {
unsigned caseNumber = 0;
[[maybe_unused]] unsigned mask = 0, emitted = 0;
std::vector<unsigned> savedMembers;
[[maybe_unused]] float score(unsigned members) {
  const std::array<float, 7> values{{-1, -.75f, -.25f, 0, .25f, .75f, 1}};
  return values[(caseNumber / 4 + members * 3) % values.size()];
}
#ifdef MNM_BRAIN_REFERENCE
std::vector<unsigned> rawMembers(unsigned ptr) {
  auto *p = reinterpret_cast<void *>(ptr);
  if (auto a = get(p, 0x35))
    return {get(reinterpret_cast<void *>(a), 0)};
  std::vector<unsigned> v;
  for (auto at = get(p, 0x29); at != get(p, 0x2d); at += 4) {
    auto x = rawMembers(get(reinterpret_cast<void *>(at), 0));
    v.insert(v.end(), x.begin(), x.end());
  }
  return v;
}
void __attribute__((thiscall)) decisionPrepare(void *, void *source, void *,
                                               int) {
  mask = 0;
  emitted = 0;
  for (auto id : rawMembers(address(source)))
    mask |= 1u << id;
}
unsigned __attribute__((thiscall)) decisionNext(void *, float *out, int *b,
                                                int) {
  if (emitted || b[2] < 1)
    return 0;
  ++emitted;
  --b[2];
  *out = score(mask);
  return 1;
}
void __attribute__((thiscall)) decisionSave(void *object) {
  savedMembers = rawMembers(get(object, 8));
}
int __attribute__((thiscall)) decisionSpecial(void *, int) { return 0; }
int __attribute__((thiscall)) decisionDistance(void *, void *, void *) {
  return 0;
}
#endif
} // namespace
#ifndef MNM_SOURCE_DECISION_FIXTURE
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
  need(!syscall(SYS_set_thread_area, &segment), "decision FS");
  unsigned short selector = (segment.entry_number << 3) | 3u;
  asm volatile("movw %0, %%fs" ::"rm"(selector));
  redirect(0x597850, reinterpret_cast<void *>(&sourceAllocate));
  redirect(0x597870, reinterpret_cast<void *>(&sourceRelease));
  redirect(0x42a340, reinterpret_cast<void *>(&sourceAppend));
  redirect(0x4289e0, reinterpret_cast<void *>(&sourceInstall));
  redirect(0x416fe0, reinterpret_cast<void *>(&sourceRegister));
  redirect(0x42b7c0, reinterpret_cast<void *>(&sourceErase));
  redirect(0x418050, reinterpret_cast<void *>(&sourceTeardown));
  redirect(0x429270, reinterpret_cast<void *>(&decisionDistance));
  redirect(0x523240, reinterpret_cast<void *>(&decisionSpecial));
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "decision RX");
#else
  (void)argc;
  (void)argv;
#endif
  unsigned slices = 0, split = 0, merge = 0, kept = 0, none = 0;
  for (unsigned n = 0; n < 512; ++n) {
    caseNumber = n;
    m::AutonomousWorld world;
    auto storage = fixture(0, world);
    storage.sources.graph.resize(3);
    storage.sources.information.resize(3);
    storage.sources.roots[2][0].reset();
    storage.pending = {};
    storage.pendingRoots.clear();
    const bool grouped = n % 2;
    [[maybe_unused]] const unsigned root = grouped ? 2 : 0;
    storage.sources.graph[0].activity = storage.sources.graph[1].activity = 3;
    storage.sources.graph[0].parent = storage.sources.graph[1].parent =
        grouped ? std::optional<unsigned>(2) : std::nullopt;
    storage.sources.graph[2].actor.reset();
    storage.sources.graph[2].children =
        grouped ? std::vector<unsigned>{0, 1} : std::vector<unsigned>{};
    storage.sources.graph[2].parent.reset();
    storage.manager.nodes =
        grouped ? std::vector<unsigned>{0, 1, 2} : std::vector<unsigned>{0, 1};
    m::OwnerSourceDecisionInputs input;
    input.members =
        grouped ? std::vector<unsigned>{0, 1} : std::vector<unsigned>{0};
    input.speeds = grouped ? std::vector<int>{n & 8 ? 48 : 24, n & 16 ? 48 : 24}
                           : std::vector<int>{24};
    if (!grouped)
      input.otherMembers = {1};
    input.splitFirst = grouped && (n & 2);
    input.weighted = n & 4;
    if (!grouped && (n & 8)) {
      input.otherTask.emplace();
      input.otherTask->completed = true;
      input.otherTask->value = float(int((n / 16) % 3) - 1) * .75f;
      m::bindOwnerSourceTask(storage, 1, 0);
    }
    m::OwnerSourceDecision state;
    m::OwnerTaskSelection selection;
    auto random = m::seedRandom(n + 17);
    unsigned nativeMask = 0, nativeEmitted = 0;
    std::vector<unsigned> nativeSaved;
    m::OwnerSourceDecisionServices svc;
    svc.select = [&](const auto &members, bool rootSource, float bias,
                     const auto *old, bool reject, int &b) {
      m::OwnerTaskSelectionServices lower;
      lower.count = 1;
      lower.root = rootSource;
      lower.prepare = [&](unsigned) {
        nativeMask = 0;
        nativeEmitted = 0;
        for (auto id : members)
          nativeMask |= 1u << id;
      };
      lower.next = [&](unsigned, int &budget) -> std::optional<float> {
        if (nativeEmitted || budget < 1)
          return {};
        ++nativeEmitted;
        --budget;
        return score(nativeMask);
      };
      lower.save = [&](unsigned) { nativeSaved = members; };
      lower.finish = [] {};
      return m::advanceOwnerTaskReplanning(selection, b, random, lower, old,
                                           100, {}, bias, true, reject);
    };
#ifdef MNM_BRAIN_REFERENCE
    RawSources raw(storage);
    if (input.otherTask)
      put(raw.tasks.data(), 0x1b, bits(input.otherTask->value));
    global(0x5faf54, address(raw.nil.data()));
    global(0x5faf58, address(raw.nil.data()));
    Bytes configs(3 * 0x5c9), object(0x160);
    std::array<unsigned, 4> table{
        {address(reinterpret_cast<void *>(&decisionPrepare)), 0,
         address(reinterpret_cast<void *>(&decisionNext)),
         address(reinterpret_cast<void *>(&decisionSave))}};
    std::array<unsigned, 1> list{{address(object.data())}};
    put(object.data(), 0, address(table.data()));
    put(raw.brain.data(), 8, address(list.data()));
    put(raw.brain.data(), 12, address(list.data() + 1));
    raw.brain[0x19] = 1;
    raw.brain[0x11dd3] = 1;
    raw.brain[0x11fe0] = 1;
    for (unsigned off : {0x11ea6u, 0x11f33u}) {
      auto *p = raw.brain.data() + off;
      for (unsigned j = 0; j < 8; ++j)
        p[0x41 + j] = 1;
    }
    for (unsigned j = 0; j < 2; ++j) {
      auto *a = raw.actors.data() + j * 0xe4b;
      auto *cfg = configs.data() + j * 0x5c9;
      put(a, 0xac, address(cfg));
      put(a, 0xde7, 0);
      put(cfg, 0x589, grouped ? input.speeds[j] : 24);
    }
    global(0x643a80 + 12, 0);
    global(0x5d865c, input.splitFirst ? 20 : 0);
    global(0x5d8650, input.weighted ? 2 : 0);
    global(0x6e1f64, 0);
    global(0x6a29d8, random.left);
    global(0x6a29dc, random.right);
    std::memcpy(reinterpret_cast<void *>(0x6a29e0), random.words.data(), 1000);
    savedMembers.clear();
#endif
    bool done = false;
    for (unsigned tick = 0; !done && tick < 100; ++tick) {
      int budget = 1 + int((n + tick) % 4);
      const int grant = budget;
      done = m::advanceOwnerSourceDecision(state, selection, input, budget,
                                           random, svc);
      ++slices;
#ifdef MNM_BRAIN_REFERENCE
      int b[3] = {0, 0, grant};
      reinterpret_cast<void(__attribute__((thiscall)) *)(
          void *, void *, int *)>(0x423590)(raw.brain.data(),
                                            raw.nodes[root].data(), b);
      record(unsigned(budget), unsigned(b[2]), "decision budget");
      record(done, raw.brain[0x11dd3], "decision completion");
      record(unsigned(state.winner), get(raw.brain.data(), 0x11dd8),
             "automatic winner");
      record(bits(selection.value - (done ? state.winningBias : 0)),
             get(raw.brain.data(), 0x11ddd), "automatic value");
      if (!done)
        record(state.stage, get(raw.brain.data(), 0x11dd4), "decision stage");
      record(random.left, get(reinterpret_cast<void *>(0x6a29d8), 0),
             "decision RNG left");
      record(random.right, get(reinterpret_cast<void *>(0x6a29dc), 0),
             "decision RNG right");
      for (unsigned k = 0; k < 250; ++k)
        record(random.words[k], get(reinterpret_cast<void *>(0x6a29e0), k * 4),
               "decision RNG word");
      need(nativeSaved == savedMembers, "decision saved member order");
#else
      (void)grant;
      for (unsigned v :
           {unsigned(budget), unsigned(done), unsigned(state.winner),
            bits(selection.value - (done ? state.winningBias : 0))})
        record(v, v, "native decision");
      if (!done)
        record(state.stage, state.stage, "native stage");
      for (unsigned v : {random.left, random.right})
        record(v, v, "native RNG");
      for (auto v : random.words)
        record(v, v, "native RNG word");
#endif
    }
    need(done, "decision slice bound");
    split += state.winner == 4;
    merge += state.winner == 2;
    kept += state.winner == 0;
    none += state.winner == -1;
  }
  need(split && merge && kept, "automatic decision witnesses");
#ifdef MNM_BRAIN_REFERENCE
  need(!std::memcmp(reinterpret_cast<void *>(0x401000), text.data(),
                    text.size()),
       "decision text unchanged");
#endif
  std::cout << "{\"success\":true,\"cases\":512,\"slices\":" << slices
            << ",\"checks\":" << checks << ",\"splits\":" << split
            << ",\"merges\":" << merge << ",\"kept\":" << kept
            << ",\"none\":" << none << ",\"digest\":\"" << digest << "\"}\n";
} catch (const std::exception &e) {
  std::cerr << "case " << caseNumber << ": " << e.what() << '\n';
  return 1;
}

#endif
