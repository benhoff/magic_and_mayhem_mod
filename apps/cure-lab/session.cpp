#include "session.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <tuple>
#include <type_traits>

namespace mnm::cure {
using namespace mnm::reconstruction::casting;
namespace {
void require(bool yes, const char *why) {
  if (!yes) throw std::invalid_argument(why);
}
std::uint32_t local(const CureSessionState &s) { return s.localOwner; }
void validate(const CureSessionState &s) {
  require(s.localOwner < 8 && s.cost && s.cost <= 0x7fffff &&
          s.magnitude <= 0x7fffffff, "Cure configuration domain");
  require(s.dimensions.x > 0 && s.dimensions.x <= 1024 &&
          s.dimensions.y > 0 && s.dimensions.y <= 1024 &&
          s.dimensions.z > 0 && s.dimensions.z <= 64 &&
          !s.actors.empty() && s.actors.size() <= 4096 && !s.effects.empty() &&
          s.effects.size() <= 4096, "Cure owned world domain");
  require(s.audio.buffers.size() <= 65536 && s.audio.outputs.size() <= 65536 &&
          s.audio.schedules.size() <= 65536, "Cure owned audio size");
  require(s.learning.notices.size() <= 20, "Cure learning notices");
  for (auto known : s.learning.known) require(known <= 1, "Cure learning flag");
  std::vector<bool> voiceOwners(s.audio.buffers.size(), false);
  for (const auto &a : s.actors) {
    require(a.resource.owner < 8 && a.health.rank == 0 &&
            a.position.x >= 0 && a.position.x < s.dimensions.x &&
            a.position.y >= 0 && a.position.y < s.dimensions.y &&
            a.position.z >= 0 && a.position.z < s.dimensions.z,
            "Cure actor domain");
    // Preflight the owned audio graph without changing the candidate or calling
    // a backend. Disabled gates still check graph structure/reference bounds.
    struct NoAudio : audio::VoiceBackend {
      audio::Status getStatus(unsigned, unsigned &) override { throw std::logic_error("preflight audio"); }
      audio::Status stop(unsigned) override { throw std::logic_error("preflight audio"); }
      audio::Status position(unsigned, unsigned) override { throw std::logic_error("preflight audio"); }
      audio::Status volume(unsigned, int) override { throw std::logic_error("preflight audio"); }
      audio::Status pan(unsigned, int) override { throw std::logic_error("preflight audio"); }
      audio::Status play(unsigned, unsigned, unsigned, unsigned) override { throw std::logic_error("preflight audio"); }
      void volumeRecord(audio::VoiceContract &, int) override { throw std::logic_error("preflight audio"); }
    } backend;
    auto status = a.status; auto graph = s.audio;
    if (status.voice) {
      require(*status.voice < voiceOwners.size() && !voiceOwners[*status.voice],
              "Cure status voice ownership");
      voiceOwners[*status.voice] = true;
    }
    // Missing matching live schedules must be rejected, even if a status query
    // would later return idle. Check membership before disabling backend calls.
    if (status.voice && *status.voice < graph.buffers.size() &&
        graph.enabled && graph.initialized && graph.buffers[*status.voice])
      require(std::any_of(graph.schedules.begin(), graph.schedules.end(),
                          [&](const auto &n) { return n.voiceAddress == *status.voice + 1; }),
              "Cure status voice has no schedule");
    graph.enabled = 0;
    clearCureStatus(status, graph, backend);
  }
  for (const auto &e : s.effects) if (e.active) {
    require(e.generation && e.effect.spell == 41 &&
            (e.effect.type == 11 || e.effect.type == 77) &&
            e.effect.source < s.actors.size() &&
            e.effect.target < s.actors.size() &&
            s.actors[e.effect.target].resource.kind == 1 &&
            (!e.effect.cachedActor || *e.effect.cachedActor < s.actors.size()) &&
            (e.effect.deliveryMode == 1 || e.effect.deliveryMode == 4),
            "Cure active effect domain");
  }
}
CurePosition position(const combat::Cell &p) { return {p.x, p.y, p.z}; }
void emit(std::vector<CureSessionEvent> &out, CureSessionEvent::Kind k,
          unsigned actor = UINT32_MAX, unsigned value = 0, CurePosition p = {}) {
  out.push_back({k, actor, value, p});
}
void resource(CureSessionState &s, unsigned index, unsigned delta,
              std::vector<CureSessionEvent> &out) {
  changeCastResource(s.actors.at(index).resource, delta, local(s),
    [&](unsigned n) { emit(out, CureSessionEvent::resourceFirst, index, n); },
    [&](unsigned n) { emit(out, CureSessionEvent::resourceSecond, index, n); });
}
struct AudioReplies : audio::VoiceBackend {
  const std::vector<CureAudioInput> &inputs;
  std::vector<CureSessionEvent> &events;
  AudioReplies(const std::vector<CureAudioInput> &i, std::vector<CureSessionEvent> &e) : inputs(i), events(e) {}
  const CureAudioInput &reply(unsigned buffer) const {
    require(buffer && buffer <= inputs.size(), "Cure missing audio reply");
    return inputs[buffer - 1];
  }
  audio::Status getStatus(unsigned b, unsigned &flags) override { const auto &r = reply(b); flags = r.flags; emit(events,CureSessionEvent::audioStatus,b,unsigned(r.statusResult)); return r.statusResult; }
  audio::Status stop(unsigned b) override { const auto r = reply(b).stopResult; emit(events,CureSessionEvent::audioStop,b,unsigned(r)); return r; }
  audio::Status position(unsigned b, unsigned p) override { require(!p, "Cure audio position"); const auto r = reply(b).positionResult; emit(events,CureSessionEvent::audioPosition,b,unsigned(r)); return r; }
  audio::Status volume(unsigned, int) override { throw std::logic_error("Cure unexpected volume"); }
  audio::Status pan(unsigned, int) override { throw std::logic_error("Cure unexpected pan"); }
  audio::Status play(unsigned, unsigned, unsigned, unsigned) override { throw std::logic_error("Cure unexpected play"); }
  void volumeRecord(audio::VoiceContract &, int) override { throw std::logic_error("Cure unexpected volume record"); }
};
void release(OwnedCureEffect &slot, std::vector<CureSessionEvent> &out, unsigned index) {
  slot.active = false; slot.visible = false;
  slot.effect.attribution = 255;
  slot.effect.contributions.fill(UINT32_MAX);
  emit(out, CureSessionEvent::released, index);
}
} // namespace

CureSession::CureSession(CureSessionState s) : state_(std::move(s)) { validate(state_); }
std::vector<CureSessionEvent> CureSession::cast(unsigned source, unsigned target,
                                              unsigned mode) {
  auto candidate = state_;
  require(source < candidate.actors.size() && target < candidate.actors.size() &&
          candidate.actors[source].active && candidate.actors[target].resource.kind == 1 &&
          (mode == 1 || mode == 4), "Cure cast domain");
  combat::AutonomousWorld world; world.dimensions = candidate.dimensions;
  world.localOwner = int(local(candidate));
  CastAdmissionState admission; admission.spells.resize(42);
  admission.spells[41].cost = candidate.cost;
  admission.spells[41].observed = candidate.observed;
  admission.profiles.push_back(candidate.learning);
  for (unsigned i = 0; i < candidate.actors.size(); ++i) {
    const auto &a = candidate.actors[i];
    combat::AutonomousCreature c;
    c.ai.slot = i; c.ai.kind = a.resource.kind; c.ai.type = 17;
    c.ai.owner = int(a.resource.owner); c.ai.active = a.active;
    c.ai.health = a.health.health; c.ai.visible = a.visible;
    c.ai.position = {a.position.x, a.position.y, a.position.z};
    c.finePosition = {a.position.x * 32 + 16, a.position.y * 32 + 16,
                      a.position.z * 16 + 8};
    c.block7c = a.resource.blocked;
    world.creatures.push_back(c);
    admission.resource.push_back(a.resource.value); admission.direction.push_back(0);
  }
  admission.resourceCounter = candidate.resourceCounter;
  std::vector<CureSessionEvent> out;
  CastAdmissionServices services;
  services.pool = [&]() -> std::optional<unsigned> {
    for (unsigned i = 0; i < candidate.effects.size(); ++i)
      if (!candidate.effects[i].active) {
        require(candidate.effects[i].generation != UINT32_MAX, "Cure effect generation exhausted");
        return i;
      }
    return std::nullopt;
  };
  services.initializePath = [] {};
  services.currentProfile = [] { return std::optional<unsigned>(0); };
  services.visibility = [&](auto &c, unsigned owner, unsigned delta) {
    emit(out, CureSessionEvent::visibility, c.ai.slot, owner);
    (void)delta; // Supplied visibility-notification endpoint; no mana policy.
  };
  services.chargeable = [](const auto &c) { return chargeableKind(c.ai.kind); };
  services.ownerMessage = [&](const auto &c) { return c.ai.owner == world.localOwner; };
  services.resource = [&](auto &c, unsigned delta) {
    resource(candidate, c.ai.slot, delta, out);
    admission.resource[c.ai.slot] = candidate.actors[c.ai.slot].resource.value;
  };
  services.insufficientOwnerMessage = [&] { emit(out, CureSessionEvent::cue, source, 0x3f3); };
  services.cue = [&](unsigned code, auto p) { emit(out, CureSessionEvent::cue, UINT32_MAX, code, position(p)); };
  services.castCue = [&](unsigned code, auto p, unsigned) { emit(out, CureSessionEvent::castCue, source, code, position(p)); };
  services.secondary = [&](unsigned slot, unsigned kind, const auto &d) {
    require(kind == 11 && d[8] == 41, "Cure direct delivery only");
    auto &e = candidate.effects.at(slot); ++e.generation;
    e.active = true; e.visible = false; e.effect = {};
    e.effect.type = 11; e.effect.source = d[6]; e.effect.target = d[7];
    e.effect.deliveryMode = d[10]; e.effect.position = candidate.actors[target].position;
    // Empty contribution ownership is an explicit native boundary, rather than
    // treating arbitrary host actor links as recovered creator-chain ownership.
    e.effect.attribution = candidate.actors[source].resource.owner;
    e.effect.contributions.fill(UINT32_MAX);
    emit(out, CureSessionEvent::created, slot, e.generation, e.effect.position);
  };
  CastDescriptor d{}; d[0] = 41; d[7] = source; d[8] = target;
  d[9] = mode; d[10] = UINT32_MAX; d[11] = 1;
  admitCast(d, world, admission, services);
  candidate.learning = admission.profiles[0];
  candidate.observed = admission.spells[41].observed;
  candidate.resourceCounter = admission.resourceCounter;
  validate(candidate); state_ = std::move(candidate); return out;
}

std::vector<CureSessionEvent> CureSession::advance(
    const std::vector<CureAnimationInput> &animation,
    const std::vector<CureAudioInput> &audioInputs) {
  auto candidate = state_;
  require(candidate.tick != UINT64_MAX, "Cure tick overflow");
  std::vector<std::optional<unsigned>> signals(candidate.effects.size());
  for (const auto &a : animation) {
    require(a.effect.slot < candidate.effects.size(), "Cure animation slot");
    const auto &e = candidate.effects[a.effect.slot];
    require(e.active && e.generation == a.effect.generation && e.effect.type == 77 &&
            !signals[a.effect.slot], "Cure stale/duplicate animation input");
    signals[a.effect.slot] = a.result;
  }
  std::vector<CureSessionEvent> out;
  AudioReplies backend(audioInputs,out);
  std::vector<CureActor> projection;
  for (const auto &a : candidate.actors) {
    CureActor c; c.health = a.health.health; c.blocked = a.resource.blocked;
    c.status = a.status.status; c.ongoingDamage = a.ongoingDamage;
    c.resource = a.resource.value; c.visible = a.visible; projection.push_back(c);
  }
  for (unsigned index = 0; index < candidate.effects.size(); ++index) {
    auto &slot = candidate.effects[index]; if (!slot.active) continue;
    auto &effect = slot.effect;
    if (effect.type == 77) {
      auto &target = candidate.actors[effect.target];
      effect.position = target.position; effect.cachedActor = effect.target;
      if (!target.active || !target.health.health) { release(slot, out, index); continue; }
      slot.visible = target.visible[local(candidate)] != 0;
      if (!signals[index] || *signals[index] != 1) continue;
    }
    CureServices services;
    auto actorIndex = [&](const CureActor &a) { return unsigned(&a - projection.data()); };
    services.health = [&](auto &a, int amount, unsigned, const auto &, unsigned) {
      const auto who = actorIndex(a); auto &owned = candidate.actors[who];
      combat::Inputs inputs; inputs.baseMaximum = owned.maximumHealth;
      inputs.owner = owned.resource.owner; inputs.selectedOwner = local(candidate);
      auto changed = combat::apply(owned.health, inputs, combat::Entry::signedHealth, amount);
      owned.health = changed.state; a.health = changed.state.health;
      for (const auto &event : changed.events)
        emit(out, CureSessionEvent::health, who, unsigned(event.kind));
    };
    services.cleanse = [&](auto &a) {
      auto &owned = candidate.actors[actorIndex(a)];
      clearCureStatus(owned.status, candidate.audio, backend); a.status = owned.status.status;
    };
    services.cue = [&](unsigned code, auto p) { emit(out, CureSessionEvent::cue, UINT32_MAX, code, p); };
    services.feedback = [&](auto &, auto &a) { emit(out, CureSessionEvent::feedback, actorIndex(a), a.health); };
    services.transition = [&](auto &e, unsigned type) {
      e.type = type; emit(out, CureSessionEvent::transition, index, type);
    };
    services.chargeable = [&](const auto &a) { return chargeableKind(candidate.actors[actorIndex(a)].resource.kind); };
    services.resource = [&](auto &a, unsigned delta) {
      const auto who = actorIndex(a); resource(candidate, who, delta, out);
      a.resource = candidate.actors[who].resource.value;
    };
    services.release = [&](auto &) { release(slot, out, index); };
    const bool deferred = effect.type == 77;
    applyCureEffect(effect, projection,
      {candidate.cost, int(candidate.magnitude), int(local(candidate))}, services);
    if (deferred) release(slot, out, index);
  }
  for (unsigned i = 0; i < projection.size(); ++i)
    candidate.actors[i].ongoingDamage = projection[i].ongoingDamage;
  ++candidate.tick; validate(candidate); state_ = std::move(candidate); return out;
}

namespace {
struct Codec {
  std::vector<std::uint8_t> bytes;
  std::size_t cursor = 0;
  bool reading = false;
  void field(std::uint32_t &v) {
    if (reading) {
      require(cursor + 4 <= bytes.size(), "Cure checkpoint truncated"); v = 0;
      for (unsigned i = 0; i < 4; ++i) v |= unsigned(bytes[cursor++]) << (i * 8);
    } else for (unsigned i = 0; i < 4; ++i) bytes.push_back(std::uint8_t(v >> (i * 8)));
  }
  void field(std::int32_t &v) { auto u = std::uint32_t(v); field(u); v = u <= 0x7fffffff ? int(u) : -1 - int(~u); }
  void field(bool &v) { unsigned n = v; field(n); require(n <= 1, "Cure checkpoint Boolean"); v = n != 0; }
  void field(std::optional<unsigned> &v) { unsigned n = v.value_or(UINT32_MAX); field(n); v = n == UINT32_MAX ? std::nullopt : std::optional<unsigned>(n); }
  template<class... T> void fields(T &...v) { (field(v), ...); }
  template<class T> void array(T &v) { for (auto &x : v) field(x); }
  template<class T, class F> void vector(std::vector<T> &v, unsigned maximum, F item) {
    unsigned size = unsigned(v.size()); field(size); require(size <= maximum, "Cure checkpoint vector size");
    if (reading) v.resize(size);
    for (auto &x : v) item(x);
  }
};
void fields(Codec &c, combat::State &h) {
  c.fields(h.health,h.bar,h.dirty,h.cooldown,h.target,h.gate9c,h.expiry,h.timer,
           h.cacheA,h.cacheB,h.marker,h.flag104,h.rank);
  c.array(h.incoming); c.array(h.ages); c.array(h.credit);
}
void fields(Codec &c, CurePosition &p) { c.fields(p.x,p.y,p.z); }
void content(Codec &c, CureSessionState &s) {
  unsigned magic = 0x5255434d, version = 1; c.fields(magic,version);
  require(magic == 0x5255434d && version == 1, "Cure checkpoint version");
  unsigned lo = unsigned(s.tick), hi = unsigned(s.tick >> 32); c.fields(lo,hi); s.tick = (std::uint64_t(hi) << 32) | lo;
  c.fields(s.cost,s.magnitude,s.localOwner,s.dimensions.x,s.dimensions.y,s.dimensions.z,s.resourceCounter);
  c.vector(s.actors,4096,[&](auto &a) {
    c.field(a.active); fields(c,a.position); fields(c,a.health);
    c.fields(a.maximumHealth,a.ongoingDamage,a.resource.kind,a.resource.value,
             a.resource.maximum,a.resource.blocked,a.resource.owner,a.resource.dirty,
             a.status.status,a.status.voice); c.array(a.visible);
  });
  c.vector(s.effects,4096,[&](auto &e) {
    c.fields(e.active,e.visible,e.generation,e.effect.spell,e.effect.type,
             e.effect.source,e.effect.target,e.effect.deliveryMode,e.effect.attribution);
    fields(c,e.effect.position); c.array(e.effect.contributions); c.field(e.effect.cachedActor);
  });
  auto &a = s.audio; c.fields(a.enabled,a.initialized,a.head,a.statusScratch,a.resultScratch);
  c.vector(a.buffers,65536,[&](auto &x) { c.field(x); });
  c.vector(a.outputs,65536,[&](auto &x) { c.field(x); });
  c.vector(a.schedules,65536,[&](auto &n) { c.fields(n.next,n.previous,n.deadline,n.volume,n.voiceAddress,n.outputAddress,n.x,n.y); });
  for (auto &flag : s.learning.known) { unsigned n = flag; c.field(n); require(n <= 1, "Cure checkpoint learning"); flag = std::uint8_t(n); }
  c.vector(s.learning.notices,20,[&](auto &n) { c.array(n); });
  for (auto &row : s.observed) c.array(row);
}
unsigned checksum(const std::vector<std::uint8_t> &v, std::size_t size) {
  unsigned hash = 2166136261u; for (std::size_t i = 0; i < size; ++i) hash = (hash ^ v[i]) * 16777619u; return hash;
}
} // namespace
std::vector<std::uint8_t> CureSession::checkpoint() const {
  auto candidate = state_; Codec c; content(c,candidate); auto sum = checksum(c.bytes,c.bytes.size()); c.field(sum); return c.bytes;
}
CureSession CureSession::restore(const std::vector<std::uint8_t> &bytes) {
  require(bytes.size() >= 12 && bytes.size() <= 8 * 1024 * 1024, "Cure checkpoint size");
  Codec c; c.bytes = bytes; c.reading = true;
  const auto expected = checksum(bytes,bytes.size()-4);
  c.cursor = bytes.size()-4; unsigned actual = 0; c.field(actual);
  require(actual == expected, "Cure checkpoint checksum"); c.cursor = 0;
  CureSessionState s; content(c,s); require(c.cursor == bytes.size()-4, "Cure checkpoint trailing data");
  return CureSession(std::move(s));
}
} // namespace mnm::cure
