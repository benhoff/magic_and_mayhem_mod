#include "../../assets/dat.hpp"
#include "../../assets/persistence.hpp"
#include "../../reconstruction/combat/autonomy_tables.hpp"
#include "../command-session/native_geometry.hpp"
#include "../command-session/native_owner_planner.hpp"
#include "../command-session/native_owner_sources.hpp"
#include "encounter.hpp"
#include <algorithm>
#include <charconv>
#include <cstring>
#include <fstream>
#include <sstream>
#include <stdexcept>
namespace mnm::encounter {
namespace {
void need(bool b, const char *s) {
  if (!b)
    throw std::invalid_argument(s);
}
void word(game::Bytes &b, unsigned n) {
  for (unsigned i = 0; i < 4; ++i)
    b.push_back(std::uint8_t(n >> (8 * i)));
}
unsigned take(const game::Bytes &b, std::size_t &at) {
  need(at <= b.size() && b.size() - at >= 4, "encounter input extent");
  unsigned v = 0;
  for (unsigned i = 0; i < 4; ++i)
    v |= unsigned(b[at++]) << (8 * i);
  return v;
}
game::Bytes read(const std::filesystem::path &p) {
  auto n = std::filesystem::file_size(p);
  need(n <= 4 * 1024 * 1024, "encounter input limit");
  std::ifstream f(p, std::ios::binary);
  need(bool(f), "encounter input missing");
  game::Bytes b(n);
  f.read(reinterpret_cast<char *>(b.data()), std::streamsize(n));
  need(bool(f), "encounter input read");
  return b;
}
assets::Config config(const game::Bytes &b) {
  auto p = assets::decodePackedContainer(b);
  need(std::holds_alternative<assets::PackedContainer>(p),
       "encounter packed CFG");
  auto c = assets::decodeConfig(std::get<assets::PackedContainer>(p).decoded);
  need(std::holds_alternative<assets::Config>(c), "encounter CFG");
  return std::get<assets::Config>(std::move(c));
}
int integer(const std::string &s) {
  int v = 0;
  auto r = std::from_chars(s.data(), s.data() + s.size(), v);
  need(r.ec == std::errc{} && r.ptr == s.data() + s.size(),
       "encounter integer");
  return v;
}
const std::string &field(const assets::Config &c, const char *section,
                         const char *key) {
  auto p = c.find(section, key);
  need(p != nullptr, "encounter CFG field missing");
  return *p;
}
std::array<int, 28> row(const assets::Config &c, const char *section) {
  std::istringstream in(field(c, section, "Creat_17"));
  std::array<int, 28> out{};
  std::string v;
  unsigned i = 0;
  while (std::getline(in, v, ',')) {
    need(i < 28, "encounter matrix width");
    auto a = v.find_first_not_of(" \t"), z = v.find_last_not_of(" \t");
    need(a != std::string::npos, "encounter matrix value");
    out[i++] = integer(v.substr(a, z - a + 1));
  }
  need(i == 28, "encounter matrix width");
  return out;
}
} // namespace
void enableOwnerPlanner(Profile &p, const std::filesystem::path &path,
                        unsigned seed) {
  auto result = assets::decodeBrainDat(read(path));
  need(std::holds_alternative<assets::BrainDat>(result),
       "owner planner DAT input");
  const auto &models = std::get<assets::BrainDat>(result).models;
  const auto found = std::find_if(models.begin(), models.end(),
                                  [](const auto &v) { return v.key == 6; });
  need(found != models.end(), "owner planner missing model6");
  cs::model::BrainInferenceModel model;
  model.key = found->key;
  model.dimension = found->dimension;
  auto number = [](unsigned v) {
    float f;
    std::memcpy(&f, &v, 4);
    return f;
  };
  for (const auto &layer : found->layers) {
    cs::model::BrainInferenceLayer out;
    out.observations = number(layer.scalarBits);
    out.outside = number(layer.tailBits);
    for (const auto &node : layer.nodes) {
      std::array<float, 12> values;
      for (unsigned i = 0; i < 12; ++i)
        values[i] = number(node[i]);
      out.nodes.push_back(values);
    }
    for (unsigned v : layer.matrix)
      out.matrix.push_back(number(v));
    model.layers.push_back(std::move(out));
  }
  auto staged = p;
  if (!staged.initial.native.ownerVisibility) {
    placeOwnerPlannerEncounter(staged);
    enableOwnerVisibility(staged);
  }
  cs::prepareNativeOwnerPlanner(staged.initial, std::move(model), 0, 0, seed);
  if(staged.scenario==8 || staged.scenario==9)cs::prepareNativeOwnerSources(staged.initial,staged.scenario==9);
  p = std::move(staged);
}
game::Bytes loadInputs(const std::filesystem::path &root, scene::MapCrop crop,
                       unsigned scenario) {
  game::Bytes b{'M', 'N', 'M', 'E', 'N', 'C', '0', '1'};
  need(scenario <= 9, "encounter scenario");
  if (scenario) {
    b[7] = '2';
    word(b, scenario);
  }
  for (auto v : {crop.x, crop.y, crop.width, crop.height})
    word(b, v);
  for (const char *p :
       {"CFG/Encrypted/creature.cfg", "CFG/Encrypted/HTH.cfg",
        "Creatures/crocodile.ani", "Realms/Celtic/Plains/CPsec01.map",
        "Realms/Celtic/Plains/Terrain.ttd"}) {
    auto v = read(root / p);
    word(b, unsigned(v.size()));
    b.insert(b.end(), v.begin(), v.end());
  }
  return b;
}
Profile decodeInputs(game::Bytes bytes) {
  need(bytes.size() >= 24 &&
           std::equal(bytes.begin(), bytes.begin() + 7, "MNMENC0") &&
           (bytes[7] == '1' || bytes[7] == '2'),
       "encounter input version");
  std::size_t at = 8;
  const unsigned scenario = bytes[7] == '2' ? take(bytes, at) : 0;
  need(scenario <= 9, "encounter scenario");
  const unsigned actors = scenario == 9 ? 6 : scenario == 8 ? 3 : scenario == 1 || scenario == 3 ? 4
                          : scenario >= 2                ? 8
                                                         : 2;
  scene::MapCrop crop;
  crop.x = take(bytes, at);
  crop.y = take(bytes, at);
  crop.width = take(bytes, at);
  crop.height = take(bytes, at);
  std::array<game::Bytes, 5> input;
  for (auto &b : input) {
    auto n = take(bytes, at);
    need(n <= 4 * 1024 * 1024 && at <= bytes.size() && n <= bytes.size() - at,
         "encounter blob extent");
    b = {bytes.begin() + at, bytes.begin() + at + n};
    at += n;
  }
  need(at == bytes.size(), "encounter trailing data");
  auto cfg = config(input[0]), hth = config(input[1]);
  auto raw = assets::creatureMovementConfig(cfg, 17);
  auto movement = reconstruction::normalizeCreatureMovementConfig(raw);
  need(movement.width == 2 && movement.height == 2 && !movement.canFly &&
           movement.swimming == 3,
       "encounter ordinary crocodile profile");
  auto a = assets::decodeAnimation(input[2]);
  auto m = assets::decodeMap(input[3]);
  auto t = assets::decodeTerrainCatalog(input[4]);
  need(std::holds_alternative<assets::Animation>(a) &&
           std::holds_alternative<assets::MapAsset>(m) &&
           std::holds_alternative<assets::TerrainCatalog>(t),
       "encounter ANI/MAP/TTD decode");
  Profile p;
  p.bytes = std::move(bytes);
  p.maximum = unsigned(integer(field(cfg, "CREATURE_17", "Health")));
  need(p.maximum > 0 && p.maximum <= 100000, "encounter health limit");
  need(field(cfg, "CREATURE_17", "IsPhysical") == "TRUE" &&
           field(cfg, "CREATURE_17", "IsUndead") == "FALSE" &&
           integer(field(cfg, "CREATURE_17", "FireRate")) == 0,
       "encounter ordinary physical profile");
  p.native.animation.push_back(std::get<assets::Animation>(std::move(a)));
  auto catalog = std::get<assets::TerrainCatalog>(std::move(t));
  auto map =
      scene::projectMapNavigation(std::get<assets::MapAsset>(m), catalog, crop);
  p.projectedObjects = map.projectedObjects;
  p.projectedReferences = map.projectedReferences;
  p.native.terrain.resize(catalog.records.size());
  p.native.terrainPropertyA0.resize(catalog.records.size());
  for (unsigned i = 0; i < catalog.records.size(); ++i) {
    std::memcpy(&p.native.terrain[i], catalog.records[i].data(), 356);
    unsigned property = 0;
    std::memcpy(&property, catalog.records[i].data() + 0xa0, 4);
    p.native.terrainPropertyA0[i] = property != 0;
  }
  auto &type = p.native.types[17];
  type.width = movement.width;
  type.height = movement.height;
  type.movementKind = movement.swimming;
  type.scalar.type_10 = movement.acceleration;
  // Pinned original initial slope coefficient at5e15f0 (float bits).
  const unsigned slope = 0x3f781062;
  std::memcpy(&type.scalar.slope_global, &slope, 4);
  type.scalar.type_d8 =
      reconstruction::groundMovementSamples(p.native.animation[0]);
  type.scalar.type_3c =
      reconstruction::groundMovementMaximum(type.scalar.type_d8);
  type.scalar.default_scalar = p.native.defaultScalar;
  p.native.maintenance.localOwner = 0;
  p.native.maintenance.drainDivisor = 1;
  p.melee.base = row(hth, "HTH_Matrix")[17];
  p.melee.stunThreshold = row(hth, "Stun_Matrix")[17];
  // Original416d80 uses config280/LRC and1a0/HTH. The supported source
  // is physical class1/type17, so target58 scaling cannot apply.
  auto &credit = p.taskCredit[17];
  credit.kind = 1;
  credit.capacity = p.maximum;
  credit.flag58 = 1;
  const auto first = row(hth, "LRC_Matrix"), second = row(hth, "HTH_Matrix");
  for (unsigned i = 0; i < 28; ++i) {
    credit.first[i] = unsigned(first[i]);
    credit.second[i] = unsigned(second[i]);
  }
  p.scenario = scenario;
  auto &c = p.initial;
  c.world.dimensions = {int(map.geometry.width), int(map.geometry.height),
                        int(map.geometry.layers)};
  c.world.creatures.resize(actors);
  c.execution.resize(actors);
  c.bindings.assign(actors, cs::model::CommandBehaviorBinding::idle);
  c.generations.assign(actors, 1);
  c.native.cells.resize(map.geometry.cells.size());
  for (unsigned i = 0; i < c.native.cells.size(); ++i) {
    const auto &v = map.geometry.cells[i];
    c.native.cells[i] = {v.definition, 0xffff, v.flags10, v.flags8};
  }
  for (auto &r : c.world.relation)
    r.fill(0);
  for (unsigned i = 0; i < 8; ++i)
    c.world.relation[i][i] = 1;
  for (unsigned i = 0; i < actors; ++i) {
    auto &actor = c.world.creatures[i];
    actor.ai.slot = i;
    actor.ai.owner = int(scenario==9 ? i/3 : scenario==8 ? unsigned(i==2) : scenario >= 5 ? 0 : scenario >= 3 ? i / 2 : i);
    actor.width = movement.width;
    actor.height = movement.height;
    actor.movementKind = movement.swimming;
    actor.ai.health = actor.ai.maximumHealth = p.maximum;
    actor.ai.bravery = integer(field(cfg, "CREATURE_17", "Bravery"));
    actor.ai.hth = row(hth, "HTH_Matrix");
    actor.ai.enemyRadius = row(hth, "FearHate_Matrix");
    actor.ai.allyRadius = row(hth, "Assist_Matrix");
    actor.ai.retaliateThreshold = row(hth, "Evasion_Matrix");
    c.execution[i].actionCallback = 15;
    c.execution[i].roamRange = integer(field(cfg, "CREATURE_17", "WanderDist"));
    c.execution[i].roamInterval =
        integer(field(cfg, "CREATURE_17", "WanderDelay"));
  }
  if (scenario && scenario<8) {
    need(c.world.dimensions.x == 12 && c.world.dimensions.y == 12,
         "crowded crop domain");
    for (int z = 0; z < c.world.dimensions.z; ++z)
      for (int y = 1; y <= 6; ++y)
        c.native.cells[(z * 12 + y) * 12 + 5].flags |= 0x4000;
  }
  cs::NativeCreatureGeometry g(c, p.native);
  std::vector<cs::model::Cell> standing;
  for (int z = 1; z < c.world.dimensions.z - 1; ++z)
    for (int y = 1; y < c.world.dimensions.y - 2; ++y)
      for (int x = 1; x < c.world.dimensions.x - 2; ++x) {
        cs::model::Cell q{x, y, z};
        if (g.legal(c.world.creatures[0], q) &&
            g.ground(c.world.creatures[0], q))
          standing.push_back(q);
      }
  bool found = false;
  if (!scenario) {
    for (auto x : standing) {
      if (found)
        break;
      for (auto y : standing)
        if (x.z == y.z && std::abs(x.x - y.x) >= 4 &&
            std::abs(x.y - y.y) <= 1) {
          c.world.creatures[0].ai.position = x;
          c.world.creatures[1].ai.position = y;
          found = true;
          break;
        }
    }
    need(found, "encounter crop has no separated crocodile stands");
  } else {
    const std::vector<cs::model::Cell> positions =
        scenario == 1
            ? std::vector<cs::model::Cell>{{1, 1, 3},
                                           {8, 1, 3},
                                           {1, 7, 3},
                                           {8, 7, 3}}
            : std::vector<cs::model::Cell>{{1, 1, 3}, {3, 1, 3}, {7, 1, 3},
                                           {9, 1, 3}, {1, 7, 3}, {3, 7, 3},
                                           {7, 7, 3}, {9, 7, 3}};
    for (unsigned i = 0; i < actors; ++i) {
      bool selected = false;
      long best = 0;
      cs::model::Cell chosen;
      for (auto q : standing) {
        bool overlaps = false;
        for (unsigned j = 0; j < i; ++j) {
          auto old = c.world.creatures[j].ai.position;
          if (std::abs(q.x - old.x) < 2 && std::abs(q.y - old.y) < 2 &&
              std::abs(q.z - old.z) < 2)
            overlaps = true;
        }
        if (overlaps)
          continue;
        long cost = (q.x - positions[i].x) * (q.x - positions[i].x) +
                    (q.y - positions[i].y) * (q.y - positions[i].y) +
                    20 * (q.z - 3) * (q.z - 3);
        if (!selected || cost < best) {
          selected = true;
          best = cost;
          chosen = q;
        }
      }
      need(selected, "crowded stand domain");
      c.world.creatures[i].ai.position = chosen;
    }
  }
  for (auto &actor : c.world.creatures) {
    if (scenario >= 3)
      actor.destination = actor.ai.position;
    actor.ai.home = actor.ai.position;
    c.execution[actor.ai.slot].roamHome = actor.ai.position;
  }
  cs::prepareNativeContinuation(c, p.native);
  if (scenario && scenario<8) {
    // A common initial move request makes the scheduler encounter the sealed
    // column and another actor's occupied footprint before autonomous combat.
    auto &actor = c.world.creatures[0];
    actor.destination = c.world.creatures[1].ai.position;
    actor.ai.behavior = 2;
    actor.routePhase = 1;
    c.bindings[0] = cs::model::CommandBehaviorBinding::move;
  }
  if (scenario >= 3 && scenario<8) {
    auto &a = c.world.creatures[0];
    auto &b = c.world.creatures[1];
    a.routePhase = 2;
    a.ai.marker = 0;
    if (scenario == 6)
      a.destination = {5, 3, 3};
    auto &route = c.actions.segments[0];
    route.count = 1;
    route.cursor = 0;
    // A retained coarse route steps into the neighboring ally's footprint.
    route.route[0].position = {b.ai.position.x - 1, b.ai.position.y,
                               b.ai.position.z};
    route.route[0].remainder = {2, 0, 0, 0};
  }
  c.backend.assign(8 + actors * 31 * 4 + 6 * 4, 0);
  std::copy_n("MNMECB01", 8, c.backend.begin());
  for (unsigned i = 0; i < actors; ++i) {
    const auto pos = 8 + i * 31 * 4;
    for (unsigned j = 0; j < 4; ++j) {
      c.backend[pos + j] = std::uint8_t(p.maximum >> (j * 8));
      c.backend[pos + 4 + j] = std::uint8_t(14u >> (j * 8));
      c.backend[pos + 16 + j] = 255;
    }
  }
  return p;
}
} // namespace mnm::encounter
