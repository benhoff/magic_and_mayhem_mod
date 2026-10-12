#include "persistence/snapshot.hpp"
#include "simulation/creature_rules_session.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm::game;
namespace {
void need(bool value, const char *why) {
  if (!value)
    throw std::runtime_error(why);
}
class Rules final : public CreatureRuleRuntime {
public:
  bool failStep = false, failCommand = false;
  void initialize(State &s, unsigned seed) const override {
    s.map = "native-rule-session-contract";
    s.systems = {'R', 'U', 'L', '1', static_cast<unsigned char>(seed)};
    Entity a;
    a.type = 17;
    a.owner = 0;
    a.x = 1;
    s.slots[0].entity = a;
    a.type = 8;
    a.owner = 1;
    a.x = 10;
    s.slots[1].entity = a;
  }
  void validate(const State &s) const override {
    need(s.campaign == Bytes{1, 2, 3}, "rule profile identity");
    need(s.systems.size() == 5 &&
             Bytes(s.systems.begin(), s.systems.begin() + 4) ==
                 Bytes{'R', 'U', 'L', '1'},
         "rule continuation identity");
    need(s.slots.size() == 4 && s.slots[0].entity && s.slots[1].entity,
         "rule capacity");
    need(s.slots[0].entity->x >= 0, "invalid staged rule actor");
  }
  std::vector<CreatureRuleEvent> step(State &s,
                                      TickInput input) const override {
    ++s.systems[4];
    if (!input.alternateMode && !input.suppressSearch)
      ++s.slots[0].entity->x;
    if (failStep)
      throw std::runtime_error("injected rule failure after mutation");
    return {{{0, s.slots[0].generation}, "advance", {}, s.systems[4]}};
  }
  void command(State &s, const CreatureIntent &intent) const override {
    auto &a = *s.slots.at(intent.actor.slot).entity;
    if (intent.kind == CreatureOrderKind::move)
      a.x = intent.position.x;
    else if (intent.kind == CreatureOrderKind::attack)
      a.target = intent.target;
    if (failCommand)
      throw std::runtime_error("injected command failure after mutation");
  }
};
template <class F> void refuses(F &&f) {
  bool failed = false;
  try {
    f();
  } catch (const std::exception &) {
    failed = true;
  }
  need(failed, "operation should refuse");
}
Bytes bytes(const CreatureRuleSession &s) {
  return encodeSnapshot(s.world().state());
}
} // namespace
int main() try {
  auto rules = std::make_shared<Rules>();
  CreatureRuleSession s(rules, {1, 2, 3}, 7, 4);
  auto start = bytes(s);
  need(s.step({false, false, false}).empty() && bytes(s) == start,
       "unadmitted tick changed continuation");
  rules->failStep = true;
  refuses([&] { s.step(); });
  need(bytes(s) == start, "failed tick was published");
  rules->failStep = false;
  CreatureIntent intent{{0, 1}, CreatureOrderKind::move, {9, 0, 0}, {}, false,
                        false};
  rules->failCommand = true;
  refuses([&] { s.command(intent); });
  need(bytes(s) == start, "failed command was published");
  rules->failCommand = false;
  intent.position.x = -1;
  refuses([&] { s.command(intent); });
  need(bytes(s) == start, "failed rule validation was published");
  intent.position.x = 3;
  intent.actor.generation = 2;
  refuses([&] { s.command(intent); });
  need(bytes(s) == start, "stale actor was admitted");
  intent.actor.generation = 1;
  intent.kind = CreatureOrderKind::attack;
  intent.target = Handle{1, 2};
  refuses([&] { s.command(intent); });
  need(bytes(s) == start, "stale target was admitted");
  intent.target = Handle{1, 1};
  s.command(intent);
  need(s.world().state().slots[0].entity->target == intent.target,
       "semantic attack target not retained");
  for (unsigned i = 0; i < 20; ++i)
    s.step();
  auto saved = bytes(s);
  auto loaded = decodeSnapshot(saved);
  World world(0);
  world.restore(loaded);
  CreatureRuleSession resumed(std::move(world), rules);
  need(bytes(resumed) == saved, "restored rule continuation differs");
  for (unsigned i = 0; i < 80; ++i) {
    TickInput input{true, i % 7 == 0, i % 5 == 0};
    s.step(input);
    resumed.step(input);
    need(bytes(s) == bytes(resumed), "resumed rule execution diverged");
  }
  loaded.systems[0] = 'X';
  World bad(0);
  bad.restore(loaded);
  refuses([&] { CreatureRuleSession rejected(std::move(bad), rules); });
  std::cout << "Native rule session: transactional ticks/commands, stale "
               "identities and 80-tick restored continuation passed.\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
