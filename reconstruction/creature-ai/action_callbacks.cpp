#include "action_callbacks.hpp"
namespace mnm::reconstruction::combat {
namespace {
void decrementEngagement(AutonomousCreature &a, AutonomousCreature &b) {
  a.ai.engaged = 0;
  int n = std::int32_t(b.ai.incoming - 1u);
  b.ai.incoming = n < 0 ? 0 : unsigned(n);
}
unsigned attackEffect(unsigned type, bool alternate) {
  switch (type) {
  case 3:
    return 0x1a3;
  case 5:
    return 0x117;
  case 8:
    return alternate ? 0 : 1;
  case 14:
    return 0x7d;
  case 19:
    return 0xfc;
  case 21:
    return 0xc6;
  default:
    return 0;
  }
}
} // namespace
void tickIdleAction(AutonomousCreature &a, CreatureExecution &,
                    AutonomousWorld &w, const ActionCallbackState &state,
                    const ActionCallbackServices &s) {
  int event = s.poll(a, false);
  if (a.animationKind == 1)
    s.poll(a, true);
  if (event == 1) {
    if (a.ai.behavior == 1 && !a.blockDE3 && a.ai.type != 22) {
      if (s.immediate(a)) {
        if (a.ai.type == 27) {
          s.reset(a);
          return;
        }
        a.ai.readiness = 0;
        if (a.ai.engaged) {
          if (a.ai.action == 10)
            decrementEngagement(a, w.creatures.at(unsigned(a.ai.target)));
          else if (a.ai.action == 20) {
            a.ai.engaged = 0;
            s.releaseEngagement(w.creatures.at(unsigned(a.alternateTarget)));
          }
        }
        a.ai.marker = 0;
        s.behavior(a, 9);
        return;
      }
      if (s.rangedOpponent(a)) {
        if (a.ai.type == 27) {
          s.reset(a);
          return;
        }
        a.ai.readiness = 0;
        if (a.ai.engaged && (a.ai.action == 10 || a.ai.action == 20)) {
          int target = a.ai.action == 10 ? a.ai.target : a.alternateTarget;
          a.ai.engaged = 0;
          s.releaseEngagement(w.creatures.at(unsigned(target)));
        }
        a.ai.marker = 0;
        s.behavior(a, 10);
        return;
      }
    }
    s.runBehavior(a);
  } else if (event == 3) {
    switch (a.ai.action) {
    case 1:
      s.cue(a, ActionCue::idle, state.sequence);
      break;
    case 6:
      s.cue(a, ActionCue::interaction, 0);
      break;
    case 16:
      s.effect(a, 0x36, a.ai.position);
      break;
    case 17:
      s.effect(a, 0xc0, a.ai.position);
      break;
    case 19:
      s.cue(a, ActionCue::remove, 0);
      break;
    default:
      break;
    }
  } else if (event == 6)
    s.effect(a, 0x203, a.ai.position);
}
void tickCorpseAction(AutonomousCreature &a, const ActionCallbackServices &s) {
  if (s.poll(a, false) == 1)
    s.rewind(a);
  a.ai.corpseA -= 1;
  if (std::int32_t(a.ai.corpseA) <= 0) {
    ++a.ai.corpseB;
    s.runBehavior(a);
  }
}
void tickMeleeAction(AutonomousCreature &a, CreatureExecution &e,
                     AutonomousWorld &w, const CreatureMaintenance &m,
                     const ActionCallbackState &state, bool alternate,
                     const ActionCallbackEnvironment &env,
                     const ActionCallbackServices &s) {
  int event = s.poll(a, false);
  auto target = [&]() -> AutonomousCreature & {
    return w.creatures.at(
        unsigned(alternate ? a.alternateTarget : a.ai.target));
  };
  if (a.animationKind == 1 && s.poll(a, true) == 2) {
    auto &b = target();
    s.defendedDamage(b, env.overlayDamage, a.ai.owner, state.contributors);
    s.notifyHit(b);
    if (!b.ai.health)
      return;
  }
  if (event == 2) {
    if ((alternate ? a.alternateTarget : a.ai.target) < 0 ||
        unsigned(alternate ? a.alternateTarget : a.ai.target) >=
            w.creatures.size())
      return;
    auto &b = target();
    decrementEngagement(a, b);
    if (alternate) {
      b.targetMovingAttacker = a.ai.slot + 1;
      int saved = a.ai.target;
      a.ai.target = a.alternateTarget;
      if (!e.block855)
        s.hit(a);
      a.ai.target = saved;
    } else {
      if (!e.block855)
        s.hit(a);
      if (b.ai.health && b.targetMovingAttacker != a.ai.slot + 1)
        b.targetMovingAttacker = a.ai.slot + 1;
    }
  } else if (event == 3)
    s.cue(a, ActionCue::strike, 0);
  else if (event == 4) {
    if (!e.block855)
      if (auto effect = attackEffect(a.ai.type, alternate))
        s.effect(a, effect, a.ai.position);
  } else if (event == 1) {
    if (a.ai.engaged && (a.ai.action == 10 || a.ai.action == 20)) {
      auto &b = w.creatures.at(
          unsigned(a.ai.action == 10 ? a.ai.target : a.alternateTarget));
      if (alternate && a.ai.action == 20) {
        a.ai.engaged = 0;
        s.releaseEngagement(b);
      } else
        decrementEngagement(a, b);
    }
    s.action(a, 15);
    unsigned delay = (s.random() & 0x7fffffffu) % 3;
    if (env.diagnostic)
      s.diagnostic(alternate ? 0x2c4d : 0x1b9f);
    a.cooldown = int(delay + 2 + (m.status77c ? 30 : 0));
  }
}
void tickInteractionAction(AutonomousCreature &a,
                           const ActionCallbackEnvironment &env,
                           const ActionCallbackServices &s) {
  int event = s.poll(a, false);
  if (a.animationKind == 1)
    s.poll(a, true);
  if (event == 2)
    s.interactionHit(a);
  else if (event == 3) {
    s.cue(a, ActionCue::strike, 0);
    s.effect(a, 0x2bf, a.ai.position);
  } else if (event == 1) {
    s.action(a, 15);
    if (a.routePhase)
      a.cooldown = 10;
    else {
      unsigned delay = (s.random() & 0x7fffffffu) % 3;
      if (env.diagnostic)
        s.diagnostic(0x1bc9);
      a.cooldown = int(delay + 2);
    }
  }
}
void tickStunAction(AutonomousCreature &a, CreatureExecution &e,
                    ActionCallbackState &state,
                    const ActionCallbackServices &s) {
  if (a.animationKind == 1)
    s.poll(a, true);
  if (a.mode != 3) {
    int event = s.poll(a, false);
    if (event == 1) {
      if (std::int32_t(e.absorptionTimer) > 0) {
        unsigned oldMode = a.mode;
        a.mode = 0;
        a.ai.behavior = 1;
        s.stun(a, e.absorptionTimer, oldMode);
        return;
      }
      if (a.ai.pending == 2 || a.ai.pending == 6) {
        s.behavior(a, a.ai.pending);
        a.ai.pending = 0;
        if (e.routeEffect) {
          s.releaseEffect(*e.routeEffect);
          e.routeEffect.reset();
        }
        for (auto &order : e.orders)
          if (order.kind == 2 && order.effect) {
            s.releaseEffect(*order.effect);
            order.effect.reset();
          }
        if (e.marker68)
          s.updateOrders(a);
      } else if (a.ai.pending == 3 && a.ai.target != -1) {
        s.behavior(a, 3);
        a.ai.pending = 0;
      } else {
        a.ai.pending = 0;
        s.reset(a);
      }
      if (a.form) {
        unsigned old = a.form;
        a.form = 0;
        s.drainForm(a, old);
      }
    } else if (event == 100) {
      if (a.mode == 1)
        s.pollColor(a, 4);
      else if (a.mode == 2)
        s.pollColor(a, 1);
    } else if (event == 3)
      s.cue(a, ActionCue::pain, 0);
  }
  if (std::int32_t(e.absorptionTimer) <= 0 && a.mode == 3)
    return;
  if (std::int32_t(e.absorptionTimer) > 0) {
    e.absorptionTimer -= 1;
    if (std::int32_t(e.absorptionTimer) <= 0)
      e.absorptionTimer = 0;
    if (e.absorptionTimer)
      return;
  } else
    e.absorptionTimer = 0;
  state.animationBreak = true;
  if (a.mode == 0)
    return;
  if (a.mode == 2)
    s.clearColor(a, 1);
  else if (a.mode == 1)
    s.clearColor(a, 4);
  a.mode = e.absorptionTimer = 0;
}
} // namespace mnm::reconstruction::combat
