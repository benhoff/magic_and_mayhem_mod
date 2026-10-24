#pragma once
#include "owner_observation_snapshots.hpp"
#include "owner_task_sources.hpp"
#include "owner_task_storage.hpp"
#include <stdexcept>

namespace mnm::reconstruction::combat {
// NoCD40209ca7 normal 415ac0/4c0920 lifecycle over checked value-address
// storage. This does not construct a brain, implement RB insertion, reset it,
// or free the enclosing brain. M also provides actorPool() and global words.
template <class M> class OwnerBrainTeardown {
  using U = std::uint32_t;
  M &m;
  OwnerTaskSources<M> sources;
  OwnerTaskStorage<M> tasks;
  OwnerObservationSnapshots<M> snapshots;
  U r(U p) { return m.get32(p); }
  void w(U p, U v) { m.set32(p, v); }
  void source(U p) {
    if (p) {
      sources.destroy(p);
      m.free(p);
    }
  }
  void task(U p) {
    if (p) {
      tasks.destroy(p);
      m.free(p);
    }
  }
  void vector(U h) {
    m.free(r(h + 4));
    for (U off : {4u, 8u, 12u})
      w(h + off, 0);
  }
  // Original whole-range erase: recurse right, destroy node, walk left.
  void nodes(U p, U nil, bool observation, unsigned depth = 0) {
    if (p == nil)
      return;
    if (!p || depth == 128)
      throw std::invalid_argument("brain tree domain");
    nodes(r(p + 8), nil, observation, depth + 1);
    U left = r(p);
    if (observation)
      snapshots.destroy(p + 24);
    m.free(p);
    nodes(left, nil, observation, depth + 1);
  }
  void tree(U h, U global, U counter, bool observation = false) {
    U head = r(h + 4), nil = r(global);
    if (!head || !nil || !r(counter))
      throw std::invalid_argument("brain tree ownership");
    nodes(r(head + 4), nil, observation);
    w(head + 4, nil);
    w(h + 12, 0);
    w(head, head);
    w(head + 8, head);
    m.free(head);
    w(h + 4, 0);
    w(h + 12, 0);
    w(counter, r(counter) - 1);
    if (!r(counter)) {
      m.free(nil);
      w(global, 0);
    }
  }
  void deque(U h) {
    while (r(h + 0x2c)) {
      w(h + 12, r(h + 12) + 4);
      w(h + 0x2c, r(h + 0x2c) - 1);
      if (!r(h + 0x2c) || r(h + 12) == r(h + 8)) {
        U slot = r(h + 16);
        w(h + 16, slot + 4);
        m.free(r(slot));
        if (!r(h + 0x2c)) {
          for (U off = 4; off <= 32; off += 4)
            w(h + off, 0);
          // 42f950 retains the freed map's pointer at+24/+28.
          m.free(r(h + 0x24));
        } else {
          U begin = r(r(h + 16));
          w(h + 4, begin);
          w(h + 8, begin + 0x1000);
          w(h + 12, begin);
        }
      }
    }
  }

public:
  explicit OwnerBrainTeardown(M &memory)
      : m(memory), sources(memory), tasks(memory), snapshots(memory) {}
  // 415ac0 leaves all owning collection headers and slots intact. Only the
  // secondary11e05 vector is deduplicated against the primary1a vector;
  // duplicate ownership elsewhere is outside the original valid domain.
  void releasePayloads(U brain) {
    for (U p = r(brain + 8); p != r(brain + 12); p += 4)
      task(r(p));
    for (U p = r(brain + 0x1e); p != r(brain + 0x22); p += 4)
      source(r(p));
    for (U p = r(brain + 0x11e09); p != r(brain + 0x11e0d); p += 4) {
      bool shared = false;
      for (U q = r(brain + 0x1e); q != r(brain + 0x22); q += 4)
        shared |= r(q) == r(p);
      if (!shared)
        source(r(p));
    }
    U pool = m.actorPool();
    for (U i = 0; i < 125; ++i)
      w(pool + i * 0xe4b + 0x678 + 4 * r(brain), 0);
    U head = r(brain + 0xbe), p = r(head);
    while (p != head) {
      task(r(p + 16));
      // 42dd00 increments the ordered tree cursor without erasing the node.
      U right = r(p + 8), nil = r(0x5faf50);
      if (right != nil) {
        p = right;
        while (r(p) != nil)
          p = r(p);
      } else {
        U parent = r(p + 4);
        while (p == r(parent + 8)) {
          p = parent;
          parent = r(parent + 4);
        }
        if (r(p + 8) != parent)
          p = parent;
      }
    }
    for (U owner = 0; owner < 8; ++owner)
      for (U bank = 0; bank < 2; ++bank) {
        U h = brain + 0x11c12 + owner * 48 + bank * 16;
        for (U q = r(h + 4); q != r(h + 8); q += 4)
          source(r(q));
      }
  }
  // Whole4c0920 normal path. The scalar maps have no value destructor;
  // task references in11dc3 are borrowed aliases of the owning collections.
  // The owner admission below is native policy, before any release.
  void destroy(U brain) {
    if (r(brain) > 1)
      throw std::invalid_argument("brain owner domain");
    releasePayloads(brain);
    tree(brain + 0x12088, 0x5faf20, 0x5faf24);
    sources.destroy(brain + 0x11f33);
    sources.destroy(brain + 0x11ea6);
    tree(brain + 0x11e15, 0x5faf58, 0x6e2048);
    vector(brain + 0x11e05);
    tree(brain + 0x11df5, 0x5faf54, 0x6e2044);
    vector(brain + 0x11de5);
    tree(brain + 0x11dc3, 0x5faf50, 0x6e2040);
    vector(brain + 0x11db3);
    // MSVC array destruction runs owner7 down to owner0, bank1 then bank0.
    for (U owner = 8; owner--;) {
      U h = brain + 0x11c02 + owner * 48;
      vector(h + 32);
      vector(h + 16);
      tree(h, 0x5faf4c, 0x6e204c, true);
    }
    tree(brain + 0xba, 0x5faf50, 0x6e2040);
    for (U off : {0x8au, 0x5au, 0x2au})
      deque(brain + off);
    vector(brain + 0x1a);
    vector(brain + 4);
  }
};
} // namespace mnm::reconstruction::combat
