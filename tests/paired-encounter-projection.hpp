#pragma once
#include "paired-encounter-state.hpp"
#include <sstream>
#include <vector>
namespace mnm::encounter::paired {
struct Word {
  std::string name;
  unsigned value;
};
using Projection = std::vector<Word>;
inline Projection project(const cs::Continuation &c, const Profile &p) {
  Projection out;
  auto add = [&](std::string key, unsigned value) {
    out.push_back({std::move(key), value});
  };
  add("tick", c.native.tick);
  for (unsigned i = 0; i < 2; ++i) {
    const auto prefix = "actor[" + std::to_string(i) + "].";
    for (const auto &[offset, f] : fields(c, i)) {
      std::ostringstream key;
      key << prefix << "0x" << std::hex << offset << " " << f.name;
      add(key.str(), f.value);
    }
    const auto &ch = c.native.animation.at(i).body;
    const auto &f = ch.state.forward;
    add(prefix + "ani.bound", ch.bound);
    add(prefix + "ani.sequence", ch.bound ? ch.sequence : 0);
    const auto base =
        ch.bound ? p.native.animation.at(ch.asset).starts.at(ch.sequence) : 0;
    add(prefix + "ani.pc", ch.bound ? base + unsigned(f.pc) + 1 : 0);
    add(prefix + "ani.displayed", ch.bound && f.displayedRecord
                                      ? base + unsigned(*f.displayedRecord) + 1
                                      : 0);
    add(prefix + "ani.active", f.active);
    add(prefix + "ani.delay", f.delay);
    add(prefix + "ani.elapsed", f.elapsed);
    add(prefix + "ani.repeats", f.repeats);
    add(prefix + "ani.break",
        f.breakFlag || c.actions.callbacks.at(i).animationBreak);
    add(prefix + "ani.reverse", ch.state.reverse);
    add(prefix + "ani.reverseCursor", ch.state.reverseCursor);
    add(prefix + "ani.reverseLength", ch.state.reverseLength);
    const auto &route = c.actions.segments.at(i);
    for (unsigned j = 0; j < route.count; ++j) {
      const auto &q = route.route.at(j);
      const auto key = prefix + "route[" + std::to_string(j) + "].";
      add(key + "x", unsigned(q.position.x));
      add(key + "y", unsigned(q.position.y));
      add(key + "z", unsigned(q.position.z));
      for (unsigned k = 0; k < 4; ++k)
        add(key + "meta[" + std::to_string(k) + "]", unsigned(q.remainder[k]));
    }
    // Health words whose storage is auxiliary rather than a canonical alias.
    auto backend = [&](unsigned word) {
      unsigned v = 0;
      auto at = 8 + i * 31 * 4 + word * 4;
      for (unsigned k = 0; k < 4; ++k)
        v |= unsigned(c.backend.at(at + k)) << (8 * k);
      return v;
    };
    add(prefix + "health.bar", backend(1));
    for (unsigned j = 0; j < 8; ++j)
      add(prefix + "damage[" + std::to_string(j) + "]", backend(13 + j));
  }
  for (unsigned i = 0; i < c.native.cells.size(); ++i) {
    const auto &v = c.native.cells[i];
    const auto key = "cell[" + std::to_string(i) + "].";
    add(key + "kind", v.kind);
    add(key + "occupant", v.occupant);
    add(key + "flags", v.flags);
    add(key + "links", v.links);
  }
  const auto random =
      c.native.randomSeeded ? c.native.random : cs::model::seedRandom(c.seed);
  add("rng.left", random.left);
  add("rng.right", random.right);
  for (unsigned i = 0; i < 250; ++i)
    add("rng[" + std::to_string(i) + "]", random.words[i]);
  add("scheduler.cursor", unsigned(c.scheduler.cursor));
  add("scheduler.phase20", unsigned(c.scheduler.phase20));
  add("scheduler.phase90", unsigned(c.scheduler.phase90));
  add("scheduler.budget", unsigned(c.scheduler.budget));
  add("scheduler.continuation", c.scheduler.continuation);
  return out;
}
inline game::Bytes values(const Projection &p) {
  game::Bytes b;
  for (const auto &v : p)
    for (unsigned i = 0; i < 4; ++i)
      b.push_back(std::uint8_t(v.value >> (8 * i)));
  return b;
}
inline std::string difference(const Projection &native,
                              const Projection &original) {
  for (unsigned i = 0; i < native.size() && i < original.size(); ++i) {
    if (native[i].name != original[i].name)
      return "projection field order differs";
    if (native[i].value != original[i].value)
      return native[i].name + " native=" + std::to_string(native[i].value) +
             " original=" + std::to_string(original[i].value);
  }
  if (native.size() != original.size())
    return "projection shape differs";
  return {};
}
inline std::uint64_t digest(std::uint64_t h, const Projection &p) {
  for (const auto &v : p)
    for (unsigned i = 0; i < 4; ++i)
      h = (h ^ ((v.value >> (8 * i)) & 255)) * 1099511628211ull;
  return h;
}
} // namespace mnm::encounter::paired
