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
  s.route = [h](auto from, auto to, const auto &parameters, int &budget) {
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
    context.unknown_route_flag = 1;
    r::SearchState state;
    auto result =
        r::route_search(context, object, 0, xyz(to), budget, state, world);
    std::vector<model::Cell> path;
    for (auto node : result.path)
      path.push_back(cell(n.coordinates(node)));
    return path;
  };
  return s;
}
void prepareNativeRegionalGraph(
    Continuation &c, const NativeCreatureProfile &p,
    const std::array<model::RegionalCreatureShape, 28> &types,
    unsigned capacity) {
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
  c.native.regionalSearch = {};
  c.native.route.useRegions = true;
}
} // namespace mnm::command_session
