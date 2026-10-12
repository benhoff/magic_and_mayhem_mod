#include "maintenance.hpp"
#include <algorithm>
namespace mnm::reconstruction::combat {
namespace {
int signedWord(unsigned n) { return std::int32_t(n); }
void decrement(unsigned &n) {
  if (n)
    --n;
}
} // namespace
bool rangedReady(const AutonomousCreature &a, const CreatureExecution &e,
                 unsigned tick) {
  return !(a.ai.type == 18 && a.ai.flying) && a.rangedDistance != 0 &&
         tick >= e.rangedDeadline && !a.ai.readiness && !a.ai.motion;
}
void maintainCreature(AutonomousCreature &a, CreatureExecution &e,
                      CreatureMaintenance &m, AutonomousWorld &w,
                      const MaintenanceEnvironment &env,
                      const MaintenanceServices &s) {
  s.repairAnimation(a);
  if (m.random98) {
    s.random();
    if (env.diagnostic)
      s.diagnostic(a);
  }
  if (a.specialVision == 20) {
    s.color(a, MaintenanceColor::remove, 3);
    s.color(a, MaintenanceColor::add, 6);
    --a.specialVision;
  } else if (signedWord(a.specialVision) > 1) {
    if (a.ai.type != 13)
      --a.specialVision;
  } else if (a.specialVision == 1) {
    a.specialVision = 0;
    s.color(a, MaintenanceColor::remove, 6);
    e.dirtyValue = 1;
  }
  auto timedColor = [&](unsigned &v, unsigned key) {
    if (signedWord(v) > 1)
      --v;
    else if (v == 1) {
      v = 0;
      s.color(a, MaintenanceColor::clear, key);
      e.dirtyValue = 1;
    }
  };
  timedColor(e.mitigation90, 5);
  timedColor(m.timer94, 6);
  a.ai.bloodLust = m.timer94 != 0;
  if (a.ai.gateFC && env.tick > m.upkeepDeadline) {
    m.upkeepDeadline = env.tick + env.upkeepInterval;
    s.upkeep(a);
  }
  s.refresh(a);
  if (env.tick > m.status118 && m.status114) {
    s.expireStatus(a);
    s.effect(a, m.statusEffect, 0x2c9);
    if (!a.ai.health)
      return;
  }
  if (e.bonusMode124)
    s.poll(a, MaintenanceAnimation::bonus);
  if (e.bonusMode124 == 2)
    s.bonusWork(a);
  if (env.localOwner != -1 && a.ai.visible.at(unsigned(env.localOwner)) == 6)
    s.color(a, MaintenanceColor::add, 2);
  if (m.lifespan > 0) {
    m.lifespan = std::max(0, m.lifespan - 5);
    if (m.lifespan <= 30 && !m.marker750) {
      s.activate(a);
      m.marker750 = 1;
    } else if (m.lifespan == 0) {
      s.removeFromWorld(a);
      s.remove(a);
      return;
    }
  }
  if (m.status744) {
    --m.status744;
    m.suspension = signedWord(unsigned(m.suspension) - 1u);
    if (m.status748)
      --m.status748;
    else {
      s.health(a, signedWord(0u - unsigned(env.effectDamage)), int(m.status74c),
               true);
      if (!a.ai.active)
        return;
      m.status748 = env.statusDamage;
    }
    if (a.mode == 2)
      e.absorptionTimer = 1;
  } else if (m.suspension < 0)
    ++m.suspension;
  if (m.flight110 && --m.flight110 == 0 &&
      (a.formCapability || a.animationKind == 3)) {
    a.flag104 = 1;
    a.block71c = 0;
    e.dirtyValue = 1;
  }
  if (m.status778 && --m.status778 == 0)
    e.dirtyValue = 1;
  if (m.status77c && --m.status77c == 0) {
    s.color(a, MaintenanceColor::clear, 1);
    e.dirtyValue = 1;
  }
  if (m.status780) {
    if (--m.status780 == 0) {
      for (const auto &b : w.creatures)
        if (b.ai.active && b.ai.slot != a.ai.slot)
          s.repairReference(b.ai.slot);
    } else
      s.repairStatus(a);
  }
  if (a.form) {
    if (--a.form == 0)
      s.expireForm(a);
    else if (!a.block723 && !a.formCapability && a.animationKind != 3 &&
             !a.mode && a.ai.behavior == 1)
      s.restoreMode(a, env.restoreMode);
  }
  if (m.animation7cc) {
    auto ev = s.poll(a, MaintenanceAnimation::form);
    if (ev == 1)
      m.animation7cc = 0;
    else if (ev == 2) {
      unsigned divisor = unsigned(std::max(1, env.drainDivisor));
      s.drain(a, env.drainQuantity / divisor);
    }
  }
  if (m.status821 && --m.status821 == 0)
    s.color(a, MaintenanceColor::remove, 4);
  if (m.status8c5 && --m.status8c5 == 0)
    s.color(a, MaintenanceColor::remove, 16);
  if (a.protection8c9 && --a.protection8c9 == 0)
    s.color(a, MaintenanceColor::remove, 15);
  if (m.status8cd > 0 && --m.status8cd == 0) {
    s.color(a, MaintenanceColor::add, 8);
    m.status8cd = -1;
  }
  if (env.tick > m.status790 && m.status78c) {
    s.health(a, -1, -1, true);
    if (!a.ai.health)
      return;
    m.status790 = env.tick + env.statusInterval;
    s.effect(a, m.damageEffect, 0x2cf);
  }
  decrement(m.status81d);
  decrement(m.status819);
  m.timer660 = unsigned(std::max(0, signedWord(m.timer660 - 1u)));
  if (a.animationKind == 3) {
    if (a.ai.flying) {
      s.poll(a, MaintenanceAnimation::flight);
      s.effect(a, m.flightEffect, 0x2ee);
      m.flight809 -= 1;
      if (signedWord(m.flight809) <= 0) {
        if (!a.block71c) {
          m.block71d = 1;
          s.repairFlight(a);
          if (!a.ai.active)
            return;
          e.dirtyValue = 1;
        }
        if (signedWord(m.flight809) <= 0) {
          s.removeFromWorld(a);
          if (m.flightEffect) {
            s.releaseEffect(*m.flightEffect);
            m.flightEffect.reset();
          }
          std::optional<unsigned> transient;
          s.effect(a, transient, 0x2f1);
        }
      }
    } else if (m.flightEffect) {
      s.releaseEffect(*m.flightEffect);
      m.flightEffect.reset();
    }
  }
  if (m.status8d5 && env.tick > m.status8d9) {
    --m.status8d5;
    s.health(a, -1, int(m.status8dd), true);
    m.status8d9 = env.tick + env.poisonInterval;
    if (!a.ai.active)
      return;
  }
  if (e.power84 < env.powerMaximum && env.tick > m.powerDeadline) {
    ++e.power84;
    m.powerDeadline = env.tick + env.powerInterval;
  }
  if (m.recent6b0)
    m.recent6b0 = unsigned(std::max(0, signedWord(m.recent6b0 - 1u)));
  if (e.block855 && s.poll(a, MaintenanceAnimation::response) == 1)
    e.block855 = 0;
  if (m.animation889 && s.poll(a, MaintenanceAnimation::auxiliary) == 1)
    m.animation889 = 0;
  if (m.animation8bd) {
    auto ev = s.poll(a, MaintenanceAnimation::owner);
    if (ev == 1)
      m.animation8bd = 0;
    else if (ev == 2)
      s.ownerEvent(a, unsigned(a.ai.owner));
  }
  for (auto &age : m.visibilityAge)
    ++age;
  s.action(a);
  s.color(a, MaintenanceColor::tick, 0);
}
} // namespace mnm::reconstruction::combat
