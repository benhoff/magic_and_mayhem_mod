#pragma once
#include "owner_knowledge.hpp"
#include <map>
#include <stdexcept>
namespace mnm::reconstruction::combat {
// NoCD40209ca7 snapshot value lifetime. Native ordered index is a
// representation policy: PE red/black topology and iterator ABI are not
// exposed. M supplies checked byte storage, free(), releaseObject(pointer,1),
// freeBuffer().
template <class M> class OwnerObservationSnapshots {
  using U = std::uint32_t;
  M &m;
  U r(U p) { return m.get32(p); }
  void w(U p, U v) { m.set32(p, v); }

public:
  static constexpr U snapshotBytes = 0xe4b, recordBytes = 0xe68;
  std::array<std::map<int, U>, 8> records;
  unsigned insertions = 0, refreshes = 0, expiries = 0;
  explicit OwnerObservationSnapshots(M &memory) : m(memory) {}
  OwnerObservationSnapshots(const OwnerObservationSnapshots &) = delete;
  OwnerObservationSnapshots &
  operator=(const OwnerObservationSnapshots &) = delete;
  // 418d50: all packed bytes, except the two auxiliary-buffer words cleared.
  // Assignment does not release the destination's previous resource pointers.
  void assign(U destination, U source) {
    for (U i = 0; i < snapshotBytes; ++i)
      m.set8(destination + i, m.get8(source + i));
    w(destination + 0x6d0, 0);
    w(destination + 0x6d4, 0);
  }
  // 418b60 and 42fb40: resource object/list are not retained by construction.
  void construct(U destination, U source) {
    assign(destination, source);
    w(destination + 0x708, 0);
    w(destination + 0x8f9, 0);
  }
  // 505f70 normal path; 52cb60 removes list nodes without freeing their values.
  void destroy(U snapshot) {
    if (U object = r(snapshot + 0x708)) {
      m.releaseObject(object, 1);
      w(snapshot + 0x708, 0);
    }
    if (U list = r(snapshot + 0x8f9)) {
      U head = r(list + 4);
      while (U node = r(head)) {
        if (node == head)
          break;
        U next = r(node), previous = r(node + 4);
        w(previous, next);
        w(next + 4, previous);
        m.free(node);
        w(list + 8, r(list + 8) - 1);
      }
      m.free(head);
      w(list + 4, 0);
      w(list + 8, 0);
      m.free(list);
      w(snapshot + 0x8f9, 0);
    }
    if (U buffer = r(snapshot + 0x6d0)) {
      m.freeBuffer(buffer);
      w(snapshot + 0x6d0, 0);
      w(snapshot + 0x6d4, 0);
    }
  }
  U snapshot(unsigned owner, int key) const {
    return records.at(owner).at(key) + 24;
  }
  // Commit the independently recovered logical mode2 update. This adapter owns
  // values, not matching/visibility policy. Resource-bearing refresh needs a
  // separate contract: actual mode2 destroys a shallow-assigned temporary.
  template <class Actor>
  void reconcile(const OwnerKnowledge &knowledge, int tick, Actor actor) {
    for (unsigned owner = 0; owner < 8; ++owner) {
      auto &index = records[owner];
      const auto &rows = knowledge.records[owner];
      for (auto it = index.begin(); it != index.end();) {
        bool found = false;
        for (const auto &row : rows)
          found |= row.memory.key == it->first;
        if (found) {
          ++it;
          continue;
        }
        U p = it->second;
        destroy(p + 24);
        m.free(p);
        it = index.erase(it);
        ++expiries;
      }
      for (const auto &row : rows) {
        int key = row.memory.key;
        auto it = index.find(key);
        if (it == index.end()) {
          U p = m.allocate(recordBytes);
          if (!p)
            throw std::runtime_error("observation allocation");
          w(p + 12, U(key));
          w(p + 16, U(row.memory.observedTick));
          w(p + 20, U(row.markedTick));
          construct(p + 24, actor(row.memory.actor.slot));
          index.emplace(key, p);
          ++insertions;
        } else {
          U p = it->second;
          if (row.memory.observedTick == tick && r(p + 16) != U(tick)) {
            U source = actor(row.memory.actor.slot);
            if (r(source + 0x708) || r(source + 0x8f9) || r(p + 24 + 0x708) ||
                r(p + 24 + 0x8f9))
              throw std::invalid_argument(
                  "resource-bearing observation refresh");
            assign(p + 24, source);
            w(p + 16, U(tick));
            ++refreshes;
          }
          w(p + 20, U(row.markedTick));
        }
      }
    }
  }
  void clear() {
    for (auto &index : records) {
      for (auto row : index) {
        destroy(row.second + 24);
        m.free(row.second);
      }
      index.clear();
    }
  }
};
} // namespace mnm::reconstruction::combat
