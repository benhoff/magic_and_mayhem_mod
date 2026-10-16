#include "session.hpp"
#include <algorithm>
#include <stdexcept>

namespace mnm::cure {
namespace r = mnm::reconstruction;
namespace c = mnm::reconstruction::casting;
namespace {
void require(bool yes, const char *why) {
  if (!yes)
    throw std::invalid_argument(why);
}
struct Frame {
  CureSessionState core;
  CureAnimationDefinition definition;
  CurePresentationState presentation;
  std::vector<game::Handle> actors;
  std::vector<std::optional<game::Handle>> effects;
};
struct Codec {
  game::Bytes bytes;
  bool reading = false;
  std::size_t at = 0;
  void field(unsigned &n) {
    if (reading) {
      require(at + 4 <= bytes.size(), "Cure World envelope truncated");
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
    require(n <= 1, "Cure World Boolean");
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
            "Cure World vector count");
    if (reading)
      v.resize(size);
    for (auto &x : v)
      item(x);
  }
  void blob(game::Bytes &v) {
    unsigned size = unsigned(v.size());
    field(size);
    require(size <= 8 * 1024 * 1024, "Cure World embedded extent");
    if (reading) {
      require(size <= bytes.size() - at, "Cure World embedded truncation");
      v.assign(bytes.begin() + at, bytes.begin() + at + size);
      at += size;
    } else
      bytes.insert(bytes.end(), v.begin(), v.end());
  }
};
game::Bytes effectTag(unsigned slot, unsigned generation) {
  Codec c;
  c.fields(slot, generation);
  return c.bytes;
}
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
void content(Codec &codec, Frame &f) {
  unsigned magic = 0x444c5743, version = 1;
  codec.fields(magic, version);
  require(magic == 0x444c5743 && version == 1, "Cure World envelope version");
  game::Bytes core;
  if (!codec.reading)
    core = CureSession(f.core).checkpoint();
  codec.blob(core);
  if (codec.reading)
    f.core = CureSession::restore(core).state();
  codec.vector(
      f.actors, unsigned(f.core.actors.size()),
      [&](auto &h) { codec.fields(h.slot, h.generation); }, true);
  codec.vector(
      f.effects, unsigned(f.core.effects.size()),
      [&](auto &h) {
        bool present = bool(h);
        codec.field(present);
        if (present) {
          game::Handle value = h.value_or(game::Handle{});
          codec.fields(value.slot, value.generation);
          h = value;
        } else
          h.reset();
      },
      true);
  auto &d = f.definition;
  codec.fields(d.entry.sequence, d.entry.assetIndex, d.entry.property,
               d.entry.opaque);
  codec.vector(d.sequence, 65536, [&](auto &row) {
    codec.fields(row.opcode, row.argument);
    codec.array(row.metadata);
  });
  require(!d.sequence.empty() && d.sequence.back().opcode == 6 &&
              d.entry.sequence < 4096 && d.entry.assetIndex < 64 &&
              std::uint64_t(d.sequence.size()) * 44 * f.core.effects.size() <=
                  16 * 1024 * 1024,
          "Cure World animation storage domain");
  auto &p = f.presentation;
  codec.fields(p.width, p.height, p.layers, p.scan);
  codec.vector(p.cells, 128 * 128 * 32, [&](auto &cell) {
    unsigned terrain = cell.terrain, head = cell.head, a = cell.otherA,
             b = cell.otherB;
    codec.fields(terrain, head, a, b, cell.flags);
    require(terrain <= 65535 && head <= 65535 && a <= 65535 && b <= 65535,
            "Cure World cell word");
    cell.terrain = std::uint16_t(terrain);
    cell.head = std::uint16_t(head);
    cell.otherA = std::uint16_t(a);
    cell.otherB = std::uint16_t(b);
  });
  codec.vector(
      p.records, unsigned(f.core.effects.size()),
      [&](auto &record) {
        auto &r = record.placement;
        codec.fields(r.active, r.type);
        codec.array(r.parameters);
        codec.array(r.position);
        codec.array(r.units);
        codec.array(r.initialPosition);
        codec.array(r.sentinels);
        unsigned next = r.next;
        codec.fields(next, r.cell, r.initialized, record.reservation,
                     record.property, record.animationReset);
        require(next <= 65535, "Cure World effect link");
        r.next = std::uint16_t(next);
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
}
Frame decode(const game::State &state) {
  require(state.systems.size() <= 32 * 1024 * 1024,
          "Cure World envelope capacity");
  Codec codec;
  codec.reading = true;
  codec.bytes = state.systems;
  Frame frame;
  content(codec, frame);
  require(codec.at == codec.bytes.size(), "Cure World trailing envelope");
  return frame;
}
game::Bytes encode(Frame frame) {
  Codec codec;
  content(codec, frame);
  require(codec.bytes.size() <= 32 * 1024 * 1024,
          "Cure World envelope capacity");
  return codec.bytes;
}
const game::Entity *find(const game::State &s, game::Handle h) {
  if (h.slot >= s.slots.size() || s.slots[h.slot].generation != h.generation ||
      !s.slots[h.slot].entity)
    return nullptr;
  return &*s.slots[h.slot].entity;
}
void validate(const Frame &f, const game::State &world) {
  (void)CureSession(f.core);
  c::validateCurePresentation(f.presentation);
  require(f.actors.size() == f.core.actors.size() &&
              f.effects.size() == f.core.effects.size() &&
              f.presentation.records.size() == f.effects.size() &&
              f.presentation.width == unsigned(f.core.dimensions.x) &&
              f.presentation.height == unsigned(f.core.dimensions.y) &&
              f.presentation.layers == unsigned(f.core.dimensions.z),
          "Cure World identity dimensions");
  std::vector<bool> used(world.slots.size(), false);
  for (unsigned i = 0; i < f.actors.size(); ++i) {
    auto h = f.actors[i];
    require(h.generation && h.slot < world.slots.size(),
            "Cure World actor handle");
    const auto *e = find(world, h);
    const auto &a = f.core.actors[i];
    if (!e) {
      require(!a.active, "Cure World missing live actor");
      continue;
    }
    require(!used[h.slot] && !e->motion &&
                e->family == game::Family::creature && e->type == 17 &&
                e->owner == a.resource.owner && e->cleaned == !a.active &&
                e->x == a.position.x && e->y == a.position.y &&
                e->z == a.position.z,
            "Cure World actor binding");
    used[h.slot] = true;
  }
  for (unsigned i = 0; i < f.effects.size(); ++i) {
    const auto &core = f.core.effects[i];
    const auto &p = f.presentation.records[i];
    require(core.active == p.placement.active &&
                core.active == bool(f.effects[i]),
            "Cure World effect activity");
    if (!core.active)
      continue;
    const auto h = *f.effects[i];
    const auto *e = find(world, h);
    require(e && !used.at(h.slot) && !e->cleaned && !e->motion &&
                e->family == game::Family::missileEffect && e->type == 41 &&
                e->state == effectTag(i, core.generation) &&
                e->target == (find(world, f.actors.at(core.effect.target))
                                  ? std::optional<game::Handle>(
                                        f.actors.at(core.effect.target))
                                  : std::nullopt) &&
                p.placement.type == core.effect.type &&
                p.placement.parameters[6] == core.effect.source &&
                p.placement.parameters[7] == core.effect.target &&
                p.placement.parameters[10] == core.effect.deliveryMode &&
                p.placement.sentinels[9] == core.effect.attribution &&
                p.placement.sentinels[10] == core.effect.contributions[0] &&
                p.placement.sentinels[11] == core.effect.contributions[1] &&
                e->x == core.effect.position.x &&
                e->y == core.effect.position.y &&
                e->z == core.effect.position.z &&
                p.placement.position ==
                    std::array<unsigned, 3>{unsigned(e->x), unsigned(e->y),
                                            unsigned(e->z)} &&
                (core.effect.type != 77 ||
                 (p.animation && !p.animationReset &&
                  p.property == f.definition.entry.property)),
            "Cure World effect binding");
    used[h.slot] = true;
  }
}
unsigned actorIndex(const Frame &f, const game::World &w, game::Handle h) {
  const auto *e = w.find(h);
  require(e && !e->cleaned, "Cure World stale actor command");
  const auto it = std::find(f.actors.begin(), f.actors.end(), h);
  require(it != f.actors.end(), "Cure World actor is not bound");
  return unsigned(it - f.actors.begin());
}
void syncActors(Frame &f, const game::State &s) {
  for (unsigned i = 0; i < f.actors.size(); ++i) {
    auto &a = f.core.actors[i];
    const auto *e = find(s, f.actors[i]);
    if (!e) {
      a.active = false;
      continue;
    }
    require(e->family == game::Family::creature && e->type == 17 &&
                e->owner == a.resource.owner,
            "Cure World actor identity changed");
    a.active = !e->cleaned;
    a.position = {e->x, e->y, e->z};
  }
}
void publish(game::World &world, Frame frame) {
  auto state = world.state();
  state.systems = encode(frame);
  validate(frame, state);
  world.restore(std::move(state));
}
} // namespace
CureWorldSession::CureWorldSession(CureSessionState core,
                                   CureAnimationDefinition definition,
                                   unsigned capacity)
    : world_(capacity) {
  Frame f;
  f.core = CureSession(std::move(core)).state();
  f.definition = std::move(definition);
  require(std::uint64_t(f.definition.sequence.size()) * 44 *
                  f.core.effects.size() <=
              16 * 1024 * 1024,
          "Cure World animation budget");
  r::NoCdAnimationPlayer initial(f.definition.sequence);
  initial.start();
  f.presentation = c::makeCurePresentation(
      unsigned(f.core.dimensions.x), unsigned(f.core.dimensions.y),
      unsigned(f.core.dimensions.z), unsigned(f.core.effects.size()));
  for (const auto &effect : f.core.effects)
    require(!effect.active, "Cure World bootstrap needs idle effects");
  f.effects.resize(f.core.effects.size());
  for (const auto &a : f.core.actors) {
    game::Entity e;
    e.family = game::Family::creature;
    e.type = 17;
    e.owner = a.resource.owner;
    e.x = a.position.x;
    e.y = a.position.y;
    e.z = a.position.z;
    e.cleaned = !a.active;
    f.actors.push_back(world_.spawn(std::move(e)));
  }
  publish(world_, std::move(f));
}
CureWorldSession::CureWorldSession(game::World world)
    : world_(std::move(world)) {
  const auto f = decode(world_.state());
  validate(f, world_.state());
}
CureSessionState CureWorldSession::state() const {
  return decode(world_.state()).core;
}
CurePresentationState CureWorldSession::presentation() const {
  return decode(world_.state()).presentation;
}
game::Handle CureWorldSession::actor(unsigned index) const {
  return decode(world_.state()).actors.at(index);
}
std::optional<game::Handle> CureWorldSession::effect(unsigned index) const {
  return decode(world_.state()).effects.at(index);
}
std::vector<CureSessionEvent> CureWorldSession::cast(game::Handle source,
                                                     game::Handle target,
                                                     unsigned mode) {
  auto candidate = world_;
  auto f = decode(candidate.state());
  const auto from = actorIndex(f, candidate, source),
             to = actorIndex(f, candidate, target);
  CureSession core(f.core);
  auto events = core.cast(from, to, mode);
  f.core = core.state();
  std::vector<c::CureCreator> creators;
  for (const auto &a : f.core.actors)
    creators.push_back({a.resource.owner, {}});
  for (const auto &event : events)
    if (event.kind == CureSessionEvent::created) {
      const auto index = event.actor;
      const auto &effect = f.core.effects.at(index);
      c::CastEffectDescriptor p{};
      for (unsigned i = 0; i < 10; ++i)
        p[i] = UINT32_MAX;
      const auto &a = f.core.actors[from].position;
      const auto &b = f.core.actors[to].position;
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
      p[10] = mode;
      p[11] = UINT32_MAX;
      p[12] = 1;
      c::createCurePresentation(f.presentation, index, p, creators, {});
      game::Entity entity;
      entity.family = game::Family::missileEffect;
      entity.type = 41;
      entity.owner = f.core.actors[from].resource.owner;
      entity.target = target;
      entity.state = effectTag(index, effect.generation);
      entity.x = b.x;
      entity.y = b.y;
      entity.z = b.z;
      f.effects[index] = candidate.spawn(std::move(entity));
    }
  publish(candidate, std::move(f));
  world_ = std::move(candidate);
  return events;
}
std::vector<CureSessionEvent>
CureWorldSession::advance(game::TickInput input,
                          const std::vector<CureAudioInput> &audio) {
  if (!input.admitted)
    return {};
  auto candidate = world_;
  auto f = decode(candidate.state());
  std::vector<CureSessionEvent> events;
  candidate.step(input, [&](game::Phase phase, const game::State &staged,
                            std::vector<game::Command> &commands) {
    if (phase != game::Phase::effects)
      return;
    syncActors(f, staged);
    std::vector<CureAnimationInput> signals;
    for (unsigned i = 0; i < f.core.effects.size(); ++i) {
      auto &e = f.core.effects[i];
      if (!e.active)
        continue;
      const auto *entity = f.effects[i] ? find(staged, *f.effects[i]) : nullptr;
      if (!entity || entity->cleaned) {
        c::releaseCurePresentation(f.presentation, i,
                                   [](unsigned) { return false; });
        e.active = false;
        e.visible = false;
        e.effect.attribution = 255;
        e.effect.contributions.fill(UINT32_MAX);
        if (entity)
          commands.push_back(
              {game::Operation::release, *f.effects[i], {}, {}, {}});
        f.effects[i].reset();
        events.push_back({CureSessionEvent::released, i, 0, {}});
        continue;
      }
      if (e.effect.type == 77) {
        const auto &a = f.core.actors.at(e.effect.target);
        c::moveCurePresentation(f.presentation, i,
                                {unsigned(a.position.x), unsigned(a.position.y),
                                 unsigned(a.position.z)});
        if (a.active && a.health.health)
          signals.push_back(
              {{i, e.generation},
               unsigned(c::tickCurePresentation(f.presentation, i))});
      }
    }
    CureSession core(f.core);
    const auto consequence = core.advance(signals, audio);
    f.core = core.state();
    for (const auto &event : consequence) {
      if (event.kind == CureSessionEvent::transition)
        c::bindCurePresentation(f.presentation, event.actor, f.definition);
      if (event.kind == CureSessionEvent::released) {
        c::releaseCurePresentation(f.presentation, event.actor,
                                   [](unsigned) { return false; });
        commands.push_back(
            {game::Operation::release, *f.effects.at(event.actor), {}, {}, {}});
        f.effects[event.actor].reset();
      }
    }
    events.insert(events.end(), consequence.begin(), consequence.end());
  });
  auto state = candidate.state();
  for (unsigned i = 0; i < f.effects.size(); ++i)
    if (f.effects[i]) {
      auto &e = *state.slots.at(f.effects[i]->slot).entity;
      const auto &p = f.core.effects[i].effect.position;
      e.x = p.x;
      e.y = p.y;
      e.z = p.z;
    }
  state.systems = encode(f);
  validate(f, state);
  candidate.restore(std::move(state));
  world_ = std::move(candidate);
  return events;
}
void CureWorldSession::relocate(game::Handle who, game::Point p) {
  auto candidate = world_;
  auto f = decode(candidate.state());
  const auto index = actorIndex(f, candidate, who);
  f.core.actors[index].position = {p.x, p.y, p.z};
  auto state = candidate.state();
  auto &e = *state.slots[who.slot].entity;
  e.x = p.x;
  e.y = p.y;
  e.z = p.z;
  state.systems = encode(f);
  validate(f, state);
  candidate.restore(std::move(state));
  world_ = std::move(candidate);
}
void CureWorldSession::changeHealth(game::Handle who, int delta) {
  auto candidate = world_;
  auto f = decode(candidate.state());
  const auto index = actorIndex(f, candidate, who);
  auto &a = f.core.actors[index];
  require(a.resource.kind == 1, "Cure World ordinary health only");
  combat::Inputs input;
  input.baseMaximum = a.maximumHealth;
  input.owner = a.resource.owner;
  input.selectedOwner = f.core.localOwner;
  a.health =
      combat::apply(a.health, input, combat::Entry::signedHealth, delta).state;
  if (!a.health.health)
    a.active = false;
  auto state = candidate.state();
  state.slots[who.slot].entity->cleaned = !a.active;
  state.systems = encode(f);
  validate(f, state);
  candidate.restore(std::move(state));
  world_ = std::move(candidate);
}
void CureWorldSession::enqueue(game::Command command) {
  world_.enqueue(command);
}
game::Bytes CureWorldSession::checkpoint() const {
  return game::encodeSnapshot(world_.state(), world_.limits());
}
CureWorldSession CureWorldSession::restore(const game::Bytes &bytes) {
  game::World world;
  world.restore(game::decodeSnapshot(bytes));
  return CureWorldSession(std::move(world));
}
} // namespace mnm::cure
