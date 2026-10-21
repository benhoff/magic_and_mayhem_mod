#include "resource-fixtures.hpp"
#include "session.hpp"
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
#include <thread>
using namespace mnm;
using namespace resource_test;
static Bytes ani() {
  constexpr unsigned count = 16, base = 44 + (count + 1) * 4;
  Bytes b(base + count * 3 * 44);
  word(b, 0, 0x494e41);
  word(b, 4, b.size());
  word(b, 8, count * 3);
  word(b, 12, 5);
  word(b, 20, count + 1);
  for (unsigned i = 0; i <= count; ++i)
    word(b, 44 + 4 * i, i * 3);
  for (unsigned i = 0; i < count; ++i) {
    auto at = base + i * 3 * 44;
    word(b, at, 0);
    word(b, at + 4, (i / 2) % 2);
    word(b, at + 44, 0);
    word(b, at + 48, 1 - (i / 2) % 2);
    word(b, at + 88, 6);
  }
  return b;
}
static assets::MapAsset grid() {
  assets::MapAsset m;
  m.width = m.height = 32;
  m.layers = 3;
  m.cells.resize(3072);
  for (auto &c : m.cells) {
    c.definition = 0;
    c.references = {0xffff, 0xffff, 0xffff};
  }
  return m;
}
static render::Image reference(const scene::SceneSession &session,
                               const scene::SceneFramePlan &plan, int width,
                               int height) {
  render::Image result{
      width, height,
      std::vector<std::uint32_t>(std::size_t(width) * height, 0x2124)};
  for (const auto &e : plan.queue) {
    const auto &d = e.draw;
    if (!d.visible)
      continue;
    const auto &sprite = std::get<assets::Sprite>(
        session.resources().resident(d.resource).image);
    auto frame = render::prepareSpriteFrame(sprite, d.frame,
                                            d.colours ? &*d.colours : nullptr);
    for (int y = 0; y < frame.pixels.height; ++y)
      for (int x = 0; x < frame.pixels.width; ++x) {
        auto at = std::size_t(y) * frame.pixels.width + x;
        const auto dx = d.anchorX - frame.originX + x,
                   dy = d.anchorY - frame.originY + y;
        if (frame.mask.pixels[at] && dx >= 0 && dy >= 0 && dx < width &&
            dy < height)
          result.pixels[std::size_t(dy) * width + dx] = frame.pixels.pixels[at];
      }
  }
  return result;
}
int main(int argc, char **argv) try {
  QGuiApplication app(argc, argv);
  QTemporaryDir directory;
  require(directory.isValid(), "Full scene fixture root");
  auto root = std::filesystem::path(directory.path().toStdString());
  write(root, "body.spr", sprite());
  write(root, "indexed.spr", sprite(true));
  write(root, "body.ani", ani());
  auto t = terrain();
  for (unsigned view = 0; view < 4; ++view)
    word(t, 16 + 0x84 + 4 * view, 1);
  t.resize(16 + 2 * 356);
  word(t, 4, t.size());
  word(t, 12, 2);
  for (unsigned view = 0; view < 4; ++view)
    word(t, 16 + 356 + 0x84 + 4 * view, 1);
  write(root, "terrain.ttd", t);
  const std::string json =
      R"({"version":1,"terrain":{"kind":"terrain","name":"tiles"},"assets":[{"id":{"kind":"terrain","name":"tiles"},"image":"indexed.spr","terrainCatalog":"terrain.ttd"},{"id":{"kind":"creature","name":"body"},"image":"body.spr","animation":"body.ani"},{"id":{"kind":"effect","name":"overlay"},"image":"indexed.spr","animation":"body.ani"}],"viewport":[256,192],"camera":{"autoFocus":false},"palette":{"modes":[0,7],"ordinal":0},"types":[{"family":"creature","type":0,"directional":true,"layers":[{"channel":0,"resource":{"kind":"creature","name":"body"},"sequence":0}]}]})";
  auto manifest =
      scene::decodeSceneManifest(Bytes(json.begin(), json.end()), store(root));
  require(manifest.types.size() == 1 && manifest.assets.size() == 3,
          "Manifest semantic recipes and profiles");
  for (const auto &bad :
       {std::string("{}"), std::string("{\"version\":1,\"unknown\":true}")})
    rejected([&] {
      scene::decodeSceneManifest(Bytes(bad.begin(), bad.end()), store(root));
    });
  auto changed =
      QJsonDocument::fromJson(QByteArray::fromStdString(json)).object();
  changed["viewport"] = QJsonArray{1.5, 192};
  auto bad = QJsonDocument(changed).toJson();
  rejected([&] {
    scene::decodeSceneManifest(Bytes(bad.begin(), bad.end()), store(root));
  });
  manifest.camera.span = 4;
  manifest.camera.diagonal = 2;
  manifest.camera.column = manifest.camera.row = 10;
  manifest.camera.f41 = 0;
  manifest.camera.cutLevel = 2;
  manifest.paletteModes = {0};
  manifest.paletteOrdinal = 0;
  manifest.useLighting = true;
  manifest.lighting = {-10, 7};
  render::GlBlitter gl;
  scene::SceneSession session(gl, store(root), manifest, {}, grid());
  scene::SceneSourceSnapshot source;
  source.epoch = 1;
  source.revision = 1;
  source.tick = 40;
  source.dimensions = {32, 32, 3};
  scene::SceneEntitySource entity;
  entity.identity = {game::Family::creature, 0, 4};
  entity.health = 20;
  entity.cell = {10, 10, 0};
  entity.fine = {320, 320, 0};
  scene::SceneLayerSource body;
  body.animation.resource = {assets::ResourceKind::creature, "body"};
  body.animation.continuation = reconstruction::AnimationControllerState{};
  body.animation.continuation->forward = {1, 0, true, 0, 0, 0, 0};
  scene::SceneLayerSource overlay;
  overlay.channel = 4;
  overlay.animation.resource = {assets::ResourceKind::effect, "overlay"};
  overlay.animation.sequence = 2;
  overlay.attachment = reconstruction::AttachmentPoint::first;
  overlay.priority = 7;
  entity.layers = {body, overlay};
  scene::SceneCreatureView view;
  view.identity = entity.identity.token();
  view.active = true;
  view.position = {10, 10, 0};
  entity.creatureView = view;
  source.entities = {entity};
  auto plan = session.prepare(source);
  require(!plan.queue.empty(), "Full scene ordered queue");
  const auto initial = session.animationSnapshot(entity.identity, 0);
  const auto lightTicks = session.lighting().cycle().ticks();
  session.render(plan, {1, 8192, std::chrono::microseconds(1000)});
  require(session.read().pixels == reference(session, plan, 256, 192).pixels,
          "Scene software GPU output differs from independent CPU composition");
  auto gpu = session.presentGpu();
  require(gpu.valid(), "Native GPU presentation");
  unsigned frames = 1;
  for (unsigned v = 1; v < 4; ++v) {
    auto camera = session.camera();
    camera.view = v;
    session.setCamera(camera);
    auto rotated = session.recompose();
    require(session.lighting().cycle().ticks() == lightTicks &&
                session.animationSnapshot(entity.identity, 0)
                        .state.forward.elapsed == initial.state.forward.elapsed,
            "Camera redraw advanced authoritative clock");
    session.render(rotated);
    require(session.read().pixels ==
                reference(session, rotated, 256, 192).pixels,
            "Camera scene GPU/CPU mismatch");
    ++frames;
  }
  auto camera = session.camera();
  camera.view = 0;
  session.setCamera(camera);
  source.revision++;
  source.tick++;
  auto advanced = session.prepare(source, {{entity.identity}});
  require(session.animationSnapshot(entity.identity, 0).state.forward.elapsed ==
                  0 &&
              session.animationSnapshot(entity.identity, 4).state.forward.pc ==
                  1,
          "Owned overlay tick modified source body or failed to advance");
  session.render(advanced);
  require(session.read().pixels ==
              reference(session, advanced, 256, 192).pixels,
          "Advanced overlay scene GPU/CPU mismatch");
  ++frames;
  auto same = source;
  same.revision++;
  auto frozen = session.prepare(same);
  require(session.lighting().cycle().ticks() == lightTicks + 1,
          "Same-tick refresh advanced lighting clock");
  auto pixels = session.read().pixels;
  auto invalid = same;
  invalid.revision++;
  invalid.entities[0].layers[0].animation.continuation->forward.pc = 999;
  rejected([&] { session.prepare(invalid); });
  session.render(frozen);
  require(session.read().pixels == pixels,
          "Rejected source changed retained animation or canvas");
  rejected([&] { session.render(plan); });
  auto gap = same;
  gap.revision++;
  gap.tick += 2;
  rejected([&] { session.prepare(gap); });
  auto doubled = same;
  doubled.revision++;
  doubled.tick++;
  rejected(
      [&] { session.prepare(doubled, {{entity.identity, entity.identity}}); });
  auto external = doubled;
  external.entities[0].layers.pop_back();
  rejected([&] { session.prepare(external, {{entity.identity}}); });
  bool wrongThread = false;
  std::thread foreign([&] {
    try {
      session.recompose();
    } catch (const std::exception &) {
      wrongThread = true;
    }
  });
  foreign.join();
  require(wrongThread, "Scene owner thread");
  scene::SceneSession other(gl, store(root), manifest, {}, grid());
  auto otherPlan = other.prepare(source);
  rejected([&] { other.render(frozen); });
  other.render(otherPlan);
  session.retire();
  rejected([&] { session.render(frozen); });
  rejected([&] { session.recompose(); });
  source.epoch++;
  source.revision = 1;
  auto restarted = session.prepare(source);
  session.render(restarted);
  require(session.read().pixels ==
              reference(session, restarted, 256, 192).pixels,
          "Scene retirement and new epoch acquisition");
  ++frames;
  auto linkedManifest = manifest;
  linkedManifest.terrainAnimation = "body.ani";
  auto linkedMap = grid();
  const std::size_t linkedAt = 10 * 32 + 10;
  linkedMap.cells[linkedAt].flags10 = 8;
  scene::SceneSession linkedSession(gl, store(root), linkedManifest, {},
                                    linkedMap,
                                    scene::SceneMapInputKind::assetSequences);
  require(linkedSession.map().linked().size() == 1,
          "Dedicated terrain ANI acquisition");
  auto empty = source;
  empty.entities.clear();
  empty.epoch = 9;
  empty.revision = 1;
  empty.tick = 0;
  linkedSession.prepare(empty);
  empty.revision++;
  empty.tick++;
  linkedSession.prepare(empty);
  empty.revision++;
  empty.tick++;
  linkedSession.prepare(empty);
  require(linkedSession.map().map().cells[linkedAt].definition == 1,
          "Actual linked terrain clock did not select TTD frame");
  auto restored = empty;
  restored.epoch++;
  restored.revision = 1;
  linkedSession.prepare(restored);
  require(linkedSession.map().map().cells[linkedAt].definition == 0,
          "New source epoch retained old linked terrain phase");
  auto spectatorManifest = manifest;
  spectatorManifest.lightPlayer = -1;
  spectatorManifest.autoFocus = true;
  scene::SceneSession spectator(gl, store(root), spectatorManifest, {}, grid());
  auto followed = source;
  followed.entities[0].cell = {12, 13, 0};
  followed.entities[0].fine = {384, 416, 0};
  spectator.prepare(followed);
  require(spectator.lighting().snapshot().spectator->column == 12 &&
              spectator.lighting().snapshot().spectator->row == 13,
          "Initial spectator lighting used the old camera before autofocus");
  const auto driver = gl.driver();
  std::cout << QJsonDocument(
                   QJsonObject{
                       {"success", true},
                       {"gpu_frames", int(frames)},
                       {"source_clock_and_rollback", true},
                       {"owner_generation_and_epoch_controls", true},
                       {"manifest_resource_and_palette_loading", true},
                       {"driver", QString::fromStdString(driver.renderer)}})
                   .toJson(QJsonDocument::Compact)
                   .constData()
            << '\n';
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
