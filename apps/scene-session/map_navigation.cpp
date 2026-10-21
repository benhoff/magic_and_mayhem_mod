#include "map_navigation.hpp"
#include "../../reconstruction/pathfinding/route_boundary.hpp"
#include "../../reconstruction/pathfinding/route_cell_support.hpp"
#include "../../reconstruction/pathfinding/route_clearance.hpp"
#include "../../reconstruction/pathfinding/route_line.hpp"
#include <cstring>
#include <limits>
#include <stdexcept>
namespace mnm::scene {
namespace r = reconstruction;
namespace model = reconstruction::combat;
namespace {
r::Coordinates xyz(model::Cell p) { return {p.x, p.y, p.z}; }
model::Cell cell(r::Coordinates p) { return {p.x, p.y, p.z}; }
bool same(const assets::MapCell &a, const assets::MapCell &b) {
  return a.definition == b.definition && a.references == b.references &&
         a.flags8 == b.flags8 && a.flags10 == b.flags10;
}
} // namespace
struct MapNavigationRepair::Geometry {
  assets::MapAsset source;
  std::shared_ptr<const std::vector<r::TerrainValidityRecord>> terrain;
  std::vector<r::OccupancyCell> cells;
  std::vector<int> rows, layers;
  Geometry(
      const assets::MapAsset &map,
      std::shared_ptr<const std::vector<r::TerrainValidityRecord>> definitions)
      : source(map), terrain(std::move(definitions)) {
    if (!map.width || map.width > 128 || !map.height || map.height > 128 ||
        !map.layers || map.layers > 32 ||
        map.cells.size() != std::size_t(map.width) * map.height * map.layers ||
        terrain->empty())
      throw std::invalid_argument("Navigation map dimensions/catalog");
    cells.resize(map.cells.size());
    for (unsigned y = 0; y < map.height; ++y)
      rows.push_back(int(y * map.width));
    for (unsigned z = 0; z < map.layers; ++z)
      layers.push_back(int(z * map.width * map.height));
    for (std::size_t i = 0; i < cells.size(); ++i) {
      const auto &c = map.cells[i];
      if (c.definition >= terrain->size())
        throw std::out_of_range("Navigation terrain definition");
      auto &v = cells[i];
      std::memcpy(v.unknown_00.data(), &c.definition, 2);
      std::memcpy(v.unknown_00.data() + 2, &c.references[0], 2);
      v.occupant_04 = c.references[1];
      std::memcpy(v.unknown_06.data(), &c.references[2], 2);
      std::memcpy(v.unknown_06.data() + 2, &c.flags8, 2);
      v.flags_0a = std::uint8_t(c.flags10);
      v.unknown_0b = std::byte(c.flags10 >> 8);
    }
  }
  r::MovementHelpers movement(int boundary) const {
    const auto &m = source;
    r::CellValidityMapView view{{{int(m.width), int(m.height), int(m.layers)},
                                 cells.data(),
                                 cells.size(),
                                 rows.data(),
                                 rows.size(),
                                 layers.data(),
                                 layers.size(),
                                 int(m.width * m.height),
                                 int(m.layers)},
                                terrain->data(),
                                terrain->size()};
    r::MovementHelpers h;
    h.dimensions = view.map.dimensions;
    h.boundary = [boundary] { return boundary; };
    h.layer_count = [count = int(m.layers)] { return count; };
    r::RecordQueryHelpers q;
    q.dimensions = h.dimensions;
    q.boundary = h.boundary;
    h = r::with_clearance_tests(h, view);
    h = r::with_record_query(h, r::with_cell_support(q, view));
    r::ValidityHelpers valid;
    valid.dimensions = h.dimensions;
    h = r::with_validity_test(h, r::with_cell_validity_test(valid, view));
    return r::with_boundary_test(h);
  }
};
model::RegionalGraphServices
MapNavigationRepair::services(std::shared_ptr<const Geometry> g, int boundary) {
  model::RegionalGraphServices s;
  const auto &map = g->source;
  s.dimensions = {int(map.width), int(map.height), int(map.layers)};
  s.boundary = boundary;
  const auto h = g->movement(boundary);
  s.kind = [g](auto q) {
    const auto &m = g->source;
    if (q.x < 0 || q.y < 0 || q.z < 0 || unsigned(q.x) >= m.width ||
        unsigned(q.y) >= m.height || unsigned(q.z) >= m.layers)
      throw std::out_of_range("Navigation kind coordinates");
    return unsigned(
        m.cells
            .at((std::size_t(q.z) * m.height + unsigned(q.y)) * m.width +
                unsigned(q.x))
            .definition);
  };
  s.valid = [g, h](auto q, auto parameters) {
    (void)g; // Retain owned arrays behind the recovered borrowed views.
    parameters.type_08 = 1;
    return h.check(r::MovementCheck::check_46b0, xyz(q), parameters);
  };
  s.legal = [g, h](auto q, const auto &parameters) {
    (void)g;
    return h.check(r::MovementCheck::check_46b0, xyz(q), parameters) &&
           !h.check(r::MovementCheck::check_41a0, xyz(q), parameters) &&
           (h.record(xyz(q), parameters) ||
            (parameters.type_44 == 2 && q.z < h.boundary()) ||
            parameters.object_104);
  };
  s.move = [g, h](auto from, auto to, const auto &parameters) {
    (void)g;
    int category = 5;
    return r::test_movement(xyz(from), xyz(to), parameters, category, -1, h);
  };
  s.line = [g, h](auto from, auto to, const auto &parameters) {
    (void)g;
    return r::test_creature_line(
        xyz(from), xyz(to), parameters,
        {h.dimensions,
         [h](auto a, auto b, const auto &v, int &category, int record) {
           return r::test_movement(a, b, v, category, record, h);
         }});
  };
  s.resumeRoute = [g, h](auto from, auto to, const auto &parameters,
                         int &budget, bool reset,
                         model::NavigationProbeState &saved) {
    (void)g;
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
MapNavigationRepair::MapNavigationRepair(const assets::MapAsset &map,
                                         const assets::TerrainCatalog &catalog,
                                         MapNavigationConfig config)
    : config_(config) {
  if (config.boundary < 0 || unsigned(config.boundary) > map.layers ||
      !config.capacity || config.capacity > 4096 || catalog.records.empty() ||
      catalog.records.size() > 65536)
    throw std::invalid_argument("Navigation configuration limits");
  for (const auto &type : config.shapes)
    if (type.height < 1 || type.height > 32 || type.width < 1 ||
        type.width > 2 || type.movementKind < 0 || type.movementKind > 3)
      throw std::invalid_argument("Navigation creature shape profile");
  auto terrain = std::make_shared<std::vector<r::TerrainValidityRecord>>(
      catalog.records.size());
  for (std::size_t i = 0; i < terrain->size(); ++i)
    std::memcpy(&(*terrain)[i], catalog.records[i].data(), 356);
  terrain_ = std::move(terrain);
  geometry_ = std::make_shared<Geometry>(map, terrain_);
  auto s = services(geometry_, config_.boundary);
  regions_ = model::buildNavigationNodes(config.capacity, s);
  model::sortNavigationNodes(regions_, config.capacity, s.dimensions);
  model::buildNavigationConnections(regions_, config.shapes, true, s);
}
model::NavigationChangeSignature
MapNavigationRepair::signature(model::Cell q) const {
  return model::navigationChangeSignature(
      q, services(geometry_, config_.boundary));
}
void MapNavigationRepair::update(const assets::MapAsset &map) {
  const auto &old = geometry_->source;
  if (map.width != old.width || map.height != old.height ||
      map.layers != old.layers || map.cells.size() != old.cells.size())
    throw std::invalid_argument(
        "Navigation map rebinding requires initialization");
  if (revision_ == std::numeric_limits<std::uint64_t>::max())
    throw std::overflow_error("Navigation revision exhausted");
  std::vector<std::size_t> changes;
  for (std::size_t i = 0; i < map.cells.size(); ++i)
    if (!same(map.cells[i], old.cells[i]))
      changes.push_back(i);
  if (changes.empty())
    return;
  auto next = std::make_shared<Geometry>(map, terrain_);
  const auto before = services(geometry_, config_.boundary),
             after = services(next, config_.boundary);
  auto maintenance = maintenance_;
  for (auto at : changes) {
    const model::Cell q{int(at % map.width), int(at / map.width % map.height),
                        int(at / (map.width * map.height))};
    const auto mode =
        model::navigationChangeMode(model::navigationChangeSignature(q, before),
                                    model::navigationChangeSignature(q, after));
    model::enqueueNavigationRepair(regions_, q, mode, before.dimensions,
                                   maintenance);
  }
  geometry_ = std::move(next);
  maintenance_ = std::move(maintenance);
  ++revision_;
}
void MapNavigationRepair::step(bool blocked) {
  if (blocked || !pending())
    return;
  auto graph = regions_;
  auto maintenance = maintenance_;
  model::maintainNavigationGraph(graph, config_.shapes, maintenance, false,
                                 services(geometry_, config_.boundary));
  regions_ = std::move(graph);
  maintenance_ = std::move(maintenance);
}
} // namespace mnm::scene
