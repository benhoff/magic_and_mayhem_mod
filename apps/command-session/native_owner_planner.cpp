#include "native_owner_planner.hpp"
#include "native_owner_sources.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <set>
#include <stdexcept>
namespace mnm::command_session {
namespace {
void need(bool b, const char *s) {
  if (!b)
    throw std::invalid_argument(s);
}
void actorDomain(const model::AutonomousCreature &a) {
  need(a.ai.type == 17 && a.ai.kind == 1 && !a.ai.ranged && !a.ai.flying &&
           !a.block7c && !a.inventoryCount && !a.form,
       "owner planner ordinary actor domain");
  for (const auto &v : a.inventory)
    need(!v.count && !v.available, "owner planner empty inventory domain");
}
} // namespace
model::OwnerPlannerActor nativeOwnerPlannerActor(const Continuation &c,
                                                 const NativeCreatureProfile &p,
                                                 unsigned slot) {
  const auto &a = c.world.creatures.at(slot);
  actorDomain(a);
  const auto &e = c.execution.at(slot);
  const auto &type = p.types.at(a.ai.type);
  model::OwnerPlannerActor out;
  out.combatValue = e.combatValue;
  out.position = a.ai.position;
  out.speed = type.routeCostFactor;
  if (c.maintenance.at(slot).status778)
    out.speed *= 2;
  if (c.maintenance.at(slot).status77c)
    out.speed /= 2;
  out.motionB63 = int(e.moveVariant);
  out.power = int(e.power84);
  out.bravery = a.ai.bravery;
  out.physical = a.ai.physical != 0;
  out.flag104 = a.flag104 != 0;
  out.defense5c5 = p.valuation.factor5c5;
  out.factor5a9 = p.valuation.factor5a9;
  out.factor5ad = type.routeFlagPenalty;
  if (a.region >= 0)
    out.danger = model::regionDanger(a, c.native.regions.at(unsigned(a.region)),
                                     c.native.regionalRiskPowers);
  model::validateOwnerPlannerActor(out);
  return out;
}
void validateNativeOwnerPlanner(const Continuation &c, bool advancing) {
  if (!c.native.ownerPlanner)
    return;
  auto state = [&](const NativeOwnerPlanner &s) {
    need(c.native.ownerVisibility && c.native.ownerKnowledge &&
             c.native.regionalDanger && c.world.creatures.size() >= 2 &&
             c.world.creatures.size() <= 8,
         "owner planner roots");
    need(s.owner < 8 && s.actor < c.world.creatures.size() &&
             c.world.creatures[s.actor].ai.owner == int(s.owner) &&
             !c.world.human[s.owner],
         "owner planner actor identity");
    for (const auto &a : c.world.creatures)
      actorDomain(a);
    need(s.model.key == 6 && s.model.dimension == 72 &&
             s.model.layers.size() == 3,
         "owner planner model identity");
    model::validateBrainInference(s.model);
    model::validateOwnerTaskSelection(s.selection, 2);
    need(s.random.left < 250 && s.random.right < 250 &&
             (s.random.right + 250 - s.random.left) % 250 == 103,
         "owner planner RNG");
    need(s.nextSerial && s.percent >= 0 && s.percent <= 10000 &&
             s.activeSerial < s.nextSerial,
         "owner planner task identity");
    need((s.active.has_value() || (s.grouped && s.grouped->fallbackActive)) ==
             (s.activeSerial != 0),
         "owner planner active task identity");
    need(std::isfinite(s.candidateScore) && s.candidateScore >= -1 &&
             s.candidateScore <= 1,
         "owner planner candidate score");
    for (float v : s.weights)
      need(std::isfinite(v) && v >= 0 && v <= 1, "owner planner task weights");
    need(s.features.empty() || s.features.size() == 72,
         "owner planner feature continuation");
    need(s.candidateReady == (s.candidate.memoryKey >= 0),
         "owner planner candidate identity");
    if (s.active)
      need(s.active->memoryKey >= 0, "owner planner active memory key");
    std::set<unsigned> members;
    for (auto actor : s.members)
      need(actor < c.world.creatures.size() &&
               c.world.creatures[actor].ai.owner == int(s.owner) &&
               members.insert(actor).second,
           "owner source member identity");
    need(s.members.empty() || (s.grouped && s.actor == s.members.front()),
         "owner source leader identity");
    validateNativeOwnerGroupPlanner(c, &s);
  };
  state(*c.native.ownerPlanner);
  if (c.native.ownerSources) {
    validateNativeOwnerSources(c, advancing);
    for (const auto &row : c.native.ownerSources->planners)
      if (row.planner)
        state(*row.planner);
    if (c.native.ownerSources->search)
      state(c.native.ownerSources->search->scratch);
  }
  for (const auto &rows : c.native.ownerKnowledge->records)
    for (const auto &r : rows) {
      need(r.planning.has_value(),
           "owner planner missing observation features");
      model::validateOwnerPlannerActor(*r.planning);
      need(r.planning->position == r.memory.actor.position,
           "owner planner snapshot position");
    }
}
void prepareNativeOwnerPlanner(Continuation &c,
                               model::BrainInferenceModel model, unsigned owner,
                               unsigned actor, unsigned seed,
                               std::array<float, 3> weights, int percent) {
  need(!c.native.ownerPlanner && c.native.tick == 0 &&
           c.native.ownerKnowledge.has_value(),
       "owner planner requires fresh perception session");
  for (const auto &records : c.native.ownerKnowledge->records)
    need(records.empty(), "owner planner requires empty initial knowledge");
  auto staged = c;
  staged.native.ownerPlanner.emplace();
  auto &s = *staged.native.ownerPlanner;
  s.grouped.emplace();
  s.grouped->replanning.emplace();
  s.grouped->scheduler.emplace();
  s.grouped->replanning->policy.ownerAvailable = true;
  s.grouped->replanning->eventInputs = true;
  s.owner = owner;
  s.actor = actor;
  s.model = std::move(model);
  s.weights = weights;
  s.percent = percent;
  s.random = model::seedRandom(seed);
  validate(staged);
  c = std::move(staged);
}
void updateNativeOwnerPlanner(Continuation &c, const NativeCreatureProfile &p,
                              model::OwnedCreatureCommands &commands,
                              const NativeOwnerTaskVerifier &verifier) {
  if (!c.native.ownerPlanner)
    return;
  validateNativeOwnerPlanner(c, true);
  if (c.native.ownerSources) {
    updateNativeOwnerSources(c, p, commands, verifier);
    return;
  }
  if (c.native.ownerPlanner->grouped) {
    updateNativeOwnerGroupPlanner(c, p, commands);
    return;
  }
  auto &s = *c.native.ownerPlanner;
  const int tick = int(c.native.tick);
  auto &actor = c.world.creatures.at(s.actor);
  if (s.active) {
    int budget = 8;
    auto result =
        model::executeOwnerHostileTask(*s.active, *c.native.ownerKnowledge,
                                       tick, s.owner, s.actor, c.world, budget);
    if (!result.continuing) {
      s.active.reset();
      s.activeSerial = 0;
    } else if (result.kind == model::OwnerHostileOrderKind::target)
      commands.target(actor, result.target);
    else if (result.kind == model::OwnerHostileOrderKind::move) {
      // 50fd50's ordinary, zero-option duplicate-move gate before 50fae0.
      if (actor.ai.behavior != 2 || !(actor.destination == result.position)) {
        c.execution.at(s.actor).commandMode = 0;
        commands.move(actor, result.position);
      }
    }
    if (s.active)
      return;
  }
  if (!actor.ai.active || !actor.ai.health)
    return;
  if (s.selection.reset) {
    s.selection.value = -2;
    s.selection.ties = 0;
    s.selection.selected = -1;
  }
  auto prepare = [&](unsigned evaluator) {
    if (evaluator == 0)
      s.fallbackEmitted = false;
    else {
      s.hostileEmitted = false;
      s.candidateReady = false;
      s.candidate = {};
      s.features.clear();
      const auto &records = c.native.ownerKnowledge->records[s.owner];
      if (records.size() != 1)
        return;
      const auto &r = records.front();
      need(r.planning.has_value(), "owner planner absent memory projection");
      // Native lab policy supplies zero path summaries for same-region
      // candidates. Original probe production and risk rejection remain
      // separate gaps; this does not assert an original successful probe.
      if (actor.region < 0 || r.memory.actor.region != actor.region ||
          !c.native.regions.at(unsigned(actor.region)).parameter)
        return;
      const auto delayWord = model::nextRandom(s.random);
      (void)delayWord;
      const unsigned age = unsigned(tick) - unsigned(r.memory.observedTick);
      const auto radius = age * unsigned(r.planning->motionB63) / 72u;
      need(radius <= 1000, "owner planner uncertainty admission");
      auto enemy = *r.planning;
      // 523240 queries current region rows through the remembered actor's
      // copied owner/flags/bravery, rather than freezing danger at observation.
      model::AutonomousCreature remembered;
      remembered.ai.owner = r.memory.actor.owner;
      remembered.ai.physical = enemy.physical;
      remembered.ai.bravery = enemy.bravery;
      remembered.flag104 = enemy.flag104;
      enemy.danger = model::regionDanger(
          remembered, c.native.regions.at(unsigned(r.memory.actor.region)),
          c.native.regionalRiskPowers);
      s.features = model::hostileTaskCandidateFeatures(
          nativeOwnerPlannerActor(c, p, s.actor), enemy, c.world.dimensions,
          int(radius * radius), {{0, 0, 0}}, false);
      const auto inference = model::inferOwnerTask(s.model, s.features);
      s.candidateScore = model::ownerTaskValue(
          {{inference[0], inference[1], inference[2]}}, s.weights, s.percent);
      s.candidate = {r.memory.key, r.memory.actor.position};
      s.candidateReady = true;
    }
  };
  model::OwnerTaskSelectionServices services{
      2,
      true,
      prepare,
      [&](unsigned evaluator, int &budget) -> std::optional<float> {
        auto &emitted = evaluator ? s.hostileEmitted : s.fallbackEmitted;
        if (emitted || budget < 1)
          return {};
        emitted = true;
        --budget;
        if (!evaluator)
          return -1.f;
        if (s.candidateReady)
          return s.candidateScore;
        return {};
      },
      [](unsigned) {},
      []() {}};
  int budget = 2;
  model::advanceOwnerTaskSelection(s.selection, budget, s.random, services);
  if (s.selection.reset && s.selection.selected == 1) {
    need(s.nextSerial < std::numeric_limits<unsigned>::max(),
         "owner planner task serial exhausted");
    s.active = s.candidate;
    s.activeSerial = s.nextSerial++;
  }
}
} // namespace mnm::command_session
