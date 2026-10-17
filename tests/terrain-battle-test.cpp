#include "persistence/snapshot.hpp"
#include "terrain_battle.hpp"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
namespace g = mnm::game;
void check(bool b, const char *m) {
  if (!b)
    throw std::runtime_error(m);
}
int main(int argc, char **argv) try {
  check(argc == 2, "usage: terrain-battle-test INSTALLATION");
  auto t = mnm::skirmish::installedTerrainBattle(argv[1]);
  auto rebound = mnm::skirmish::decodeTerrainBattle(t.bytes);
  check(rebound.bytes == t.bytes &&
            rebound.navigation->binding() == t.navigation->binding(),
        "terrain binding round trip");
  auto d = t.navigation->binding().dimensions;
  unsigned standing = 0, blocked = 0;
  std::vector<g::Point> legal;
  for (int y = 0; y < d.y; ++y)
    for (int x = 0; x < d.x; ++x) {
      try {
        t.navigation->validateSpawnPosition({x, y, int(t.plane)});
        ++standing;
        legal.push_back({x, y, int(t.plane)});
      } catch (const std::invalid_argument &) {
        ++blocked;
      }
    }
  check(standing >= 12 && blocked >= 44, "standing and sealed cells");
  g::World w(8);
  auto s = w.state();
  s.map = mnm::skirmish::terrainBattleMap;
  s.navigation = t.navigation->binding();
  w.restore(s);
  g::MovementSession movement(w, t.navigation);
  g::Entity e;
  e.type = 17;
  e.x = t.wizards[0].x;
  e.y = t.wizards[0].y;
  e.z = t.wizards[0].z;
  auto h = movement.spawn(e);
  // Real installed-profile routing and restart after each movement boundary.
  g::Point goal = t.wizards[0];
  unsigned farthest = 0;
  for (auto p : legal) {
    auto distance =
        unsigned(std::max(std::abs(p.x - e.x), std::abs(p.y - e.y)));
    if (distance > farthest &&
        t.navigation->plan(*movement.world().find(h), p, 300).status ==
            g::PlanStatus::reachable) {
      goal = p;
      farthest = distance;
    }
  }
  check(farthest >= 5, "terrain connected creature corridor");
  movement.move(h, goal);
  unsigned moved = 0;
  for (unsigned i = 0; i < 80; ++i) {
    auto before = *movement.world().find(h);
    movement.step();
    auto &after = *movement.world().find(h);
    if (before.x != after.x || before.y != after.y)
      ++moved;
    const bool arrived = after.motion->action == g::Action::arrived;
    auto snapshot = movement.world().state();
    g::World fresh(0);
    fresh.restore(snapshot);
    g::MovementSession restored(std::move(fresh), rebound.navigation);
    check(g::encodeSnapshot(restored.world().state()) ==
              g::encodeSnapshot(snapshot),
          "terrain movement checkpoint");
    movement = std::move(restored);
    if (arrived)
      break;
  }
  auto final = *movement.world().find(h);
  check(g::Point{final.x, final.y, final.z} == goal && moved >= 5,
        "installed terrain route arrival");
  // Occupancy cannot admit an overlapping second full profile.
  auto unchanged = g::encodeSnapshot(movement.world().state());
  bool refused = false;
  try {
    movement.spawn(final);
  } catch (const std::invalid_argument &) {
    refused = true;
  }
  check(refused && unchanged == g::encodeSnapshot(movement.world().state()),
        "terrain overlapping spawn rollback");
  auto bad = t.bytes;
  bad.back() ^= 1;
  refused = false;
  try {
    mnm::skirmish::decodeTerrainBattle(bad);
  } catch (const std::invalid_argument &) {
    refused = true;
  }
  check(refused, "terrain malformed binding refusal");
  // Older owned v1 layouts retain their saved wizard origins on decode.
  std::string prior(t.bytes.begin(), t.bytes.end());
  const std::string current = "\"wizards\":[[5,1,7],[8,8,7]]";
  auto at = prior.find(current);
  check(at != std::string::npos, "current rear-wizard binding differs");
  prior.replace(at, current.size(), "\"wizards\":[[5,1,7],[8,6,7]]");
  g::Bytes historical(prior.begin(), prior.end());
  auto old = mnm::skirmish::decodeTerrainBattle(historical);
  check(old.bytes == historical && old.wizards[1] == g::Point{8, 6, 7} &&
            old.navigation->binding() == t.navigation->binding(),
        "saved terrain wizard origins were regenerated");
  auto solids = mnm::skirmish::terrainBattleSolids(t.bytes);
  check(solids.size() == std::size_t(d.x * d.y * d.z), "terrain solid volume");
  std::cout << "{\"success\":true,\"plane\":" << t.plane
            << ",\"layers\":" << d.z << ",\"standing\":" << standing
            << ",\"blocked\":" << blocked << ",\"movement_steps\":" << moved
            << ",\"width\":" << t.navigation->creatureFootprint().x
            << ",\"height\":" << t.navigation->creatureFootprint().z
            << ",\"wizard0\":[" << t.wizards[0].x << ',' << t.wizards[0].y
            << ',' << t.wizards[0].z << "],\"wizard1\":[" << t.wizards[1].x
            << ',' << t.wizards[1].y << ',' << t.wizards[1].z << "]}\n";
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
