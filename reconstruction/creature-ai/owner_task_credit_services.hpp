#pragma once
#include "contribution_account.hpp"
#include "resource.hpp"
#include "segment_completion.hpp"
#include <cstring>
#include <map>
namespace mnm::reconstruction::combat {
// NoCD40209ca7 concrete contribution providers. Checked PE32 values only;
// all actor/source/task pointers are borrowed. Normal acyclic inputs, readonly
// profiles and non-reentrant task storage. M::service supplies named external
// presentation/registration/item/world services; it cannot reenter this
// adapter.
template <class M> class OwnerTaskCreditServices {
  using U = std::uint32_t;
  M &m;
  const ContributionProfiles &profiles;
  U r(U p) { return m.get32(p); }
  void w(U p, U v) { m.set32(p, v); }
  static int si(U v) {
    int n;
    std::memcpy(&n, &v, 4);
    return n;
  }
  static bool wizard(U k) { return k == 0 || k == 24 || k == 25 || k == 26; }
  AutonomousCreature creature(U p) {
    AutonomousCreature a;
    a.ai.slot = r(p);
    a.ai.type = r(p + 0xa8);
    a.ai.kind = r(r(p + 0xac));
    a.ai.owner = si(r(p + 0x174));
    a.ai.health = r(p + 0xe4);
    a.ai.active = r(p + 4) != 0;
    a.block7c = r(p + 0x7c);
    for (U i = 0; i < 42; ++i) {
      U at = p + 0x17c + i * 24;
      a.inventory[i] = {r(at), r(at + 4), r(at + 8), r(at + 20)};
    }
    return a;
  }
  U endpoint(U id, U self, std::initializer_list<U> args = {}) {
    return m.service(id, self, args);
  }

public:
  OwnerTaskCreditServices(M &memory, const ContributionProfiles &p)
      : m(memory), profiles(p) {}
  // 416c30/42e610: query existing signed-key task index; no insertion/ownership
  // or PE iterator ABI is exposed. Native depth bound is admission policy.
  U taskById(U brain, int key) {
    if (key < 0)
      return 0;
    U head = r(brain + 0xbe), nil = r(0x5faf50), node = r(head + 4),
      best = head;
    for (unsigned depth = 0; node != nil; ++depth) {
      if (!node || depth >= 128)
        throw std::invalid_argument("task index ancestry");
      if (si(r(node + 12)) < key)
        node = r(node + 8);
      else {
        best = node;
        node = r(node);
      }
    }
    return best != head && si(r(best + 12)) == key ? r(best + 16) : 0;
  }
  U taskForActor(U actor, unsigned family) {
    if (family > 1)
      throw std::invalid_argument("task source family");
    U p = r(actor + 0x678 + family * 4);
    if (!p)
      return 0;
    for (unsigned depth = 0;; ++depth) {
      if (depth >= 128)
        throw std::invalid_argument("task source ancestry");
      U parent = r(p + 0x39);
      if (!parent)
        return r(p + 5);
      p = parent;
    }
  }
  void creditBoth(U actor, U value, U field = 0x2f) {
    if (field != 0x2f && field != 0x33)
      throw std::invalid_argument("task counter field");
    for (unsigned family = 0; family < 2; ++family)
      if (U task = taskForActor(actor, family))
        w(task + field, r(task + field) + value);
  }
  U account(U type, U level, U quantity, float amount = 0, float delta = 0,
            float extra = 0) {
    return contributionAccount(profiles, type, si(level), si(quantity), amount,
                               delta, extra);
  }
  U associated(int owner) {
    std::array<AssociatedOwnerRecord, 8> records;
    for (U i = 0; i < 8; ++i)
      records[i] = {r(0x643a80 + i * 340), r(0x643a8c + i * 340)};
    auto slot = associatedCreature(owner, records, r(0x6def5c));
    return slot ? r(0x6def58) + *slot * 0xe4b : 0;
  }
  U maximum(U actor) {
    U cfg = r(actor + 0xac), base = r(cfg + 0x18), kind = r(cfg);
    if (wizard(kind)) {
      int owner = si(r(actor + 0x174));
      if (owner != -1) {
        if (owner < 0 || owner >= 8)
          throw std::invalid_argument("maximum owner");
        int slot = si(r(0x643a90 + U(owner) * 340));
        if (slot != -1) {
          if (slot < 0)
            throw std::invalid_argument("maximum commander");
          return r(0x65b290 + U(slot) * 0x93a);
        }
      }
      return r(actor + 0x8c1) ? r(actor + 0x8c1) : base;
    }
    U rank = r(actor + 0x124);
    if (rank == 1 || rank == 2)
      return base + U(si(base * r(rank == 1 ? 0x5e1910 : 0x5e1914)) / 100);
    return base;
  }
  U aggregateResource(U actor) {
    U sum = r(actor + 0xe8);
    for (U i = 0; i < 42; ++i) {
      U at = actor + 0x17c + i * 24;
      if (r(at + 20) && r(at) == 19)
        sum += r(at + 4) << 8;
    }
    return sum;
  }
  void changeResource(U actor, U delta) {
    auto a = creature(actor);
    CreatureResourceState s;
    s.current = r(actor + 0xe8);
    s.maximum = r(actor + 0xec);
    U dirty = r(actor + 0x92e);
    CreatureResourceServices callbacks;
    auto display = [&](U id, U receiver, U quantity) {
      w(actor + 0xe8, s.current);
      endpoint(id, receiver, {quantity});
      s.current = r(actor + 0xe8);
    };
    callbacks.primaryDisplay = [&](U q) { display(0x4d6f30, 0x6a2dd0, q); };
    callbacks.secondaryDisplay = [&](U q) { display(0x4d3a10, 0x6c4e40, q); };
    changeCreatureResource(a, s, dirty, delta, si(r(0x644520)), callbacks);
    w(actor + 0xe8, s.current);
    w(actor + 0x92e, dirty);
  }
  void snapshotResource(U actor, int owner, U delta) {
    auto a = creature(actor);
    CreatureResourceState s;
    s.current = r(actor + 0xe8);
    s.maximum = r(actor + 0xec);
    for (U i = 0; i < 8; ++i)
      s.ownerSnapshot[i] = r(actor + 0xe2b + 4 * i);
    updateCreatureResourceSnapshot(a, s, owner, delta);
    w(actor + 0xe2b + 4 * U(owner), s.ownerSnapshot.at(unsigned(owner)));
  }
  // Inline509faa..50a0da, including its unsigned deadline and controller gate.
  // Other maintenance siblings belong to maintainCreature and M::service.
  void periodicResource(U source) {
    U now = r(0x6c4830);
    if (!r(source + 0xfc) || r(source + 0x100) >= now)
      return;
    w(source + 0x100, now + r(0x5e1904));
    int owner = si(r(source + 0x174));
    if (owner < 0 || owner >= 8)
      throw std::invalid_argument("periodic owner");
    if (r(0x643a8c + U(owner) * 340) == ~U(0))
      return;
    U subject = associated(owner);
    if (!subject)
      throw std::invalid_argument("periodic subject");
    float before = float(r(subject + 0xe8) >> 8);
    changeResource(subject, r(0x5e1908) << 8);
    float delta = float(r(subject + 0xe8) >> 8) - before;
    U amount = account(r(source + 0xa8), 0, 0,
                       float(si(aggregateResource(subject)) / 256), delta);
    creditBoth(source, amount);
  }
  void setPending(U actor, U value) {
    if (value) {
      endpoint(0x49d120, 0x689498, {actor});
      w(actor + 0xfc, value);
      w(actor + 0x100, r(0x5e1904) + r(0x6c4830));
    } else
      w(actor + 0xfc, 0);
  }
  U activate(U actor) {
    setPending(actor, 1);
    float ratio =
        float(static_cast<long double>(si(r(0x5e1908))) / si(r(0x5e1904)));
    creditBoth(actor, account(r(actor + 0xa8), 0, 0, 0, 0, ratio));
    return 1;
  }
  // 514990 normal nonlethal branch. Death/form/scenario cleanup has its own
  // recovered components and is deliberately not invented by this provider.
  U damage(U actor, U amount, const std::optional<std::array<int, 2>> &ids) {
    U health = r(actor + 0xe4);
    if (!health || !amount)
      return 1;
    if (amount >= health)
      throw std::invalid_argument("nonlethal task damage domain");
    for (U i = 0; i < 8; ++i)
      if (r(actor + 0x24 + 4 * i)) {
        w(actor + 0xdeb + 4 * i, amount);
        w(actor + 0xe0b + 4 * i, 0);
      }
    U value = account(r(actor + 0xa8), health, amount);
    w(actor + 0xe4, health - amount);
    if (si(r(actor + 0x660)) <= 0 && r(actor + 0x174) == r(0x644520)) {
      endpoint(0x4d9890, 0x6de8a0, {actor, 1});
      w(actor + 0x660, 250);
    }
    if (ids)
      for (U i = 0; i < 2; ++i)
        if (U task = taskById(i ? 0x644550 : 0x690600, (*ids)[i])) {
          U reward = wizard(r(r(actor + 0xac))) ? value * 2 : value;
          w(task + 0x2f, r(task + 0x2f) + reward);
        }
    creditBoth(actor, value, 0x33);
    return 1;
  }
  // Positive5076f0 branch with concrete maximum, account and recipient paths.
  void heal(U actor, U delta, bool notify) {
    if (m.get8(0x6e9858) || !r(actor + 0xe4))
      return;
    if (delta > 0x7fffffffu)
      throw std::invalid_argument("positive task healing");
    U before = r(actor + 0xe4), health = before + delta;
    if (maximum(actor) <= health)
      health = maximum(actor);
    w(actor + 0xe4, health);
    if (notify)
      creditBoth(actor, account(r(actor + 0xa8), health, health - before));
    if (associated(si(r(0x644520))) == actor)
      endpoint(0x4d3a30, 0x6c4e40, {health});
    if (r(actor + 0x967))
      endpoint(0x4637c0, r(actor + 0x967));
    U cap = maximum(actor),
      bar = cap ? (r(actor + 0xe4) * 15) / maximum(actor) : 0;
    w(actor + 0x6c, std::min(bar, 14u));
    endpoint(0x58f830, actor + 0x6b4, {r(actor + 0xe4)});
    endpoint(0x58f890, actor + 0x6b4);
    w(actor + 0x92e, 1);
  }
  // Compose existing512460 normal no-landing/no-region branch with shared
  // source/task counters. The projection does not own or retire those pointers.
  void completeSegment(U actor) {
    if (m.get8(actor + 0x71c) || r(0x6e1f3c) || si(r(actor + 0x963)) > 0)
      throw std::invalid_argument("normal task completion domain");
    auto a = creature(actor);
    CreatureExecution e;
    MotionActionState motion;
    MovementBehaviorState landing;
    SegmentCompletionState state;
    SegmentCompletionEnvironment env;
    SegmentCompletionServices services;
    state.cursor = r(actor + 0x977);
    state.count = r(actor + 0x97b);
    state.scorePendingFc = r(actor + 0xfc) != 0;
    for (U i = 0; i < 16; ++i)
      state.route[i].position = {si(r(actor + 0x97f + i * 28)),
                                 si(r(actor + 0x983 + i * 28)),
                                 si(r(actor + 0x987 + i * 28))};
    std::map<U, U> nodes, tasks;
    std::vector<U> recipients;
    auto project = [&](auto &&self, U p, unsigned depth) -> U {
      if (depth >= 128)
        throw std::invalid_argument("completion source ancestry");
      if (auto it = nodes.find(p); it != nodes.end())
        return it->second;
      U index = U(state.contributionNodes.size());
      nodes.emplace(p, index);
      state.contributionNodes.emplace_back();
      if (U parent = r(p + 0x39)) {
        U next = self(self, parent, depth + 1);
        state.contributionNodes[index].parent = next;
      } else if (U task = r(p + 5)) {
        auto it = tasks.find(task);
        U id;
        if (it == tasks.end()) {
          id = U(recipients.size());
          tasks.emplace(task, id);
          recipients.push_back(task);
          state.contributionTotals.push_back(r(task + 0x33));
        } else
          id = it->second;
        state.contributionNodes[index].owner = id;
      }
      return index;
    };
    for (U i = 0; i < 2; ++i)
      if (U p = r(actor + 0x678 + 4 * i))
        state.contributionRoots[i] = project(project, p, 0);
    auto flush = [&] {
      for (U i = 0; i < recipients.size(); ++i)
        w(recipients[i] + 0x33, state.contributionTotals[i]);
    };
    env.scoreNumerator = si(r(0x5e1908));
    env.scoreDenominator = si(r(0x5e1904));
    services.snap = [&](auto &a, Cell p, U type) {
      endpoint(0x5070e0, actor, {U(p.x), U(p.y), U(p.z), type});
      a.ai.position = p;
    };
    services.score = [&](U type, float ratio) {
      w(actor + 0xfc, 0);
      return account(type, 0, 0, 0, 0, ratio);
    };
    auto invoke = [&](U id, auto &a) {
      flush();
      w(actor + 0xfc, state.scorePendingFc);
      endpoint(id, actor);
      a.ai.active = r(actor + 4) != 0;
    };
    services.environment = [&](auto &a) { invoke(0x51a8f0, a); };
    services.animation = [&](auto &a) { invoke(0x50f850, a); };
    services.terrain = [&](auto &a) { invoke(0x51aa30, a); };
    services.reset = [&](auto &) { endpoint(0x50ab30, actor); };
    // Original zeroes these before position snap and credit; publish that
    // order.
    w(actor + 0xb57, 0);
    w(actor + 0xb5b, 0);
    w(actor + 0x618, 0);
    completeMotionSegment(a, e, motion, landing, state, env, services);
    flush();
    w(actor + 0xfc, state.scorePendingFc);
    w(actor + 0x977, state.cursor);
    if (a.ai.active && state.cursor >= state.count)
      w(actor + 0xb8b, 0);
  }
};
} // namespace mnm::reconstruction::combat
