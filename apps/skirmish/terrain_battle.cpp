#include "terrain_battle.hpp"
#include "../world-scene/map_navigation.hpp"
#include "creature_profile.hpp"
#include "frozen_navigation.hpp"
#include "route_world.hpp"
#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>
#include <cstring>
#include <deque>
#include <stdexcept>
namespace mnm::skirmish {
namespace {
namespace g = game;
void need(bool b, const char *m) {
  if (!b)
    throw std::invalid_argument(m);
}
constexpr std::array<g::Point, 8> directions{{{0, -1, 0},
                                              {1, -1, 0},
                                              {1, 0, 0},
                                              {1, 1, 0},
                                              {0, 1, 0},
                                              {-1, 1, 0},
                                              {-1, 0, 0},
                                              {-1, -1, 0}}};
std::vector<std::byte> native(const g::Bytes &b) {
  std::vector<std::byte> v(b.size());
  std::memcpy(v.data(), b.data(), b.size());
  return v;
}
std::uint32_t word(const g::Bytes &b, std::size_t p) {
  need(p + 4 <= b.size(), "terrain word extent");
  unsigned n = 0;
  for (unsigned i = 0; i < 4; ++i)
    n |= unsigned(b[p + i]) << (8 * i);
  return n;
}
void put(g::Bytes &b, std::size_t p, unsigned n) {
  need(p + 4 <= b.size(), "terrain field extent");
  for (unsigned i = 0; i < 4; ++i)
    b[p + i] = std::uint8_t(n >> (8 * i));
}
QString digest(const g::Bytes &b) {
  return QCryptographicHash::hash(
             QByteArray(reinterpret_cast<const char *>(b.data()), b.size()),
             QCryptographicHash::Sha256)
      .toHex();
}
g::Bytes read(assets::AssetStore &s, const std::string &path) {
  auto f = s.open(path);
  need(std::holds_alternative<std::unique_ptr<assets::AssetFile>>(f),
       "terrain asset unavailable");
  auto b = assets::readWhole(*std::get<std::unique_ptr<assets::AssetFile>>(f),
                             16 * 1024 * 1024);
  need(std::holds_alternative<assets::Bytes>(b), "terrain asset read failure");
  return std::get<assets::Bytes>(std::move(b));
}
// Keep battle routing on the admitted standing layer. Legality/clearance uses
// the existing recovered frozen helper; BFS, sealed edges and scheduling are
// explicit native policies. Full-world topology/search equivalence is excluded.
class Navigation final : public g::Navigation {
  std::shared_ptr<const g::Navigation> base_;
  g::NavigationBinding binding_;
  int plane_;
  bool inside(g::Point p) const {
    auto d = binding_.dimensions;
    return p.z == plane_ && p.x > 0 && p.y > 0 && p.x + 2 < d.x &&
           p.y + 2 < d.y;
  }
  std::optional<g::RoutePoint> edge(const g::Entity &e, g::Point p) const {
    if (!inside(p) || e.z != plane_)
      return {};
    auto dx = p.x - e.x, dy = p.y - e.y;
    if (dx < -1 || dx > 1 || dy < -1 || dy > 1 || (!dx && !dy))
      return {};
    try {
      base_->validateSpawnPosition(p);
    } catch (const std::invalid_argument &) {
      return {};
    }
    const auto direction =
        std::find_if(directions.begin(), directions.end(),
                     [&](auto q) { return q.x == dx && q.y == dy; }) -
        directions.begin();
    for (int category : {0, 4}) {
      g::RoutePoint q{p, int(direction), 0, category, 0};
      if (base_->accepts(e, q))
        return q;
    }
    return {};
  }
  g::RoutePlan search(const g::Entity &e, g::Point goal, unsigned budget,
                      const g::State *world, g::Handle self) const {
    g::RoutePlan out;
    if (!inside(goal))
      return out;
    const auto d = binding_.dimensions;
    auto index = [&](g::Point p) { return p.y * d.x + p.x; };
    const g::Point origin{e.x, e.y, e.z};
    std::vector<int> parents(d.x * d.y, -1);
    std::vector<g::RoutePoint> points(d.x * d.y);
    std::deque<g::Point> queue{origin};
    parents.at(index(origin)) = index(origin);
    while (!queue.empty()) {
      if (out.expansions == budget) {
        out.status = g::PlanStatus::budgetExhausted;
        return out;
      }
      auto p = queue.front();
      queue.pop_front();
      ++out.expansions;
      if (p == goal) {
        std::vector<g::RoutePoint> reverse;
        for (int at = index(p); at != index(origin); at = parents[at])
          reverse.push_back(points[at]);
        std::reverse(reverse.begin(), reverse.end());
        if (reverse.size() > 16)
          reverse.resize(16);
        out.route = std::move(reverse);
        out.status = g::PlanStatus::reachable;
        return out;
      }
      auto probe = e;
      probe.x = p.x;
      probe.y = p.y;
      probe.z = p.z;
      for (auto delta : directions) {
        g::Point q{p.x + delta.x, p.y + delta.y, p.z};
        if (!inside(q) || parents[index(q)] != -1)
          continue;
        auto step = edge(probe, q);
        if (!step)
          continue;
        // Occupied snapshot retains actual source position; query each
        // hypothetical edge with an owned source projection.
        if (world) {
          auto projected = *world;
          auto &body = *projected.slots.at(self.slot).entity;
          body.x = p.x;
          body.y = p.y;
          body.z = p.z;
          // The frozen overlay excludes self by slot; do not validate
          // this hypothetical whole-world placement as publication.
          if (!base_->acceptsInWorld(projected, self, *step))
            continue;
        }
        parents[index(q)] = index(p);
        points[index(q)] = *step;
        queue.push_back(q);
      }
    }
    return out;
  }

public:
  Navigation(g::Bytes b, int plane)
      : base_(sandbox::loadFrozenNavigationBytes(native(b), {}, true, true)),
        binding_(base_->binding()), plane_(plane) {
    need(plane > 0 && plane < binding_.dimensions.z,
         "terrain battle standing plane");
    for (unsigned char c : std::string("battle-plane-v1")) {
      binding_.fingerprint ^= c;
      binding_.fingerprint *= 1099511628211ULL;
    }
    binding_.fingerprint ^= unsigned(plane);
    binding_.fingerprint *= 1099511628211ULL;
  }
  g::NavigationBinding binding() const override { return binding_; }
  unsigned creatureType() const override { return base_->creatureType(); }
  unsigned maxMovingCreatures() const override { return 32; }
  g::Point creatureFootprint() const override {
    return base_->creatureFootprint();
  }
  bool supportsStationaryOccupants() const override { return true; }
  void validateOccupants(const g::State &s) const override {
    base_->validateOccupants(s);
  }
  void validateSpawnPosition(g::Point p) const override {
    need(inside(p), "outside battle standing layer");
    base_->validateSpawnPosition(p);
  }
  g::RoutePlan plan(const g::Entity &e, g::Point p, unsigned b) const override {
    return search(e, p, b, nullptr, {});
  }
  g::RoutePlan planInWorld(const g::State &s, g::Handle h, g::Point p,
                           unsigned b) const override {
    return search(*s.slots.at(h.slot).entity, p, b, &s, h);
  }
  bool accepts(const g::Entity &e, const g::RoutePoint &p) const override {
    auto q = edge(e, p.position);
    return q && q->direction == p.direction &&
           q->verticalDelta == p.verticalDelta && q->category == p.category;
  }
  bool acceptsInWorld(const g::State &s, g::Handle h,
                      const g::RoutePoint &p) const override {
    return accepts(*s.slots.at(h.slot).entity, p) &&
           base_->acceptsInWorld(s, h, p);
  }
};
struct Stored {
  g::Bytes frozen;
  QString mapHash, ttdHash, cfgHash, aniHash;
  unsigned plane = 0;
  std::array<g::Point, 2> wizards;
};
g::Bytes encode(const Stored &s) {
  QJsonArray w;
  for (auto p : s.wizards)
    w.append(QJsonArray{p.x, p.y, p.z});
  QJsonObject o{
      {"version", 1},
      {"map", "Realms/Celtic/Plains/CPsec01.map"},
      {"realm", "Realms/Celtic/Plains"},
      {"crop", QJsonArray{4, 4, 12, 12}},
      {"map_sha256", s.mapHash},
      {"ttd_sha256", s.ttdHash},
      {"cfg_sha256", s.cfgHash},
      {"ani_sha256", s.aniHash},
      {"plane", int(s.plane)},
      {"wizards", w},
      {"frozen", QString::fromLatin1(
                     QByteArray(reinterpret_cast<const char *>(s.frozen.data()),
                                s.frozen.size())
                         .toBase64())}};
  auto data = QJsonDocument(o).toJson(QJsonDocument::Compact);
  g::Bytes out{'M', 'N', 'M', 'T', 'E', 'R', '0', '1'};
  out.insert(out.end(), data.begin(), data.end());
  return out;
}
Stored parse(const g::Bytes &b) {
  need(b.size() > 8 && b.size() <= 2 * 1024 * 1024 &&
           std::equal(b.begin(), b.begin() + 8, "MNMTER01"),
       "terrain battle bundle version/extent");
  QJsonParseError e;
  auto doc = QJsonDocument::fromJson(
      QByteArray(reinterpret_cast<const char *>(b.data() + 8), b.size() - 8),
      &e);
  need(e.error == QJsonParseError::NoError && doc.isObject(),
       "terrain battle JSON");
  auto o = doc.object();
  Stored s;
  auto hash = [&](const char *k) {
    auto h = o[k].toString();
    need(h.size() == 64 && std::all_of(h.begin(), h.end(),
                                       [](QChar c) {
                                         return (c >= '0' && c <= '9') ||
                                                (c >= 'a' && c <= 'f');
                                       }),
         "terrain input hash");
    return h;
  };
  s.mapHash = hash("map_sha256");
  s.ttdHash = hash("ttd_sha256");
  s.cfgHash = hash("cfg_sha256");
  s.aniHash = hash("ani_sha256");
  s.plane = unsigned(o["plane"].toInt(-1));
  auto w = o["wizards"].toArray();
  need(w.size() == 2, "terrain wizard positions");
  for (int i = 0; i < 2; ++i) {
    auto p = w[i].toArray();
    need(p.size() == 3, "terrain wizard coordinate");
    s.wizards[i] = {p[0].toInt(-1), p[1].toInt(-1), p[2].toInt(-1)};
  }
  auto raw = QByteArray::fromBase64(o["frozen"].toString().toLatin1());
  s.frozen.assign(raw.begin(), raw.end());
  need(encode(s) == b, "noncanonical terrain battle bundle");
  return s;
}
TerrainBattle finish(const Stored &s) {
  auto nav = std::make_shared<Navigation>(s.frozen, int(s.plane));
  auto d = nav->binding().dimensions;
  need(d.x == 12 && d.y == 12 && d.z >= 2 && d.z <= 28 &&
           nav->creatureType() == 17 && nav->creatureFootprint().x == 2,
       "terrain battle profile/dimensions");
  for (auto p : s.wizards) {
    need(p.z == int(s.plane), "terrain wizard plane mismatch");
    nav->validateSpawnPosition(p);
  }
  need(std::abs(s.wizards[0].x - s.wizards[1].x) >= 3 ||
           std::abs(s.wizards[0].y - s.wizards[1].y) >= 3,
       "terrain wizard footprint overlap");
  TerrainBattle result{encode(s), nav, s.wizards, s.plane, s.frozen, {}};
  for (int x = 1; x < d.x - 2; ++x)
    for (int y = 1; y < d.y - 2; ++y)
      try {
        nav->validateSpawnPosition({x, y, int(s.plane)});
        result.standing.push_back({x, y, int(s.plane)});
      } catch (const std::invalid_argument &) {
      }
  return result;
}
} // namespace
TerrainBattle installedTerrainBattle(const std::filesystem::path &root) {
  auto store = assets::AssetStore::create(root);
  need(std::holds_alternative<assets::AssetStore>(store),
       "terrain installation unavailable");
  auto &a = std::get<assets::AssetStore>(store);
  auto map = read(a, "Realms/Celtic/Plains/CPsec01.map"),
       ttd = read(a, "Realms/Celtic/Plains/Terrain.ttd"),
       cfg = read(a, "CFG/Encrypted/creature.cfg"),
       ani = read(a, "Creatures/crocodile.ani");
  auto m = assets::decodeMap(map);
  auto t = assets::decodeTerrainCatalog(ttd);
  auto packed = assets::decodePackedContainer(cfg);
  auto animation = assets::decodeAnimation(ani);
  need(std::holds_alternative<assets::MapAsset>(m) &&
           std::holds_alternative<assets::TerrainCatalog>(t) &&
           std::holds_alternative<assets::PackedContainer>(packed) &&
           std::holds_alternative<assets::Animation>(animation),
       "terrain installed format");
  auto config =
      assets::decodeConfig(std::get<assets::PackedContainer>(packed).decoded);
  need(std::holds_alternative<assets::Config>(config), "terrain creature CFG");
  const auto c = reconstruction::normalizeCreatureMovementConfig(
      assets::creatureMovementConfig(std::get<assets::Config>(config), 17));
  need(c.width == 2 && !c.canFly && c.height >= 1 && c.height <= 5 &&
           c.acceleration <= 1000000,
       "terrain ordinary Crocodile profile");
  auto projected = scene::projectMapNavigation(
      std::get<assets::MapAsset>(m), std::get<assets::TerrainCatalog>(t),
      {4, 4, 12, 12});
  auto &b = projected.frozen;
  unsigned cursor = 92;
  for (unsigned i = 0; i < 4; ++i)
    cursor += word(b, 64 + i * 4);
  unsigned object = cursor, type = object + 0xd07, generator = type + 0x198;
  const auto samples = reconstruction::groundMovementSamples(
      std::get<assets::Animation>(animation));
  put(b, object + 0xa8, c.type);
  put(b, type + 8, c.width);
  put(b, type + 12, c.height);
  put(b, type + 16, c.acceleration);
  put(b, type + 0x44, c.swimming);
  for (unsigned i = 0; i < 48; ++i)
    put(b, type + 0xd8 + i * 4, samples[i]);
  put(b, generator + 0x589, reconstruction::groundMovementMaximum(samples));
  Stored out{b, digest(map), digest(ttd), digest(cfg), digest(ani), 0, {}};
  // Deterministically choose the layer with the most supported 2x2 origins.
  std::vector<g::Point> standing;
  for (unsigned z = 1; z < projected.geometry.layers; ++z) {
    auto nav = std::make_shared<Navigation>(b, int(z));
    std::vector<g::Point> here;
    for (int y = 1; y < 10; ++y)
      for (int x = 1; x < 10; ++x)
        try {
          nav->validateSpawnPosition({x, y, int(z)});
          here.push_back({x, y, int(z)});
        } catch (const std::invalid_argument &) {
        }
    if (here.size() > standing.size()) {
      standing = std::move(here);
      out.plane = z;
    }
  }
  need(standing.size() >= 12, "terrain crop lacks a bounded encounter layer");
  auto nav = std::make_shared<Navigation>(b, int(out.plane));
  unsigned best = 0;
  for (auto p : standing)
    for (auto q : standing) {
      unsigned distance =
          unsigned(std::max(std::abs(p.x - q.x), std::abs(p.y - q.y)));
      if (distance <= best)
        continue;
      g::Entity e;
      e.type = 17;
      e.x = p.x;
      e.y = p.y;
      e.z = p.z;
      e.motion = g::CreatureMotion{};
      e.motion->origin = e.motion->destination = p;
      auto route = nav->plan(e, q, 300);
      if (route.status == g::PlanStatus::reachable) {
        best = distance;
        out.wizards = {p, q};
      }
    }
  need(best >= 5, "terrain crop lacks separated connected creature corridor");
  // Native encounter authoring: reserve a rear wizard origin within summon
  // range of the connected corridor end, leaving that end for its creature.
  const auto front = out.wizards[1], player = out.wizards[0];
  auto rear = front;
  unsigned rearDistance = best;
  int rearTravel = 1000;
  for (auto p : standing) {
    const auto dx = std::abs(p.x - front.x), dy = std::abs(p.y - front.y);
    const auto distance =
        unsigned(std::max(std::abs(p.x - player.x), std::abs(p.y - player.y)));
    if (std::max(dx, dy) > 2 || (dx < 2 && dy < 2) || distance <= best)
      continue;
    if (distance > rearDistance ||
        (distance == rearDistance && dx + dy < rearTravel)) {
      rear = p;
      rearDistance = distance;
      rearTravel = dx + dy;
    }
  }
  need(rear != front, "terrain crop lacks a rear wizard summon origin");
  out.wizards[1] = rear;
  return finish(out);
}
TerrainBattle decodeTerrainBattle(const game::Bytes &b) {
  return finish(parse(b));
}
void validateTerrainCreatureProfile(const TerrainBattle &terrain,
                                    const game::Bytes &profile) {
  need(profile.size() >= 20 && profile.size() <= 2 * 1024 * 1024 &&
           std::equal(profile.begin(), profile.begin() + 8, "MNMMPR01"),
       "terrain creature profile wire");
  std::size_t at = 8;
  std::array<game::Bytes, 3> inputs;
  for (auto &input : inputs) {
    const auto n = word(profile, at);
    at += 4;
    need(at <= profile.size() && n <= profile.size() - at,
         "terrain creature input extent");
    input.assign(profile.begin() + at, profile.begin() + at + n);
    at += n;
  }
  need(at == profile.size(), "terrain creature trailing bytes");
  const auto stored = parse(terrain.bytes);
  need(digest(inputs[0]) == stored.cfgHash &&
           digest(inputs[2]) == stored.aniHash,
       "terrain movement/melee CFG/ANI mismatch");
}
std::vector<bool> terrainBattleSolids(const game::Bytes &b) {
  auto s = parse(b);
  auto w = reconstruction::decode_route_world(native(s.frozen));
  std::vector<bool> out;
  out.reserve(w->cells.size());
  for (const auto &c : w->cells) {
    std::uint16_t definition = 0, flags = 0;
    std::memcpy(&definition, &c, 2);
    std::memcpy(&flags, reinterpret_cast<const char *>(&c) + 10, 2);
    out.push_back(definition != 0 || (flags & 0x4000));
  }
  return out;
}
} // namespace mnm::skirmish
