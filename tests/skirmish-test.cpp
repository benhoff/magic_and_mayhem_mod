#include "arena.hpp"
#include "persistence/snapshot.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <unistd.h>
using namespace mnm::game;
namespace {
void check(bool value, const char *reason) {
    if (!value)
        throw std::runtime_error(reason);
}
template <class F> void refused(F action) {
    bool caught = false;
    try {
        action();
    } catch (const std::exception &) {
        caught = true;
    }
    check(caught, "expected refusal");
}
Bytes bytes(const SkirmishSession &s) { return encodeSnapshot(s.world().state()); }
SkirmishSession rebound(const SkirmishSession &s) {
    World w(0);
    w.restore(decodeSnapshot(bytes(s)));
    return {std::move(w), mnm::skirmish::arenaNavigation()};
}
SkirmishSession edited(const SkirmishSession &s, SkirmishState meta, CombatState combat) {
    auto state = s.world().state();
    state.campaign = encodeSkirmish(meta);
    state.systems = encodeCombat(combat);
    World w(0);
    w.restore(state);
    return {std::move(w), mnm::skirmish::arenaNavigation()};
}
SkirmishSession composed(CombatSession combat, SkirmishState meta) {
    auto state = combat.world().state();
    state.campaign = encodeSkirmish(meta);
    World w(0);
    w.restore(state);
    return {std::move(w), mnm::skirmish::arenaNavigation()};
}
const Handle player{0, 1}, enemy{1, 1};
SkirmishOrder summon(Point p = {2, 5, 1}) { return {SkirmishOrderKind::summon, player, {}, p}; }
SkirmishOrder bolt(Handle h = enemy) { return {SkirmishOrderKind::bolt, player, h, {}}; }
bool event(const SkirmishReport &report, SkirmishEventKind kind) {
    return std::any_of(report.events.begin(), report.events.end(),
                       [&](const auto &e) { return e.kind == kind; });
}
void timingAndContinuation() {
    SkirmishSession s(mnm::skirmish::arenaNavigation());
    s.enqueue(bolt());
    auto initial = bytes(s);
    s.step({false, false, false});
    check(bytes(s) == initial, "nonadmission changed pending spell");
    auto copy = rebound(s);
    s.step();
    copy.step();
    check(bytes(s) == bytes(copy), "pending order continuation differs");
    check(s.state().wizards[0].mana == 16 && s.state().wizards[0].remaining == 2 &&
              s.state().projectiles.empty(),
          "cast admission debit/windup");
    auto before = s.state();
    s.step({true, true, false});
    check(encodeSkirmish(before) == encodeSkirmish(s.state()),
          "alternate admission advanced spell");
    s.step();
    check(s.state().wizards[0].remaining == 1, "cast skipped windup");
    auto r = s.step();
    check(event(r, SkirmishEventKind::launched) && s.state().projectiles[0].remaining == 4,
          "launch skipped flight tick");
    for (int i = 0; i < 3; ++i) {
        auto resumed = rebound(s);
        r = s.step();
        resumed.step();
        check(bytes(s) == bytes(resumed), "flight continuation differs");
        check(s.combat().combat().fighters[1].health == 48, "projectile hit early");
    }
    r = s.step();
    check(event(r, SkirmishEventKind::impact) && s.combat().combat().fighters[1].health == 38,
          "impact health/timing");
    check(s.world().find(player)->x == 1 && s.world().find(enemy)->x == 6, "wizard moved");
}
void ordersAndRefusals() {
    SkirmishSession s(mnm::skirmish::arenaNavigation());
    auto before = bytes(s);
    refused([&] { s.enqueue({SkirmishOrderKind::move, player, {}, Point{2, 5, 1}}); });
    refused([&] { s.enqueue({SkirmishOrderKind::summon, enemy, {}, Point{5, 2, 1}}); });
    refused([&] { s.enqueue(bolt(player)); });
    refused([&] { s.enqueue(bolt({1, 2})); });
    refused([&] { s.enqueue(summon({0, 0, 1})); });
    check(bytes(s) == before, "refused enqueue changed world");
    s.enqueue(summon());
    s.enqueue(bolt());
    auto r = s.step();
    check(event(r, SkirmishEventKind::refused) && s.state().wizards[0].mana == 12,
          "busy refusal changed debit");
    s.step();
    s.step();
    check(s.state().units.size() == 4, "summon completion");
    const Handle sentinel{2, 1};
    s.enqueue({SkirmishOrderKind::move, sentinel, {}, Point{1, 4, 1}});
    auto saved = rebound(s);
    s.step();
    saved.step();
    check(bytes(s) == bytes(saved), "pending movement continuation");
    s.enqueue({SkirmishOrderKind::stop, sentinel, {}, {}});
    s.step();
    check(s.combat().combat().fighters[2].control == CombatControl::hold, "stop failed to hold");
    s.enqueue({SkirmishOrderKind::attack, sentinel, enemy, {}});
    s.step();
    check(s.combat().combat().fighters[2].control == CombatControl::attack,
          "attack did not retain explicit target");
    auto meta = s.state();
    meta.wizards[0].mana = 0;
    auto poor = edited(s, meta, s.combat().combat());
    poor.enqueue(bolt());
    r = poor.step();
    check(event(r, SkirmishEventKind::refused) && poor.state().wizards[0].spell == Spell::none,
          "insufficient mana started cast");
    SkirmishSession occupied(mnm::skirmish::arenaNavigation());
    occupied.enqueue(summon({1, 6, 1}));
    r = occupied.step();
    check(event(r, SkirmishEventKind::refused) && occupied.state().wizards[0].mana == 20,
          "blocked placement spent mana");
    SkirmishSession queue(mnm::skirmish::arenaNavigation());
    for (int i = 0; i < 64; ++i)
        queue.enqueue(bolt());
    before = bytes(queue);
    refused([&] { queue.enqueue(bolt()); });
    check(bytes(queue) == before, "queue overflow changed world");
}
void completeMatch() {
    SkirmishSession s(mnm::skirmish::arenaNavigation());
    unsigned continuations = 0;
    for (unsigned tick = 0; tick < 100 && s.state().outcome == Outcome::running; ++tick) {
        if (tick == 0)
            s.enqueue(summon());
        if (tick == 3 || tick == 6 || tick == 9 || tick == 12)
            s.enqueue(bolt());
        auto resumed = rebound(s);
        s.step();
        resumed.step();
        check(bytes(s) == bytes(resumed), "match continuation differs");
        ++continuations;
    }
    check(s.state().outcome == Outcome::victory && continuations < 100,
          "authored victory not reached");
    auto terminal = bytes(s);
    s.step();
    check(bytes(s) == terminal, "terminal match advanced");
    refused([&] { s.enqueue(bolt()); });
    SkirmishSession idle(mnm::skirmish::arenaNavigation());
    for (unsigned i = 0; i < 200 && idle.state().outcome == Outcome::running; ++i)
        idle.step();
    check(idle.state().outcome == Outcome::defeat, "idle rival never wins");
    std::cout << "Victory at " << continuations << " ticks; defeat at " << idle.world().state().tick
              << " ticks.\n";
}
void interruptionsAndBounds() {
    SkirmishSession s(mnm::skirmish::arenaNavigation());
    s.enqueue(summon());
    s.step();
    auto combat = s.combat();
    auto meta = s.state();
    Entity e;
    e.type = s.world().find(player)->type;
    e.owner = 0;
    e.x = 2;
    e.y = 5;
    e.z = 1;
    auto h = combat.spawn(e, {24, 6, 2, 3}, false, CombatControl::hold);
    meta.units.push_back({h, UnitRole::sentinel});
    auto occupied = composed(combat, meta);
    occupied.step();
    auto r = occupied.step();
    check(event(r, SkirmishEventKind::refused) && occupied.state().wizards[0].mana == 20,
          "completion collision did not refund bounded mana");
    auto current = s.state();
    current.wizards[0].mana = 20;
    current.wizards[0].spell = Spell::none;
    current.wizards[0].remaining = 0;
    current.wizards[0].aim = {};
    auto saturated = edited(s, current, s.combat().combat());
    for (int i = 0; i < 9; ++i)
        saturated.step();
    check(saturated.state().wizards[0].mana == 20, "regeneration exceeds maximum");
    SkirmishSession moving(mnm::skirmish::arenaNavigation());
    for (int i = 0; i < 3; ++i)
        moving.step();
    moving.enqueue(bolt({2, 1}));
    bool missed = false;
    for (int i = 0; i < 8; ++i)
        missed = event(moving.step(), SkirmishEventKind::missed) || missed;
    check(missed, "fixed-aim projectile followed moving target");
    SkirmishSession draw(mnm::skirmish::arenaNavigation());
    meta = draw.state();
    auto fighters = draw.combat().combat();
    fighters.fighters[0].health = 10;
    fighters.fighters[1].health = 10;
    meta.nextProjectile = 3;
    meta.projectiles = {{1, 0, player, enemy, {1, 6, 1}, {6, 1, 1}, 1},
                        {2, 1, enemy, player, {6, 1, 1}, {1, 6, 1}, 1}};
    auto simultaneous = edited(draw, meta, fighters);
    simultaneous.step();
    check(simultaneous.state().outcome == Outcome::draw,
          "committed airborne bolts did not resolve simultaneous terminal damage");
    meta = draw.state();
    meta.nextProjectile = 33;
    for (unsigned i = 1; i <= 32; ++i)
        meta.projectiles.push_back({i, 0, player, enemy, {1, 6, 1}, {6, 1, 1}, 4});
    auto full = edited(draw, meta, draw.combat().combat());
    full.enqueue(bolt());
    full.step();
    full.step();
    r = full.step();
    check(event(r, SkirmishEventKind::refused), "projectile capacity not refused at launch");
    meta = draw.state();
    meta.pending.push_back(bolt());
    auto pending = edited(draw, meta, draw.combat().combat());
    auto state = pending.world().state();
    state.slots[1].generation = 2;
    World invalid(0);
    invalid.restore(state);
    refused([&] { SkirmishSession stale(std::move(invalid), mnm::skirmish::arenaNavigation()); });
}
void restoreRollback() {
    SkirmishSession s(mnm::skirmish::arenaNavigation());
    s.enqueue(bolt());
    s.step();
    auto before = bytes(s);
    const auto directory =
        std::filesystem::path(std::getenv("TMPDIR") ? std::getenv("TMPDIR") : "/tmp") /
        ("mnm-skirmish-restore-" + std::to_string(getpid()));
    check(std::filesystem::create_directory(directory), "restore directory collision");
    struct Cleanup {
        std::filesystem::path path;
        ~Cleanup() { std::filesystem::remove_all(path); }
    } cleanup{directory};
    const auto good = directory / "good.mnms";
    check(writeSnapshot(good, s.world().state()).durable, "write checkpoint");
    s.restore(good, [](const State &) { return mnm::skirmish::arenaNavigation(); });
    check(bytes(s) == before, "restore changed exact cast state");
    refused([&] { s.restore(good, {}); });
    refused([&] {
        s.restore(good, [](const State &) -> std::shared_ptr<const Navigation> { return {}; });
    });
    refused([&] {
        s.restore(good, [](const State &) -> std::shared_ptr<const Navigation> {
            throw std::runtime_error("missing resources");
        });
    });
    check(bytes(s) == before, "failed resolver changed owned state");
    auto bad = s.world().state();
    bad.navigation->fingerprint ^= 1;
    const auto wrong = directory / "wrong.mnms";
    check(writeSnapshot(wrong, bad).durable, "write mismatch fixture");
    refused(
        [&] { s.restore(wrong, [](const State &) { return mnm::skirmish::arenaNavigation(); }); });
    check(bytes(s) == before, "wrong resources changed owned state");
}
void malformed() {
    SkirmishSession s(mnm::skirmish::arenaNavigation());
    auto meta = s.state();
    auto combat = s.combat().combat();
    auto inconsistent = s.world().state();
    inconsistent.slots[0].entity->motion->action = Action::planning;
    inconsistent.slots[0].entity->motion->destination = {2, 5, 1};
    World movingWizard(0);
    movingWizard.restore(inconsistent);
    refused(
        [&] { SkirmishSession moving(std::move(movingWizard), mnm::skirmish::arenaNavigation()); });
    auto control = combat;
    control.fighters[0].control = static_cast<CombatControl>(99);
    refused([&] { edited(s, meta, control); });
    auto badCombat = s.world().state().systems;
    badCombat[16 + 48 + 4] = 2;
    refused([&] { decodeCombat(badCombat); });
    meta.units[0].role = UnitRole::sentinel;
    refused([&] { edited(s, meta, combat); });
    meta = s.state();
    meta.wizards[0].mana = 21;
    refused([&] { edited(s, meta, combat); });
    meta = s.state();
    meta.wizards[0].remaining = 1;
    refused([&] { edited(s, meta, combat); });
    meta = s.state();
    meta.pending.push_back({SkirmishOrderKind::move, enemy, {}, Point{5, 2, 1}});
    refused([&] { edited(s, meta, combat); });
    meta = s.state();
    meta.outcome = Outcome::victory;
    refused([&] { edited(s, meta, combat); });
    auto raw = s.world().state().campaign;
    raw[8] ^= 1;
    refused([&] { decodeSkirmish(raw); });
    raw = s.world().state().campaign;
    raw.push_back(0);
    refused([&] { decodeSkirmish(raw); });
    for (unsigned n : {0U, 8U, 27U, 100U}) {
        raw = s.world().state().campaign;
        raw.resize(n);
        refused([&] { decodeSkirmish(raw); });
    }
}
} // namespace
int main() try {
    timingAndContinuation();
    ordersAndRefusals();
    completeMatch();
    interruptionsAndBounds();
    restoreRollback();
    malformed();
    std::cout << "Skirmish contract checks pass.\n";
    return 0;
} catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
}
