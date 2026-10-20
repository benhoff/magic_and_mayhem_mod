#include "entity_lighting.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm;
static void require(bool ok, int line) {
  if (!ok)
    throw std::runtime_error("Entity lighting producer test line " +
                             std::to_string(line));
}
#define check(...) require((__VA_ARGS__), __LINE__)
template <class F> static void refused(F action) {
  bool caught = false;
  try {
    action();
  } catch (const std::exception &) {
    caught = true;
  }
  check(caught);
}
int main() try {
  assets::EffectLightingFields fields;
  fields.values[2] = {"16", "TRUE", "TRUE"};
  scene::EntityLighting lights(32, 32, 3, fields,
                               {{game::Family::mapLinked, 50, 2}}, {-50, 7});
  game::World world(8);
  game::Entity creature;
  creature.x = creature.y = 8;
  auto c = world.spawn(creature);
  game::Entity effect;
  effect.family = game::Family::missileEffect;
  effect.type = 2;
  effect.x = effect.y = 8;
  auto e = world.spawn(effect);
  game::Entity artifact;
  artifact.x = artifact.y = 8;
  artifact.family = game::Family::mapLinked;
  artifact.type = 50;
  auto a = world.spawn(artifact);
  game::CombatState health;
  game::Fighter f;
  f.actor = c;
  f.health = 20;
  health.fighters.push_back(f);
  scene::EntityLightControls controls;
  controls.camera.column = controls.camera.row = 8;
  controls.camera.span = 16;
  const auto initial = lights.cycle().field().buffers();
  lights.step(world.state(), health, controls);
  const auto &input = lights.snapshot();
  check(input.creatures[c.slot].lightEnabled &&
        input.objects[e.slot].lightIndex == 16 &&
        input.objects[a.slot].lightIndex == 16 &&
        input.objects[e.slot].admissionColumn == 8);
  for (unsigned tick = 0; tick < 16; ++tick)
    lights.step(world.state(), health, controls);
  check(lights.cycle().field().buffers() != initial);
  auto moved = world.state();
  moved.slots[e.slot].entity->x = 9;
  world.restore(moved);
  lights.step(world.state(), health, controls);
  check(lights.snapshot().objects[e.slot].column == 9 &&
        lights.snapshot().objects[e.slot].admissionColumn == 8);
  controls.readmit = {e};
  lights.step(world.state(), health, controls);
  check(lights.snapshot().objects[e.slot].admissionColumn == 9);
  controls.readmit.clear();
  health.fighters[0].health = 0;
  lights.step(world.state(), health, controls);
  check(!lights.snapshot().creatures[c.slot].lightEnabled &&
        lights.snapshot().changed == 1);
  game::CreatureEncounter encounter;
  game::CreatureActor actor;
  actor.handle = c;
  actor.health = 11;
  encounter.actors.push_back(actor);
  lights.step(world.state(), encounter, controls);
  check(lights.snapshot().creatures[c.slot].lightEnabled);
  const auto before = lights.cycle().ticks();
  auto stale = health;
  stale.fighters[0].actor.generation++;
  refused([&] { lights.step(world.state(), stale, controls); });
  check(lights.cycle().ticks() == before);
  refused([&] { lights.step(world.state(), game::CombatState{}, controls); });
  auto duplicate = health;
  duplicate.fighters.push_back(duplicate.fighters[0]);
  refused([&] { lights.step(world.state(), duplicate, controls); });
  check(lights.cycle().ticks() == before);
  world.release(e);
  effect.x = 11;
  auto replacement = world.spawn(effect);
  check(replacement.slot == e.slot && replacement.generation != e.generation);
  lights.step(world.state(), health, controls);
  check(lights.snapshot().objects[e.slot].admissionColumn == 11);
  controls.readmit = {e};
  refused([&] { lights.step(world.state(), health, controls); });
  controls.readmit.clear();
  controls.player = -1;
  controls.camera.column = -1;
  controls.camera.row = -2;
  scene::EntityLighting spectator(32, 32, 3, assets::EffectLightingFields{}, {},
                                  {-50, 7});
  for (unsigned i = 0; i < 8; ++i)
    spectator.step(world.state(), health, controls);
  check(spectator.snapshot().spectator &&
        spectator.snapshot().spectator->column == 31 &&
        spectator.snapshot().spectator->row == 30 &&
        spectator.cycle().creatures().published);
  check(spectator.cycle().field().at(31, 30, 0) > -127);
  auto invalid = world.state();
  invalid.slots[a.slot].entity->type = 99;
  invalid.slots[a.slot].entity->family = game::Family::missileEffect;
  const auto old = lights.cycle().field().buffers();
  refused([&] { lights.step(invalid, health, controls); });
  check(old == lights.cycle().field().buffers());
  refused([&] {
    scene::EntityLighting bad(
        32, 32, 3, fields,
        {{game::Family::mapLinked, 50, 2}, {game::Family::mapLinked, 50, 3}});
  });
  std::cout << "{\"success\":true,\"typed_health_and_installed_profile_"
               "fields\":true,\"actual_light_field\":true,\"generation_"
               "retirement\":true,\"separate_admission_history\":true,"
               "\"spectator_camera\":true,\"atomic_refusal\":true}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
