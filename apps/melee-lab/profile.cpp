#include "profile.hpp"
#include "../skirmish/health_rules.hpp"
#include "melee.hpp"
#include "no_cd.hpp"
#include "persistence.hpp"
#include "persistence/snapshot.hpp"
#include <QCryptographicHash>
#include <algorithm>
#include <charconv>
#include <fstream>
#include <sstream>
#include <stdexcept>
namespace mnm::melee_lab {
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
game::Bytes read(const std::filesystem::path &p) {
  std::ifstream f(p, std::ios::binary);
  need(bool(f), "melee profile input missing");
  auto n = std::filesystem::file_size(p);
  need(n <= 4 * 1024 * 1024, "melee profile input limit");
  game::Bytes b(n);
  f.read(reinterpret_cast<char *>(b.data()), std::streamsize(n));
  need(bool(f), "melee profile input read");
  return b;
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
class Driver final : public game::MeleeDriver {
  std::uint64_t identity_;
  std::uint32_t maximum_;
  c::MeleeInputs parameters_;
  std::vector<assets::AnimationRecord> attack_;

public:
  Driver(std::uint64_t identity, unsigned maximum, c::MeleeInputs p,
         std::vector<assets::AnimationRecord> attack)
      : identity_(identity), maximum_(maximum), parameters_(p),
        attack_(std::move(attack)) {}
  std::uint64_t identity() const override { return identity_; }
  std::uint32_t maximumHealth() const override { return maximum_; }
  game::Bytes random(std::uint32_t seed) const override {
    return randomBytes(c::seedRandom(seed));
  }
  game::Bytes start() const override {
    reconstruction::NoCdAnimationPlayer a(attack_);
    a.start();
    return cursorBytes(a.state());
  }
  void validate(const game::Bytes &cursor, const game::Bytes &random,
                bool attacking) const override {
    (void)randomState(random);
    if (!attacking) {
      need(cursor.empty(), "inactive melee actor retains cursor");
      return;
    }
    reconstruction::NoCdAnimationPlayer a(attack_);
    a.restore(cursorState(cursor));
    need(a.state().active, "attack requires active animation");
    // Accept only reachable states of this finite shipped forward sequence.
    auto probe = reconstruction::NoCdAnimationPlayer(attack_);
    probe.start();
    bool reachable = false;
    for (unsigned i = 0; i < 256 && probe.state().active; ++i) {
      if (cursorBytes(probe.state()) == cursor) {
        reachable = true;
        break;
      }
      probe.tick();
    }
    need(reachable, "unreachable melee cursor continuation");
  }
  game::MeleeSample advance(game::Bytes &cursor,
                            game::Bytes &random) const override {
    validate(cursor, random, true);
    auto rng = randomState(random);
    reconstruction::NoCdAnimationPlayer a(attack_);
    a.restore(cursorState(cursor));
    game::MeleeSample out;
    out.animationEvent = a.tick();
    if (out.animationEvent == 2) {
      auto r = c::resolveMelee(parameters_, [&] {
        ++out.draws;
        return c::nextRandom(rng);
      });
      need(r.amount >= 0 && r.amount <= 1000000,
           "melee result outside native domain");
      out.damage = unsigned(r.amount);
    } else if (out.animationEvent == 1) {
      auto r = c::meleeEvent({}, 1, true, true, false, false, 0, [&] {
        ++out.draws;
        return c::nextRandom(rng);
      });
      out.finished = true;
      out.cooldown = r.state.cooldown;
    }
    cursor = cursorBytes(a.state());
    random = randomBytes(rng);
    return out;
  }
};
} // namespace
Profile decodeProfile(game::Bytes b) {
  need(b.size() >= 8 && std::string(b.begin(), b.begin() + 8) == "MNMMPR01",
       "unsupported melee profile version");
  std::size_t at = 8;
  std::array<game::Bytes, 3> inputs;
  for (auto &item : inputs) {
    auto n = get(b, at);
    at += 4;
    need(n <= 4 * 1024 * 1024 && at <= b.size() && n <= b.size() - at,
         "melee profile blob extent");
    item = game::Bytes(b.begin() + at, b.begin() + at + n);
    at += n;
  }
  need(at == b.size(), "trailing melee profile bytes");
  auto creature = config(inputs[0]), hth = config(inputs[1]);
  need(field(creature, "CREATURE_17", "IsPhysical") == "TRUE" &&
           field(creature, "CREATURE_17", "IsUndead") == "FALSE" &&
           integer(field(creature, "CREATURE_17", "TileSizeXY")) == 2 &&
           integer(field(creature, "CREATURE_17", "FireRate")) == 0,
       "unsupported Crocodile profile");
  const auto maximum = integer(field(creature, "CREATURE_17", "Health"));
  need(maximum > 0 && maximum <= 1000000, "Crocodile health bound");
  c::MeleeInputs p;
  p.base = row(field(hth, "HTH_Matrix", "Creat_17"))[17];
  p.stunThreshold = row(field(hth, "Stun_Matrix", "Creat_17"))[17];
  need(p.base >= 0 && p.base <= 500000 && p.stunThreshold >= -1 &&
           p.stunThreshold <= 99,
       "Crocodile melee matrix bound");
  auto animation = assets::decodeAnimation(inputs[2]);
  need(std::holds_alternative<assets::Animation>(animation),
       "invalid Crocodile ANI");
  const auto &a = std::get<assets::Animation>(animation);
  // Explicit facing0 / caller-selected attack24. Original selector/reach is
  // outside this lab; all eight facing variants are compared separately.
  need(a.starts.size() > 25, "Crocodile attack sequence missing");
  std::vector<assets::AnimationRecord> attack(a.records.begin() + a.starts[24],
                                              a.records.begin() + a.starts[25]);
  reconstruction::NoCdAnimationPlayer probe(attack);
  probe.start();
  unsigned hits = 0;
  for (unsigned i = 0; i < 256 && probe.state().active; ++i) {
    auto e = probe.tick();
    if (e == 2)
      ++hits;
    need(e == 0 || e == 1 || e == 2 || e == 3,
         "unsupported melee animation event");
  }
  need(hits == 1 && !probe.state().active,
       "bounded one-hit Crocodile attack required");
  const auto hash = QCryptographicHash::hash(
      QByteArrayView(reinterpret_cast<const char *>(b.data()),
                     qsizetype(b.size())),
      QCryptographicHash::Sha256);
  std::uint64_t id = 0;
  for (unsigned i = 0; i < 8; ++i)
    id |= std::uint64_t(std::uint8_t(hash[i])) << (8 * i);
  need(id != 0, "empty melee identity");
  return {std::move(b), std::make_shared<Driver>(id, unsigned(maximum), p,
                                                 std::move(attack))};
}
Profile loadProfile(const std::filesystem::path &root) {
  game::Bytes b{'M', 'N', 'M', 'M', 'P', 'R', '0', '1'};
  for (const char *p : {"CFG/Encrypted/creature.cfg", "CFG/Encrypted/HTH.cfg",
                        "Creatures/crocodile.ani"}) {
    auto in = read(root / p);
    word(b, unsigned(in.size()));
    b.insert(b.end(), in.begin(), in.end());
  }
  return decodeProfile(std::move(b));
}
game::MeleeExchangeSession create(Profile p, std::uint32_t seed) {
  return {p.driver, skirmish::ordinaryHealthRules(), std::move(p.bytes), seed};
}
game::MeleeExchangeSession load(const std::filesystem::path &path) {
  auto w = game::readSnapshot(path);
  auto p = decodeProfile(w.campaign);
  game::World world(0);
  world.restore(std::move(w));
  return {std::move(world), p.driver, skirmish::ordinaryHealthRules()};
}
} // namespace mnm::melee_lab
