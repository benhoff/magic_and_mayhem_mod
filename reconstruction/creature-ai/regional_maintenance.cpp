#include "regional_maintenance.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
int wrap(int v, int d) {
  v %= d;
  return v < 0 ? v + d : v;
}
int xy(Cell a, Cell b, Extent d) {
  const int x = std::abs(wrapped_difference(a.x, b.x, d.x));
  const int y = std::abs(wrapped_difference(a.y, b.y, d.y));
  return 2 * std::max(x, y) + std::min(x, y);
}
int pathCost(const std::vector<Cell> &path, Extent d) {
  int cost = 0;
  for (unsigned i = 1; i < path.size(); ++i)
    cost += 4 * distance_metric(
                    2 * wrapped_difference(path[i].x, path[i - 1].x, d.x),
                    2 * wrapped_difference(path[i].y, path[i - 1].y, d.y),
                    path[i].z - path[i - 1].z);
  return cost;
}
CreatureMovementParameters parameters(int h, int w, int free = 0,
                                      int kind = 0) {
  return {h, w, free, kind, 1, 0, 0};
}
RegionConnection zero() { return {0, 0, {}}; }
RegionConnection *existing(NavigationRegion &v, int to) {
  for (auto &e : v.connections)
    if (e.cost && e.destination == to)
      return &e;
  return nullptr;
}
RegionConnection *slot(NavigationRegion &v, int to) {
  if (auto *e = existing(v, to))
    return e;
  for (auto &e : v.connections)
    if (!e.cost)
      return &e;
  return nullptr;
}
bool clearance(Cell from, Cell to, int group, int width, int free, int kind,
               bool reset, int &budget, NavigationMaintenance &m,
               const std::array<RegionalCreatureShape, 28> &types,
               const RegionalGraphServices &s) {
  const bool unbounded = budget == 0;
  if (reset)
    m.height = 0;
  const auto heights =
      navigationHeights(types, group == 0, group == 1, kind, width);
  const unsigned base = unsigned(group * 3);
  bool progressed = false;
  while (m.height < 6 && heights[unsigned(m.height)] != -1) {
    const int height = heights[unsigned(m.height)];
    const auto admitted = [&](const auto &e) {
      return e.clearance[base] &&
             e.clearance[base + unsigned(width)] >= height &&
             (group != 4 || e.clearance[14 + unsigned(width)] <= height);
    };
    if (!admitted(m.working) && (!m.closing || admitted(m.previous))) {
      bool reached = false;
      const auto p = parameters(height, width, free, kind);
      if (s.legal(from, p) && s.legal(to, p)) {
        const bool fresh = reset || progressed;
        if (fresh)
          m.remaining = admitted(m.previous) ? 125 : (group ? 31 : 41);
        int local = unbounded ? 125 : std::min(budget, m.remaining);
        const int before = local;
        int cost = 0;
        if (fresh)
          reached = navigationDirectRoute(from, to, p, local, cost, s);
        if (!reached) {
          const auto path = s.resumeRoute(from, to, p, local, fresh, m.probe);
          m.remaining -= before - local;
          budget -= before - local;
          if (local < 1 && m.remaining > 0)
            return false;
          reached = !path.empty() && path.back() == to;
          if (reached)
            cost = pathCost(path, s.dimensions);
        }
        if (reached) {
          if (!m.working.cost)
            m.working.cost = cost;
          m.working.destination = m.other;
          m.working.clearance[base] = 1;
          auto &high = m.working.clearance[base + unsigned(width)];
          high = std::int8_t(std::max(int(high), height));
          if (group == 4) {
            auto &low = m.working.clearance[14 + unsigned(width)];
            low = std::int8_t(std::min(int(low), height));
          }
        }
      }
      if (group != 4 &&
          ((reached && m.height == 1) || (!reached && m.height == 0)))
        return true;
      progressed = true;
    }
    ++m.height;
  }
  return true;
}
void publish(std::vector<NavigationRegion> &g, NavigationMaintenance &m) {
  auto &a = g.at(unsigned(m.active)), &b = g.at(unsigned(m.other));
  if (!m.working.cost) {
    if (auto *e = existing(a, m.other))
      *e = zero();
    if (auto *e = existing(b, m.active))
      *e = zero();
    return;
  }
  auto *left = slot(a, m.other), *right = slot(b, m.active);
  if ((!left || !right) && m.working.clearance[0]) {
    const auto evict = [&](NavigationRegion &v,
                           int source) -> RegionConnection * {
      RegionConnection *chosen = nullptr;
      for (auto &e : v.connections)
        if (!e.clearance[0] && (!chosen || chosen->cost < e.cost))
          chosen = &e;
      if (chosen) {
        auto &peer = g.at(unsigned(chosen->destination));
        // Original reciprocal removal checks the destination even in empty
        // slots.
        for (auto &e : peer.connections)
          if (e.destination == source) {
            e = zero();
            break;
          }
        *chosen = zero();
      }
      return chosen;
    };
    if (!left)
      left = evict(a, m.active);
    if (!right)
      right = evict(b, m.other);
  }
  if (left && right) {
    *left = m.working;
    left->destination = m.other;
    *right = m.working;
    right->destination = m.active;
  }
}
bool connection(std::vector<NavigationRegion> &g, NavigationMaintenance &m,
                bool reset, int &budget,
                const std::array<RegionalCreatureShape, 28> &types,
                const RegionalGraphServices &s) {
  if (reset) {
    m.working = zero();
    m.stage = 0;
    m.previous = zero();
    if (auto *e = existing(g.at(unsigned(m.active)), m.other))
      m.previous = *e;
    if (m.opening)
      m.working = m.previous;
  }
  const auto from = g.at(unsigned(m.active)).position,
             to = g.at(unsigned(m.other)).position;
  const auto legal = [&](int width, int free, int kind) {
    auto p = parameters(1, width, free, kind);
    return s.legal(from, p) && s.legal(to, p);
  };
  while (m.stage < 8) {
    const int stage = m.stage, width = stage % 2 ? 1 : 2;
    const int group = stage < 2 ? 0 : stage < 4 ? 1 : stage < 6 ? 3 : 4;
    const int free = group == 1, kind = group == 3 ? 3 : group == 4 ? 1 : 0;
    // Stage-entry copies are repeated on a resumed original call too.
    if (stage == 1 && m.working.clearance[0])
      m.working.clearance[1] = m.working.clearance[2];
    if (stage == 2 && m.working.clearance[0]) {
      m.working.clearance[3] = 1;
      m.working.clearance[4] = m.working.clearance[1];
      m.working.clearance[5] = m.working.clearance[2];
    }
    if (stage == 4 && m.working.clearance[0]) {
      m.working.clearance[9] = 1;
      m.working.clearance[10] = m.working.clearance[1];
      m.working.clearance[11] = m.working.clearance[2];
    }
    if (stage == 6 && s.boundary) {
      m.working.clearance[15] = m.working.clearance[16] = 6;
      if (m.working.clearance[0]) {
        m.working.clearance[12] = 1;
        m.working.clearance[13] = m.working.clearance[1];
        m.working.clearance[14] = m.working.clearance[2];
        m.working.clearance[15] = m.working.clearance[16] = 0;
      }
    }
    const bool attempt =
        (group < 3 || s.boundary) && (group == 4 || legal(width, free, kind));
    if (attempt && !clearance(from, to, group, width, free, kind, reset, budget,
                              m, types, s))
      return false;
    if (stage == 0 && attempt && m.working.clearance[0] &&
        m.working.clearance[2] ==
            navigationHeights(types, true, false, 0, 2)[1]) {
      const auto v = m.working.clearance[2];
      for (unsigned base : {0u, 3u, 6u, 9u, 12u}) {
        m.working.clearance[base] = 1;
        m.working.clearance[base + 1] = m.working.clearance[base + 2] = v;
      }
      m.working.clearance[15] = m.working.clearance[16] = 0;
      ++m.stage;
      break;
    }
    if ((stage == 2 || stage == 4 || stage == 6) && attempt) {
      const unsigned base = unsigned(group * 3);
      if (m.working.clearance[base]) {
        m.working.clearance[base + 1] = std::max(m.working.clearance[base + 1],
                                                 m.working.clearance[base + 2]);
        if (group == 4)
          m.working.clearance[15] =
              std::min(m.working.clearance[15], m.working.clearance[16]);
      }
    }
    ++m.stage;
    reset = true;
  }
  publish(g, m);
  return true;
}
} // namespace
NavigationChangeSignature
navigationChangeSignature(Cell q, const RegionalGraphServices &s) {
  NavigationChangeSignature out;
  out.ground = s.legal(q, parameters(1, 1));
  out.free = !s.legal(q, parameters(1, 1, 1, 3));
  if (q.z + 1 < s.dimensions.z) {
    ++q.z;
    out.height = s.legal(q, parameters(1, 1));
  }
  return out;
}
int navigationChangeMode(NavigationChangeSignature a,
                         NavigationChangeSignature b) {
  if (!a.ground) {
    if (!a.free)
      return 0;
    if (!b.free)
      return a.height == b.height ? 1 : 0;
  } else if (!b.ground) {
    if (!b.free)
      return 0;
    return a.height == b.height ? 2 : 0;
  }
  return a.height == b.height ? -1 : 0;
}
void enqueueNavigationRepair(const std::vector<NavigationRegion> &g, Cell p,
                             int mode, Extent d, NavigationMaintenance &m) {
  if (g.empty() || mode == -1)
    return;
  if (mode < 0 || mode > 2)
    throw std::invalid_argument("navigation repair mode");
  const int start = regionLowerBound(g, wrap(p.x - 4, d.x));
  const int end = regionLowerBound(g, wrap(p.x + 5, d.x));
  if (start < 0 || end < 0)
    throw std::invalid_argument("navigation repair index");
  for (int pass = 0; pass < 2; ++pass) {
    if (pass && start <= end)
      break;
    const int first = pass ? 0 : start;
    const int last = pass ? end : end <= start ? int(g.size()) - 1 : end;
    for (int i = first; i <= last; ++i) {
      const auto q = g.at(unsigned(i)).position;
      if (xy(p, q, d) >= 10)
        continue;
      const int priority = distance_metric(
          std::abs(wrapped_difference(p.x, q.x, d.x)),
          std::abs(wrapped_difference(p.y, q.y, d.y)), std::abs(p.z - q.z));
      unsigned at = 0;
      for (; at < m.queue.size(); ++at)
        if (m.queue[at].node == i)
          break;
      if (at < m.queue.size()) {
        if (m.queue[at].mode != mode)
          m.queue[at].mode = 0;
        if (m.queue[at].priority <= priority)
          continue;
        m.queue[at].priority = priority;
      } else {
        if (m.queue[m.write].node != -1)
          continue;
        at = m.write;
        m.queue[at] = {i, mode, priority};
        m.write = (m.write + 1) % unsigned(m.queue.size());
      }
      for (unsigned j = m.read; j != at; j = (j + 1) % unsigned(m.queue.size()))
        if (m.queue[at].priority < m.queue[j].priority)
          std::swap(m.queue[at], m.queue[j]);
    }
  }
}
bool navigationMaintenancePending(const NavigationMaintenance &m) {
  return m.active != -1 || m.queue[m.read].node != -1;
}
void maintainNavigationGraph(std::vector<NavigationRegion> &g,
                             const std::array<RegionalCreatureShape, 28> &types,
                             NavigationMaintenance &m, bool blocked,
                             const RegionalGraphServices &s) {
  if (blocked)
    return;
  bool reset = false;
  if (m.active == -1 && m.queue[m.read].node != -1) {
    const auto e = m.queue[m.read];
    m.active = e.node;
    m.opening = e.mode == 1;
    m.closing = e.mode == 2;
    m.queue[m.read].node = -1;
    m.read = (m.read + 1) % unsigned(m.queue.size());
    m.other = m.stage = m.height = 0;
    reset = true;
  }
  if (m.active == -1)
    return;
  int budget = 10;
  while (m.other < int(g.size())) {
    if (m.other != m.active) {
      const auto a = g.at(unsigned(m.active)).position,
                 b = g.at(unsigned(m.other)).position;
      const int distance = xy(a, b, s.dimensions) / 2;
      if (distance < 9) {
        auto p = parameters(1, 1);
        if (!reset || distance <= 5 || s.legal(a, p) == s.legal(b, p)) {
          if (!connection(g, m, reset, budget, types, s))
            return;
          reset = true;
        }
      }
    }
    ++m.other;
  }
  m.active = -1;
}
} // namespace mnm::reconstruction::combat
