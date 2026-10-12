#include "melee_exchange.hpp"
#include <algorithm>
#include <stdexcept>
namespace mnm::game {
namespace {
void need(bool b, const char *why) {
  if (!b)
    throw std::invalid_argument(why);
}
struct Writer {
  Bytes bytes;
  void word(std::uint32_t v) {
    for (unsigned i = 0; i < 4; ++i)
      bytes.push_back(std::uint8_t(v >> (8 * i)));
  }
  void wide(std::uint64_t v) {
    word(std::uint32_t(v));
    word(std::uint32_t(v >> 32));
  }
  void blob(const Bytes &b) {
    need(b.size() <= 16384, "exchange blob limit");
    word(unsigned(b.size()));
    bytes.insert(bytes.end(), b.begin(), b.end());
  }
};
struct Reader {
  const Bytes &bytes;
  std::size_t at = 8;
  std::uint32_t word() {
    need(at <= bytes.size() && bytes.size() - at >= 4, "truncated exchange");
    std::uint32_t v = 0;
    for (unsigned i = 0; i < 4; ++i)
      v |= std::uint32_t(bytes[at++]) << (8 * i);
    return v;
  }
  std::uint64_t wide() {
    auto lo = word(), hi = word();
    return std::uint64_t(hi) << 32 | lo;
  }
  Bytes blob() {
    auto n = word();
    need(n <= 16384 && at <= bytes.size() && n <= bytes.size() - at,
         "exchange blob extent");
    Bytes b(bytes.begin() + at, bytes.begin() + at + n);
    at += n;
    return b;
  }
};
constexpr char magic[] = "MNMMLX01";
} // namespace
Bytes encodeExchange(const ExchangeState &s) {
  need(s.driver && s.healthRules, "exchange rules identity required");
  Writer w;
  w.bytes.insert(w.bytes.end(), magic, magic + 8);
  w.wide(s.driver);
  w.wide(s.healthRules);
  w.blob(s.random);
  for (const auto &a : s.actors) {
    w.word(a.actor.slot);
    w.word(a.actor.generation);
    w.word(a.health);
    w.word(a.cooldown);
    w.word(unsigned(a.phase));
    w.blob(a.cursor);
  }
  return w.bytes;
}
ExchangeState decodeExchange(const Bytes &b) {
  need(b.size() >= 8 && std::equal(b.begin(), b.begin() + 8, magic),
       "invalid melee exchange version");
  Reader r{b};
  ExchangeState s;
  s.driver = r.wide();
  s.healthRules = r.wide();
  s.random = r.blob();
  for (auto &a : s.actors) {
    a.actor = {r.word(), r.word()};
    a.health = r.word();
    a.cooldown = r.word();
    a.phase = ExchangePhase(r.word());
    a.cursor = r.blob();
  }
  need(r.at == b.size(), "trailing melee exchange bytes");
  (void)encodeExchange(s);
  return s;
}
void MeleeExchangeSession::validate(const State &w,
                                    const ExchangeState &s) const {
  need(driver_ && healthRules_ && s.driver == driver_->identity() &&
           s.healthRules == healthRules_->identity(),
       "melee exchange rules mismatch");
  need(driver_->maximumHealth() > 0 && driver_->maximumHealth() <= 1000000,
       "exchange health bound");
  need(w.slots.size() == 2 && w.pending.empty(),
       "exchange requires two stationary actors");
  for (unsigned i = 0; i < 2; ++i) {
    const auto &a = s.actors[i];
    need(a.actor.slot == i && a.actor.generation == w.slots[i].generation &&
             w.slots[i].entity.has_value(),
         "exchange actor identity");
    const auto &e = *w.slots[i].entity;
    need(e.family == Family::creature && e.type == 17 && e.owner == i &&
             e.x == int(i) + 2 && e.y == 3 && e.z == 1 && !e.motion &&
             !e.target && e.state.empty(),
         "exchange stationary geometry");
    need(a.health <= driver_->maximumHealth() && unsigned(a.phase) <= 2 &&
             (a.health == 0) == e.cleaned &&
             (a.health == 0) == (a.phase == ExchangePhase::dead),
         "exchange health/lifecycle");
    need(a.phase == ExchangePhase::recovering
             ? (a.cooldown >= 1 && a.cooldown <= 34)
             : a.cooldown == 0,
         "exchange cooldown");
    driver_->validate(a.cursor, s.random, a.phase == ExchangePhase::attacking);
  }
}
MeleeExchangeSession::MeleeExchangeSession(std::shared_ptr<const MeleeDriver> d,
                                           std::shared_ptr<const HealthRules> h,
                                           Bytes profile, std::uint32_t seed)
    : world_(2), driver_(std::move(d)), healthRules_(std::move(h)) {
  need(driver_ && healthRules_, "exchange drivers required");
  ExchangeState s;
  s.driver = driver_->identity();
  s.healthRules = healthRules_->identity();
  s.random = driver_->random(seed);
  for (unsigned i = 0; i < 2; ++i) {
    Entity e;
    e.type = 17;
    e.owner = i;
    e.x = int(i) + 2;
    e.y = 3;
    e.z = 1;
    s.actors[i].actor = world_.spawn(e);
    s.actors[i].health = driver_->maximumHealth();
    s.actors[i].cursor = driver_->start();
  }
  auto w = world_.state();
  w.map = "native-crocodile-melee-lab";
  w.campaign = std::move(profile);
  w.systems = encodeExchange(s);
  validate(w, s);
  world_.restore(std::move(w));
}
MeleeExchangeSession::MeleeExchangeSession(World w,
                                           std::shared_ptr<const MeleeDriver> d,
                                           std::shared_ptr<const HealthRules> h)
    : world_(std::move(w)), driver_(std::move(d)), healthRules_(std::move(h)) {
  validate(world_.state(), state());
}
bool MeleeExchangeSession::finished() const {
  auto s = state();
  return !s.actors[0].health || !s.actors[1].health;
}
std::vector<ExchangeEvent> MeleeExchangeSession::step(TickInput input) {
  if (!input.admitted || finished())
    return {};
  auto candidate = world_;
  auto s = state();
  std::vector<ExchangeEvent> events;
  candidate.step(input);
  if (!input.alternateMode) {
    for (unsigned i = 0; i < 2; ++i) {
      auto &a = s.actors[i];
      auto &b = s.actors[1 - i];
      if (!a.health || !b.health)
        continue;
      if (a.phase == ExchangePhase::recovering) {
        if (--a.cooldown == 0) {
          a.phase = ExchangePhase::attacking;
          a.cursor = driver_->start();
        }
        continue;
      }
      const auto sample = driver_->advance(a.cursor, s.random);
      need(sample.damage <= 1000000 &&
               (!sample.damage || sample.animationEvent == 2) &&
               (sample.finished
                    ? (sample.animationEvent == 1 && sample.cooldown >= 2 &&
                       sample.cooldown <= 34)
                    : sample.cooldown == 0),
           "invalid melee driver sample");
      ExchangeEvent event{a.actor,       b.actor, sample.animationEvent,
                          sample.damage, 0,       sample.draws,
                          false};
      if (sample.damage) {
        const auto after =
            healthRules_->change(b.health, driver_->maximumHealth(),
                                 -std::int32_t(sample.damage), true);
        need(after <= b.health, "exchange damage increased health");
        event.applied = b.health - after;
        b.health = after;
        if (!after) {
          b.phase = ExchangePhase::dead;
          b.cursor.clear();
          b.cooldown = 0;
          event.died = true;
        }
      }
      if (sample.finished) {
        a.phase = ExchangePhase::recovering;
        a.cooldown = sample.cooldown;
        a.cursor.clear();
      }
      if (sample.animationEvent || sample.draws)
        events.push_back(event);
    }
  }
  auto w = candidate.state();
  for (unsigned i = 0; i < 2; ++i)
    w.slots[i].entity->cleaned = !s.actors[i].health;
  w.systems = encodeExchange(s);
  validate(w, s);
  candidate.restore(std::move(w));
  world_ = std::move(candidate);
  return events;
}
} // namespace mnm::game
