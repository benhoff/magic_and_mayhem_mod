#pragma once
#include "../apps/command-session/native_regional_danger.hpp"
#include "regional-graph-encounter-state.hpp"
namespace mnm::encounter::regional_danger {
inline Profile profile(game::Bytes inputs, unsigned scenario) {
  auto p = scenario == 3 ? decodeInputs(std::move(inputs))
                         : regional_graph::profile(std::move(inputs), 0);
  if (scenario == 3)
    enableRegionalNavigation(p);
  auto &c = p.initial;
  c.world.creatures[1].ai.owner = 1;
  if (scenario != 3) {
    for (auto &a : c.world.creatures) {
      a.ai.target = -1;
      a.ai.enemyRadius.fill(0);
      a.ai.allyRadius.fill(0);
    }
    c.execution[0].commandMode = 1;
  }
  cs::NativeCreatureGeometry geometry(c, p.native);
  for (auto &a : c.world.creatures) {
    a.region = cs::model::lookupNavigationRegion(
        c.native.regions, a.ai.position,
        cs::nativeRegionalQuery(geometry, a, {a.ai.slot, a.ai.owner, 0, 0}),
        c.world.dimensions, p.native.boundary, 0, 0,
        cs::nativeRegionalServices(c, p.native, geometry, a));
    c.execution[a.ai.slot].combatValue[5] = 10000 + int(a.ai.slot) * 1000;
  }
  enableRegionalDanger(p);
#ifdef MNM_OWNER_KNOWLEDGE
  cs::prepareNativeOwnerKnowledge(c);
#endif
  if (scenario == 2) {
    auto &in = c.native.regionalDanger->inputs;
    in.fireRange = 6;
    in.firePower = 17;
    in.effects = {{0x41, 1, 6, c.native.regions[10].position},
                  {0x46, 1, 6, c.native.regions[20].position}};
    in.hazards = {{{1, 2, 3}, 3, 10}, {{11, 2, 3}, 4, 7}, {{31, 3, 3}, 5, 13}};
  }
  return p;
}
inline Profile profile(const std::filesystem::path &install,
                       unsigned scenario) {
  return profile(loadInputs(install, {0, 0, 32, 32}, 0), scenario);
}
inline bool visibilityEvent(unsigned scenario, unsigned tick) {
#ifdef MNM_OWNER_KNOWLEDGE
  return scenario == 1 && (tick == 64 || tick == 1408);
#else
  return scenario == 1 && (tick == 64 || tick == 256);
#endif
}
inline bool diplomacyEvent(unsigned scenario, unsigned tick) {
  return scenario == 2 && tick == 128;
}
inline void policy(cs::Continuation &c, unsigned scenario) {
  if (visibilityEvent(scenario, c.native.tick))
    c.world.creatures[1].ai.visible[0] = c.native.tick != 64;
  if (diplomacyEvent(scenario, c.native.tick))
    c.world.relation[0][1] = c.world.relation[1][0] = 1;
}
inline std::shared_ptr<const cs::Runtime> runtime(const Profile &p,
                                                  unsigned scenario) {
  auto lower = consequences(p);
  auto providers = cs::nativeProviders(p.native, lower);
  return std::make_shared<const cs::Runtime>(
      p.initial, p.bytes, [providers, lower, scenario](cs::Continuation &c) {
        auto hooks = providers(c);
        auto advance = hooks.advance;
        hooks.advance = [&c, lower, scenario,
                         advance](auto &state, auto &commands, auto input) {
          policy(c, scenario);
          if ((scenario != 3 && c.native.tick == 32) ||
              (scenario == 1 && c.native.tick == 96)) {
            const unsigned slot = c.native.tick == 96 ? 1 : 0;
            commands.move(c.world.creatures[slot],
                          slot ? c.execution[0].roamHome
                               : c.world.creatures[0].destination);
          }
          if (scenario == 1 && c.native.tick ==
#ifdef MNM_OWNER_KNOWLEDGE
              1792
#else
              384
#endif
          ) {
            auto fx = lower(c);
            fx.bindCommands(commands);
            fx.maintenance.health(c.world.creatures[1], -10000, -1, false);
          }
          return advance(state, commands, input);
        };
        return hooks;
      });
}
inline paired::Projection
dangerProjection(const std::vector<cs::model::NavigationRegion> &g,
                 const cs::model::RegionalDangerState &s,
                 const cs::model::RegionalDangerInputs &in) {
  paired::Projection out;
  auto add = [&](const std::string &name, unsigned v) {
    out.push_back({name, v});
  };
  add("danger.reset", s.resetDanger);
  add("danger.owner", s.dangerOwner);
  add("danger.memory", s.memoryKey);
  add("danger.region", s.dangerRegion);
  add("danger.commander", s.commander);
  add("danger.total", s.dangerTotal);
  add("deduction.reset", s.resetDeduction);
  add("deduction.owner", s.deductionOwner);
  add("deduction.actor", s.actorCursor);
  add("deduction.region", s.deductionRegion);
  add("deduction.total", s.deductionTotal);
  for (unsigned i = 0; i < g.size(); ++i) {
    const auto prefix = "danger.region[" + std::to_string(i) + "].";
    for (unsigned owner = 0; owner < 8; ++owner)
      for (unsigned j = 0; j < 7; ++j) {
        add(prefix + "danger[" + std::to_string(owner) + "][" +
                std::to_string(j) + "]",
            g[i].danger[owner][j]);
        add(prefix + "deduction[" + std::to_string(owner) + "][" +
                std::to_string(j) + "]",
            g[i].deductedDanger[owner][j]);
      }
    for (unsigned j = 0; j < 7; ++j) {
      add(prefix + "pending[" + std::to_string(j) + "]", s.pendingDanger[i][j]);
      add(prefix + "deduct.pending[" + std::to_string(j) + "]",
          s.pendingDeduction[i][j]);
    }
    add(prefix + "base", g[i].dangerBase);
    add(prefix + "base.pending", s.pendingBase[i]);
  }
  for (unsigned owner = 0; owner < 8; ++owner) {
    add("danger.owner.total[" + std::to_string(owner) + "]",
        s.dangerTotals[owner]);
    add("deduction.owner.total[" + std::to_string(owner) + "]",
        s.deductionTotals[owner]);
    for (const auto &memory : in.memories[owner]) {
      const auto prefix = "memory[" + std::to_string(owner) + "][" +
                          std::to_string(memory.key) + "].";
      const auto &a = memory.actor;
      add(prefix + "tick", memory.observedTick);
      add(prefix + "x", a.position.x);
      add(prefix + "y", a.position.y);
      add(prefix + "z", a.position.z);
      add(prefix + "region", a.region);
      add(prefix + "health", a.health);
      add(prefix + "threat", a.threat);
      add(prefix + "resource", a.resource);
      add(prefix + "maximum", a.maximumResource);
    }
  }
  for (unsigned i = 0; i < s.actorSupport.size(); ++i)
    add("danger.actor.support[" + std::to_string(i) + "]", s.actorSupport[i]);
  return out;
}
inline paired::Projection projection(const cs::Continuation &c,
                                     const Profile &p) {
  auto out = regional::projection(c, p);
  for (unsigned i = 0; i < c.execution.size(); ++i)
    for (unsigned j = 0; j < 6; ++j)
      out.push_back(
          {"value[" + std::to_string(i) + "][" + std::to_string(j) + "]",
           unsigned(c.execution[i].combatValue[j])});
  auto danger =
      dangerProjection(c.native.regions, c.native.regionalDanger->state,
                       c.native.regionalDanger->inputs);
  out.insert(out.end(), danger.begin(), danger.end());
  return out;
}
} // namespace mnm::encounter::regional_danger
