#include "../apps/command-session/runtime.hpp"
#include "native_source.hpp"
#include "resource-fixtures.hpp"
#include "source_snapshot.hpp"
#include <iostream>
using namespace mnm;
using namespace resource_test;
int main() try {
  game::World world(4);
  game::Entity actor;
  actor.type = 3;
  actor.x = 5;
  actor.y = 7;
  actor.z = 1;
  actor.motion = game::CreatureMotion{};
  auto &motion = *actor.motion;
  motion.fine = game::FineMotion{};
  motion.fine->fine = {167, 225, 19};
  motion.fine->animation = game::AnimationCursor{9, 2, 1, true, 4, 3, 2, 1};
  auto motionCopy = actor.motion;
  actor.motion.reset();
  auto h = world.spawn(actor);
  auto state = world.state();
  state.slots[h.slot].entity->motion = motionCopy;
  state.tick = 42;
  state.navigation = game::NavigationBinding{{32, 32, 3}, 123};
  game::CombatState combat;
  game::Fighter fighter;
  fighter.actor = h;
  fighter.health = 7;
  combat.fighters.push_back(fighter);
  scene::SceneTypeProfile profile;
  profile.type = 3;
  profile.directional = true;
  scene::SceneLayerSource body;
  body.animation.resource = {assets::ResourceKind::creature, "body"};
  profile.layers = {body};
  auto snapshot = scene::worldSceneSnapshot(state, combat, {profile}, 9, 10);
  const auto &source = snapshot.entities.at(0);
  require(source.identity.generation == h.generation && source.health == 7 &&
              source.fine == game::Point{167, 225, 19} &&
              source.layers[0].animation.continuation->forward.elapsed == 3 &&
              source.layers[0].animation.pose->displayed == 1 &&
              source.creatureView->state.mainSequenceBase == 8,
          "World actual cursor, pose, health and fine coordinates");
  require(state.slots[h.slot].entity->motion->fine->animation->elapsed == 3,
          "Source adapter advanced source clock");
  auto stale = combat;
  stale.fighters[0].actor.generation++;
  rejected([&] { scene::worldSceneSnapshot(state, stale, {profile}, 1, 1); });
  rejected([&] {
    scene::worldSceneSnapshot(state, game::CombatState{}, {profile}, 1, 1);
  });
  rejected([&] { scene::worldSceneSnapshot(state, combat, {}, 1, 1); });
  auto owned = state;
  owned.slots[h.slot].entity->motion->fine.reset();
  owned.slots[h.slot].entity->motion->terrainMotion = true;
  rejected([&] { scene::worldSceneSnapshot(owned, combat, {profile}, 1, 1); });
  auto explicitPosition = scene::worldSceneSnapshot(
      owned, combat, {profile}, 1, 1,
      [](const auto &) { return game::Point{44, 55, 66}; });
  require(explicitPosition.entities[0].fine == game::Point{44, 55, 66},
          "Identified fine provider");
  game::CreatureEncounter encounter;
  game::CreatureActor typed;
  typed.handle = h;
  typed.health = 11;
  encounter.actors.push_back(typed);
  require(scene::worldSceneSnapshot(state, encounter, {profile}, 1, 1)
                  .entities[0]
                  .health == 11,
          "Encounter typed living health");
  command_session::Continuation native;
  native.native.present = true;
  native.native.profileStamp = {1, 2};
  native.world.dimensions = {32, 32, 3};
  native.world.creatures.resize(1);
  native.generations = {4};
  native.execution.resize(1);
  native.actions.motion.resize(1);
  native.native.animation.resize(1);
  native.native.cells.resize(32 * 32 * 3);
  auto &creature = native.world.creatures[0];
  creature.ai.slot = 0;
  creature.ai.type = 3;
  creature.ai.active = true;
  creature.ai.health = 19;
  creature.ai.position = {5, 7, 1};
  creature.ai.visible.fill(1);
  creature.finePosition = {168, 225, 19};
  creature.width = 2;
  native.native.animation[0].body.bound = true;
  native.native.animation[0].body.sequence = 9;
  native.native.animation[0].body.state.forward = {2, 1, true, 4, 3, 2, 1};
  scene::NativeSceneSourceBindings binding;
  binding.profileStamp = {1, 2};
  binding.animations = {body.animation.resource};
  binding.fallbackBases.fill(288);
  auto projected = scene::nativeSceneSnapshot(native, binding, 2, 3);
  require(projected.terrain->cells.size() == 3072 &&
              projected.entities[0].health == 19 &&
              projected.entities[0].fine == game::Point{168, 225, 19} &&
              projected.entities[0]
                      .layers[0]
                      .animation.continuation->forward.elapsed == 3,
          "Native provider current controller and geometry projection");
  binding.profileStamp[0]++;
  rejected([&] { scene::nativeSceneSnapshot(native, binding, 2, 3); });
  binding.profileStamp[0]--;
  creature.animationKind = 3;
  rejected([&] { scene::nativeSceneSnapshot(native, binding, 2, 3); });
  binding.specialCatalog = {body.animation.resource};
  binding.special[3] = reconstruction::CreatureSpecialView{8, 0};
  auto special = scene::nativeSceneSnapshot(native, binding, 2, 3);
  require(special.entities[0].layers.back().channel == 1,
          "Explicit special producer role");
  creature.animationKind = 0;
  native.execution[0].block855 = 1;
  rejected([&] { scene::nativeSceneSnapshot(native, binding, 2, 3); });
  binding.secondaryAuxiliary[3] = 2;
  native.native.animation[0].auxiliary[2] = native.native.animation[0].body;
  require(scene::nativeSceneSnapshot(native, binding, 2, 3)
                  .entities[0]
                  .layers.back()
                  .channel == 2,
          "Explicit secondary producer role");
  native.execution[0].block855 = 0;
  native.native.wizards = command_session::NativeWizardState{};
  auto &effects = native.native.wizards->effects;
  effects.resize(1);
  effects[0].active = true;
  effects[0].generation = 5;
  effects[0].placement.type = 2;
  effects[0].placement.position = {4, 5, 1};
  effects[0].placement.units = {128, 160, 16};
  effects[0].placement.initialized = true;
  effects[0].placement.sentinels[6] = 3;
  effects[0].placement.sentinels[7] = 6;
  auto withEffect = scene::nativeSceneSnapshot(native, binding, 2, 3);
  require(withEffect.entities.back().layers[0].channel == 3 &&
              withEffect.entities.back().fine == game::Point{128, 160, 16} &&
              withEffect.entities.back().lightAdmission ==
                  std::optional<game::Point>{{3, 6, 1}},
          "Effect draw and separate admission producer");
  effects[0].placement.sentinels[7] = 99;
  rejected([&] { scene::nativeSceneSnapshot(native, binding, 2, 3); });
  effects[0].placement.sentinels[7] = 6;
  effects[0].placement.type = 20;
  rejected([&] { scene::nativeSceneSnapshot(native, binding, 2, 3); });
  scene::SceneIdentity effectId{game::Family::missileEffect, 0, 5};
  scene::SceneEffectView camera;
  camera.identity = effectId.token();
  camera.active = true;
  camera.state.type = 20;
  camera.state.orientationIndex = 12;
  binding.effectCamera[effectId] = camera;
  require(scene::nativeSceneSnapshot(native, binding, 2, 3)
                  .entities.back()
                  .effectView->state.orientationIndex == 12,
          "Identified effect camera producer");
  binding.effectCamera[effectId].identity++;
  rejected([&] { scene::nativeSceneSnapshot(native, binding, 2, 3); });
  binding.effectCamera.clear();
  effects.clear();
  creature.animationKind = 2;
  rejected([&] { scene::nativeSceneSnapshot(native, binding, 2, 3); });
  binding.linkedEffects[{game::Family::creature, 0, 4}] = std::nullopt;
  require(scene::nativeSceneSnapshot(native, binding, 2, 3)
              .entities[0]
              .creatureView.has_value(),
          "Identified absence of linked effect");
  creature.animationKind = 0;
  native.actions.objects.objects.resize(1);
  native.actions.objects.heads.resize(3072, 0xffff);
  native.actions.objects.heads[5] = 0;
  native.actions.objects.objects[0].kind = 8;
  native.actions.objects.objects[0].next = 0xffff;
  rejected([&] { scene::nativeSceneSnapshot(native, binding, 2, 3); });
  binding.objectGenerations = {7};
  profile.family = game::Family::mapLinked;
  profile.type = 8;
  binding.objectProfiles = {profile};
  auto objects = scene::nativeSceneSnapshot(native, binding, 2, 3);
  require(objects.entities.back().identity.generation == 7 &&
              objects.entities.back().cell == game::Point{5, 0, 0},
          "Identified object generation and anchored source cell");
  native.actions.objects.objects[0].next = 0;
  rejected([&] { scene::nativeSceneSnapshot(native, binding, 2, 3); });
  std::cout
      << "{\"success\":true,\"world_and_native_projection\":true,\"clock_and_"
         "identity_controls\":true,\"classified_producer_roles\":true}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
