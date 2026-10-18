#pragma once
#include "../reconstruction/creature-ai/owner_group_task.hpp"
#include <fstream>
#include <iterator>
#include <stdexcept>
namespace mnm::test::owner_trace {
namespace m = mnm::reconstruction::combat;
struct Frame {
  m::OwnerGroupTask before, after;
  m::OwnerKnowledge knowledge;
  m::AutonomousWorld world;
  std::vector<unsigned> members;
  std::vector<int> raw, net, threat;
  int tick = 0;
  unsigned owner = 0;
  int percent = 0, initialBudget = 0, finalBudget = 0;
  m::OwnerSourceGroupResult result;
};
struct Writer {
  std::vector<unsigned char> bytes;
  template <class T> void word(T v) {
    auto u = std::uint32_t(v);
    for (unsigned j = 0; j < 4; ++j)
      bytes.push_back(std::uint8_t(u >> (j * 8)));
  }
  void point(m::Cell p) {
    word(p.x);
    word(p.y);
    word(p.z);
  }
  template <class T> void list(const std::vector<T> &v) {
    word(v.size());
    for (auto x : v)
      word(x);
  }
  void task(const m::OwnerGroupTask &t) {
    word(t.root);
    word(t.risk);
    word(t.groups.nodes.size());
    for (const auto &n : t.groups.nodes) {
      word(n.memoryKey);
      word(n.parent);
      point(n.position);
      word(n.region);
      word(n.uncertaintySquared);
      word(n.flag104);
      word(n.activity);
      list(n.children);
    }
  }
  void frame(const Frame &f) {
    word(f.tick);
    word(f.owner);
    word(f.percent);
    word(f.initialBudget);
    word(f.finalBudget);
    point({f.world.dimensions.x, f.world.dimensions.y, f.world.dimensions.z});
    for (const auto &row : f.world.relation)
      for (auto x : row)
        word(x);
    word(f.world.creatures.size());
    for (const auto &v : f.world.creatures) {
      const auto &a = v.ai;
      for (int x :
           {int(a.slot), int(a.type), int(a.kind), int(a.health), int(a.owner),
            int(a.physical), int(a.engaged), int(a.incoming), int(a.readiness),
            int(a.dirty932), int(a.active), int(a.ranged)})
        word(x);
      point(a.position);
    }
    list(f.members);
    list(f.raw);
    list(f.net);
    list(f.threat);
    const auto &rows = f.knowledge.records.at(f.owner);
    word(rows.size());
    for (const auto &r : rows) {
      word(r.memory.key);
      word(r.markedTick);
      word(r.memory.actor.slot);
    }
    task(f.before);
    task(f.after);
    word(f.result.continuing);
    word(f.result.orders.size());
    for (const auto &o : f.result.orders) {
      word(o.continuing);
      word(o.stop);
      word(o.kind);
      word(o.target);
      point(o.position);
    }
  }
};
struct Reader {
  const std::vector<unsigned char> &bytes;
  std::size_t at = 0;
  unsigned word() {
    if (bytes.size() - at < 4)
      throw std::runtime_error("truncated task trace");
    unsigned x = 0;
    for (unsigned j = 0; j < 4; ++j)
      x |= unsigned(bytes[at++]) << (j * 8);
    return x;
  }
  int number() {
    auto x = word();
    return x <= INT32_MAX ? int(x) : -1 - int(~x);
  }
  unsigned count(unsigned bound) {
    auto n = word();
    if (n > bound)
      throw std::runtime_error("task trace count");
    return n;
  }
  bool flag() {
    auto n = word();
    if (n > 1)
      throw std::runtime_error("task trace flag");
    return n;
  }
  m::Cell point() { return {number(), number(), number()}; }
  template <class T> std::vector<T> list(unsigned bound) {
    std::vector<T> out;
    auto n = count(bound);
    for (unsigned i = 0; i < n; ++i)
      out.push_back(T(number()));
    return out;
  }
  m::OwnerGroupTask task() {
    m::OwnerGroupTask out;
    out.root = word();
    out.risk = number();
    auto n = count(251);
    out.groups.nodes.resize(n);
    for (auto &v : out.groups.nodes) {
      v.memoryKey = number();
      v.parent = number();
      v.position = point();
      v.region = number();
      v.uncertaintySquared = word();
      v.flag104 = flag();
      v.activity = word();
      v.children = list<unsigned>(251);
    }
    return out;
  }
  Frame frame() {
    Frame f;
    f.tick = number();
    f.owner = count(7);
    f.percent = number();
    f.initialBudget = number();
    f.finalBudget = number();
    auto d = point();
    f.world.dimensions = {d.x, d.y, d.z};
    for (auto &row : f.world.relation)
      for (auto &x : row)
        x = std::uint8_t(word());
    auto n = count(8);
    f.world.creatures.resize(n);
    for (auto &v : f.world.creatures) {
      auto &a = v.ai;
      a.slot = number();
      a.type = number();
      a.kind = number();
      a.health = word();
      a.owner = number();
      a.physical = word();
      a.engaged = word();
      a.incoming = word();
      a.readiness = word();
      a.dirty932 = word();
      a.active = flag();
      a.ranged = flag();
      a.position = point();
    }
    f.members = list<unsigned>(3);
    f.raw = list<int>(3);
    f.net = list<int>(3);
    f.threat = list<int>(8);
    n = count(125);
    for (unsigned i = 0; i < n; ++i) {
      m::OwnerMemory row;
      row.memory.key = number();
      row.markedTick = number();
      row.memory.actor.slot = number();
      f.knowledge.records[f.owner].push_back(row);
    }
    f.before = task();
    f.after = task();
    f.result.continuing = flag();
    n = count(3);
    for (unsigned i = 0; i < n; ++i) {
      m::OwnerGroupOrder o;
      o.continuing = flag();
      o.stop = flag();
      o.kind = m::OwnerHostileOrderKind(word());
      o.target = number();
      o.position = point();
      f.result.orders.push_back(o);
    }
    return f;
  }
};
inline void append(std::ostream &out, const Frame &f) {
  Writer w;
  w.frame(f);
  Writer size;
  size.word(w.bytes.size());
  out.write(reinterpret_cast<const char *>(size.bytes.data()), 4);
  out.write(reinterpret_cast<const char *>(w.bytes.data()),
            std::streamsize(w.bytes.size()));
  if (!out)
    throw std::runtime_error("task trace write");
}
inline std::vector<unsigned> commands(const Frame &f) {
  std::vector<unsigned> out{unsigned(f.result.continuing)};
  for (unsigned i = 0; i < f.result.orders.size(); ++i) {
    const auto &o = f.result.orders[i];
    if (o.stop)
      out.insert(out.end(), {f.members.at(i), 3});
    else if (o.kind == m::OwnerHostileOrderKind::target)
      out.insert(out.end(), {f.members.at(i), 1, unsigned(o.target)});
    else if (o.kind == m::OwnerHostileOrderKind::move)
      out.insert(out.end(), {f.members.at(i), 2, unsigned(o.position.x),
                             unsigned(o.position.y), unsigned(o.position.z)});
  }
  return out;
}
} // namespace mnm::test::owner_trace
