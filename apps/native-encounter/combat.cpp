#include "../command-session/native_owner_visibility.hpp"
#include "../command-session/native_owner_task_inputs.hpp"
#include "../../reconstruction/combat/autonomy_tables.hpp"
#include "../../reconstruction/combat/health.hpp"
#include "../command-session/native_geometry.hpp"
#include "../command-session/native_route.hpp"
#include "encounter.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::encounter {
namespace {
namespace m = cs::model;
constexpr unsigned healthWords = 31;
unsigned statsOffset(const cs::Continuation &c){return 8+unsigned(c.world.creatures.size())*healthWords*4;}
void need(bool b, const char *s) {
  if (!b)
    throw std::invalid_argument(s);
}
unsigned get(const game::Bytes &b, unsigned at) {
  need(at <= b.size() && b.size() - at >= 4, "encounter backend extent");
  unsigned n = 0;
  for (unsigned i = 0; i < 4; ++i)
    n |= unsigned(b[at + i]) << (8 * i);
  return n;
}
void put(game::Bytes &b, unsigned at, unsigned n) {
  need(at <= b.size() && b.size() - at >= 4, "encounter backend extent");
  for (unsigned i = 0; i < 4; ++i)
    b[at + i] = std::uint8_t(n >> (8 * i));
}
class Combat {
  cs::Continuation &c;
  std::shared_ptr<const Profile> p;
  m::OwnedCreatureCommands *commands = nullptr;
  void count(unsigned i, unsigned n = 1) {
    auto before = get(c.backend, statsOffset(c) + 4 * i);
    need(n <= 0xffffffffu - before, "encounter statistic overflow");
    put(c.backend, statsOffset(c) + 4 * i, before + n);
  }
  m::State healthState(const m::AutonomousCreature &a) {
    m::State s;
    unsigned at = 8 + a.ai.slot * healthWords * 4;
    auto word = [&] {
      auto n = get(c.backend, at);
      at += 4;
      return n;
    };
    s.health = word();
    s.bar = word();
    s.dirty = word();
    s.cooldown = word();
    s.target = word();
    s.gate9c = word();
    s.expiry = word();
    s.timer = word();
    s.cacheA = word();
    s.cacheB = word();
    s.marker = word();
    s.flag104 = word();
    s.rank = word();
    for (auto &v : s.incoming)
      v = word();
    for (auto &v : s.ages)
      v = word();
    for (auto &v : s.credit)
      v = word();
    const auto &e = c.execution.at(a.ai.slot);
    const auto &m = c.maintenance.at(a.ai.slot);
    s.health = a.ai.health;
    s.dirty = e.dirtyValue;
    s.marker = a.ai.marker;
    s.ages = m.visibilityAge;
    s.gate9c = e.absorptionTimer;
    s.cooldown = m.timer660;
    s.target = unsigned(a.ai.target);
    s.expiry = m.status778;
    s.timer = m.status780;
    s.cacheA = m.animation889;
    s.cacheB = m.animation8bd;
    s.flag104 = a.flag104;
    s.rank = e.bonusMode124;
    return s;
  }
  void store(m::AutonomousCreature &a, const m::State &s) {
    unsigned at = 8 + a.ai.slot * healthWords * 4;
    auto word = [&](unsigned n) {
      put(c.backend, at, n);
      at += 4;
    };
    for (auto n :
         {s.health, s.bar, s.dirty, s.cooldown, s.target, s.gate9c, s.expiry,
          s.timer, s.cacheA, s.cacheB, s.marker, s.flag104, s.rank})
      word(n);
    for (auto n : s.incoming)
      word(n);
    for (auto n : s.ages)
      word(n);
    for (auto n : s.credit)
      word(n);
    auto &e = c.execution.at(a.ai.slot);
    auto &m = c.maintenance.at(a.ai.slot);
    a.ai.health = s.health;
    a.ai.target = int(s.target);
    m.timer660 = s.cooldown;
    m.status778 = s.expiry;
    m.status780 = s.timer;
    m.animation889 = s.cacheA;
    m.animation8bd = s.cacheB;
    a.flag104 = s.flag104;
    e.bonusMode124 = s.rank;
    e.absorptionTimer = s.gate9c;
    e.dirtyValue = s.dirty;
    a.ai.marker = s.marker;
    m.visibilityAge = s.ages;
  }
  void remove(m::AutonomousCreature &a) {
    cs::NativeCreatureGeometry g(c, p->native);
    g.reserve(a, a.ai.position, false);
    if (a.ai.motion) {
      const auto &r = c.actions.segments.at(a.ai.slot);
      if (r.cursor < r.count)
        g.reserve(a, r.route.at(r.cursor).position, false);
    }
    a.ai.active = false;
    a.ai.motion = false;
    // Native fixed-pool lifetime policy retains the identity and dead
    // descriptor.
    for (auto &peer : c.world.creatures)
      if (peer.ai.active && peer.ai.target == int(a.ai.slot))
        commands->reset(peer);
  }
  void repairPeers(unsigned removed) {
    m::ExecutionServices services;
    services.action = [this](auto &a, unsigned kind) {
      commands->action(a, kind);
    };
    services.behavior = [this](auto &a, unsigned kind) {
      commands->bindOriginal(a, kind);
      commands->dispatch(a);
    };
    services.releaseEffect = [](unsigned) {
      throw std::invalid_argument("encounter cleanup effect unavailable");
    };
    services.peerEngagement = [](auto &a, unsigned) {
      if (a.ai.incoming)
        --a.ai.incoming;
    };
    for (auto &peer : c.world.creatures)
      if (peer.ai.active && peer.ai.slot != removed)
        m::repairCreatureReference(peer, c.execution.at(peer.ai.slot), c.world,
                                   int(removed), services);
  }
  m::StunState controlState(const m::AutonomousCreature &a) {
    m::StunState s;
    const auto &e = c.execution.at(a.ai.slot);
    s.type = a.ai.type;
    s.health = a.ai.health;
    s.mode = a.mode;
    s.behavior = a.ai.behavior;
    s.pending = a.ai.pending;
    s.action = a.ai.action;
    s.engaged = a.ai.engaged;
    s.incoming = a.ai.target >= 0
                     ? c.world.creatures.at(unsigned(a.ai.target)).ai.incoming
                     : 0;
    s.marker = a.ai.marker;
    s.readiness = a.ai.readiness;
    s.animationDirty = 0;
    s.animationKind = a.animationKind;
    s.configFlag = a.configFlags;
    s.form = a.form;
    s.flight = a.ai.flying;
    s.savedHeight = 0;
    s.blockDE3 = a.blockDE3;
    s.block962 = e.block962;
    s.block963 = e.block963;
    s.block71c = a.block71c;
    s.block723 = a.block723;
    s.timer = int(e.absorptionTimer);
    s.target = a.ai.target;
    s.position = a.ai.position;
    s.attachment = false;
    s.queueEffects = false;
    s.surface = false;
    return s;
  }
  void applyControl(m::AutonomousCreature &a, const m::StunResult &r) {
    const auto &s = r.state;
    auto &e = c.execution.at(a.ai.slot);
    if (a.ai.engaged && a.ai.target >= 0)
      c.world.creatures.at(unsigned(a.ai.target)).ai.incoming = s.incoming;
    a.ai.engaged = s.engaged;
    a.ai.pending = s.pending;
    a.ai.marker = s.marker;
    a.ai.readiness = s.readiness;
    a.mode = s.mode;
    e.absorptionTimer = unsigned(s.timer);
    a.blockDE3 = s.blockDE3;
    e.block962 = s.block962;
    e.block963 = s.block963;
    for (auto event : r.events) {
      if (event.kind == m::ControlEffect::behavior)
        commands->transition(a, unsigned(event.argument));
      else if (event.kind == m::ControlEffect::reset)
        commands->reset(a);
      else
        throw std::invalid_argument(
            "unsupported ordinary encounter control consequence");
    }
  }
  bool knockback(m::AutonomousCreature &a,
                 const m::AutonomousCreature &source) {
    m::KnockbackInput input;
    input.actor = controlState(a);
    input.motion = a.ai.motion;
    input.facing = c.actions.motion.at(a.ai.slot).direction;
    input.response = unsigned(m::relativeDirection(
        source.ai.position, a.ai.position, input.facing, c.world.dimensions));
    input.dimensions = c.world.dimensions;
    std::copy_n(m::originalRouteOffsets.begin(), 8, input.offsets.begin());
    cs::NativeCreatureGeometry g(c, p->native);
    auto r = m::admitKnockback(
        input, m::nextRandom(c.native.random),
        [&](auto q) { return g.legal(a, q); },
        [&](auto q) {
          return cs::immediateNativeRoute(c,p->native,a,q,a.routeMode);
        });
    return r.admitted;
  }

public:
  Combat(cs::Continuation &state, std::shared_ptr<const Profile> profile)
      : c(state), p(std::move(profile)) {
    need(c.backend.size() == statsOffset(c) + 24 &&
             std::equal(c.backend.begin(), c.backend.begin() + 8, "MNMECB01"),
         "encounter backend version");
    need(c.world.creatures.size() == p->initial.world.creatures.size() && c.world.creatures.size()<=8, "encounter fixed actor pool");
    for (const auto &a : c.world.creatures)
      need(a.ai.type == 17 && a.ai.kind == 1 && a.width == 2 && a.height == 2 &&
               !a.form && !a.mode && !a.ai.flying &&
               !c.execution.at(a.ai.slot).bonusMode124,
           "encounter ordinary actor domain");
  }
  void health(m::AutonomousCreature &a, m::Entry entry, int amount,
              int source = -1, int sourceSlot = -1, bool notify = true) {
    need(commands != nullptr, "encounter operation context missing");
    m::Inputs input;
    input.baseMaximum = p->maximum;
    input.slot = a.ai.slot;
    input.owner = unsigned(a.ai.owner);
    input.selectedOwner = 0;
    input.attributionOwner = source;
    input.notify = notify;
    input.visibleMask = 0;
    for (unsigned i = 0; i < 8; ++i)
      if (a.ai.visible[i])
        input.visibleMask |= 1u << i;
    input.contributions = true;
    input.contributionIds = {0xffffffffu, 0xffffffffu};
    input.creditAvailable = {false, false};
    input.status = 0;
    input.blocked = a.block7c;
    auto before = a.ai.health;
    auto r = m::apply(healthState(a), input, entry, amount);
    store(a, r.state);
    if (before && !a.ai.health)
      repairPeers(a.ai.slot);
    count(1, before >= a.ai.health ? before - a.ai.health : 0);
    for (auto event : r.events) {
      if (event.kind == m::Effect::feedback && c.native.ownerPlanner &&
          c.native.ownerPlanner->grouped &&
          c.native.ownerPlanner->grouped->replanning &&
          c.native.ownerPlanner->grouped->replanning->eventInputs) {
        // Ordinary empty-mana/rank0 feedback. Lethal cleanup has already
        // zeroed health and emits quantity0; do not credit pre-cleanup HP.
        const auto credited = m::contributionAccount(
            p->taskCredit, a.ai.type, 0, int(event.a), 0, 0, 0);
        if (amount < 0 || entry != m::Entry::signedHealth)
          cs::creditNativeOwnerDamage(c, a.ai.slot, sourceSlot, credited);
        else {
          auto &planner = *c.native.ownerPlanner;
          auto &previous = planner.grouped->replanning->previous;
          if (previous && planner.actor == a.ai.slot)
            m::addOwnerTaskReward(*previous, credited);
        }
      }
      if (event.kind == m::Effect::behavior) {
        count(4);
        a.ai.marker = 0;
        c.native.search.at(a.ai.slot) = {};
        commands->transition(a, event.a);
      } else if (event.kind == m::Effect::pain)
        count(5);
    }
  }
  void stun(m::AutonomousCreature &a, unsigned duration, unsigned mode) {
    need(mode == 0, "encounter only ordinary stun");
    auto r = m::beginStun(controlState(a), int(duration), mode);
    bool admitted = std::any_of(r.events.begin(), r.events.end(), [](auto e) {
      return e.kind == m::ControlEffect::behavior && e.argument == 8;
    });
    applyControl(a, r);
    if (admitted)
      count(2);
  }
  void hit(m::AutonomousCreature &a) {
    need(a.ai.target >= 0, "encounter hit target");
    auto &b = c.world.creatures.at(unsigned(a.ai.target));
    auto parameters = p->melee;
    auto &e = c.execution.at(a.ai.slot);
    need(!a.block7c && !b.block7c, "encounter no evasion domain");
    parameters.charge = e.previousMove != 0;
    parameters.chargeScalar = int(e.previousMoveVariant);
    e.previousMove = 0;
    parameters.power = int(e.valueScale8ed);
    parameters.height = a.ai.position.z - b.ai.position.z;
    parameters.heightBonus = 25;
    parameters.chargeBonus = 62;
    constexpr int response[8] = {0, 0, 25, 50, 75, 50, 25, 0};
    parameters.responseModifier = response[m::relativeDirection(
        a.ai.position, b.ai.position, c.actions.motion.at(b.ai.slot).direction,
        c.world.dimensions)];
    parameters.targetResponse = c.execution.at(b.ai.slot).routeRenewal != 0 ||
                                c.maintenance.at(b.ai.slot).recent6b0 != 0;
    parameters.bloodLust = a.ai.bloodLust;
    parameters.attackerPhysical = a.ai.physical != 0;
    parameters.targetPhysical = b.ai.physical != 0;
    m::MeleeDispatch dispatch;
    dispatch.event = [&](const auto &event) {
      if (event.kind == m::MeleeEffect::damage) {
        count(0);
        health(b, m::Entry::defendedDamage, event.value, a.ai.owner, int(a.ai.slot));
      } else if (event.kind == m::MeleeEffect::stunHandled) {
        applyControl(b, m::knockbackResponse(controlState(b)));
        count(3);
      } else if (event.kind == m::MeleeEffect::stunFallback)
        stun(b, unsigned(event.value), 0);
      else if (event.kind == m::MeleeEffect::attackSound)
        count(5);
    };
    dispatch.tryStun = [&](int) { return knockback(b, a); };
    m::resolveMelee(
        parameters, [&] { return m::nextRandom(c.native.random); }, dispatch);
  }
  cs::NativeConsequences services(const std::shared_ptr<Combat> &self) {
    cs::NativeConsequences f;
    f.bindCommands = [self](auto &context) { self->commands = &context; };
    auto noop = [](auto &) {};
    auto no = [](auto &) { return false; };
    f.execution.movementRenewal = no;
    f.execution.adjustDuplicate = [](auto &) { return false; };
    f.execution.duplicateMove = [](auto &, auto q) { return q; };
    f.callbacks.hit = [self](auto &a) { self->hit(a); };
    f.callbacks.stun = [self](auto &a, unsigned d, unsigned mode) {
      self->stun(a, d, mode);
    };
    f.callbacks.rangedOpponent = no;
    f.callbacks.cue = [self](auto &, auto, unsigned) { self->count(5); };
    f.callbacks.effect = [self](auto &, unsigned, auto) { self->count(5); };
    f.callbacks.notifyHit = noop;
    f.callbacks.updateOrders = noop;
    f.callbacks.releaseEngagement = [](auto &a) {
      if (a.ai.incoming)
        --a.ai.incoming;
    };
    f.callbacks.clearColor = [](auto &, unsigned) {};
    f.callbacks.pollColor = [](auto &, unsigned) {};
    f.callbacks.releaseEffect = [](unsigned) {
      throw std::invalid_argument("encounter effect token unavailable");
    };
    f.callbacks.interactionHit = [](auto &) {
      throw std::invalid_argument("encounter spell unavailable");
    };
    f.behavior.blocked = no;
    f.behavior.rangedOpponent = no;
    f.behavior.restoreForm = [](auto &, unsigned) {
      throw std::invalid_argument("encounter form unavailable");
    };
    f.motion.response = no;
    f.motion.visualFacing = noop;
    f.motion.footstep = [self](auto &) { self->count(5); };
    f.segment.precheck = no;
    f.segment.immediate = no;
    f.segment.fallback = no;
    f.segment.visualFacing = noop;
    f.segment.hazard = [](const auto &, auto) { return false; };
    f.completion.environment = noop;
    f.completion.animation = noop;
    f.completion.terrain = noop;
    if (c.native.ownerPlanner && c.native.ownerPlanner->grouped &&
        c.native.ownerPlanner->grouped->replanning &&
        c.native.ownerPlanner->grouped->replanning->eventInputs)
      f.completion.score = [self](unsigned type, float ratio) {
        return m::contributionAccount(self->p->taskCredit, type, 0, 0, 0, 0, ratio);
      };
    f.movement.fallback = no;
    f.movement.renew = [](auto &, const auto &) {};
    f.movement.clearOverlay = noop;
    f.movement.invalidate = noop;
    f.movement.settle = noop;
    f.movement.refresh = noop;
    f.movement.remove = [self](auto &a) { self->remove(a); };
    f.maintenance.repairAnimation = noop;
    f.maintenance.refresh = noop;
    f.maintenance.color = [](auto &, auto, unsigned) {};
    f.maintenance.upkeep = noop;
    f.maintenance.health = [self](auto &a, int n, int owner, bool notify) {
      self->health(a, m::Entry::signedHealth, n, owner, -1, notify);
    };
    f.maintenance.remove = [self](auto &a) { self->remove(a); };
    f.maintenance.removeFromWorld = noop;
    f.death.remove = [self](auto &a) { self->remove(a); };
    f.death.registerCorpse = noop;
    f.death.releaseReference = noop;
    f.death.releaseUpkeep = noop;
    f.death.releaseOccupancy = [self](auto &a, auto q) {
      cs::NativeCreatureGeometry g(self->c, self->p->native);
      g.reserve(a, q, false);
    };
    f.autonomy.geometry.hazardOffsets = m::originalHazardOffsets;
    f.autonomy.geometry.escapeOffsets = m::originalEscapeOffsets;
    f.autonomy.geometry.retreatOffsets = m::originalRetreatOffsets;
    f.autonomy.geometry.routeOffsets = m::originalRouteOffsets;
    f.autonomy.geometry.hazardous = [](const auto &, auto) { return false; };
    f.autonomy.geometry.objectReachable = [](const auto &, auto, auto) {
      return false;
    };
    f.autonomy.geometry.lineOfSight = [self](const auto &a, auto target) {
      if(!self->c.native.ownerVisibility)return true;
      return cs::nativePointSight(self->c,self->p->native,a.ai.slot,target);
    };
    f.autonomy.geometry.threatReaches = [](const auto &, const auto &) {
      return false;
    };
    f.autonomy.prepareInteraction = noop;
    f.autonomy.useInventory = [](auto &, auto &) {
      throw std::invalid_argument("encounter inventory unavailable");
    };
    f.autonomy.refreshInventory = [](unsigned) {};
    f.autonomy.orient = [](auto) {};
    f.autonomy.changeHealth = [self](auto &a, int n) {
      self->health(a, m::Entry::signedHealth, n);
    };
    f.region = [](auto &) { return 0; };
    f.ownerWork = noop;
    return f;
  }
};
} // namespace
Statistics statistics(const cs::Continuation &c) {
  Statistics s;
  unsigned *out[] = {&s.hits,       &s.damage, &s.stuns,
                     &s.knockbacks, &s.deaths, &s.cues};
  for (unsigned i = 0; i < 6; ++i)
    *out[i] = get(c.backend, statsOffset(c) + 4 * i);
  return s;
}
cs::NativeConsequenceFactory consequences(const Profile &p) {
  auto owned = std::make_shared<const Profile>(p);
  return [owned](auto &c) {
    auto fx = std::make_shared<Combat>(c, owned);
    return fx->services(fx);
  };
}
std::shared_ptr<const cs::Runtime> runtime(const Profile &p) {
  return std::make_shared<const cs::Runtime>(
      p.initial, p.bytes, cs::nativeProviders(p.native, consequences(p)));
}
game::CreatureRuleSession create(const Profile &p, unsigned seed) {
  return {runtime(p), p.bytes, seed, unsigned(p.initial.world.creatures.size())};
}
} // namespace mnm::encounter
