#include "../../game/persistence/snapshot.hpp"
#include "session.hpp"
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <fstream>
#include <iostream>
int main(int argc, char **argv) try {
  QGuiApplication app(argc, argv);
  if (argc != 5) {
    std::cerr << "Usage: native-scene-session ASSET_ROOT "
                 "MANIFEST_PATH WORLD_CHECKPOINT OUTPUT.png\n";
    return 2;
  }
  using namespace mnm;
  auto created = assets::AssetStore::create(argv[1]);
  if (auto *e = std::get_if<assets::Error>(&created))
    throw std::runtime_error(e->detail);
  auto store = std::get<assets::AssetStore>(std::move(created));
  std::ifstream input(argv[2], std::ios::binary | std::ios::ate);
  if (!input || input.tellg() < 0 || input.tellg() > 1024 * 1024)
    throw std::runtime_error(
        "Scene manifest input exceeds 1 MiB or cannot be read");
  assets::Bytes bytes(static_cast<std::size_t>(input.tellg()));
  input.seekg(0);
  input.read(reinterpret_cast<char *>(bytes.data()),
             std::streamsize(bytes.size()));
  if (!input)
    throw std::runtime_error("Scene manifest read failed");
  auto manifest = scene::decodeSceneManifest(bytes, store);
  const auto state = game::readSnapshot(argv[3]);
  const auto source = scene::worldSceneSnapshot(
      state, game::decodeCombat(state.systems), manifest.types, 1, 1);
  render::GlBlitter gl;
  scene::SceneSession session(gl, std::move(store), std::move(manifest));
  auto plan = session.prepare(source);
  session.render(plan);
  auto image = session.present();
  if (!image.save(QString::fromLocal8Bit(argv[4])))
    throw std::runtime_error("Could not save scene image");
  std::cout << QJsonDocument(
                   QJsonObject{{"success", true},
                               {"entities", int(source.entities.size())},
                               {"draws", int(plan.queue.size())},
                               {"width", image.width()},
                               {"height", image.height()}})
                   .toJson(QJsonDocument::Compact)
                   .constData()
            << '\n';
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
