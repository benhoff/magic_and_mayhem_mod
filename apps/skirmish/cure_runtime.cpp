#include "cure_runtime.hpp"
#include "../../reconstruction/casting/cure_presentation.hpp"
#include "../cure-lab/session.hpp"
#include "health_rules.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::skirmish {
namespace g = game;
namespace r = reconstruction;
namespace c = r::casting;
namespace {
using namespace mnm::cure;
constexpr unsigned capacity = 8;
void require(bool yes, const char *why) {
  if (!yes)
    throw std::invalid_argument(why);
}
struct ActorStatus {
  g::Handle actor;
  unsigned status = 0, ongoingDamage = 0;
};
struct Stored {
  bool spheres = false;
  g::Bytes sphereData;
  CureAnimationDefinition definition;
  std::uint64_t tick = 0;
  unsigned resourceCounter = 0;
  CastLearningState learning;
  std::array<std::array<unsigned, 8>, 8> observed{};
  std::vector<ActorStatus> actors;
  std::array<OwnedCureEffect, capacity> effects;
  std::array<std::optional<g::Handle>, capacity> entities;
  c::CurePresentationState presentation;
};
struct Codec {
  game::Bytes bytes;
  bool reading = false;
  std::size_t at = 0;
  void field(unsigned &n) {
    if (reading) {
      require(at + 4 <= bytes.size(), "Cure battle envelope truncated");
      n = 0;
      for (unsigned i = 0; i < 4; ++i)
        n |= unsigned(bytes[at++]) << (i * 8);
    } else
      for (unsigned i = 0; i < 4; ++i)
        bytes.push_back(std::uint8_t(n >> (i * 8)));
  }
  void field(int &n) {
    auto u = unsigned(n);
    field(u);
    n = u <= 0x7fffffff ? int(u) : -1 - int(~u);
  }
  void field(bool &b) {
    unsigned n = b;
    field(n);
    require(n <= 1, "Cure battle Boolean");
    b = n != 0;
  }
  void field(std::optional<unsigned> &n) {
    unsigned u = n.value_or(UINT32_MAX);
    field(u);
    n = u == UINT32_MAX ? std::nullopt : std::optional<unsigned>(u);
  }
  template <class... T> void fields(T &...n) { (field(n), ...); }
  template <class T> void array(T &v) {
    for (auto &n : v)
      field(n);
  }
  template <class T, class F>
  void vector(std::vector<T> &v, unsigned limit, F item, bool exact = false) {
    unsigned size = unsigned(v.size());
    field(size);
    require(size <= limit && (!exact || size == limit),
            "Cure battle vector count");
    if (reading)
      v.resize(size);
    for (auto &x : v)
      item(x);
  }
  void blob(game::Bytes &v) {
    unsigned size = unsigned(v.size());
    field(size);
    require(size <= 8 * 1024 * 1024, "Cure battle embedded extent");
    if (reading) {
      require(size <= bytes.size() - at, "Cure battle embedded truncation");
      v.assign(bytes.begin() + at, bytes.begin() + at + size);
      at += size;
    } else
      bytes.insert(bytes.end(), v.begin(), v.end());
  }
};
void animationState(Codec &c, r::AnimationState &s) {
  unsigned pc = unsigned(s.pc);
  std::optional<unsigned> displayed =
      s.displayedRecord ? std::optional<unsigned>(unsigned(*s.displayedRecord))
                        : std::nullopt;
  c.fields(pc, displayed, s.active, s.delay, s.elapsed, s.repeats, s.breakFlag);
  s.pc = pc;
  s.displayedRecord =
      displayed ? std::optional<std::size_t>(*displayed) : std::nullopt;
}
void content(Codec &codec, Stored &s) {
  unsigned magic = s.spheres ? 0x32524243 : 0x31524243;
  codec.field(magic);
  require(magic == 0x31524243 || magic == 0x32524243, "Cure battle version");
  s.spheres = magic == 0x32524243;
  unsigned lo = unsigned(s.tick), hi = unsigned(s.tick >> 32);
  codec.fields(lo, hi);
  s.tick = std::uint64_t(hi) << 32 | lo;
  codec.field(s.resourceCounter);
  for (auto &v : s.learning.known) {
    unsigned n = v;
    codec.field(n);
    require(n <= 1, "Cure battle learning");
    v = std::uint8_t(n);
  }
  codec.vector(s.learning.notices, 20, [&](auto &v) { codec.array(v); });
  for (auto &row : s.observed)
    codec.array(row);
  codec.vector(s.actors, 32, [&](auto &a) {
    codec.fields(a.actor.slot, a.actor.generation, a.status, a.ongoingDamage);
  });
  auto &d = s.definition;
  codec.fields(d.entry.sequence, d.entry.assetIndex, d.entry.property,
               d.entry.opaque);
  codec.vector(d.sequence, 4096, [&](auto &row) {
    codec.fields(row.opcode, row.argument);
    codec.array(row.metadata);
  });
  require(!d.sequence.empty() && d.sequence.back().opcode == 6 &&
              d.entry.sequence < 4096 && d.entry.assetIndex < 64,
          "Cure battle animation domain");
  for (unsigned i = 0; i < capacity; ++i) {
    auto &e = s.effects[i];
    auto &v = e.effect;
    codec.fields(e.active, e.visible, e.generation, v.spell, v.type, v.source,
                 v.target, v.deliveryMode, v.attribution, v.position.x,
                 v.position.y, v.position.z);
    codec.array(v.contributions);
    codec.field(v.cachedActor);
    auto &h = s.entities[i];
    bool present = bool(h);
    codec.field(present);
    if (present) {
      auto n = h.value_or(g::Handle{});
      codec.fields(n.slot, n.generation);
      h = n;
    } else
      h.reset();
  }
  auto &p = s.presentation;
  codec.fields(p.width, p.height, p.layers, p.scan);
  require(p.width >= 4 && p.width <= 16 && p.height >= 4 && p.height <= 16 &&
              p.layers >= 2 && p.layers <= 32 && std::uint64_t(p.width)*p.height*p.layers<=4096,
          "Cure battle spatial dimensions");
  codec.vector(
      p.cells, p.width*p.height*p.layers,
      [&](auto &cell) {
        unsigned t = cell.terrain, h = cell.head, a = cell.otherA,
                 b = cell.otherB;
        codec.fields(t, h, a, b, cell.flags);
        require(t <= 65535 && h <= 65535 && a <= 65535 && b <= 65535,
                "Cure battle cell word");
        cell.terrain = std::uint16_t(t);
        cell.head = std::uint16_t(h);
        cell.otherA = std::uint16_t(a);
        cell.otherB = std::uint16_t(b);
      },
      true);
  codec.vector(
      p.records, capacity,
      [&](auto &record) {
        auto &v = record.placement;
        codec.fields(v.active, v.type);
        codec.array(v.parameters);
        codec.array(v.position);
        codec.array(v.units);
        codec.array(v.initialPosition);
        codec.array(v.sentinels);
        unsigned next = v.next;
        codec.fields(next, v.cell, v.initialized, record.reservation,
                     record.property, record.animationReset);
        require(next <= 65535, "Cure battle effect link");
        v.next = std::uint16_t(next);
        bool bound = bool(record.animation);
        codec.field(bound);
        if (bound) {
          r::AnimationState state;
          if (!codec.reading)
            state = record.animation->player.state();
          animationState(codec, state);
          if (codec.reading) {
            r::EffectAnimationBinding binding{
                74, d.entry, r::NoCdAnimationPlayer(d.sequence)};
            binding.player.restore(state);
            record.animation = std::move(binding);
          }
        } else
          record.animation.reset();
      },
      true);
  if (s.spheres) codec.blob(s.sphereData);
}
Stored decode(const g::Bytes &bytes) {
  require(bytes.size() <= 4 * 1024 * 1024, "Cure battle byte capacity");
  Codec codec;
  codec.reading = true;
  codec.bytes = bytes;
  Stored s;
  content(codec, s);
  require(codec.at == bytes.size(), "Cure battle trailing bytes");
  return s;
}
g::Bytes encode(Stored s) {
  Codec codec;
  content(codec, s);
  require(codec.bytes.size() <= 4 * 1024 * 1024, "Cure battle byte capacity");
  return codec.bytes;
}
const g::Fighter *fighter(const g::CombatState &s, g::Handle h) {
  for (const auto &f : s.fighters)
    if (f.actor == h)
      return &f;
  return nullptr;
}
unsigned index(const Stored &s, g::Handle h) {
  for (unsigned i = 0; i < s.actors.size(); ++i)
    if (s.actors[i].actor == h)
      return i;
  throw std::invalid_argument("Cure battle actor is not enrolled");
}
void enroll(Stored &s, const g::CombatSession &combat) {
  const auto fighters = combat.combat();
  for (const auto &f : fighters.fighters) {
    if (std::any_of(s.actors.begin(), s.actors.end(),
                    [&](const auto &a) { return a.actor == f.actor; }))
      continue;
    unsigned free = unsigned(s.actors.size());
    for (unsigned i = 0; i < s.actors.size(); ++i) {
      if (fighter(fighters, s.actors[i].actor))
        continue;
      const bool used =
          std::any_of(s.effects.begin(), s.effects.end(), [&](const auto &e) {
            return e.active &&
                   (e.effect.source == i || e.effect.target == i ||
                    e.effect.cachedActor == std::optional<unsigned>(i));
          });
      if (!used) {
        free = i;
        break;
      }
    }
    if (free < s.actors.size())
      s.actors[free] = {f.actor, 0, 0};
    else {
      require(s.actors.size() < 32, "Cure actor enrollment capacity");
      s.actors.push_back({f.actor, 0, 0});
    }
  }
}
CureSessionState project(const Stored &s, const g::CombatSession &combat,
                         const g::SkirmishState &meta) {
  CureSessionState core;
  core.tick = s.tick;
  core.cost = cureCost;
  core.magnitude = cureMagnitude;
  core.dimensions = {int(s.presentation.width), int(s.presentation.height), int(s.presentation.layers)};
  core.resourceCounter = s.resourceCounter;
  core.learning = s.learning;
  core.observed = s.observed;
  core.effects.assign(s.effects.begin(), s.effects.end());
  auto fighters = combat.combat();
  for (const auto &a : s.actors) {
    OwnedCureActor value;
    const auto *body = combat.world().find(a.actor);
    const auto *f = fighter(fighters, a.actor);
    value.active = body && f && f->health && !body->cleaned;
    value.health.health = f ? f->health : 0;
    value.maximumHealth = f ? f->stats.maximumHealth : 24;
    if (body) {
      value.position = {body->x, body->y, body->z};
      value.resource.owner = body->owner;
    }
    value.resource.kind = 1;
    for (const auto &w : meta.wizards)
      if (w.actor == a.actor) {
        value.resource.kind = 0;
        value.resource.value = w.mana * 256;
        value.resource.maximum = g::SkirmishRules::manaMaximum;
      }
    value.status.status = a.status;
    value.ongoingDamage = a.ongoingDamage;
    value.visible.fill(1);
    core.actors.push_back(value);
  }
  return core;
}
g::Bytes tag(unsigned slot, unsigned generation) {
  Codec c;
  c.fields(slot, generation);
  return c.bytes;
}
void validate(const Stored &s, const g::CombatSession &combat,
              const g::SkirmishState &meta,const sphere::Environment &environment) {
  require(combat.effects().identity == (s.spheres ? sphereBattleIdentity : cureBattleIdentity) &&
              combat.healthRules()->identity() == ordinaryHealthIdentity,
          "Cure battle runtime/health binding");
  for (const auto &order : meta.pending)
    require(s.spheres || order.kind != g::SkirmishOrderKind::sphere, "pending Sphere without bound runtime");
  for (unsigned i = 0; i < s.actors.size(); ++i) {
    require(s.actors[i].actor.generation != 0, "Cure battle actor generation");
    for (unsigned j = 0; j < i; ++j)
      require(!(s.actors[i].actor == s.actors[j].actor),
              "Cure battle duplicate actor");
  }
  auto hydrated = s;
  enroll(hydrated, combat);
  (void)CureSession(project(hydrated, combat, meta));
  c::validateCurePresentation(s.presentation);
  unsigned count = 0;
  for (unsigned i = 0; i < capacity; ++i) {
    const auto &e = s.effects[i];
    const auto &p = s.presentation.records.at(i);
    require(e.active == bool(s.entities[i]) && e.active == p.placement.active,
            "Cure battle effect activity");
    if (!e.active)
      continue;
    ++count;
    require(e.effect.source < s.actors.size() &&
                e.effect.target < s.actors.size(),
            "Cure battle actor index");
    const auto h = *s.entities[i];
    const auto *body = combat.world().find(h);
    auto target = s.actors[e.effect.target].actor;
    require(
        body && body->owner == 0 && body->type == 41 &&
            body->family == g::Family::missileEffect && !body->motion &&
            !body->cleaned && body->state == tag(i, e.generation) &&
            body->target == std::optional<g::Handle>(target) &&
            combat.world().find(target) && p.placement.type == e.effect.type &&
            p.placement.parameters[6] == e.effect.source &&
            p.placement.parameters[7] == e.effect.target &&
            p.placement.parameters[10] == e.effect.deliveryMode &&
            p.placement.sentinels[9] == e.effect.attribution &&
            p.placement.sentinels[10] == e.effect.contributions[0] &&
            p.placement.sentinels[11] == e.effect.contributions[1] &&
            body->x == e.effect.position.x && body->y == e.effect.position.y &&
            body->z == e.effect.position.z &&
            p.placement.position ==
                std::array<unsigned, 3>{unsigned(body->x), unsigned(body->y),
                                        unsigned(body->z)} &&
            (e.effect.type != 77 ||
             (p.animation && !p.animationReset &&
              p.property == s.definition.entry.property)),
        "Cure battle effect identity/spatial binding");
    const auto roster = combat.effects().entities;
    require(std::count(roster.begin(), roster.end(), h) == 1,
            "Cure battle missing effect roster");
    for (unsigned j = 0; j < i; ++j)
      require(!(s.entities[j] == s.entities[i]),
              "Cure battle duplicate effect identity");
  }
  if (s.spheres) { sphere::validate(s.sphereData, combat, meta, environment); count += unsigned(sphere::views(s.sphereData).size()); }
  require(count == combat.effects().entities.size(),
          "Cure battle effect roster count");
  if (meta.outcome != g::Outcome::running)
    require(count == 0, "terminal battle retains Cure");
}
g::CombatReport consequences(Stored &s, const CureSessionState &after,
                             g::CombatSession &combat, g::SkirmishState &meta) {
  g::CombatReport report;
  auto old = combat.combat();
  for (unsigned i = 0; i < s.actors.size(); ++i) {
    const auto h = s.actors[i].actor;
    const auto &a = after.actors.at(i);
    const auto *f = fighter(old, h);
    if (f && a.health.health != f->health) {
      require(f->health && a.health.health > f->health,
              "Cure battle cannot damage or resurrect");
      auto changed =
          combat.heal(h, a.health.health - f->health, meta.wizards[0].actor);
      require(fighter(combat.combat(), h)->health == a.health.health,
              "Cure battle health adapter mismatch");
      report.events.insert(report.events.end(), changed.events.begin(),
                           changed.events.end());
    }
    for (auto &w : meta.wizards)
      if (w.actor == h) {
        require(a.resource.value % 256 == 0 &&
                    a.resource.value <= g::SkirmishRules::manaMaximum * 256,
                "Cure battle resource units");
        w.mana = a.resource.value / 256;
      }
    s.actors[i].status = a.status.status;
    s.actors[i].ongoingDamage = a.ongoingDamage;
  }
  std::copy(after.effects.begin(), after.effects.end(), s.effects.begin());
  s.tick = after.tick;
  s.resourceCounter = after.resourceCounter;
  s.learning = after.learning;
  s.observed = after.observed;
  return report;
}
class Runtime final : public g::SkirmishEffects {
  CureAnimationDefinition definition_;
  g::Point dimensions_;
  bool spheres_;
  sphere::Environment sphereEnvironment_;

public:
  explicit Runtime(CureAnimationDefinition d,g::Point dimensions,bool spheres=false,sphere::Environment environment={})
    : definition_(std::move(d)),dimensions_(dimensions),spheres_(spheres),sphereEnvironment_(std::move(environment)) {
    require(dimensions.x>=4 && dimensions.x<=16 && dimensions.y>=4 && dimensions.y<=16 && dimensions.z>=2 && dimensions.z<=32 && std::uint64_t(dimensions.x)*dimensions.y*dimensions.z<=4096,"Cure runtime dimensions");
    require(!spheres_ || sphereEnvironment_.dimensions==dimensions_,"Sphere/Cure environment dimensions");
    r::NoCdAnimationPlayer player(definition_.sequence);
    player.start();
  }
  std::uint64_t identity() const override { return spheres_ ? sphereBattleIdentity : cureBattleIdentity; }
  g::Bytes initial() const override {
    Stored s;
    s.spheres = spheres_;
    if (spheres_) s.sphereData = sphere::initial(sphereEnvironment_);
    s.definition = definition_;
    s.presentation = c::makeCurePresentation(unsigned(dimensions_.x), unsigned(dimensions_.y), unsigned(dimensions_.z), capacity);
    return encode(s);
  }
  void validate(const g::CombatSession &combat,
                const g::SkirmishState &meta) const override {
    const auto stored=decode(combat.effects().data);
    require(combat.world().state().navigation && combat.world().state().navigation->dimensions==dimensions_ &&
                stored.presentation.width==unsigned(dimensions_.x) && stored.presentation.height==unsigned(dimensions_.y) && stored.presentation.layers==unsigned(dimensions_.z),"Cure mission spatial binding mismatch");
    require(stored.spheres==spheres_,"Cure/Sphere runtime profile mismatch");
    ::mnm::skirmish::validate(stored, combat, meta,sphereEnvironment_);
  }
  bool supports(g::Spell spell) const override { return spell==g::Spell::cure || (spheres_ && spell==g::Spell::sphere); }
  g::SkirmishRefusal castSpell(g::CombatSession &combat, g::SkirmishState &meta,
                              g::Spell spell, g::Handle source, g::Handle target) const override {
    if (spell==g::Spell::cure) return cast(combat,meta,source,target);
    if (spell!=g::Spell::sphere || !spheres_) return g::SkirmishRefusal::target;
    auto stored=decode(combat.effects().data);
    auto refusal=sphere::cast(stored.sphereData,combat,meta,source,target);
    if (refusal==g::SkirmishRefusal::none) {
      combat.updateEffects(encode(stored)); validate(combat,meta);
    }
    return refusal;
  }
  g::SkirmishRefusal cast(g::CombatSession &combat, g::SkirmishState &meta,
                          g::Handle source, g::Handle target) const override {
    auto stored = decode(combat.effects().data);
    enroll(stored, combat);
    auto core = project(stored, combat, meta);
    auto from = index(stored, source), to = index(stored, target);
    if (!(source == meta.wizards[0].actor) || !core.actors[from].active ||
        !core.actors[to].active || core.actors[to].resource.kind != 1 ||
        core.actors[to].resource.owner != 0)
      return g::SkirmishRefusal::target;
    if (meta.wizards[0].mana < cureCost)
      return g::SkirmishRefusal::mana;
    if (std::none_of(stored.effects.begin(), stored.effects.end(),
                     [](const auto &e) {
                       return !e.active && e.generation != UINT32_MAX;
                     }) ||
        std::none_of(combat.world().state().slots.begin(),
                     combat.world().state().slots.end(), [](const auto &slot) {
                       return !slot.entity && slot.generation;
                     }))
      return g::SkirmishRefusal::capacity;
    CureSession session(core);
    auto events = session.cast(from, to);
    auto candidate = combat;
    auto stagedMeta = meta;
    bool created = false;
    for (const auto &event : events)
      if (event.kind == CureSessionEvent::created) {
        created = true;
        const auto slot = event.actor;
        const auto &a = core.actors[from].position;
        const auto &b = core.actors[to].position;
        c::CastEffectDescriptor p{};
        std::fill_n(p.begin(), 10, UINT32_MAX);
        p[0] = unsigned(a.x * 32 + 16);
        p[1] = unsigned(a.y * 32 + 16);
        p[2] = unsigned(a.z * 16 + 8);
        p[3] = unsigned(b.x * 32 + 16);
        p[4] = unsigned(b.y * 32 + 16);
        p[5] = unsigned(b.z * 16 + 8);
        p[6] = from;
        p[7] = to;
        p[8] = 41;
        p[9] = 0;
        p[10] = 1;
        p[11] = UINT32_MAX;
        p[12] = 1;
        std::vector<c::CureCreator> creators;
        for (const auto &actor : core.actors)
          creators.push_back({actor.resource.owner, {}});
        c::createCurePresentation(stored.presentation, slot, p, creators, {});
        g::Entity e;
        e.family = g::Family::missileEffect;
        e.type = 41;
        e.owner = 0;
        e.x = b.x;
        e.y = b.y;
        e.z = b.z;
        e.target = target;
        e.state = tag(slot, session.state().effects[slot].generation);
        stored.entities[slot] = candidate.spawnEffect(std::move(e));
      }
    if (!created)
      return g::SkirmishRefusal::target;
    consequences(stored, session.state(), candidate, stagedMeta);
    candidate.updateEffects(encode(stored));
    validate(candidate, stagedMeta);
    combat = std::move(candidate);
    meta = std::move(stagedMeta);
    return g::SkirmishRefusal::none;
  }
  g::CombatReport step(g::CombatSession &combat, g::SkirmishState &meta,
                       g::TickInput) const override {
    auto stored = decode(combat.effects().data);
    enroll(stored, combat);
    auto core = project(stored, combat, meta);
    std::vector<CureAnimationInput> signals;
    for (unsigned i = 0; i < capacity; ++i)
      if (core.effects[i].active && core.effects[i].effect.type == 77) {
        const auto &target = core.actors.at(core.effects[i].effect.target);
        if (target.active && target.health.health) {
          c::moveCurePresentation(stored.presentation, i,
                                  {unsigned(target.position.x),
                                   unsigned(target.position.y),
                                   unsigned(target.position.z)});
          signals.push_back(
              {{i, core.effects[i].generation},
               unsigned(c::tickCurePresentation(stored.presentation, i))});
        }
      }
    CureSession session(core);
    auto events = session.advance(signals);
    for (const auto &event : events) {
      if (event.kind == CureSessionEvent::transition)
        c::bindCurePresentation(stored.presentation, event.actor,
                                stored.definition);
      if (event.kind == CureSessionEvent::released) {
        c::releaseCurePresentation(stored.presentation, event.actor,
                                   [](unsigned) { return false; });
        combat.releaseEffect(*stored.entities[event.actor]);
        stored.entities[event.actor].reset();
      }
    }
    auto report = consequences(stored, session.state(), combat, meta);
    for (unsigned i = 0; i < capacity; ++i)
      if (stored.entities[i]) {
        const auto &p = stored.effects[i].effect.position;
        combat.relocateEffect(*stored.entities[i], {p.x, p.y, p.z});
      }
    if (stored.spheres) {
      auto extra=sphere::step(stored.sphereData,combat);
      report.events.insert(report.events.end(),extra.events.begin(),extra.events.end());
    }
    combat.updateEffects(encode(stored));
    validate(combat, meta);
    return report;
  }
  void removed(g::CombatSession &combat, g::SkirmishState &,
               g::Handle actor) const override {
    auto s = decode(combat.effects().data);
    for (unsigned i = 0; i < capacity; ++i)
      if (s.entities[i]) {
        const auto &effect = s.effects[i].effect;
        if (s.actors.at(effect.target).actor == actor ||
            s.actors.at(effect.source).actor == actor) {
          c::releaseCurePresentation(s.presentation, i,
                                     [](unsigned) { return false; });
          combat.releaseEffect(*s.entities[i]);
          s.entities[i].reset();
          s.effects[i].active = false;
          s.effects[i].visible = false;
          s.effects[i].effect.attribution = 255;
          s.effects[i].effect.contributions.fill(UINT32_MAX);
        }
      }
    if (s.spheres) sphere::removed(s.sphereData,combat,actor);
    combat.updateEffects(encode(s));
  }
  void cancel(g::CombatSession &combat, g::SkirmishState &) const override {
    auto s = decode(combat.effects().data);
    for (unsigned i = 0; i < capacity; ++i)
      if (s.entities[i]) {
        c::releaseCurePresentation(s.presentation, i,
                                   [](unsigned) { return false; });
        combat.releaseEffect(*s.entities[i]);
        s.entities[i].reset();
        s.effects[i].active = false;
        s.effects[i].visible = false;
        s.effects[i].effect.attribution = 255;
        s.effects[i].effect.contributions.fill(UINT32_MAX);
      }
    if (s.spheres) sphere::cancel(s.sphereData,combat);
    combat.updateEffects(encode(s));
  }
};
} // namespace
std::shared_ptr<const g::SkirmishEffects>
cureRuntime(CureAnimationDefinition definition) {
  return cureRuntime(std::move(definition),g::Point{8,8,3});
}
std::shared_ptr<const g::SkirmishEffects>
cureRuntime(CureAnimationDefinition definition,g::Point dimensions) {
  return std::make_shared<Runtime>(std::move(definition),dimensions);
}
std::shared_ptr<const g::SkirmishEffects> cureRuntime() {return cureRuntime(g::Point{8,8,3});}
std::shared_ptr<const g::SkirmishEffects> cureRuntime(g::Point dimensions) {
  // Authored fixture timing/presentation, not copied or guessed installed ANI.
  CureAnimationDefinition d;
  d.entry = {0, 0, 2, 0};
  d.sequence = {{1, 2, {}}, {0, 0, {}}, {0, 1, {}},
                {0, 2, {}}, {0, 3, {}}, {6, 0, {}}};
  return cureRuntime(std::move(d),dimensions);
}
std::shared_ptr<const g::SkirmishEffects> battleRuntime(sphere::Environment environment) {
  auto old=cureRuntime(environment.dimensions);
  auto stored=decode(old->initial());
  return std::make_shared<Runtime>(stored.definition,environment.dimensions,true,std::move(environment));
}
std::vector<sphere::View> sphereViews(const g::CombatSession &combat) {
  if(combat.effects().identity!=sphereBattleIdentity) return {};
  return sphere::views(decode(combat.effects().data).sphereData);
}
std::shared_ptr<const g::SkirmishEffects>
resolveCureRuntime(const g::State &state) {
  const auto sections = g::decodeBattleSystems(state.systems);
  if (!sections.effects.identity)
    return {};
  require(sections.effects.identity == cureBattleIdentity || sections.effects.identity == sphereBattleIdentity,
          "unknown battle effect runtime");
  require(bool(state.navigation),"Cure navigation binding required");
  auto stored=decode(sections.effects.data);
  require(stored.spheres==(sections.effects.identity==sphereBattleIdentity),"battle payload/profile mismatch");
  auto environment=stored.spheres?sphere::environment(stored.sphereData):sphere::Environment{};
  if(stored.spheres && state.map==g::arenaMission().map)
    require(sphere::sameEnvironment(environment,sphere::Environment{}),"Arena Sphere environment mismatch");
  return std::make_shared<Runtime>(stored.definition,state.navigation->dimensions,stored.spheres,std::move(environment));
}
std::vector<CureView> cureViews(const g::CombatSession &combat) {
  if (!combat.effects().identity)
    return {};
  require(combat.effects().identity == cureBattleIdentity || combat.effects().identity == sphereBattleIdentity,
          "unknown battle effect view");
  auto s = decode(combat.effects().data);
  std::vector<CureView> result;
  for (unsigned i = 0; i < capacity; ++i)
    if (s.entities[i]) {
      const auto &e = s.effects[i].effect;
      result.push_back({*s.entities[i],
                        s.actors.at(e.target).actor,
                        {e.position.x, e.position.y, e.position.z}});
    }
  return result;
}
} // namespace mnm::skirmish
