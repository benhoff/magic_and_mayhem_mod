#include "profile.hpp"
#include "../melee-lab/profile.hpp"
#include "../skirmish/health_rules.hpp"
#include "ai.hpp"
#include "melee.hpp"
#include "no_cd.hpp"
#include "persistence.hpp"
#include "persistence/snapshot.hpp"
#include <algorithm>
#include <charconv>
#include <fstream>
#include <sstream>
#include <stdexcept>
namespace mnm::creature_lab {
namespace {
namespace c = reconstruction::combat;
void need(bool b, const char *s) {
  if (!b)
    throw std::invalid_argument(s);
}
void word(game::Bytes &b, std::uint32_t n) {
  for (unsigned i = 0; i < 4; ++i)
    b.push_back(std::uint8_t(n >> (8 * i)));
}
std::uint32_t get(const game::Bytes &b, std::size_t at) {
  need(at <= b.size() && b.size() - at >= 4, "melee profile extent");
  std::uint32_t n = 0;
  for (unsigned i = 0; i < 4; ++i)
    n |= std::uint32_t(b[at + i]) << (8 * i);
  return n;
}
assets::Config config(const game::Bytes &b) {
  auto packed = assets::decodePackedContainer(b);
  need(std::holds_alternative<assets::PackedContainer>(packed),
       "invalid encrypted melee CFG");
  auto result =
      assets::decodeConfig(std::get<assets::PackedContainer>(packed).decoded);
  need(std::holds_alternative<assets::Config>(result),
       "invalid melee CFG text");
  return std::get<assets::Config>(result);
}
std::string field(const assets::Config &c, const char *s, const char *k) {
  auto v = c.find(s, k);
  need(v != nullptr, "missing melee profile field");
  return *v;
}
int integer(const std::string &s) {
  int v;
  auto r = std::from_chars(s.data(), s.data() + s.size(), v);
  need(r.ec == std::errc{} && r.ptr == s.data() + s.size(),
       "invalid melee profile integer");
  return v;
}
std::vector<int> row(const std::string &s) {
  std::vector<int> out;
  std::istringstream in(s);
  std::string item;
  while (std::getline(in, item, ',')) {
    const auto a = item.find_first_not_of(" \t"),
               z = item.find_last_not_of(" \t");
    need(a != std::string::npos, "empty melee matrix value");
    out.push_back(integer(item.substr(a, z - a + 1)));
  }
  need(out.size() == 28, "melee matrix row width");
  return out;
}
game::Bytes randomBytes(const c::RandomState &s) {
  game::Bytes b;
  word(b, s.left);
  word(b, s.right);
  for (auto n : s.words)
    word(b, n);
  return b;
}
c::RandomState randomState(const game::Bytes &b) {
  need(b.size() == 1008, "melee RNG size");
  c::RandomState s;
  s.left = get(b, 0);
  s.right = get(b, 4);
  need(s.left < 250 && s.right < 250 && (s.right + 250 - s.left) % 250 == 103,
       "melee RNG cursors");
  for (unsigned i = 0; i < 250; ++i)
    s.words[i] = get(b, 8 + i * 4);
  return s;
}
game::Bytes cursorBytes(const reconstruction::AnimationState &s) {
  game::Bytes b;
  word(b, unsigned(s.pc));
  word(b, s.displayedRecord ? unsigned(*s.displayedRecord) : 0xffffffffU);
  word(b, s.active);
  word(b, s.delay);
  word(b, s.elapsed);
  word(b, s.repeats);
  word(b, s.breakFlag);
  return b;
}
reconstruction::AnimationState cursorState(const game::Bytes &b) {
  need(b.size() == 28, "melee cursor size");
  reconstruction::AnimationState s;
  s.pc = get(b, 0);
  if (get(b, 4) != 0xffffffffU)
    s.displayedRecord = get(b, 4);
  need(get(b, 8) <= 1, "melee cursor active flag");
  s.active = get(b, 8);
  s.delay = get(b, 12);
  s.elapsed = get(b, 16);
  s.repeats = get(b, 20);
  s.breakFlag = get(b, 24);
  return s;
}
c::Cell cell(game::Point p) { return {p.x, p.y, p.z}; }
c::Extent extent(game::Point p) { return {p.x, p.y, p.z}; }
class Rules final : public game::CreatureRules {
  std::uint64_t id_;
  unsigned maximum_;
  c::MeleeInputs parameters_;
  c::AiCreature template_;
  assets::Animation animation_;
  std::vector<assets::AnimationRecord> sequence(bool stun,
                                                unsigned facing) const {
    // Explicit native animation selection, separate from original
    // 50db60/50e400.
    const unsigned index = (stun ? 112 : 24) + facing;
    need(index + 1 < animation_.starts.size(), "creature sequence absent");
    return {animation_.records.begin() + animation_.starts[index],
            animation_.records.begin() + animation_.starts[index + 1]};
  }

public:
  Rules(std::uint64_t id, unsigned maximum, c::MeleeInputs p, c::AiCreature ai,
        assets::Animation a)
      : id_(id), maximum_(maximum), parameters_(p), template_(ai),
        animation_(std::move(a)) {}
  std::uint64_t identity() const override { return id_; }
  unsigned maximumHealth() const override { return maximum_; }
  int width() const override { return 2; }
  game::Bytes random(unsigned seed) const override {
    return randomBytes(c::seedRandom(seed));
  }
  void validate(const game::CreatureActor &a,
                const game::Bytes &rng) const override {
    (void)randomState(rng);
    bool active = a.phase == game::CreaturePhase::attacking ||
                  a.phase == game::CreaturePhase::stunned;
    if (!active) {
      need(a.cursor.empty(), "inactive creature cursor");
      return;
    }
    auto records = sequence(a.phase == game::CreaturePhase::stunned, a.facing);
    reconstruction::NoCdAnimationPlayer player(records);
    player.restore(cursorState(a.cursor));
    need(player.state().active, "active creature animation required");
    player.start();
    bool found = false;
    for (unsigned tick = 0; tick < 1024 && player.state().active; ++tick) {
      if (cursorBytes(player.state()) == a.cursor) {
        found = true;
        break;
      }
      player.tick();
    }
    need(found, "unreachable creature animation state");
  }
  std::int32_t select(unsigned self, const game::State &w,
                      const game::CreatureEncounter &s) const override {
    c::AiWorld view;
    view.dimensions = extent(s.dimensions);
    for (unsigned i = 0; i < 8; ++i)
      view.relation[i][i] = 1;
    for (unsigned i = 0; i < s.actors.size(); ++i) {
      auto a = template_;
      const auto &v = s.actors[i];
      const auto &e = *w.slots[i].entity;
      a.slot = i;
      a.owner = e.owner;
      a.health = v.health;
      a.maximumHealth = maximum_;
      a.position = {e.x, e.y, e.z};
      a.home = cell(v.home);
      a.target = v.target;
      a.recent = v.recent;
      a.age = v.age;
      a.behavior = v.phase == game::CreaturePhase::attacking ? 3
                   : v.phase == game::CreaturePhase::stunned ? 8
                                                             : 1;
      a.incoming = 0;
      for (const auto &b : s.actors)
        if (b.target == int(i) && b.phase == game::CreaturePhase::attacking)
          ++a.incoming;
      a.engaged = v.phase == game::CreaturePhase::attacking;
      view.creatures.push_back(a);
    }
    const auto &a = view.creatures[self];
    auto admit = [&](unsigned slot) {
      return slot != self && view.creatures[slot].health;
    };
    auto result = c::selectRetaliation(a, view, admit);
    if (!result.target)
      result = c::selectAssist(
          a, view, [](unsigned, unsigned) { return true; }, admit);
    if (!result.target)
      result = c::selectEnemy(a, view, false, admit);
    if (!result.target)
      result = c::selectEnemy(a, view, true, admit);
    return result.target ? int(*result.target) : -1;
  }
  bool reach(game::Point a, game::Point b, game::Point dimensions,
             const std::function<bool(game::Point, game::Point)> &diagonal)
      const override {
    return c::meleeReach({cell(a), a.z * 16, 2, 2, {}, 0},
                         {cell(b), b.z * 16, 2, 2, {}, 0}, extent(dimensions),
                         [&](c::Cell x, c::Cell y) {
                           return diagonal({x.x, x.y, x.z}, {y.x, y.y, y.z});
                         })
        .admitted;
  }
  unsigned facing(game::Point a, game::Point b,
                  game::Point dim) const override {
    return unsigned(c::relativeDirection(cell(b), cell(a), 0, extent(dim)));
  }
  game::Bytes start(bool stun, unsigned facing) const override {
    reconstruction::NoCdAnimationPlayer a(sequence(stun, facing));
    a.start();
    return cursorBytes(a.state());
  }
  int animation(game::Bytes &cursor, bool stun,
                unsigned facing) const override {
    reconstruction::NoCdAnimationPlayer a(sequence(stun, facing));
    a.restore(cursorState(cursor));
    auto event = a.tick();
    cursor = cursorBytes(a.state());
    return event;
  }
  game::DamageRoll damage(game::Bytes &random) const override {
    auto rng = randomState(random);
    auto r = c::resolveMelee(parameters_, [&] { return c::nextRandom(rng); });
    random = randomBytes(rng);
    bool stun = false;
    for (auto e : r.events)
      if (e.kind == c::MeleeEffect::stunQuery)
        stun = true;
    need(r.amount >= 0 && r.amount <= 1000000, "creature damage domain");
    return {unsigned(r.amount), stun};
  }
  game::StunRoll
  stun(game::Bytes &random, const game::Entity &e, const game::CreatureActor &a,
       const game::Entity &attacker, game::Point dimensions,
       const std::function<bool(game::Point)> &legal) const override {
    auto rng = randomState(random);
    c::KnockbackInput p;
    p.actor.health = a.health;
    p.actor.timer = a.stunTimer;
    p.actor.position = {e.x, e.y, e.z};
    p.actor.behavior = a.phase == game::CreaturePhase::stunned     ? 8
                       : a.phase == game::CreaturePhase::knockback ? 14
                       : a.phase == game::CreaturePhase::pursuing  ? 2
                                                                   : 3;
    p.actor.pending = 3;
    p.facing = a.facing;
    p.response = unsigned(
        c::relativeDirection({attacker.x, attacker.y, attacker.z},
                             p.actor.position, p.facing, extent(dimensions)));
    p.dimensions = extent(dimensions);
    p.offsets = {{{0, -1, 0},
                  {1, -1, 0},
                  {1, 0, 0},
                  {1, 1, 0},
                  {0, 1, 0},
                  {-1, 1, 0},
                  {-1, 0, 0},
                  {-1, -1, 0}}};
    auto r = c::admitKnockback(
        p, c::nextRandom(rng),
        [&](c::Cell q) { return legal({q.x, q.y, q.z}); },
        [&](c::Cell q) { return legal({q.x, q.y, q.z}); });
    game::StunRoll out;
    if (r.admitted) {
      out.knockback =
          game::Point{r.requested->x, r.requested->y, r.requested->z};
    } else {
      auto duration = int((c::nextRandom(rng) & 0x7fffffffU) % 20) + 20;
      auto result = c::beginStun(p.actor, duration, 0);
      out.duration = result.state.timer;
    }
    random = randomBytes(rng);
    return out;
  }
  int stunTick(int timer, int event) const override {
    c::StunState s;
    s.behavior = 8;
    s.pending = 3;
    s.timer = timer;
    return c::tickStun(s, event).state.timer;
  }
  unsigned cooldown(game::Bytes &random) const override {
    auto rng = randomState(random);
    auto result = c::meleeEvent({}, 1, true, true, false, false, 0,
                                [&] { return c::nextRandom(rng); });
    random = randomBytes(rng);
    return result.state.cooldown;
  }
};
} // namespace
Profile decodeProfile(game::Bytes b) {
  auto legacy = melee_lab::decodeProfile(b);
  std::size_t at = 8;
  std::array<game::Bytes, 3> input;
  for (auto &item : input) {
    auto n = get(b, at);
    at += 4;
    item = {b.begin() + at, b.begin() + at + n};
    at += n;
  }
  auto hth = config(input[1]);
  c::MeleeInputs p;
  p.base = row(field(hth, "HTH_Matrix", "Creat_17"))[17];
  p.stunThreshold = row(field(hth, "Stun_Matrix", "Creat_17"))[17];
  c::AiCreature ai;
  ai.hth = {};
  auto copy = [&](auto &dest, const char *name) {
    auto v = row(field(hth, name, "Creat_17"));
    std::copy(v.begin(), v.end(), dest.begin());
  };
  copy(ai.hth, "HTH_Matrix");
  copy(ai.enemyRadius, "FearHate_Matrix");
  copy(ai.allyRadius, "Assist_Matrix");
  copy(ai.retaliateThreshold, "Evasion_Matrix");
  ai.bravery = 30;
  auto animation = assets::decodeAnimation(input[2]);
  need(std::holds_alternative<assets::Animation>(animation),
       "creature animation input");
  auto rules =
      std::make_shared<Rules>(legacy.driver->identity() ^ 0x4341493235ULL,
                              legacy.driver->maximumHealth(), p, ai,
                              std::get<assets::Animation>(animation));
  // Reject profiles without bounded attacks and stun sequences in all facings.
  for (unsigned f = 0; f < 8; ++f)
    for (bool stun : {false, true}) {
      auto cursor = rules->start(stun, f);
      bool stopped = false;
      for (unsigned i = 0; i < 1024; ++i)
        if (rules->animation(cursor, stun, f) == 1) {
          stopped = true;
          break;
        }
      need(stopped, "unbounded creature sequence");
    }
  return {std::move(b), rules};
}
Profile loadProfile(const std::filesystem::path &root) {
  return decodeProfile(melee_lab::loadProfile(root).bytes);
}
game::CreatureEncounterSession create(Profile p, unsigned seed) {
  return {p.rules, skirmish::ordinaryHealthRules(), std::move(p.bytes), seed};
}
game::CreatureEncounterSession load(const std::filesystem::path &p) {
  auto w = game::readSnapshot(p);
  auto profile = decodeProfile(w.campaign);
  game::World world(0);
  world.restore(std::move(w));
  return {std::move(world), profile.rules, skirmish::ordinaryHealthRules()};
}
} // namespace mnm::creature_lab
