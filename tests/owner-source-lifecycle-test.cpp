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
void traceGraph(const m::SourceGraph &a, const m::SourceGraph &b,
                const std::vector<unsigned> &ids, bool cursors) {
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
  need(!syscall(SYS_set_thread_area, &segment), "source lifecycle FS");
  unsigned short selector = (segment.entry_number << 3) | 3u;
  asm volatile("movw %0, %%fs" ::"rm"(selector));
  redirect(0x597850, reinterpret_cast<void *>(&sourceAllocate));
  redirect(0x597870, reinterpret_cast<void *>(&sourceRelease));
  redirect(0x42a340, reinterpret_cast<void *>(&sourceAppend));
  redirect(0x4289e0, reinterpret_cast<void *>(&sourceInstall));
  redirect(0x416fe0, reinterpret_cast<void *>(&sourceRegister));
  redirect(0x42b7c0, reinterpret_cast<void *>(&sourceErase));
  redirect(0x418050, reinterpret_cast<void *>(&sourceTeardown));
  const Bytes text(reinterpret_cast<unsigned char *>(0x401000),
                   reinterpret_cast<unsigned char *>(0x5c0000));
  need(!mprotect(reinterpret_cast<void *>(0x401000), 0x1bf000,
                 PROT_READ | PROT_EXEC),
       "source lifecycle RX");
#else
  (void)argc;
  (void)argv;
#endif
  unsigned pruned = 0, protectedCases = 0;
  for (unsigned n = 0; n < 4096; ++n) {
    m::AutonomousWorld world;
    auto input = fixture(n, world), s = input;
    for (unsigned i = 0; i < 5; ++i)
      for (unsigned j = 0; j < 8; ++j) {
        auto &node = s.sources.graph[i];
        node.free[j] = (n >> (j % 8)) & 1;
        node.position[j] =
            node.children.empty() ? 0 : (n + j) % (node.children.size() + 1);
        node.nested[j] = (n + j) % 8;
      }
    input = s;
    const unsigned parent = n % 2 ? 3 : 4, child = n % 2 ? 0 : 3;
    if (n & 1)
      m::detachSourceChild(s.sources.graph, parent, child);
    else
      m::clearSourceChildren(s.sources.graph, parent);
#ifdef MNM_BRAIN_REFERENCE
    RawSources raw(input);
    if (n & 1)
      reinterpret_cast<void(__attribute__((thiscall)) *)(void *, unsigned)>(
          0x407780)(raw.nodes[parent].data(), raw.pointer(child));
    else
      reinterpret_cast<void(__attribute__((thiscall)) *)(void *)>(0x407820)(
          raw.nodes[parent].data());
    const auto old = raw.graph(input.sources.graph);
#else
    const auto old = s.sources.graph;
#endif
    traceGraph(s.sources.graph, old, {0, 1, 2, 3, 4}, true);
  }
  for (unsigned n = 0; n < 1024; ++n) {
    m::AutonomousWorld world;
    auto input = fixture(n, world), s = input;
    const std::optional<unsigned> current =
        n % 5 == 0 ? std::optional<unsigned>(3) : std::nullopt;
    const std::optional<unsigned> other =
        n % 7 == 0 ? std::optional<unsigned>(4) : std::nullopt;
    auto removed = m::pruneOwnerSources(s, world, current, other);
    pruned += removed.size();
    protectedCases += removed.empty();
#ifdef MNM_BRAIN_REFERENCE
    RawSources raw(input);
    put(raw.brain.data(), 0x11daf, current ? raw.pointer(*current) : 0);
    put(raw.brain.data(), 0x11fc8, other ? raw.pointer(*other) : 0);
    reinterpret_cast<void(__attribute__((thiscall)) *)(void *)>(0x417580)(
        raw.brain.data());
    const auto old = raw.graph(input.sources.graph);
    auto actual =
        raw.list(get(raw.brain.data(), 0x1e), get(raw.brain.data(), 0x22));
    record(removed.size(), raw.released.size(), "pruned source count");
    for (unsigned i = 0; i < removed.size(); ++i)
      record(removed[i], raw.released[i], "prune swap removal order");
    record(s.manager.nodes.size(), actual.size(), "live source vector count");
    for (unsigned i = 0; i < actual.size(); ++i)
      record(s.manager.nodes[i], actual[i], "live source vector order");
    for (unsigned i = 0; i < 3; ++i) {
      const auto &q = i == 0   ? s.pending.general
                      : i == 1 ? s.pending.wizard
                               : s.pending.other;
      const auto list = raw.list(get(raw.brain.data() + 0x2a + i * 0x30, 12),
                                 get(raw.brain.data() + 0x2a + i * 0x30, 28));
      record(q.size(), list.size(), "pruned queue count");
      for (unsigned j = 0; j < q.size(); ++j)
        record(q[j], list[j], "pruned queue order");
      record(s.sources.roots[i][0].value_or(UINT32_MAX),
             raw.id(get(raw.actors.data() + i * 0xe4b, 0x678)),
             "pruned actor root");
    }
    auto pending = raw.list(get(raw.brain.data(), 0x11db7),
                            get(raw.brain.data(), 0x11dbb));
    record(s.pendingRoots.size(), pending.size(), "pruned root work count");
    for (unsigned i = 0; i < pending.size(); ++i)
      record(s.pendingRoots[i], pending[i], "pruned root work order");
    record(unsigned(s.sources.mode), *reinterpret_cast<unsigned *>(0x5d854c),
           "prune restores mode");
#else
    const auto old = s.sources.graph;
    record(removed.size(), removed.size(), "pruned source count");
    for (auto id : removed)
      record(id, id, "prune swap removal order");
    record(s.manager.nodes.size(), s.manager.nodes.size(),
           "live source vector count");
    for (auto id : s.manager.nodes)
      record(id, id, "live source vector order");
    for (unsigned i = 0; i < 3; ++i) {
      const auto &q = i == 0   ? s.pending.general
                      : i == 1 ? s.pending.wizard
                               : s.pending.other;
      record(q.size(), q.size(), "pruned queue count");
      for (auto id : q)
        record(id, id, "pruned queue order");
      record(s.sources.roots[i][0].value_or(UINT32_MAX),
             s.sources.roots[i][0].value_or(UINT32_MAX), "pruned actor root");
    }
    record(s.pendingRoots.size(), s.pendingRoots.size(),
           "pruned root work count");
    for (auto id : s.pendingRoots)
      record(id, id, "pruned root work order");
    record(unsigned(s.sources.mode), unsigned(s.sources.mode),
           "prune restores mode");
#endif
    traceGraph(s.sources.graph, old, s.manager.nodes, true);
    m::validateOwnerSourceStorage(s, 3);
  }
  unsigned splits = 0, merges = 0, displaced = 0;
  for (unsigned n = 0; n < 512; ++n) {
    m::AutonomousWorld world;
    auto s = fixture(0, world);
    s.sources.graph.resize(2);
    s.sources.information.resize(2);
    s.sources.graph[0].parent.reset();
    s.sources.graph[1].parent.reset();
    s.sources.graph[0].activity = s.sources.graph[1].activity = 3;
    s.sources.roots[2][0].reset();
    s.manager.nodes = {0, 1};
    s.pending = {};
    s.pendingRoots.clear();
    unsigned left = 0, right = 1;
    auto wrap = [&](unsigned child) {
      const unsigned id = s.sources.graph.size();
      m::SourceGraphNode group;
      group.free.fill(1);
      group.children = {child};
      s.sources.graph.push_back(group);
      s.sources.graph[child].parent = id;
      s.sources.information.emplace_back();
      s.manager.nodes.push_back(id);
      return id;
    };
    const bool partition = n & 1;
    if (partition) {
      left = wrap(0);
      s.sources.graph[left].children.push_back(1);
      s.sources.graph[1].parent = left;
    } else {
      if (n & 2)
        left = wrap(left);
      if (n & 4)
        right = wrap(right);
    }
    if (n & 8)
      m::bindOwnerSourceTask(s, left, 0);
    if (!partition && (n & 16))
      m::bindOwnerSourceTask(s, right, 1);
    const auto input = s;
    const auto change = partition ? m::splitOwnerSource(s, world, left, {0})
                                  : m::mergeOwnerSources(s, left, right);
    splits += partition;
    merges += !partition;
    displaced += change.retiredTasks.size();
#ifdef MNM_BRAIN_REFERENCE
    RawSources raw(input);
    raw.brain[0x19] = 1;
    put(raw.brain.data(), 0x11dd4, 5);
    put(raw.brain.data(), 0x11dd8, partition ? 4 : 2);
    put(raw.brain.data(), 0x11ddd, bits(0.5f));
    if (partition) {
      raw.brain[0x11e25] = 0;
      raw.brain[0x11e26] = 1;
      put(raw.brain.data(), 0x11fd8, 0);
    } else
      put(raw.brain.data(), 0x11fd4, raw.pointer(right));
    int budget[3] = {0, 0, 1};
    reinterpret_cast<void(__attribute__((thiscall)) *)(void *, void *, int *)>(
        0x423590)(raw.brain.data(), raw.nodes[left].data(), budget);
    const auto old = raw.graph(input.sources.graph);
    record(s.sources.graph.size(), old.size(), "selected topology node count");
    record(change.retiredTasks.size(), raw.retiredTasks.size(),
           "displaced task count");
    for (unsigned i = 0; i < change.retiredTasks.size(); ++i)
      record(change.retiredTasks[i], raw.retiredTasks[i],
             "displaced task retirement order");
    record(partition ? left : change.root, raw.installed,
           "selected task recipient source");
    record(partition ? change.root : UINT32_MAX, raw.registered,
           "split remainder registration");
    auto actual =
        raw.list(get(raw.brain.data(), 0x1e), get(raw.brain.data(), 0x22));
    record(s.manager.nodes.size(), actual.size(),
           "selected topology manager count");
    for (unsigned i = 0; i < actual.size(); ++i)
      record(s.manager.nodes[i], actual[i], "selected topology manager order");
#else
    const auto old = s.sources.graph;
    record(s.sources.graph.size(), old.size(), "selected topology node count");
    record(change.retiredTasks.size(), change.retiredTasks.size(),
           "displaced task count");
    for (auto task : change.retiredTasks)
      record(task, task, "displaced task retirement order");
    record(partition ? left : change.root, partition ? left : change.root,
           "selected task recipient source");
    record(partition ? change.root : UINT32_MAX,
           partition ? change.root : UINT32_MAX,
           "split remainder registration");
    record(s.manager.nodes.size(), s.manager.nodes.size(),
           "selected topology manager count");
    for (auto id : s.manager.nodes)
      record(id, id, "selected topology manager order");
#endif
    traceGraph(s.sources.graph, old, s.manager.nodes, false);
    m::validateOwnerSourceStorage(s, 3);
  }
  need(pruned && protectedCases, "source maintenance branches");
#ifdef MNM_BRAIN_REFERENCE
  need(!std::memcmp(reinterpret_cast<void *>(0x401000), text.data(),
                    text.size()),
       "source lifecycle original text unchanged");
#endif
  std::cout << "{\"success\":true,\"cases\":5632,\"checks\":" << checks
            << ",\"pruned\":" << pruned << ",\"protected\":" << protectedCases
            << ",\"splits\":" << splits << ",\"merges\":" << merges
            << ",\"displaced_tasks\":" << displaced << ",\"digest\":\""
            << digest << "\"}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
