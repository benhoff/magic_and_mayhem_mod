#include "native_regional_graph.hpp"
#include <algorithm>
#include <cstring>
#include <stdexcept>
namespace mnm::command_session {
namespace r = reconstruction;
namespace {
r::Coordinates xyz(model::Cell p) { return {p.x, p.y, p.z}; }
model::Cell cell(r::Coordinates p) { return {p.x, p.y, p.z}; }
} // namespace
model::RegionalGraphServices
nativeRegionalGraphServices(Continuation &c, const NativeCreatureProfile &p,
                            NativeCreatureGeometry &geometry) {
  model::RegionalGraphServices s;
  s.dimensions = c.world.dimensions;
  s.boundary = p.boundary;
  auto h = geometry.movement();
  s.kind = [&c](auto q) {
    auto d = c.world.dimensions;
    return unsigned(c.native.cells.at((q.z * d.y + q.y) * d.x + q.x).kind);
  };
  // A width-one wrapper equals the direct4f3440 scan used by placement.
  s.valid = [h](auto q, auto parameters) {
    parameters.type_08 = 1;
    return h.check(r::MovementCheck::check_46b0, xyz(q), parameters);
  };
  s.legal = [h](auto q, const auto &parameters) {
    return h.check(r::MovementCheck::check_46b0, xyz(q), parameters) &&
           !h.check(r::MovementCheck::check_41a0, xyz(q), parameters) &&
           (h.record(xyz(q), parameters) ||
            (parameters.type_44 == 2 && q.z < h.boundary()) ||
            parameters.object_104);
  };
  s.move = [h](auto from, auto to, const auto &parameters) {
    int category = 5;
    return r::test_movement(xyz(from), xyz(to), parameters, category, -1, h);
  };
  s.line = [h](auto from, auto to, const auto &parameters) {
    return r::test_creature_line(
        xyz(from), xyz(to), parameters,
        {h.dimensions,
         [h](auto a, auto b, const auto &v, int &category, int record) {
           return r::test_movement(a, b, v, category, record, h);
         }});
  };
  s.resumeRoute = [h](auto from, auto to, const auto &parameters, int &budget,
                      bool reset, model::NavigationProbeState &saved) {
    const auto d = h.dimensions;
    r::NeighborHelpers n;
    n.dimensions = d;
    n.node_at = [d](auto q) {
      return unsigned((q.z * d.y + q.y) * d.x + q.x + 1);
    };
    n.coordinates = [d](unsigned node) {
      --node;
      return r::Coordinates{int(node % unsigned(d.x)),
                            int(node / unsigned(d.x) % unsigned(d.y)),
                            int(node / unsigned(d.x * d.y))};
    };
    n.record = [h, parameters](const auto &, auto q) {
      return r::NeighborRecord{0, false, h.record(q, parameters)};
    };
    n = r::with_standard_movement_test(n, h);
    r::NeighborDescriptor descriptor;
    descriptor.mode = 1;
    descriptor.start = xyz(from);
    descriptor.target = xyz(to);
    std::memcpy(descriptor.unknown_18.data(), &parameters, 22);
    r::SearchWorld world{d, n.node_at, n.coordinates,
                         [n, descriptor](auto current, const auto &prior,
                                         const auto &, unsigned) {
                           return r::generate_neighbors(current, prior,
                                                        descriptor, n);
                         }};
    r::ObjectPrefix object{};
    const auto start = xyz(from);
    std::memcpy(reinterpret_cast<unsigned char *>(&object) + 8, &start, 12);
    r::RouteContextPrefix context{};
    context.unknown_route_flag = reset ? 1 : 0;
    r::SearchState state;
    state.best_node = saved.best;
    state.best_heuristic = saved.heuristic;
    state.initialized = saved.initialized;
    for (const auto &v : saved.open)
      state.open.emplace(v.priority, v.node);
    for (const auto &v : saved.records)
      state.records.emplace(v.node, v.record);
    auto result =
        r::route_search(context, object, 0, xyz(to), budget, state, world);
    saved = {};
    saved.best = state.best_node;
    saved.heuristic = state.best_heuristic;
    saved.initialized = state.initialized;
    for (const auto &v : state.open)
      saved.open.push_back({v.first, v.second});
    for (const auto &v : state.records)
      saved.records.push_back({v.first, v.second});
    std::vector<model::Cell> path;
    for (auto node : result.path)
      path.push_back(cell(n.coordinates(node)));
    return path;
  };
  s.route = [search = s.resumeRoute](auto from, auto to, const auto &parameters,
                                     int &budget) {
    model::NavigationProbeState state;
    return search(from, to, parameters, budget, true, state);
  };
  return s;
}
void prepareNativeRegionalGraph(
    Continuation &c, const NativeCreatureProfile &p,
    const std::array<model::RegionalCreatureShape, 28> &types,
    unsigned capacity) {
  for (const auto &shape : types)
    if (shape.height < 1 || shape.height > 5 || shape.width < 1 ||
        shape.width > 2 || shape.movementKind < 0 || shape.movementKind > 3)
      throw std::invalid_argument("native graph shape domain");
  if (!c.native.regions.empty() || c.native.regionalSearch.initialized)
    throw std::invalid_argument("regional construction needs an empty graph");
  for (const auto &route : c.actions.segments)
    if (route.regionPending)
      throw std::invalid_argument(
          "regional construction has retained actor routes");
  NativeCreatureGeometry geometry(c, p);
  auto services = nativeRegionalGraphServices(c, p, geometry);
  auto graph = model::buildNavigationNodes(capacity, services);
  model::sortNavigationNodes(graph, capacity, c.world.dimensions);
  model::buildNavigationConnections(graph, types, true, services);
  c.native.regions = std::move(graph);
  c.native.regionalMaintenance.emplace();
  c.native.regionalMaintenance->shapes = types;
  c.native.regionalSearch = {};
  c.native.route.useRegions = true;
}
void maintainNativeRegionalGraph(Continuation &c,
                                 const NativeCreatureProfile &p, bool blocked) {
  if (!c.native.regionalMaintenance)
    return;
  auto &m = *c.native.regionalMaintenance;
  if (!model::navigationMaintenancePending(m.state) || blocked)
    return;
  NativeCreatureGeometry geometry(c, p);
  auto services = nativeRegionalGraphServices(c, p, geometry);
  model::maintainNavigationGraph(c.native.regions, m.shapes, m.state, false,
                                 services);
}
void editNativeTerrain(Continuation &c, const NativeCreatureProfile &p,
                       const std::vector<NativeTerrainEdit> &edits) {
  if (!c.native.present || !c.native.regionalMaintenance ||
      !c.native.route.useRegions)
    throw std::invalid_argument(
        "terrain edit requires configured native regional graph");
  if (edits.size() > 4096)
    throw std::invalid_argument("native terrain edit bound");
  auto staged = c;
  const auto d = staged.world.dimensions;
  const auto index = [&](model::Cell q) {
    if (q.x < 0 || q.x >= d.x || q.y < 0 || q.y >= d.y || q.z < 0 || q.z >= d.z)
      throw std::invalid_argument("terrain edit coordinate");
    return unsigned((q.z * d.y + q.y) * d.x + q.x);
  };
  const auto signature = [&](model::Cell q) {
    NativeCreatureGeometry geometry(staged, p);
    auto services = nativeRegionalGraphServices(staged, p, geometry);
    return model::navigationChangeSignature(q, services);
  };
  for (const auto &edit : edits) {
    const auto at = index(edit.position);
    if (edit.kind >= p.terrain.size() || (edit.flagsMask & 3) ||
        (edit.flagsValue & ~edit.flagsMask))
      throw std::invalid_argument("terrain edit descriptor");
    const auto before = signature(edit.position);
    auto &cell = staged.native.cells.at(at);
    cell.kind = edit.kind;
    cell.flags =
        std::uint16_t((cell.flags & ~edit.flagsMask) | edit.flagsValue);
    const auto after = signature(edit.position);
    model::enqueueNavigationRepair(staged.native.regions, edit.position,
                                   model::navigationChangeMode(before, after),
                                   d, staged.native.regionalMaintenance->state);
  }
  validate(staged);
  c = std::move(staged);
}
} // namespace mnm::command_session
