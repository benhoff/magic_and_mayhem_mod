#include "wizards.hpp"
#include "../../reconstruction/creature-ai/ownership_helpers.hpp"
#include "../../reconstruction/creature-ai/wizard_spell_control.hpp"
#include "../../reconstruction/rendering/creature_footprint.hpp"
#include "../../reconstruction/rendering/effect_motion_initializer.hpp"
#include "../command-session/native_geometry.hpp"
#include "../command-session/native_wizards.hpp"
#include <algorithm>
#include <climits>
#include <cstring>
#include <stdexcept>
namespace mnm::encounter {
namespace {
namespace m = cs::model;
namespace r = reconstruction;
namespace cast = reconstruction::casting;
void need(bool b, const char *why) {
  if (!b)
    throw std::invalid_argument(why);
}
unsigned increment(unsigned &n) {
  need(n != UINT32_MAX, "wizard counter exhausted");
  return ++n;
}
int wrap(int n, int period) {
  n %= period;
  return n < 0 ? n + period : n;
}
bool living(const m::AutonomousCreature &a) {
  return a.ai.active && a.ai.health;
}
class WizardFrame {
  cs::Continuation &c;
  const Profile &profile;
  m::OwnedCreatureCommands &commands;
  const cs::NativeConsequences &fx;
  cs::NativeWizardState &s;
  cs::NativeCreatureGeometry geometry;
  std::vector<unsigned> costs;
  std::vector<assets::AnimationRecord> sequence(unsigned asset,
                                                unsigned index) const {
    const auto &a = profile.native.animation.at(asset);
    const auto first = a.starts.at(index), end = a.starts.at(index + 1);
    return {a.records.begin() + first, a.records.begin() + end};
  }
  void startAnimation(cs::NativeWizardEffect &e, unsigned asset,
                      unsigned index) {
    r::NoCdAnimationController player(sequence(asset, index));
    player.start();
    e.animationAsset = asset;
    e.animationSequence = index;
    e.animation = player.state();
  }
  int animate(cs::NativeWizardEffect &e) {
    r::NoCdAnimationController player(
        sequence(e.animationAsset, e.animationSequence));
    player.restore(e.animation);
    const int result = player.tick();
    e.animation = player.state();
    return result;
  }
  cs::NativeWizardController &controller(unsigned actor) {
    const auto p =
        std::find_if(s.controllers.begin(), s.controllers.end(),
                     [&](const auto &v) { return v.actor == actor; });
    need(p != s.controllers.end(), "wizard controller missing");
    return *p;
  }
  void mana(unsigned actor, unsigned delta) {
    auto &value = s.admission.resource.at(actor);
    const auto maximum = controller(actor).maximumMana * 256u;
    const unsigned next = value + delta;
    // 507f00's ordinary bounded resource admission is owned by this host.
    value = next & 0x80000000u ? 0 : std::min(maximum, next);
  }
  int wait(unsigned actor, unsigned spell, unsigned count) {
    const auto &a = c.world.creatures.at(actor);
    m::WizardMana input{a.ai.kind, unsigned(a.ai.owner),
                        s.admission.resource.at(actor),
                        controller(actor).maximumMana, a.inventory};
    std::vector<m::WizardManaProducer> producers;
    for (const auto &v : s.controllers) {
      const auto &p = c.world.creatures.at(v.actor);
      producers.push_back(
          {unsigned(p.ai.owner), p.ai.active, living(p), v.producing});
    }
    return m::wizardSpellWait(input, costs.at(spell), count, producers,
                              int(s.regenerationRate),
                              int(s.regenerationPeriod));
  }
  void useMana(unsigned actor) {
    for (auto &item : c.world.creatures.at(actor).inventory)
      if (item.kind == 19 && item.available && item.count) {
        mana(actor, item.magnitude * 256u);
        --item.count;
        if (!item.count)
          item.available = 0;
        increment(s.manaUses);
        return;
      }
  }
  unsigned distance(unsigned a, unsigned b) const {
    return unsigned(m::creatureDistance(c.world.creatures.at(a).ai.position,
                                        c.world.creatures.at(b).ai.position,
                                        c.world.dimensions));
  }
  bool inRange(unsigned actor, unsigned spell, unsigned target) const {
    return distance(actor, target) <= unsigned(s.spells.at(spell).range);
  }
  int prepare(unsigned actor, unsigned spell, unsigned target) {
    if (!living(c.world.creatures.at(target)))
      return 1;
    auto &a = c.world.creatures.at(actor);
    if (a.ai.motion || a.ai.behavior == 8 || a.mode || a.form || a.blockDE3 ||
        a.block71c || a.block723)
      return 1;
    if (a.ai.engaged) {
      commands.reset(a);
      return 1;
    }
    if (inRange(actor, spell, target))
      return 0;
    if (!a.ai.motion && a.ai.action == 15)
      commands.move(a, c.world.creatures.at(target).ai.position);
    return 1;
  }
  std::optional<m::Cell> summonPoint(unsigned actor, unsigned slot) {
    const auto origin = c.world.creatures.at(actor).ai.position;
    for (int radius = 2; radius <= 5; ++radius)
      for (int x = -radius; x <= radius; ++x)
        for (int y = -radius; y <= radius; ++y) {
          if (std::abs(x) != radius && std::abs(y) != radius)
            continue;
          m::Cell q{wrap(origin.x + x, c.world.dimensions.x),
                    wrap(origin.y + y, c.world.dimensions.y), origin.z};
          const auto &a = c.world.creatures.at(slot);
          if (geometry.legal(a, q) && geometry.ground(a, q))
            return q;
        }
    return std::nullopt;
  }
  bool summon(unsigned actor, m::Cell aim) {
    const auto owner = c.world.creatures.at(actor).ai.owner;
    auto at = std::find_if(
        c.world.creatures.begin(), c.world.creatures.end(), [&](const auto &a) {
          return a.ai.type == 17 && !a.ai.active && !a.ai.health &&
                 a.ai.owner == owner && !s.usedSummonSlots.at(a.ai.slot);
        });
    if (at == c.world.creatures.end())
      return false;
    const auto point = summonPoint(actor, at->ai.slot);
    if (!point)
      return false;
    (void)aim;
    at->ai.health = at->ai.maximumHealth;
    at->ai.active = true;
    s.usedSummonSlots.at(at->ai.slot) = true;
    at->ai.position = *point;
    at->ai.home = at->destination = *point;
    c.execution.at(at->ai.slot).roamHome = *point;
    geometry.snap(*at, *point);
    geometry.reserve(*at, *point, true);
    commands.reset(*at);
    increment(s.summons);
    return true;
  }
  r::EffectTerrainCatalog terrainCatalog() const {
    r::EffectTerrainCatalog catalog;
    for (const auto &t : profile.native.terrain) {
      std::array<std::uint8_t,32> bitmap{};
      std::memcpy(bitmap.data(), &t, 32);
      catalog.push_back(bitmap);
    }
    return catalog;
  }
  r::EffectCreatureWorld collisionWorld() const {
    r::EffectCreatureWorld world;
    for (const auto &cell : c.native.cells)
      world.cells.push_back(cell.occupant == 0xffff ? r::NoEffect : unsigned(cell.occupant-1));
    for (const auto &a : c.world.creatures) {
      r::EffectCollisionCreature out;
      out.id=a.ai.slot;
      out.status=living(a)?a.ai.type:27;
      out.units={unsigned(a.finePosition.x),unsigned(a.finePosition.y),unsigned(a.finePosition.z)};
      out.footprint=r::initializeCreatureFootprint(a.height,a.width==2);
      world.creatures.push_back(out);
    }
    return world;
  }
  void launch(cs::NativeWizardEffect &e, const cast::CastEffectDescriptor &d,
              r::EffectPlacementPool &pool) {
    const auto dims=c.world.dimensions;
    auto input=d;
    input[6]=UINT32_MAX;
    input[9]=unsigned(s.spells.at(73).speed);
    pool.place(0,3,input);
    e.placement=pool.records()[0];
    e.placement.parameters[6]=d[6];
    const auto &normalized=e.placement.parameters;
    const std::array<unsigned,3> from{normalized[0],normalized[1],normalized[2]},
                                 to{normalized[3],normalized[4],normalized[5]};
    std::array<unsigned,3> startup{};
    r::initializeEffectMotionRecord(e.placement,e.movement.movement,startup,from,to,
        unsigned(dims.x),unsigned(dims.y),unsigned(dims.z),pool.cells());
    e.movement.movement.previousPosition=e.placement.position;
    pool.records()[0]=e.placement;
  }
  // Native policy: only spend on a current-position shot whose first physical
  // collision is its intended opponent. Original44a500 scoring/prediction is
  // independent; this uses the same owned terrain/footprints as delivery.
  bool clearShot(unsigned source, unsigned target) {
    const auto &a=c.world.creatures.at(source), &b=c.world.creatures.at(target);
    const auto offset=s.admission.muzzle.at(a.ai.type).at(c.actions.motion.at(source).direction);
    cast::CastEffectDescriptor d{};
    d[0]=unsigned(a.finePosition.x+a.aimXY+offset.x);
    d[1]=unsigned(a.finePosition.y+a.aimXY+offset.y);
    d[2]=unsigned(a.finePosition.z+a.aimZ+offset.z);
    d[3]=unsigned(b.finePosition.x+b.aimXY);
    d[4]=unsigned(b.finePosition.y+b.aimXY);
    d[5]=unsigned(b.finePosition.z+b.aimZ);
    d[6]=source;d[7]=target;d[8]=73;
    const auto dims=c.world.dimensions;
    r::EffectPlacementPool pool(unsigned(dims.x),unsigned(dims.y),unsigned(dims.z),1);
    for(unsigned i=0;i<pool.cells().size();++i){
      pool.cells()[i].terrain=c.native.cells[i].kind;
      pool.cells()[i].flags=unsigned(c.native.cells[i].links)|(unsigned(c.native.cells[i].flags)<<16);
    }
    cs::NativeWizardEffect e;
    launch(e,d,pool);
    const auto world=collisionWorld();const auto catalog=terrainCatalog();
    for(unsigned frame=0;frame<unsigned(s.spells[73].duration);++frame){
      const auto code=r::transitionEffectCreatureCatalog(pool,0,e.movement,unsigned(dims.x),
          unsigned(dims.y),unsigned(dims.z),world,{},true,catalog);
      if(code==0)return pool.records()[0].parameters[7]==target;
      if(code!=3)return false;
    }
    return false;
  }
  void createEffect(unsigned slot, unsigned type,
                    const cast::CastEffectDescriptor &d) {
    auto &e = s.effects.at(slot);
    need(!e.active, "wizard effect already active");
    const unsigned generation = e.generation;
    e = {};
    e.generation = generation;
    increment(e.generation);
    e.active = true;
    e.spell = d[8];
    if (e.spell == 41) {
      need(type == 11, "wizard Cure delivery family");
      e.cure.type = type;
      e.cure.source = d[6];
      e.cure.target = d[7];
      e.cure.deliveryMode = d[10];
      e.cure.attribution = unsigned(c.world.creatures.at(d[6]).ai.owner);
      e.cure.position = {int(d[3]), int(d[4]), int(d[5])};
      startAnimation(e, s.cureAsset, s.cureSequence);
      return;
    }
    need(e.spell == 73 && type == 3, "wizard projectile delivery family");
    e.projectile.source = e.projectile.attacker = d[6];
    e.projectile.target = d[7];
    e.projectile.attribution = unsigned(c.world.creatures.at(d[6]).ai.owner);
    e.projectile.remaining = unsigned(s.spells.at(73).duration);
    const auto dims = c.world.dimensions;
    r::EffectPlacementPool pool(unsigned(dims.x), unsigned(dims.y),
                                unsigned(dims.z), 1);
    for (unsigned i = 0; i < pool.cells().size(); ++i)
      pool.cells()[i].terrain = c.native.cells.at(i).kind;
    launch(e, d, pool);
    startAnimation(e, s.projectileAsset, s.projectileSequence);
  }
  void castSpell(unsigned actor, unsigned spell, unsigned target, m::Cell aim) {
    const auto before = s.casts;
    if (spell == 73 && !clearShot(actor,target)) {
      increment(s.refusedCasts);
      return;
    }
    s.admission.direction.at(actor) = c.actions.motion.at(actor).direction;
    cast::CastDescriptor descriptor{};
    descriptor[0] = spell;
    descriptor[7] = actor;
    descriptor[8] = spell == 17 ? UINT32_MAX : target;
    descriptor[4] = unsigned(aim.x * 32 + 16);
    descriptor[5] = unsigned(aim.y * 32 + 16);
    descriptor[6] = unsigned(aim.z * 16 + 8);
    descriptor[9] = 1;
    descriptor[10] = UINT32_MAX;
    cast::CastAdmissionServices lower;
    lower.pool = [&]() -> std::optional<unsigned> {
      for (unsigned i = 0; i < s.effects.size(); ++i)
        if (!s.effects[i].active)
          return i;
      return std::nullopt;
    };
    lower.initializePath = [] {};
    lower.currentProfile = [&] {
      return std::optional<unsigned>(unsigned(c.world.localOwner));
    };
    lower.visibility = [](auto &, unsigned, unsigned) {};
    lower.containsPoint = [](const auto &, auto) { return false; };
    lower.chargeable = [](const auto &a) { return m::isWizardKind(a.ai.kind); };
    lower.ownerMessage = [](const auto &) { return false; };
    lower.resource = [&](auto &a, unsigned delta) { mana(a.ai.slot, delta); };
    lower.insufficientOwnerMessage = [] {};
    lower.cue = [](unsigned, auto) {};
    lower.castCue = [](unsigned, auto, unsigned) {};
    lower.path = [&](auto, auto, auto &) {
      return inRange(actor, spell, target);
    };
    lower.secondary = [&](unsigned slot, unsigned type, const auto &effect) {
      if (spell == 17) {
        if (!summon(actor, aim)) {
          mana(actor, costs[17] * 256u);
          return;
        }
      } else
        createEffect(slot, type, effect);
      increment(s.casts);
    };
    lower.adjust = [](unsigned, const auto &, auto &) {
      throw std::invalid_argument("wizard relocation unsupported");
      return false;
    };
    lower.relocate = [](auto &, auto, bool) {
      throw std::invalid_argument("wizard relocation unsupported");
    };
    lower.areaEffect = [](auto &, unsigned, unsigned) {
      throw std::invalid_argument("wizard area effect unsupported");
    };
    cast::admitCast(descriptor, c.world, s.admission, lower);
    if (s.casts == before)
      increment(s.refusedCasts);
  }
  void select(cs::NativeWizardController &v) {
    int budget = int(s.planningAllocation);
    m::OwnerTaskSelectionServices lower;
    lower.root = true;
    lower.count = 4;
    lower.prepare = [&](unsigned) {
      v.cursor = 0;
      v.candidate.reset();
    };
    lower.next = [&](unsigned family, int &remaining) -> std::optional<float> {
      if (family == 0) {
        if (v.cursor++)
          return std::nullopt;
        return -1.f;
      }
      const unsigned spell = family == 1 ? 41 : family == 2 ? 73 : 17;
      if (!v.learned[spell] || wait(v.actor, spell, 1) == INT_MAX)
        return std::nullopt;
      while (remaining > 0 && v.cursor < c.world.creatures.size()) {
        const unsigned id = v.cursor++;
        --remaining;
        const auto &a = c.world.creatures.at(id);
        const auto &wizard = c.world.creatures.at(v.actor);
        if (!living(a))
          continue;
        const bool allied = c.world.relation.at(unsigned(wizard.ai.owner))
                                .at(unsigned(a.ai.owner)) != 0;
        cs::NativeWizardPlan p;
        p.spell = spell;
        p.target = id;
        p.stand = wizard.ai.position;
        p.aim = a.ai.position;
        if (spell == 41) {
          if (!allied || a.block7c || a.ai.health >= a.ai.maximumHealth ||
              (a.ai.health >= a.ai.maximumHealth / 2 &&
               a.ai.maximumHealth - a.ai.health <
                   unsigned(s.spells[41].magnitude)))
            continue;
          p.score =
              m::isWizardKind(a.ai.kind) && a.ai.health < a.ai.maximumHealth / 2
                  ? 1.f
                  : std::min(.95f, float(a.ai.maximumHealth - a.ai.health) /
                                       float(a.ai.maximumHealth));
        } else if (spell == 73) {
          if (allied || !clearShot(v.actor,id))
            continue;
          p.score = .45f + .25f / (1.f + float(distance(v.actor, id)));
        } else {
          if (id != v.actor)
            continue;
          unsigned population = 0, enemies = 0, free = 0;
          for (const auto &member : c.world.creatures) {
            if (member.ai.owner == wizard.ai.owner && member.ai.type == 17) {
              population += living(member);
              free += !member.ai.active && !member.ai.health &&
                      !s.usedSummonSlots[member.ai.slot];
            } else if (living(member))
              ++enemies;
          }
          if (!free || population >= 3 || !enemies)
            continue;
          p.score = .8f;
        }
        v.candidate = p;
        return p.score;
      }
      return std::nullopt;
    };
    lower.save = [&](unsigned family) {
      if (family)
        v.best = v.candidate;
      else
        v.best.reset();
    };
    lower.finish = [] {};
    m::advanceOwnerTaskSelection(v.selection, budget, s.random, lower);
    if (v.selection.reset) {
      increment(s.decisions);
      if (v.best) {
        auto p = *v.best;
        p.serial = s.nextSerial;
        increment(s.nextSerial);
        p.started = s.tick;
        unsigned life = 10;
        if (p.spell == 73) {
          const auto &target = c.world.creatures.at(p.target);
          life = (unsigned(target.aimZ) - unsigned(target.finePosition.z) +
                  unsigned(c.world.dimensions.z) * 16u) /
                     12u +
                 unsigned(s.spells[73].duration) / 2u + 5u;
        } else if (p.spell == 41) {
          r::NoCdAnimationController player(
              sequence(s.cureAsset, s.cureSequence));
          player.start();
          unsigned frames = 0;
          do {
            need(frames < 65536, "wizard Cure animation duration");
            ++frames;
          } while (player.tick() == 0);
          life = frames + 5;
        }
        p.expires = s.tick + life;
        v.active = p;
      }
      v.best.reset();
      v.candidate.reset();
      v.cursor = 0;
      v.selection = {};
      v.nextThink = s.tick + 1;
    }
  }
  void execute(cs::NativeWizardController &v) {
    auto &p = *v.active;
    m::WizardTargetTaskServices lower;
    lower.wait = [&](unsigned a, unsigned spell, unsigned count) {
      return wait(a, spell, count);
    };
    lower.useMana = [&](unsigned a) { useMana(a); };
    lower.prepare = [&](unsigned a, unsigned spell, unsigned target) {
      return prepare(a, spell, target);
    };
    lower.cast = [&](unsigned a, unsigned spell, unsigned target) {
      castSpell(a, spell, target, c.world.creatures.at(target).ai.position);
    };
    lower.blocked = [](unsigned) { return false; };
    bool continuing = true;
    if (p.spell == 41) {
      m::WizardTargetTask task{41, v.actor, p.target, p.issued};
      continuing = m::executeWizardTargetTask(
          task, c.world, s.admission.resource, costs, lower);
      p.issued = task.issued;
    } else if (p.spell == 73) {
      m::WizardProjectileTask task{
          {73, v.actor, p.target, p.issued}, p.stand, {0, 0, p.stand}};
      continuing = m::executeWizardProjectileTask(
          task, c.world, s.admission.resource, costs, lower,
          [&](unsigned actor, const auto &after) {
            auto &a = c.world.creatures.at(actor);
            if (after.command == 2 && !(a.ai.position == after.destination) &&
                !a.ai.motion)
              commands.move(a, after.destination);
          });
      p.issued = task.cast.issued;
    } else if (!p.issued && wait(v.actor, 17, 1) == 0 &&
               prepare(v.actor, 17, v.actor) == 0) {
      castSpell(v.actor, 17, v.actor, p.aim);
      p.issued = true;
    }
    if (!continuing || (p.spell == 17 && p.issued)) {
      v.active.reset();
      increment(s.retirements);
      v.nextThink = s.tick + 1;
    }
  }
  void cure(cs::NativeWizardEffect &e) {
    if (e.cure.type == 77 && animate(e) != 1)
      return;
    std::vector<cast::CureActor> actors;
    for (const auto &a : c.world.creatures) {
      cast::CureActor out;
      out.health = a.ai.health;
      out.blocked = a.block7c;
      out.status = c.maintenance.at(a.ai.slot).status114;
      out.ongoingDamage = c.maintenance.at(a.ai.slot).status8d5;
      out.resource = s.admission.resource.at(a.ai.slot);
      out.visible = a.ai.visible;
      actors.push_back(out);
    }
    cast::CureServices lower;
    lower.health = [&](auto &a, int amount, unsigned owner, const auto &,
                       unsigned) {
      const auto id = unsigned(&a - actors.data());
      const unsigned before = a.health;
      fx.maintenance.health(c.world.creatures.at(id), amount, int(owner), true);
      a.health = c.world.creatures.at(id).ai.health;
      if (a.health > before)
        increment(s.heals);
    };
    lower.cleanse = [&](auto &a) {
      auto &state = c.maintenance.at(unsigned(&a - actors.data()));
      state.status114 = 0;
      a.status = 0;
    };
    lower.cue = [](unsigned, auto) {};
    lower.feedback = [](auto &, auto &) {};
    lower.transition = [&](auto &effect, unsigned type) {
      effect.type = type;
      startAnimation(e, s.cureAsset, s.cureSequence);
    };
    lower.chargeable = [&](const auto &a) {
      return m::isWizardKind(
          c.world.creatures.at(unsigned(&a - actors.data())).ai.kind);
    };
    lower.resource = [&](auto &a, unsigned delta) {
      const auto id = unsigned(&a - actors.data());
      mana(id, delta);
      a.resource = s.admission.resource[id];
    };
    lower.release = [&](auto &) { e.active = false; };
    const auto before = e.cure.type;
    cast::applyCureEffect(
        e.cure, actors, {costs[41], s.spells[41].magnitude, c.world.localOwner},
        lower);
    if (before == 77 && !actors.at(e.cure.target).blocked) {
      c.maintenance.at(e.cure.target).status8d5 =
          actors.at(e.cure.target).ongoingDamage;
      e.active = false;
    }
  }
  void projectiles() {
    const auto dims = c.world.dimensions;
    const auto catalog = terrainCatalog();
    const auto actors = collisionWorld();
    r::EffectPlacementPool pool(unsigned(dims.x), unsigned(dims.y),
                                unsigned(dims.z), s.effectCapacity);
    for (unsigned i = 0; i < pool.cells().size(); ++i) {
      pool.cells()[i].terrain = c.native.cells[i].kind;
      pool.cells()[i].flags = unsigned(c.native.cells[i].links) |
                              (unsigned(c.native.cells[i].flags) << 16);
    }
    for (unsigned i = 0; i < s.effects.size(); ++i)
      if (s.effects[i].active && s.effects[i].spell == 73) {
        const auto &saved = s.effects[i].placement;
        auto input = saved.parameters;
        input[6] = UINT32_MAX;
        for (unsigned k = 0; k < 3; ++k)
          input[k] = input[k + 3] = saved.units[k];
        pool.place(i, 3, input);
        const auto next = pool.records()[i].next;
        pool.records()[i] = saved;
        pool.records()[i].next = next;
      }
    for (unsigned i = 0; i < s.effects.size(); ++i) {
      auto &e = s.effects[i];
      if (!e.active || e.spell != 73)
        continue;
      std::vector<cast::ProjectileActor> targets;
      for (const auto &a : c.world.creatures)
        targets.push_back(
            {a.ai.health, a.ai.recent, unsigned(a.ai.age), a.ai.visible});
      cast::ProjectileServices lower;
      lower.motion = [&](auto &effect) {
        const auto result = r::transitionEffectCreatureCatalog(
            pool, i, e.movement, unsigned(dims.x), unsigned(dims.y),
            unsigned(dims.z), actors, {}, true, catalog);
        e.placement = pool.records()[i];
        if (result == 0)
          effect.target = e.placement.parameters[7];
        return result;
      };
      lower.damage = [&](auto &a, int amount, unsigned owner, const auto &) {
        const auto id = unsigned(&a - targets.data());
        fx.damage(c.world.creatures.at(id), amount, int(owner),
                  e.projectile.attacker);
        a.health = c.world.creatures.at(id).ai.health;
        increment(s.impacts);
      };
      lower.bounce = [&](auto &, bool) { e.active = false; increment(s.bounces); };
      lower.release = [&](auto &) { e.active = false; increment(s.expirations); };
      lower.animationTick = [&] { return animate(e); };
      lower.animationRestart = [&] {
        startAnimation(e, s.projectileAsset, s.projectileSequence);
      };
      lower.visibleDamage = [](auto &) {};
      lower.startAudio = [](auto &effect) { effect.audio = 1; };
      lower.followAudio = [](auto &) {};
      cast::updateProjectile(e.projectile, targets, s.spells[73].magnitude,
                             c.world.localOwner, lower);
      if (e.projectile.cachedActor) {
        const auto id = *e.projectile.cachedActor;
        c.world.creatures.at(id).ai.recent = targets[id].recentAttacker;
        c.world.creatures.at(id).ai.age = targets[id].recentTime;
        // Native bounded delivery releases a resolved projectile; original
        // impact visual/bounce lifetime remains a separate policy boundary.
        e.active = false;
      }
    }
  }

public:
  WizardFrame(cs::Continuation &state, const Profile &p,
              m::OwnedCreatureCommands &cmd,
              const cs::NativeConsequences &effects)
      : c(state), profile(p), commands(cmd), fx(effects), s(*c.native.wizards),
        geometry(c, p.native) {
    for (const auto &v : s.admission.spells)
      costs.push_back(v.cost);
    for (const auto &e : s.effects) if (e.active) {
      r::NoCdAnimationController player(sequence(e.animationAsset,e.animationSequence));
      player.restore(e.animation);
    }
  }
  void run() {
    need(s.tick != UINT32_MAX && s.tick + 1 == c.native.tick,
         "native wizard frame clock");
    s.tick = c.native.tick;
    for (auto &v : s.controllers) {
      auto &a = c.world.creatures.at(v.actor);
      if (!living(a)) {
        if (v.active) {
          v.active.reset();
          increment(s.retirements);
        }
        v.best.reset();
        v.candidate.reset();
        v.selection = {};
        v.cursor = 0;
        continue;
      }
      if (s.tick % s.regenerationPeriod == 0) {
        unsigned producers = 0;
        for (const auto &other : s.controllers)
          if (other.producing && living(c.world.creatures.at(other.actor)) &&
              c.world.creatures.at(other.actor).ai.owner == a.ai.owner)
            ++producers;
        if (producers) {
          mana(v.actor, s.regenerationRate * producers * 256u);
          increment(s.regeneration);
        }
      }
      if (v.active && s.tick >= v.active->expires) {
        v.active.reset();
        increment(s.retirements);
      }
      if (!v.active && s.tick >= v.nextThink)
        select(v);
      if (v.active)
        execute(v);
    }
    for (auto &e : s.effects)
      if (e.active && e.spell == 41)
        cure(e);
    if (std::any_of(s.effects.begin(), s.effects.end(),
                    [](const auto &e) { return e.active && e.spell == 73; }))
      projectiles();
    cs::validateNativeWizards(c);
  }
};
} // namespace
void advanceWizardWorld(cs::Continuation &c, const Profile &p,
                        m::OwnedCreatureCommands &commands,
                        const cs::NativeConsequences &fx) {
  WizardFrame(c, p, commands, fx).run();
}
} // namespace mnm::encounter
