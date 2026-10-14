#include "runtime.hpp"
#include <limits>
#include <stdexcept>
namespace mnm::command_session {
namespace {
void need(bool ok, const char *why) {
  if (!ok) throw std::invalid_argument(why);
}
model::OwnedCreatureCommands context(Continuation &s, const Hooks &h) {
  need(bool(h.execution) && bool(h.idle) && bool(h.follow) && bool(h.queue) &&
       (bool(h.body) || bool(h.commandBodies)) && bool(h.supplied) && bool(h.release) && bool(h.peer),
       "native command providers are incomplete");
  auto execution = [factory = h.execution](unsigned slot) {
    auto result = factory(slot);
    auto height = result.terrainHeight;
    result.terrainHeight = [height](model::Cell p) {
      need(bool(height), "native command terrain provider is missing");
      const auto value = height(p);
      const auto aim = std::int64_t(p.z) * 16 + value;
      need(aim >= std::numeric_limits<std::int32_t>::min() &&
           aim <= std::numeric_limits<std::int32_t>::max(),
           "native command terrain aim overflows");
      return value;
    };
    return result;
  };
  model::OwnedCreatureCommands commands{s.world, s.execution, s.bindings,
      execution, h.idle, h.follow, h.queue, h.body, h.supplied, h.release, h.peer};
  if (h.commandBodies) commands.installBodies(h.commandBodies);
  return commands;
}
} // namespace
Runtime::Runtime(Continuation initial, game::Bytes profile, HookFactory hooks)
    : initial_(std::move(initial)), profile_(std::move(profile)), hooks_(std::move(hooks)) {
  need(bool(hooks_), "native command providers are missing");
  command_session::validate(initial_);
}
void Runtime::initialize(game::State &s, std::uint32_t seed) const {
  need(s.campaign == profile_, "native command profile differs");
  auto continuation = initial_;
  need(s.slots.size() >= continuation.world.creatures.size(), "native command capacity is too small");
  s.slots.resize(continuation.world.creatures.size());
  continuation.generations.clear();
  for (const auto &slot : s.slots) continuation.generations.push_back(slot.generation);
  continuation.seed = seed;
  s.map = "native-command-session-v1";
  for (auto &slot : s.slots) slot.entity = game::Entity{};
  publish(s, continuation);
}
Continuation Runtime::read(const game::State &s) const {
  need(s.campaign == profile_ && s.map == "native-command-session-v1", "native command profile differs");
  auto c = decode(s.systems);
  need(c.world.creatures.size() == initial_.world.creatures.size() &&
       c.world.dimensions.x == initial_.world.dimensions.x &&
       c.world.dimensions.y == initial_.world.dimensions.y &&
       c.world.dimensions.z == initial_.world.dimensions.z,
       "native command profile geometry differs");
  need(c.world.creatures.size() == s.slots.size(), "native command actor pool differs");
  need(c.maintenance.size() == initial_.maintenance.size() &&
       c.movement.size() == initial_.movement.size(),
       "native command behavior profile differs");
  for (unsigned i = 0; i < s.slots.size(); ++i) {
    const auto &slot = s.slots[i]; const auto &a = c.world.creatures[i];
    need(slot.generation == c.generations[i] && slot.entity.has_value(), "native command actor generation differs");
    const auto &e = *slot.entity;
    need(e.family == game::Family::creature && !e.cleaned && e.type == a.ai.type &&
         e.owner == unsigned(a.ai.owner) && e.x == a.ai.position.x &&
         e.y == a.ai.position.y && e.z == a.ai.position.z, "native command actor projection differs");
    const auto target = a.ai.target < 0 ? std::optional<game::Handle>{} :
        std::optional<game::Handle>{{unsigned(a.ai.target), c.generations.at(unsigned(a.ai.target))}};
    need(e.target == target, "native command target projection differs");
  }
  return c;
}
void Runtime::validate(const game::State &s) const { (void)read(s); }
void Runtime::publish(game::State &s, const Continuation &c) const {
  command_session::validate(c);
  need(c.world.creatures.size() == s.slots.size(), "native command publication capacity differs");
  for (unsigned i = 0; i < s.slots.size(); ++i) {
    auto &slot = s.slots[i]; const auto &a = c.world.creatures[i];
    need(slot.generation == c.generations[i] && slot.entity.has_value(), "native command publication identity differs");
    auto &e = *slot.entity;
    e.family = game::Family::creature; e.type = a.ai.type; e.owner = unsigned(a.ai.owner);
    e.x = a.ai.position.x; e.y = a.ai.position.y; e.z = a.ai.position.z;
    e.target = a.ai.target < 0 ? std::optional<game::Handle>{} :
        std::optional<game::Handle>{{unsigned(a.ai.target), c.generations.at(unsigned(a.ai.target))}};
  }
  s.systems = encode(c);
}
std::vector<game::CreatureRuleEvent> Runtime::step(game::State &s, game::TickInput input) const {
  auto c = read(s); auto h = hooks_(c); auto commands = context(c, h);
  need(bool(h.advance), "native command frame provider is missing");
  auto events = h.advance(c, commands, input);
  for (const auto &event : events) {
    need(event.actor.slot < c.generations.size() && event.actor.generation == c.generations[event.actor.slot],
         "native command event actor is stale");
    if (event.target) need(event.target->slot < c.generations.size() &&
        event.target->generation == c.generations[event.target->slot], "native command event target is stale");
  }
  publish(s, c);
  return events;
}
void Runtime::command(game::State &s, const game::CreatureIntent &intent) const {
  auto c = read(s);
  need(intent.actor.slot < c.generations.size() && c.generations[intent.actor.slot] == intent.actor.generation,
       "native command actor is stale");
  if (intent.target) need(intent.target->slot < c.generations.size() &&
      c.generations[intent.target->slot] == intent.target->generation, "native command target is stale");
  auto h = hooks_(c); auto commands = context(c, h);
  auto &a = c.world.creatures.at(intent.actor.slot);
  const model::Cell p{intent.position.x, intent.position.y, intent.position.z};
  if (intent.kind == game::CreatureOrderKind::move ||
      intent.kind == game::CreatureOrderKind::coordinateAttack) {
    const auto d = c.world.dimensions;
    need(p.x >= 0 && p.x < d.x && p.y >= 0 && p.y < d.y &&
         p.z >= 0 && p.z < d.z, "native command position is outside the map");
  }
  const auto target = intent.target ? int(intent.target->slot) : -1;
  if (intent.kind == game::CreatureOrderKind::attack || intent.kind == game::CreatureOrderKind::follow)
    need(intent.target.has_value(), "native command requires a target");
  if (intent.queued) {
    need(intent.kind == game::CreatureOrderKind::move || intent.kind == game::CreatureOrderKind::attack ||
         intent.kind == game::CreatureOrderKind::coordinateAttack, "this native command cannot be queued");
    model::CreatureOrder order;
    order.kind = intent.kind == game::CreatureOrderKind::move ? 2 :
        intent.kind == game::CreatureOrderKind::attack ? 3 : 4;
    order.destination = p; order.target = target; order.repeat = intent.repeat;
    commands.append(a, order);
  } else {
    need(!intent.repeat, "repeat requires a queued native command");
    switch (intent.kind) {
    case game::CreatureOrderKind::move: commands.move(a, p); break;
    case game::CreatureOrderKind::attack: commands.target(a, target); break;
    case game::CreatureOrderKind::coordinateAttack: commands.coordinate(a, p); break;
    case game::CreatureOrderKind::follow: commands.follow(a, target); break;
    case game::CreatureOrderKind::clear: commands.clear(a); break;
    default: throw std::invalid_argument("unknown native command");
    }
  }
  publish(s, c);
}
} // namespace mnm::command_session
