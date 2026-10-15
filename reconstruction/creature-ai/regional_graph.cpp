#include "regional_graph.hpp"
#include <algorithm>
#include <optional>
#include <stdexcept>
namespace mnm::reconstruction::combat {
namespace {
CreatureMovementParameters parameters(int height, int width, int free = 0,
                                      int kind = 0) {
  return {height, width, free, kind, 1, 0, 0};
}
MapDimensions dims(const RegionalGraphServices &s) {
  return {s.dimensions.x, s.dimensions.y, s.dimensions.z};
}
Coordinates xyz(Cell p) { return {p.x, p.y, p.z}; }
Cell cell(Coordinates p) { return {p.x, p.y, p.z}; }
int metric(Cell a, Cell b, const RegionalGraphServices &s) {
  return distance_metric(2 * wrapped_difference(a.x, b.x, s.dimensions.x),
                         2 * wrapped_difference(a.y, b.y, s.dimensions.y),
                         a.z - b.z);
}
// Same maximum-selection shortsort/middle-pivot partition as original59c02f.
template <class T, class Key> void originalSort(std::vector<T> &v, Key key) {
  std::function<void(int, int)> sort = [&](int lo, int hi) {
    if (hi - lo + 1 <= 8) {
      while (hi > lo) {
        int max = lo;
        for (int i = lo + 1; i <= hi; ++i)
          if (key(v[i]) > key(v[max]))
            max = i;
        std::swap(v[max], v[hi--]);
      }
      return;
    }
    const int mid = lo + (hi - lo + 1) / 2;
    std::swap(v[mid], v[lo]);
    int a = lo, b = hi + 1;
    for (;;) {
      do {
        ++a;
      } while (a <= hi && key(v[a]) <= key(v[lo]));
      do {
        --b;
      } while (b > lo && key(v[b]) >= key(v[lo]));
      if (b < a)
        break;
      std::swap(v[a], v[b]);
    }
    std::swap(v[lo], v[b]);
    if (lo < b - 1)
      sort(lo, b - 1);
    if (a < hi)
      sort(a, hi);
  };
  if (v.size() > 1)
    sort(0, int(v.size()) - 1);
}
int lookup(const std::vector<NavigationRegion> &g, Cell p,
           CreatureMovementParameters q, const RegionalGraphServices &s) {
  RegionalServices lower;
  lower.legal = [&](Cell v) { return s.legal(v, q); };
  lower.line = [&](Cell a, Cell b) { return s.line(a, b, q); };
  lower.boundedRoute = [&](Cell a, Cell b) {
    int budget = 12;
    auto path = s.route(a, b, q, budget);
    return std::find(path.begin(), path.end(), b) != path.end();
  };
  return lookupNavigationRegion(g, p, {q, false, false, false}, s.dimensions,
                                s.boundary, 0, 0, lower);
}
// 542220: strictly improving greedy path; charge movement probes in units
// of1/26.
} // namespace
bool navigationDirectRoute(Cell from, Cell to,
                           const CreatureMovementParameters &p, int &budget,
                           int &cost, const RegionalGraphServices &s) {
  const auto distance = [&](Cell v) {
    return distance_metric(wrapped_difference(v.x, to.x, s.dimensions.x),
                           wrapped_difference(v.y, to.y, s.dimensions.y),
                           2 * (v.z - to.z));
  };
  const auto start = from;
  Cell previous{0, 0, -1};
  int best = distance(from), probes = budget * 26;
  cost = 0;
  bool reached = true;
  while (!(from == to)) {
    std::optional<Cell> chosen;
    for (auto raw : adjacent_cells(xyz(from), dims(s))) {
      const auto v = cell(raw);
      if (v == previous)
        continue;
      const int h = distance(v);
      if (h >= best)
        continue;
      --probes;
      if (s.move(from, v, p)) {
        best = h;
        chosen = v;
      }
    }
    if (!chosen) {
      reached = false;
      break;
    }
    cost += metric(from, *chosen, s) * 4;
    previous = from;
    from = *chosen;
  }
  budget = probes / 26;
  return reached && unsigned(cost) <= unsigned(metric(start, to, s) * 4 + 16);
}
Cell adjustNavigationNode(Cell p, const RegionalGraphServices &s) {
  CreatureMovementParameters q;
  bool found = false;
  for (int height : {4, 1}) {
    for (auto mode :
         std::array<std::pair<int, int>, 3>{{{0, 0}, {0, 3}, {1, 0}}}) {
      q = parameters(height, 1, mode.first, mode.second);
      if (s.legal(p, q)) {
        found = true;
        break;
      }
    }
    if (found)
      break;
  }
  if (!found)
    return p;
  auto wide = q;
  wide.type_08 = 2;
  if (s.legal(p, wide))
    return p;
  for (auto raw : adjacent_cells(xyz(p), dims(s))) {
    auto v = cell(raw); // 53b600 rejects XY wrap here.
    if (std::abs(v.x - p.x) > 1 || std::abs(v.y - p.y) > 1)
      continue;
    if (s.move(p, v, q) && s.legal(v, q) && s.legal(v, wide))
      return v;
  }
  return p;
}
std::array<int, 6>
navigationHeights(const std::array<RegionalCreatureShape, 28> &types,
                  bool ground, bool free, int kind, int width) {
  std::array<int, 6> out;
  out.fill(-1);
  unsigned count = 0;
  for (unsigned i = 0; i < types.size(); ++i) {
    if (i == 22)
      continue;
    const auto &v = types[i];
    if (!(ground || free || (kind && v.movementKind == kind)) ||
        (width && v.width != width))
      continue;
    if (std::find(out.begin(), out.begin() + count, v.height) !=
        out.begin() + count)
      continue;
    if (count == out.size())
      throw std::invalid_argument("original height-list capacity exceeded");
    out[count++] = v.height;
  }
  if (count > 1) {
    unsigned low = 0, high = 0;
    for (unsigned i = 1; i < count; ++i) {
      if (out[high] < out[i])
        high = i;
      if (out[i] < out[low])
        low = i;
    }
    if (low) {
      std::swap(out[0], out[low]);
      if (high == 0)
        high = low;
    }
    if (high != 1)
      std::swap(out[1], out[high]);
  }
  return out;
}
void sortNavigationNodes(std::vector<NavigationRegion> &g, unsigned capacity,
                         Extent d) {
  if (capacity < g.size() || capacity > 4096)
    throw std::invalid_argument("regional sort capacity");
  std::vector<std::pair<int, NavigationRegion>> records;
  const int sentinel = (d.x - 1) * d.x * d.y + d.z + (d.y - 1) * d.x;
  for (const auto &v : g)
    records.push_back(
        {v.position.x * d.x * d.y + v.position.y * d.x + v.position.z, v});
  records.resize(capacity, {sentinel, {}});
  originalSort(records, [](const auto &v) { return v.first; });
  for (unsigned i = 0; i < g.size(); ++i)
    g[i] = std::move(records[i].second);
}
std::vector<NavigationRegion>
buildNavigationNodes(unsigned capacity, const RegionalGraphServices &s) {
  if (!capacity || capacity > 4096 || s.dimensions.x < 4 ||
      s.dimensions.y < 4 || s.dimensions.z < 1 || !s.kind || !s.valid ||
      !s.legal || !s.move || !s.line || !s.route)
    throw std::invalid_argument("navigation construction admission");
  std::vector<NavigationRegion> g;
  const auto valid = CreatureMovementParameters{1, 0, 0, 0, 0, 0, 0};
  const auto append = [&](Cell p) {
    NavigationRegion v;
    for (auto &e : v.connections)
      e.destination = 0;
    v.position = p;
    g.push_back(v);
    return g.size() == capacity;
  };
  const auto sort = [&] { sortNavigationNodes(g, capacity, s.dimensions); };
  // Scan a column from z, keeping only the useful endpoints of empty vertical
  // runs.
  const auto column = [&](int x, int y, int z, bool have, Cell last) {
    bool scan = !have;
    for (; z < s.dimensions.z; ++z) {
      Cell p{x, y, z};
      const auto kind = s.kind(p);
      if (!scan && !kind)
        continue;
      scan = true;
      if (!s.valid(p, valid))
        continue;
      auto v = adjustNavigationNode(p, s);
      scan = false;
      if (have && !kind && !s.legal(v, parameters(1, 1, 0, 3)) &&
          s.line(last, v, parameters(1, 2, 1)))
        continue;
      if (append(v))
        return true;
      have = true;
      last = v;
    }
    if (have)
      g.back().parameter = 1;
    return false;
  };
  for (int x = 2; x < s.dimensions.x; x += 5)
    for (int y = 2; y < s.dimensions.y; y += 5)
      if (column(x, y, 0, false, {}))
        return g;
  sort();
  for (int x = 0; x < s.dimensions.x; ++x)
    for (int y = 0; y < s.dimensions.y; ++y)
      for (int z = 0; z < s.dimensions.z; ++z) {
        Cell p{x, y, z};
        if (!s.valid(p, valid))
          continue;
        bool uncovered = false;
        if (s.legal(p, parameters(4, 1)) &&
            lookup(g, p, parameters(4, 1), s) == -1)
          uncovered = true;
        else if (s.legal(p, parameters(1, 1))) {
          uncovered = lookup(g, p, parameters(1, 1), s) == -1;
        } else if (s.legal(p, parameters(1, 1, 0, 3))) {
          uncovered = lookup(g, p, parameters(1, 1, 0, 3), s) == -1;
        }
        if (!uncovered)
          continue;
        auto v = adjustNavigationNode(p, s);
        if (append(v))
          return g;
        if (column(x, y, z + 1, true, v))
          return g;
        sort();
        z = s.dimensions.z;
      }
  return g;
}
namespace {
RegionConnection connection(Cell from, Cell to,
                            const std::array<RegionalCreatureShape, 28> &types,
                            int destination, int adjustment,
                            const RegionalGraphServices &s) {
  RegionConnection e;
  e.destination = destination;
  e.clearance.fill(0);
  e.clearance[15] = e.clearance[16] = 6;
  const auto legal = [&](int width, int free, int kind) {
    auto p = parameters(1, width, free, kind);
    return s.legal(from, p) && s.legal(to, p);
  };
  const auto attempt = [&](int group, int width, int free, int kind) {
    auto list = navigationHeights(types, group == 0, group == 1, kind, width);
    for (unsigned i = 0; i < list.size() && list[i] != -1; ++i) {
      const int height = list[i];
      const unsigned base = unsigned(group * 3);
      if (e.clearance[base] && e.clearance[base + unsigned(width)] >= height &&
          (group != 4 || e.clearance[14 + unsigned(width)] <= height))
        continue;
      auto p = parameters(height, width, free, kind);
      if (!s.legal(from, p) || !s.legal(to, p)) {
        if (group != 4 && i == 0)
          break;
        continue;
      }
      int budget = (group ? 62 : 125) + adjustment, cost = 0;
      bool reached = navigationDirectRoute(from, to, p, budget, cost, s);
      if (!reached) {
        auto path = s.route(from, to, p, budget);
        reached = !path.empty() && path.back() == to;
        if (reached) {
          cost = 0;
          for (unsigned j = 1; j < path.size(); ++j)
            cost += metric(path[j - 1], path[j], s) * 4;
        }
      }
      if (reached) {
        if (!e.cost)
          e.cost = cost;
        e.clearance[base] = 1;
        e.clearance[base + unsigned(width)] = std::int8_t(
            std::max(int(e.clearance[base + unsigned(width)]), height));
        if (group == 4)
          e.clearance[14 + unsigned(width)] = std::int8_t(
              std::min(int(e.clearance[14 + unsigned(width)]), height));
      }
      if (group != 4 && (reached || i == 0)) {
        if (i == 1 || !reached)
          break;
      }
    }
  };
  if (legal(2, 0, 0)) {
    attempt(0, 2, 0, 0);
    auto h = navigationHeights(types, true, false, 0, 2);
    if (e.clearance[0] && e.clearance[2] == h[1]) {
      auto v = e.clearance[2];
      for (unsigned base : {0u, 3u, 6u, 9u, 12u}) {
        e.clearance[base] = 1;
        e.clearance[base + 1] = v;
        e.clearance[base + 2] = v;
      }
      e.clearance[15] = e.clearance[16] = 0;
      return e;
    }
  }
  if (legal(1, 0, 0)) {
    if (e.clearance[0])
      e.clearance[1] = e.clearance[2];
    attempt(0, 1, 0, 0);
  }
  for (unsigned base : {3u, 9u, 12u})
    if (e.clearance[0]) {
      e.clearance[base] = 1;
      e.clearance[base + 1] = e.clearance[1];
      e.clearance[base + 2] = e.clearance[2];
    }
  if (e.clearance[12])
    e.clearance[15] = e.clearance[16] = 0;
  if (legal(2, 1, 0)) {
    attempt(1, 2, 1, 0);
    if (e.clearance[3])
      e.clearance[4] = std::max(e.clearance[4], e.clearance[5]);
  }
  if (legal(1, 1, 0))
    attempt(1, 1, 1, 0);
  if (s.boundary) {
    if (legal(2, 0, 3)) {
      attempt(3, 2, 0, 3);
      if (e.clearance[9])
        e.clearance[10] = std::max(e.clearance[10], e.clearance[11]);
    }
    if (legal(1, 0, 3))
      attempt(3, 1, 0, 3);
    // Original group4 is not endpoint-gated by a one-cell probe.
    attempt(4, 2, 0, 1);
    if (e.clearance[12]) {
      e.clearance[13] = std::max(e.clearance[13], e.clearance[14]);
      e.clearance[15] = std::min(e.clearance[15], e.clearance[16]);
    }
    attempt(4, 1, 0, 1);
  }
  return e;
}
} // namespace
void buildNavigationConnections(
    std::vector<NavigationRegion> &g,
    const std::array<RegionalCreatureShape, 28> &types, bool wrap,
    const RegionalGraphServices &s, int adjustment) {
  for (auto &v : g) {
    v.connections = {};
    for (auto &e : v.connections)
      e.destination = 0;
  }
  for (unsigned i = 0; i < g.size(); ++i)
    for (unsigned j = i + 1; j < g.size(); ++j) {
      const auto a = g[i].position, b = g[j].position;
      int x = std::abs(a.x - b.x), y = std::abs(a.y - b.y);
      if (wrap) {
        x = std::min(x, s.dimensions.x - x);
        y = std::min(y, s.dimensions.y - y);
      }
      const int distance = (2 * std::max(x, y) + std::min(x, y)) / 2;
      if (distance >= 9)
        continue;
      const auto p = parameters(1, 1, 0, 3);
      if (distance > 5 && s.legal(a, p) != s.legal(b, p))
        continue;
      auto e = connection(a, b, types, int(j), adjustment, s);
      if (!e.cost)
        continue;
      auto slot = [&](auto &v, int to) {
        for (auto &c : v.connections)
          if (c.cost && c.destination == to)
            return &c;
        for (auto &c : v.connections)
          if (!c.cost)
            return &c;
        return static_cast<RegionConnection *>(nullptr);
      };
      auto *left = slot(g[i], int(j)), *right = slot(g[j], int(i));
      if (left && right) {
        *left = e;
        e.destination = int(i);
        *right = e;
      }
    }
}
} // namespace mnm::reconstruction::combat
