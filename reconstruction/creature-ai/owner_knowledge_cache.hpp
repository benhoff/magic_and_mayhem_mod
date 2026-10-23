#pragma once
#include "owner_memory_groups.hpp"
#include "owner_task_sources.hpp"
#include <algorithm>
#include <climits>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>
namespace mnm::reconstruction::combat {
// NoCD 40209ca7: one owner's two source-cache vectors at knowledge+10/+20.
// Memory records/snapshots are borrowed. Successful bounded allocations only;
// observation-tree allocation/erasure and World scheduling remain separate.
// M adds allocateCacheSource() to the checked OwnerTaskSources memory
// interface.
template <class M> class OwnerKnowledgeCache {
  using U = std::uint32_t;
  M &m;
  OwnerTaskSources<M> sources;
  U r(U p) { return m.get32(p); }
  void w(U p, U v) { m.set32(p, v); }
  static int si(U v) {
    int x;
    std::memcpy(&x, &v, 4);
    return x;
  }
  static int norm(int v, int d) {
    v %= d;
    return v < 0 ? v + d : v;
  }
  static int delta(int to, int from, int d) {
    int v = norm(to - from, d);
    return v > d / 2 ? v - d : v;
  }
  U h(unsigned bank) {
    if (bank > 1)
      throw std::invalid_argument("knowledge cache bank");
    return header + 0x10 + 16 * bank;
  }
  void append(U at, U value) {
    U b = r(at + 4), e = r(at + 8), c = r(at + 12);
    if (e == c) {
      U n = b ? (e - b) / 4 : 0, cap = n + std::max(n, 1u),
        p = m.allocate(cap * 4);
      if (!p)
        throw std::runtime_error("cache vector allocation");
      for (U i = 0; i < n; ++i)
        w(p + 4 * i, r(b + 4 * i));
      m.free(b);
      b = p;
      w(at + 4, b);
      w(at + 12, b + cap * 4);
      w(at + 8, b + n * 4);
    }
    w(r(at + 8), value);
    w(at + 8, r(at + 8) + 4);
  }
  U make(unsigned bank, U actor = 0) {
    U p = m.allocateCacheSource();
    if (!p)
      throw std::runtime_error("cache source allocation");
    w(p, ~U(0));
    m.set8(p + 4, 0);
    for (U o :
         {5u, 0xdu, 0x11u, 0x15u, 0x1du, 0x29u, 0x2du, 0x31u, 0x39u, 0x3du})
      w(p + o, 0);
    // 4174f0 uses bank as its tag;417e80 uses the snapshot argument.
    m.set8(p + 0x25, actor ? actor & 255 : bank);
    w(p + 0x35, actor);
    w(p + 0x41, 0x01010101);
    w(p + 0x45, 0x01010101);
    if (actor) {
      w(p + 9, ~U(0));
      w(p + 0x89, 3);
    }
    append(h(bank), p);
    return p;
  }
  void detach(U parent, U child) {
    U at = r(parent + 0x29), end = r(parent + 0x2d);
    while (at != end && r(at) != child)
      at += 4;
    if (at == end)
      throw std::invalid_argument("cache detached child");
    for (U p = at + 4; p != end; p += 4) {
      w(p - 4, r(p));
    }
    w(parent + 0x2d, end - 4);
    for (U i = 0; i < 8; ++i)
      if (!m.get8(parent + 0x41 + i) && at < r(parent + 0x49 + 4 * i))
        w(parent + 0x49 + 4 * i, r(parent + 0x49 + 4 * i) - 4);
    w(child + 0x39, 0);
  }

public:
  U header = 0;
  unsigned builds = 0, hits = 0, invalidations = 0;
  explicit OwnerKnowledgeCache(M &memory) : m(memory), sources(memory) {
    header = m.allocate(48);
    if (!header)
      throw std::runtime_error("cache header allocation");
    for (U i = 0; i < 48; ++i)
      m.set8(header + i, 0);
  }
  OwnerKnowledgeCache(const OwnerKnowledgeCache &) = delete;
  OwnerKnowledgeCache &operator=(const OwnerKnowledgeCache &) = delete;
  template <class Borrow>
  U prepare(unsigned bank, const OwnerKnowledge &knowledge, unsigned owner,
            int tick, Extent d,
            const std::array<std::array<std::uint8_t, 8>, 8> &allied,
            const OwnerMemoryGroupServices &services, Borrow snapshot) {
    U at = h(bank);
    if (r(at + 4) != r(at + 8)) {
      ++hits;
      return r(r(at + 4));
    }
    if (owner >= 8 || knowledge.records[owner].size() > 125 || d.x != 32 ||
        d.y != 32 || d.z != 8 || !services.regional || !services.line)
      throw std::invalid_argument("knowledge cache domain");
    ++builds;
    U root = make(bank);
    for (const auto &row : knowledge.records[owner]) {
      const auto &a = row.memory.actor;
      if (a.owner < 0 || a.owner >= 8)
        throw std::invalid_argument("cache snapshot owner");
      if (!a.health || allied[owner][unsigned(a.owner)] ||
          (!bank && row.markedTick != tick))
        continue;
      if (row.memory.key < 0 || !row.planning)
        throw std::invalid_argument("cache snapshot key/planning");
      U radius = (U(tick) - U(row.memory.observedTick)) *
                 U(row.planning->motionB63) / 72;
      if (radius > 32000)
        throw std::invalid_argument("cache uncertainty");
      U leaf = make(bank, snapshot(row.memory.key));
      w(leaf + 9, U(row.memory.key));
      for (auto v : {std::pair<U, U>{0xd, U(norm(a.position.x, d.x))},
                     {0x11, U(norm(a.position.y, d.y))},
                     {0x15, U(a.position.z)},
                     {0x19, U(a.region)},
                     {0x1d, radius * radius},
                     {0x21, U(a.flag104)}})
        w(leaf + v.first, v.second);
      U group = make(bank);
      sources.attach(group, leaf);
      for (U off : {0xdu, 0x11u, 0x15u, 0x19u, 0x1du, 0x21u})
        w(group + off, r(leaf + off));
      sources.attach(root, group);
    }
    for (;;) {
      std::vector<U> roots;
      for (U p = r(root + 0x29); p != r(root + 0x2d); p += 4)
        roots.push_back(r(p));
      int best = INT_MAX, middle = -1;
      U first = 0, second = 0;
      for (U i = 0; i < roots.size(); ++i)
        for (U j = i + 1; j < roots.size(); ++j) {
          U a = roots[i], b = roots[j];
          int x = delta(si(r(b + 0xd)), si(r(a + 0xd)), d.x),
              y = delta(si(r(b + 0x11)), si(r(a + 0x11)), d.y),
              z = si(r(b + 0x15)) - si(r(a + 0x15));
          int metric =
              x * x + y * y + z * z / 4 + si((r(a + 0x1d) + r(b + 0x1d)) / 2);
          if (metric >= best || r(a + 0x21) != r(b + 0x21))
            continue;
          int center = -1;
          bool reachable =
              si(r(a + 0x19)) < 0 || si(r(b + 0x19)) < 0
                  ? services.line(
                        {si(r(a + 0xd)), si(r(a + 0x11)), si(r(a + 0x15))},
                        {si(r(b + 0xd)), si(r(b + 0x11)), si(r(b + 0x15))})
                  : services.regional(si(r(a + 0x19)), si(r(b + 0x19)), center);
          if (reachable) {
            best = metric;
            first = a;
            second = b;
            middle = center;
          }
        }
      if (best > 100)
        break;
      detach(root, second);
      sources.attach(first, second);
      w(first + 0xd,
        U(norm(si(r(first + 0xd)) +
                   delta(si(r(second + 0xd)), si(r(first + 0xd)), d.x) / 2,
               d.x)));
      w(first + 0x11,
        U(norm(si(r(first + 0x11)) +
                   delta(si(r(second + 0x11)), si(r(first + 0x11)), d.y) / 2,
               d.y)));
      w(first + 0x15, U(si(r(first + 0x15)) +
                        (si(r(second + 0x15)) - si(r(first + 0x15))) / 2));
      w(first + 0x19, U(middle));
      w(first + 0x1d, U(best));
    }
    return root;
  }
  OwnerMemoryGroups projection(unsigned bank) {
    U at = h(bank);
    OwnerMemoryGroups out;
    if (r(at + 4) == r(at + 8))
      return out;
    auto visit = [&](auto &&self, U p, int parent) -> U {
      U id = U(out.nodes.size());
      out.nodes.emplace_back();
      out.nodes[id].parent = parent;
      if (parent >= 0) {
        auto &n = out.nodes[id];
        n.position = {si(r(p + 0xd)), si(r(p + 0x11)), si(r(p + 0x15))};
        n.region = si(r(p + 0x19));
        n.uncertaintySquared = si(r(p + 0x1d));
        n.flag104 = si(r(p + 0x21));
        if (r(p + 0x35)) {
          n.memoryKey = si(r(p + 9));
          n.activity = r(p + 0x89);
        }
      }
      for (U q = r(p + 0x29); q != r(p + 0x2d); q += 4) {
        U child = self(self, r(q), int(id));
        out.nodes[id].children.push_back(child);
      }
      return id;
    };
    visit(visit, r(r(at + 4)), -1);
    validateOwnerMemoryGroups(out, {32, 32, 8});
    return out;
  }
  // 4181d0 before mode dispatch: bank0 then bank1, normal forward destruction,
  // 42f250 clears vector length while retaining its backing capacity.
  void invalidate() {
    for (unsigned bank = 0; bank < 2; ++bank) {
      U at = h(bank);
      for (U p = r(at + 4); p != r(at + 8); p += 4)
        if (U source = r(p)) {
          sources.destroy(source);
          m.free(source);
        }
      w(at + 8, r(at + 4));
    }
    ++invalidations;
  }
  // Host disposal after the observed invalidation; not a recovered whole-brain
  // 4156b0 destructor or observation-tree/snapshot release policy.
  void close() {
    invalidate();
    for (unsigned bank = 0; bank < 2; ++bank)
      m.free(r(h(bank) + 4));
    m.free(header);
    header = 0;
  }
};
} // namespace mnm::reconstruction::combat
