#include "../apps/skirmish/health_rules.hpp"
#include "persistence/snapshot.hpp"
#include "profile.hpp"
#include <iostream>
#include <stdexcept>
using namespace mnm;
namespace {
void check(bool b, const char *s) {
  if (!b)
    throw std::runtime_error(s);
}
template <class F> void refused(F f) {
  bool caught = false;
  try {
    f();
  } catch (const std::exception &) {
    caught = true;
  }
  check(caught, "malformed exchange accepted");
}
class BadDriver final : public game::MeleeDriver {
  std::shared_ptr<const game::MeleeDriver> delegate_;

public:
  bool throwing = false;
  explicit BadDriver(std::shared_ptr<const game::MeleeDriver> d)
      : delegate_(std::move(d)) {}
  std::uint64_t identity() const override { return delegate_->identity(); }
  std::uint32_t maximumHealth() const override {
    return delegate_->maximumHealth();
  }
  game::Bytes random(std::uint32_t s) const override {
    return delegate_->random(s);
  }
  game::Bytes start() const override { return delegate_->start(); }
  void validate(const game::Bytes &c, const game::Bytes &r,
                bool a) const override {
    delegate_->validate(c, r, a);
  }
  game::MeleeSample advance(game::Bytes &c, game::Bytes &r) const override {
    (void)delegate_->advance(c, r);
    if (throwing)
      throw std::runtime_error("injected driver failure");
    return {0, 1000001, 0, 0, false};
  }
};
} // namespace
int main(int argc, char **argv) try {
  check(argc == 2, "expected installation");
  auto p = melee_lab::loadProfile(argv[1]);
  auto session = melee_lab::create(p, 1);
  check(session.maximumHealth() == 360, "shipped profile health");
  auto original = game::encodeSnapshot(session.world().state());
  check(session.step({false, false, false}).empty() &&
            game::encodeSnapshot(session.world().state()) == original,
        "nonadmitted tick changed state");
  auto before = session.state();
  session.step({true, true, false});
  check(game::encodeExchange(before) == game::encodeExchange(session.state()),
        "alternate tick changed attack or RNG");
  auto bad = std::make_shared<BadDriver>(p.driver);
  game::World w(0);
  w.restore(session.world().state());
  game::MeleeExchangeSession broken(w, bad, skirmish::ordinaryHealthRules());
  const auto baseline = game::encodeSnapshot(broken.world().state());
  refused([&] { broken.step(); });
  check(game::encodeSnapshot(broken.world().state()) == baseline,
        "bad sample changed checkpoint");
  bad->throwing = true;
  refused([&] { broken.step(); });
  check(game::encodeSnapshot(broken.world().state()) == baseline,
        "throwing sample changed checkpoint");
  auto restore = [&](game::State state) {
    game::World world(0);
    world.restore(std::move(state));
    return game::MeleeExchangeSession(std::move(world), p.driver,
                                      skirmish::ordinaryHealthRules());
  };
  unsigned corruptions = 0;
  for (unsigned mode = 0; mode < 9; ++mode) {
    auto state = session.world().state();
    auto s = session.state();
    switch (mode) {
    case 0:
      s.driver ^= 1;
      break;
    case 1:
      s.healthRules ^= 1;
      break;
    case 2:
      s.random[0] = 255;
      break;
    case 3:
      s.actors[0].cursor[0] = 255;
      break;
    case 4:
      s.actors[0].health = 361;
      break;
    case 5:
      s.actors[0].actor.generation++;
      break;
    case 6:
      s.actors[0].phase = game::ExchangePhase::recovering;
      s.actors[0].cooldown = 0;
      break;
    case 7:
      state.slots[0].entity->x++;
      break;
    case 8:
      s.actors[0].cursor[16] = 1;
      break;
    }
    state.systems = game::encodeExchange(s);
    refused([&] { restore(state); });
    ++corruptions;
  }
  unsigned hits = 0, deaths = 0, resumes = 0;
  for (unsigned tick = 0; tick < 4096 && !session.finished(); ++tick) {
    auto saved = session.world().state();
    auto clone = restore(game::decodeSnapshot(game::encodeSnapshot(saved)));
    auto a = session.step(), b = clone.step();
    check(game::encodeSnapshot(session.world().state()) ==
              game::encodeSnapshot(clone.world().state()),
          "one-tick restored continuation differs");
    check(a.size() == b.size(), "restored event count differs");
    ++resumes;
    for (auto e : a) {
      hits += e.animation == 2;
      deaths += e.died;
    }
  }
  check(session.finished() && deaths == 1 && hits > 1,
        "complete ordinary exchange");
  const auto final = game::encodeSnapshot(session.world().state());
  check(session.step().empty() &&
            game::encodeSnapshot(session.world().state()) == final,
        "terminal exchange advanced");
  std::cout << "{\"success\":true,\"resumes\":" << resumes
            << ",\"refusals\":" << corruptions << ",\"hits\":" << hits
            << ",\"deaths\":" << deaths
            << ",\"tick\":" << session.world().state().tick << "}\n";
  return 0;
} catch (const std::exception &e) {
  std::cerr << e.what() << '\n';
  return 1;
}
