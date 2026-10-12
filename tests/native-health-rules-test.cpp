#include "arena.hpp"
#include "health_rules.hpp"
#include "persistence/snapshot.hpp"
#include "simulation/occupancy.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
using namespace mnm::game;
namespace {
void check(bool v, const char *reason) {
    if (!v)
        throw std::runtime_error(reason);
}
template <class F> void refused(F f) {
    bool caught = false;
    try {
        f();
    } catch (const std::exception &) {
        caught = true;
    }
    check(caught, "expected refusal");
}
Bytes bytes(const CombatSession &s) { return encodeSnapshot(s.world().state()); }
Entity body(int x, unsigned owner) {
    Entity e;
    e.x = x;
    e.y = 2;
    e.z = 1;
    e.owner = owner;
    return e;
}
CombatSession session(std::shared_ptr<const HealthRules> rules) {
    auto nav = mnm::skirmish::arenaNavigation();
    World world(4);
    auto s = world.state();
    s.map = "ordinary-health-test";
    s.navigation = nav->binding();
    world.restore(s);
    return {std::move(world), nav, std::move(rules)};
}
class FaultRules final : public HealthRules {
  public:
    std::uint64_t identity() const override { return 3; }
    std::uint32_t change(std::uint32_t current, std::uint32_t, std::int32_t delta,
                         bool) const override {
        return delta < 0 ? current + 1 : 0;
    }
};
class ThrowRules final : public HealthRules {
  public:
    std::uint64_t identity() const override { return 4; }
    std::uint32_t change(std::uint32_t, std::uint32_t, std::int32_t, bool) const override {
        throw std::runtime_error("backend failed");
    }
};
void healthAndBinding() {
    auto s = session(mnm::skirmish::ordinaryHealthRules());
    auto a = s.spawn(body(1, 0), {24, 6, 2, 3});
    auto b = s.spawn(body(6, 1), {24, 6, 2, 3});
    check(s.combat().healthRulesIdentity == mnm::skirmish::ordinaryHealthIdentity,
          "missing recovered health binding");
    check(std::string(s.world().state().systems.begin(), s.world().state().systems.begin() + 8) ==
              "MNMCOM03",
          "expected v3 bound checkpoint");
    auto r = s.damage(b, 10, a);
    check(r.events.size() == 1 && r.events[0].amount == 10 && s.combat().fighters[1].health == 14,
          "defended native damage");
    r = s.heal(b, 8, a);
    check(r.events.size() == 1 && r.events[0].kind == CombatEventKind::healed &&
              r.events[0].amount == 8 && s.combat().fighters[1].health == 22,
          "direct healing");
    r = s.heal(b, 1000000, a);
    check(r.events[0].amount == 2 && s.combat().fighters[1].health == 24,
          "original heal cap not applied");
    World world(0);
    world.restore(decodeSnapshot(bytes(s)));
    CombatSession resumed(std::move(world), mnm::skirmish::arenaNavigation(),
                          mnm::skirmish::ordinaryHealthRules());
    check(bytes(resumed) == bytes(s), "rules-bound restore differs");
    World wrong(0);
    wrong.restore(s.world().state());
    refused([&] { CombatSession mismatch(std::move(wrong), mnm::skirmish::arenaNavigation()); });
    r = s.damage(b, 1000000, a);
    check(r.events.size() == 2 && r.events[0].amount == 24 &&
              r.events[1].kind == CombatEventKind::died && s.world().find(b)->cleaned,
          "ordinary numeric death did not use owned cleanup");
    Occupancy occupied(s.world().state(), {8, 8, 3}, 0, {1, 1, 1});
    check(!occupied.owner({6, 2, 1}), "dead creature retained occupancy");
    auto before = bytes(s);
    refused([&] { s.heal(b, 1, a); });
    check(bytes(s) == before, "dead-target healing mutated state");
    refused([&] { s.heal(a, 0, a); });
    refused([&] { s.heal(a, 1000001, a); });
    refused([&] { s.heal(a, 1, {0, 2}); });
    check(bytes(s) == before, "invalid healing changed state");
    refused([&] { mnm::skirmish::resolveHealthRules(99); });
    refused([&] { mnm::skirmish::ordinaryHealthRules()->change(25, 24, 1, false); });
    auto bad = s.world().state().systems;
    bad[16] = 0;
    bad[17] = 0;
    bad[18] = 0;
    bad[19] = 0;
    bad[20] = 0;
    bad[21] = 0;
    bad[22] = 0;
    bad[23] = 0;
    refused([&] { decodeCombat(bad); });
}
void transaction() {
    auto fault = session(std::make_shared<FaultRules>());
    auto a = fault.spawn(body(1, 0));
    auto b = fault.spawn(body(6, 1));
    auto before = bytes(fault);
    refused([&] { fault.damage(b, 1, a); });
    refused([&] { fault.heal(b, 1, a); });
    check(bytes(fault) == before, "bad health backend published state");
    auto throwing = session(std::make_shared<ThrowRules>());
    throwing.spawn(body(1, 0));
    throwing.spawn(body(2, 1));
    throwing.step();
    throwing.step();
    before = bytes(throwing);
    refused([&] { throwing.step(); });
    check(bytes(throwing) == before, "post-movement backend failure published partial combat tick");
}
void legacyAndSkirmish() {
    const auto directory =
        std::filesystem::path(std::getenv("TMPDIR") ? std::getenv("TMPDIR") : "/tmp") /
        ("mnm-health-binding-" + std::to_string(getpid()));
    check(std::filesystem::create_directory(directory), "test directory collision");
    struct Cleanup {
        std::filesystem::path p;
        ~Cleanup() { std::filesystem::remove_all(p); }
    } cleanup{directory};
    SkirmishSession legacy(mnm::skirmish::arenaNavigation());
    const auto old = directory / "legacy.mnms";
    check(writeSnapshot(old, legacy.world().state()).durable, "legacy publication");
    auto loaded = mnm::skirmish::load(old);
    check(loaded.combat().combat().healthRulesIdentity == 0 &&
              encodeSnapshot(loaded.world().state()) == encodeSnapshot(legacy.world().state()),
          "legacy rules silently migrated");
    auto native = mnm::skirmish::newSession();
    check(native.combat().combat().healthRulesIdentity == mnm::skirmish::ordinaryHealthIdentity,
          "arena did not choose recovered health");
    native.enqueue({SkirmishOrderKind::summon, {0, 1}, {}, Point{2, 5, 1}});
    const auto checkpoint = directory / "pending.mnms";
    check(writeSnapshot(checkpoint, native.world().state()).durable, "bound arena publication");
    auto resumed = mnm::skirmish::load(checkpoint);
    native.step();
    resumed.step();
    check(encodeSnapshot(native.world().state()) == encodeSnapshot(resumed.world().state()),
          "pending skirmish health binding continuation");
    const auto before = encodeSnapshot(native.world().state());
    refused([&] {
        native.restore(old, [](const State &) { return mnm::skirmish::arenaNavigation(); });
    });
    check(encodeSnapshot(native.world().state()) == before,
          "wrong health backend restore mutated skirmish");
}
} // namespace
int main() try {
    healthAndBinding();
    transaction();
    legacyAndSkirmish();
    std::cout << "Recovered ordinary health, heal cap/death, bound/legacy checkpoints and backend "
                 "rollback pass.\n";
    return 0;
} catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
}
