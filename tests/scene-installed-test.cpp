#include "../game/persistence/snapshot.hpp"
#include "resource-fixtures.hpp"
#include "session.hpp"
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
using namespace mnm;
using namespace resource_test;
int main(int argc, char **argv) try {
  QGuiApplication app(argc, argv);
  if (argc != 3)
    throw std::invalid_argument(
        "Installed scene test ASSET_ROOT OUTPUT_DIRECTORY");
  auto root = std::filesystem::path(argv[1]),
       out = std::filesystem::path(argv[2]);
  require(std::filesystem::is_directory(out), "Installed output directory");
  const std::string json =
      R"({"version":1,"map":"Realms/Celtic/Forest/CFsec01.map","terrainAnimation":"Realms/Celtic/Forest/terrain.ani","mps":"Realms/Celtic/Forest/CFsec01.mps","terrain":{"kind":"terrain","name":"forest"},"assets":[{"id":{"kind":"terrain","name":"forest"},"image":"Realms/Celtic/Forest/Terrain.spr","terrainCatalog":"Realms/Celtic/Forest/Terrain.ttd"},{"id":{"kind":"creature","name":"crocodile"},"image":"Creatures/Crocodile.spr","animation":"Creatures/crocodile.ani"}],"viewport":[512,256],"lighting":{"enabled":false,"effectConfig":"CFG/Encrypted/effects.cfg","terrainConfig":"CFG/Encrypted/chaos.cfg"},"palette":{"modes":[0],"globalConfig":"CFG/Encrypted/chaos.cfg","preferences":"CFG/prefs.cfg"},"types":[{"family":"creature","type":2,"layers":[{"channel":0,"resource":{"kind":"creature","name":"crocodile"},"sequence":288}]}]})";
  write(out, "scene.json", Bytes(json.begin(), json.end()));
  auto manifest =
      scene::decodeSceneManifest(Bytes(json.begin(), json.end()), store(root));
  render::GlBlitter gl;
  scene::SceneSession session(gl, store(root), manifest);
  const auto &map = session.map().map();
  game::World world(4);
  game::Entity creature;
  creature.type = 2;
  creature.x = int(map.width / 2);
  creature.y = int(map.height / 2);
  auto h = world.spawn(creature);
  auto state = world.state();
  state.map = "installed-forest-section";
  state.navigation = game::NavigationBinding{
      {int(map.width), int(map.height), int(map.layers)}, 1};
  game::CombatState combat;
  game::Fighter fighter;
  fighter.actor = h;
  fighter.health = 10;
  combat.fighters = {fighter};
  state.systems = game::encodeCombat(combat);
  game::writeSnapshot(out / "world.mnw", state);
  auto source = scene::worldSceneSnapshot(state, combat, manifest.types, 1, 1);
  auto plan = session.prepare(source);
  if (plan.queue.size() <= 1)
    std::cerr << "Installed debug dimensions " << map.width << "," << map.height
              << "," << map.layers << " camera cut "
              << session.camera().cutLevel << " draws " << plan.queue.size()
              << "\n";
  require(plan.queue.size() > 1,
          "Installed scene did not acquire terrain and body draws");
  session.render(plan);
  auto image = session.present();
  require(image.save(QString::fromStdString((out / "installed.png").string())),
          "Installed output image");
  bool nonempty = false;
  for (auto p : session.read().pixels)
    nonempty |= p != 0x2124;
  require(nonempty, "Installed scene is blank");
  require(session.resources().stats().residentResources == 2,
          "Installed resource acquisition");
  auto placements = session.map().placements().placements.size();
  std::cout << QJsonDocument(
                   QJsonObject{{"success", true},
                               {"draws", int(plan.queue.size())},
                               {"placements_retained", int(placements)},
                               {"map_width", int(map.width)},
                               {"map_height", int(map.height)},
                               {"map_layers", int(map.layers)},
                               {"installed_resources_and_profiles", true},
                               {"checkpoint_for_cli", true}})
                   .toJson(QJsonDocument::Compact)
                   .constData()
            << '\n';
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
