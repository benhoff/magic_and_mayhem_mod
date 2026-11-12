#include "resource-fixtures.hpp"
#include "session.hpp"
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <algorithm>
#include <iostream>
#include <thread>
using namespace mnm;
using namespace resource_test;
static Bytes ani() {
  constexpr unsigned count = 32, base = 44 + (count + 1) * 4;
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
  // A queued turn belongs to the completed frame, never to prepare/redraw.
  auto scheduledManifest = manifest;
  scheduledManifest.lightPlayer = -1;
  const assets::ResourceId specialId{assets::ResourceKind::effect, "special"};
  auto specialRecipe = manifest.assets[2];
  specialRecipe.id = specialId;
  scheduledManifest.assets.push_back(specialRecipe);
  scene::SceneSession scheduled(gl, store(root), scheduledManifest, {}, grid());
  auto live = source;
  live.epoch = 30;
  live.revision = 1;
  live.tick = 100;
  live.entities[0].layers[0].animation.continuation.reset();
  live.entities[0].creatureView->state.direction = 3;
  live.entities[0].creatureView->state.animationKind = 3;
  live.entities[0].creatureView->special = reconstruction::CreatureSpecialView{16, 0};
  auto specialLayer = overlay;
  specialLayer.channel = 1;
  specialLayer.animation.resource = body.animation.resource;
  specialLayer.animation.sequence = 16;
  live.entities[0].layers.push_back(specialLayer);
  live.specialCatalog = {specialId};
  const auto actorId = live.entities[0].identity;
  auto missingSpecial = live;
  missingSpecial.entities[0].layers.pop_back();
  rejected([&] { scheduled.prepare(missingSpecial); });
  auto aliasedGeneration = live;
  auto duplicateSlot = live.entities[0];
  duplicateSlot.identity.generation++;
  duplicateSlot.creatureView->identity = duplicateSlot.identity.token();
  aliasedGeneration.entities.push_back(duplicateSlot);
  rejected([&] { scheduled.prepare(aliasedGeneration); });
  auto queued = scheduled.prepare(live);
  const auto scheduledLightTicks = scheduled.lighting().cycle().ticks();
  scheduled.queueCameraTurn(reconstruction::CameraTurn::clockwise);
  scheduled.queueCameraTurn(reconstruction::CameraTurn::counterclockwise);
  rejected([&] { scheduled.queueCameraTurn(reconstruction::CameraTurn(42)); });
  require(scheduled.camera().view == 0, "Queued turn ran before a frame");
  rejected([&] { scheduled.render(queued, {0, 8192, std::chrono::microseconds(1000)}); });
  require(scheduled.camera().view == 0 && scheduled.cameraLifecycle().clockwise &&
              scheduled.cameraLifecycle().counterclockwise,
          "Failed submission consumed a queued turn");
  scheduled.render(queued);
  require(scheduled.camera().view == 1 && !scheduled.cameraLifecycle().clockwise &&
              scheduled.cameraLifecycle().counterclockwise &&
              scheduled.cameraLifecycle().redrawFrames == 3 &&
              scheduled.minimap().view == 1 && scheduled.minimap().dirty,
          "Successful frame did not apply clockwise priority/minimap delivery");
  const auto selectedSpecial = scheduled.animationSnapshot(actorId, 1);
  require(selectedSpecial.resource == specialId && selectedSpecial.sequence == 17,
          "Queued rotation failed to acquire/bind the special catalog resource");
  // Resubmitting the same frozen plan does not consume the other request.
  scheduled.render(queued);
  require(scheduled.camera().view == 1 && scheduled.cameraLifecycle().counterclockwise &&
              scheduled.cameraLifecycle().redrawFrames == 3,
          "Repeated plan completed the same frame twice");
  auto nextFrame = scheduled.recompose();
  bool actualSpecial = false;
  for (const auto &entry : nextFrame.queue)
    actualSpecial |= entry.actor == actorId && entry.draw.resource == specialId;
  require(actualSpecial && nextFrame.resources.count(specialId),
          "Scene draw/plan lifetime used the source recipe after catalog rebind");
  scheduled.render(nextFrame);
  require(scheduled.read().pixels == reference(scheduled, nextFrame, 256, 192).pixels,
          "Rebound special resource differs in actual GPU pixels");
  ++frames;
  require(scheduled.camera().view == 0 && !scheduled.cameraLifecycle().counterclockwise,
          "Counterclockwise request did not wait for the next completed frame");
  auto settled = scheduled.recompose();
  scheduled.render(settled);
  require(scheduled.cameraLifecycle().redrawFrames == 2 &&
              scheduled.lighting().cycle().ticks() == scheduledLightTicks,
          "Frame redraw scheduling added an animation/light source clock");
  // Frozen redraws preserve special animation phase, rather than replaying a
  // source-view mismatch and restarting it on each composition.
  live.revision++;
  live.tick++;
  auto ticking = scheduled.prepare(live, {{actorId}});
  const auto phase = scheduled.animationSnapshot(actorId, 1).state.forward;
  scheduled.recompose();
  require(scheduled.animationSnapshot(actorId, 1).state.forward.pc == phase.pc &&
              scheduled.animationSnapshot(actorId, 1).state.forward.elapsed == phase.elapsed,
          "Frozen recomposition restarted an owned special controller");
  rejected([&] { scheduled.render(ticking); });
  auto movedCamera = scheduled.camera();
  movedCamera.column = 14;
  movedCamera.row = 15;
  scheduled.setCamera(movedCamera);
  auto redrawn = scheduled.recompose();
  require(scheduled.lighting().cycle().ticks() == scheduledLightTicks + 1,
          "Spectator camera redraw advanced the light cycle");
  live.revision++;
  live.tick++;
  auto relocatedLight = scheduled.prepare(live);
  require(scheduled.lighting().snapshot().spectator->column == 14 &&
              scheduled.lighting().snapshot().spectator->row == 15,
          "Admitted source tick used stale spectator camera data");
  rejected([&] { scheduled.render(redrawn); });
  scheduled.render(relocatedLight);
  // Linked effect rotations use the refreshed creator direction and current
  // generation. A stale reference is rejected before drawing/consumption.
  live.entities[0].creatureView->state.animationKind = 2;
  scene::SceneEntitySource linkedEffect;
  linkedEffect.identity = {game::Family::missileEffect, 0, 9};
  linkedEffect.type = 52;
  linkedEffect.cell = {10, 10, 0};
  linkedEffect.fine = {320, 320, 0};
  auto linkedLayer = body;
  linkedLayer.channel = 3;
  linkedLayer.animation.continuation.reset();
  linkedLayer.animation.sequence = 24;
  linkedEffect.layers = {linkedLayer};
  scene::SceneEffectView linkedView;
  linkedView.identity = linkedEffect.identity.token();
  linkedView.active = true;
  linkedView.state.type = 52;
  linkedView.target = scene::SceneActorReference{0, actorId.token()};
  linkedEffect.effectView = linkedView;
  live.entities[0].creatureView->linkedEffect =
      scene::SceneActorReference{0, linkedEffect.identity.token()};
  live.entities.push_back(linkedEffect);
  live.effectViews.sequenceBases.resize(89);
  live.effectViews.sequenceBases[45] = 24;
  live.revision++;
  live.tick++;
  auto linkedFrame = scheduled.prepare(live);
  scheduled.queueCameraTurn(reconstruction::CameraTurn::clockwise);
  scheduled.render(linkedFrame);
  require(scheduled.animationSnapshot(linkedEffect.identity, 3).sequence == 25,
          "Linked rotation did not follow current creature view direction");
  live.entities[0].creatureView->linkedEffect->identity++;
  live.revision++;
  live.sourceView = scheduled.camera().view;
  auto staleLinked = scheduled.prepare(live);
  scheduled.queueCameraTurn(reconstruction::CameraTurn::clockwise);
  const auto savedView = scheduled.camera().view;
  rejected([&] { scheduled.render(staleLinked); });
  require(scheduled.camera().view == savedView && scheduled.cameraLifecycle().clockwise,
          "Stale linked producer partially consumed the queued frame");
  live.entities.erase(live.entities.begin() + 1);
  live.entities[0].creatureView->linkedEffect.reset();
  live.revision++;
  auto retiredEffect = scheduled.prepare(live);
  rejected([&] { scheduled.animationSnapshot(linkedEffect.identity, 3); });
  scheduled.render(retiredEffect);
  scheduled.retire();
  require(!scheduled.cameraLifecycle().clockwise && !scheduled.cameraLifecycle().counterclockwise,
          "Retirement preserved queued camera work");
  rejected([&] { scheduled.render(retiredEffect); });
  live.epoch++;
  live.revision = 1;
  live.entities[0].identity.generation++;
  live.entities[0].creatureView->identity = live.entities[0].identity.token();
  scheduled.prepare(live);
  rejected([&] { scheduled.animationSnapshot(actorId, 1); });
  // Exercise the public source producer against a changing native World,
  // rather than only feeding authored SceneSourceSnapshot values.
  game::World currentWorld(4);
  game::Entity currentCreature;
  currentCreature.x = currentCreature.y = 10;
  const auto currentHandle = currentWorld.spawn(currentCreature);
  auto currentState = currentWorld.state();
  currentState.navigation = game::NavigationBinding{{32, 32, 3}, 1};
  game::CombatState currentHealth;
  game::Fighter currentFighter;
  currentFighter.actor = currentHandle;
  currentFighter.health = 30;
  currentHealth.fighters.push_back(currentFighter);
  scene::SceneSession currentSession(gl, store(root), scheduledManifest, {}, grid());
  auto currentPlan = currentSession.prepare(
      scene::worldSceneSnapshot(currentState, currentHealth, manifest.types, 50, 1));
  currentSession.queueCameraTurn(reconstruction::CameraTurn::clockwise);
  currentSession.render(currentPlan);
  require(currentSession.animationSnapshot(
              {game::Family::creature, currentHandle.slot, currentHandle.generation}, 0)
                  .sequence == 6,
          "World producer direction did not reach the actual rotation consumer");
  currentState.tick++;
  currentState.slots[currentHandle.slot].entity->x = 12;
  currentHealth.fighters[0].health = 0;
  auto currentUpdate = currentSession.prepare(
      scene::worldSceneSnapshot(currentState, currentHealth, manifest.types, 50, 2));
  const auto &currentLights = currentSession.lighting().snapshot().creatures;
  require(std::count_if(currentLights.begin(), currentLights.end(),
                        [](const auto &light) { return light.active; }) == 1 &&
              currentLights[0].column == 12 &&
              !currentLights[0].lightEnabled && currentState.tick == 1,
          "Current World health/cell did not reach the light producer unchanged");
  currentSession.render(currentUpdate);
  require(currentSession.read().pixels == reference(currentSession, currentUpdate, 256, 192).pixels,
          "Current World producer frame differs from CPU composition");
  ++frames;
  const auto driver = gl.driver();
  std::cout << QJsonDocument(
                   QJsonObject{
                       {"success", true},
                       {"gpu_frames", int(frames)},
                       {"source_clock_and_rollback", true},
                       {"queued_frame_priority_and_idempotence", true},
                       {"linked_generation_retirement", true},
                       {"actual_catalog_resource_delivery", true},
                       {"current_world_producer_delivery", true},
                       {"spectator_live_camera_source_clock", true},
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
