#include "combat.hpp"
#include "occupancy.hpp"
#include "persistence/snapshot.hpp"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <stdexcept>
#include <tuple>

namespace mnm::game {
namespace {
constexpr std::array<std::uint8_t, 8> magic{{'M', 'N', 'M', 'C', 'O', 'M', '0', '1'}};
constexpr std::array<std::uint8_t, 8> magic3{{'M', 'N', 'M', 'C', 'O', 'M', '0', '3'}};
constexpr std::array<std::uint8_t, 8> magic2{{'M', 'N', 'M', 'C', 'O', 'M', '0', '2'}};
void require(bool v, const char *reason) {
    if (!v)
        throw std::invalid_argument(reason);
}
void statsValid(const CombatStats &s) {
    require(s.maximumHealth >= 1 && s.maximumHealth <= 1000000 && s.damage >= 1 &&
                s.damage <= 1000000 && s.windupTicks >= 1 && s.windupTicks <= 100000 &&
                s.recoveryTicks >= 1 && s.recoveryTicks <= 100000,
            "invalid native combat stats");
}
const Entity *entity(const State &s, Handle h) {
    if (h.slot >= s.slots.size())
        return nullptr;
    const auto &slot = s.slots[h.slot];
    return slot.generation == h.generation && slot.entity ? &*slot.entity : nullptr;
}
Fighter *fighter(CombatState &c, Handle h) {
    for (auto &f : c.fighters)
        if (f.actor == h)
            return &f;
    return nullptr;
}
std::int64_t distance(const Entity &a, const Entity &b) {
    return std::max(std::llabs(std::int64_t(a.x) - b.x), std::llabs(std::int64_t(a.y) - b.y));
}
bool adjacent(const Entity &a, const Entity &b) { return a.z == b.z && distance(a, b) == 1; }
Point position(const Entity &e) { return {e.x, e.y, e.z}; }
bool eligible(const State &s, const Fighter &a, const Fighter &b, std::uint32_t range) {
    const auto *own = entity(s, a.actor);
    const auto *other = entity(s, b.actor);
    return own && other && !(a.actor == b.actor) && b.health && !other->cleaned &&
           own->owner != other->owner && own->z == other->z && distance(*own, *other) <= range;
}
void clearAttack(Fighter &f) {
    f.target.reset();
    f.remaining = 0;
    f.action = f.health ? CombatAction::idle : CombatAction::dead;
    if (f.control == CombatControl::attack || !f.health) {
        if (f.control != CombatControl::automatic)
            f.control = CombatControl::hold;
        f.destination.reset();
    }
}
void orderValid(const CombatOrder &o) {
    require(static_cast<unsigned>(o.kind) <= 2 &&
                ((o.kind == CombatOrderKind::attack) == bool(o.target)) &&
                ((o.kind == CombatOrderKind::move) == bool(o.destination)),
            "invalid combat order encoding");
}
void stopMotion(Entity &e) {
    auto &m = *e.motion;
    m.pose = e.cleaned ? std::nullopt : displayedPose(m);
    m.action = Action::cancelled;
    m.route.clear();
    m.next = 0;
    m.goal.reset();
    m.origin = position(e);
    m.fine.reset();
    m.previous.reset();
    m.segmentTicks = 0;
}
void applyDamage(State &s, CombatState &c, Fighter &victim, std::uint32_t amount, Handle source,
                 std::vector<CombatEvent> &events, const HealthRules &rules) {
    if (!victim.health)
        return;
    const auto before = victim.health;
    const auto after =
        rules.change(before, victim.stats.maximumHealth, -static_cast<std::int32_t>(amount), true);
    require(after <= before, "damage backend increased health");
    amount = before - after;
    victim.health = after;
    if (!amount)
        return;
    events.push_back({CombatEventKind::hit, source, victim.actor, amount});
    if (victim.health)
        return;
    auto &e = *s.slots[victim.actor.slot].entity;
    e.cleaned = true;
    e.target.reset();
    stopMotion(e);
    clearAttack(victim);
    events.push_back({CombatEventKind::died, victim.actor, source, 0});
    for (auto &peer : c.fighters)
        if (peer.target == std::optional<Handle>(victim.actor)) {
            clearAttack(peer);
            stopMotion(*s.slots[peer.actor.slot].entity);
        }
}
std::optional<Point> approach(const State &s, const Navigation &nav, Handle h,
                              const Entity &target) {
    const auto &own = *entity(s, h);
    MovementReservations occupied(s, nav.binding().dimensions, nav.creatureType(),
                                  nav.creatureFootprint());
    std::vector<Point> candidates;
    for (int y = -1; y <= 1; ++y)
        for (int x = -1; x <= 1; ++x) {
            if (!x && !y)
                continue;
            Point p{target.x + x, target.y + y, target.z};
            if (!contains(p, nav.binding()))
                continue;
            // Reservations include unfinished fine edges, not only logical positions.
            bool free = true;
            const auto footprint = nav.creatureFootprint();
            for (int z = 0; z < footprint.z; ++z) {
                Point cell{p.x, p.y, p.z + z};
                if (!contains(cell, nav.binding())) {
                    free = false;
                    break;
                }
                const auto owner = occupied.owner(cell);
                if (owner && !(*owner == h)) {
                    free = false;
                    break;
                }
            }
            if (free)
                candidates.push_back(p);
        }
    const auto key = [&](Point p) {
        return std::tuple{
            std::max(std::llabs(std::int64_t(own.x) - p.x), std::llabs(std::int64_t(own.y) - p.y)),
            p.y, p.x};
    };
    std::sort(candidates.begin(), candidates.end(),
              [&](Point a, Point b) { return key(a) < key(b); });
    if (candidates.empty())
        return {};
    return candidates.front(); // Unreachable endpoints remain explicitly blocked.
}
} // namespace
Bytes encodeCombat(const CombatState &c) {
    require(c.acquisitionRange >= 1 && c.acquisitionRange <= 64 && c.fighters.size() <= 32,
            "invalid combat bounds");
    const bool extended = c.healthRulesIdentity || !c.pending.empty() ||
                          std::any_of(c.fighters.begin(), c.fighters.end(), [](const Fighter &f) {
                              return f.control != CombatControl::automatic || bool(f.destination);
                          });
    require(c.pending.size() <= 64, "combat order capacity exceeded");
    const auto &tag = c.healthRulesIdentity ? magic3 : extended ? magic2 : magic;
    Bytes b(tag.begin(), tag.end());
    const auto word = [&](std::uint32_t n) {
        for (unsigned i = 0; i < 4; ++i)
            b.push_back(std::uint8_t(n >> (8 * i)));
    };
    word(c.acquisitionRange);
    word(static_cast<std::uint32_t>(c.fighters.size()));
    if (c.healthRulesIdentity) {
        word(static_cast<std::uint32_t>(c.healthRulesIdentity));
        word(static_cast<std::uint32_t>(c.healthRulesIdentity >> 32));
    }
    for (const auto &f : c.fighters) {
        statsValid(f.stats);
        word(f.actor.slot);
        word(f.actor.generation);
        word(f.stats.maximumHealth);
        word(f.stats.damage);
        word(f.stats.windupTicks);
        word(f.stats.recoveryTicks);
        word(f.health);
        word(static_cast<std::uint32_t>(f.action));
        word(f.remaining);
        word(f.target ? 1 : 0);
        word(f.target ? f.target->slot : 0);
        word(f.target ? f.target->generation : 0);
        if (extended) {
            word(static_cast<unsigned>(f.control));
            word(f.destination ? 1 : 0);
            const auto p = f.destination.value_or(Point{});
            word(static_cast<unsigned>(p.x));
            word(static_cast<unsigned>(p.y));
            word(static_cast<unsigned>(p.z));
        }
    }
    if (extended) {
        word(static_cast<unsigned>(c.pending.size()));
        for (const auto &o : c.pending) {
            orderValid(o);
            word(static_cast<unsigned>(o.kind));
            word(o.subject.slot);
            word(o.subject.generation);
            word(o.target ? 1 : 0);
            word(o.target ? o.target->slot : 0);
            word(o.target ? o.target->generation : 0);
            const auto p = o.destination.value_or(Point{});
            word(static_cast<unsigned>(p.x));
            word(static_cast<unsigned>(p.y));
            word(static_cast<unsigned>(p.z));
        }
    }
    return b;
}
CombatState decodeCombat(const Bytes &bytes) {
    const auto sections = decodeBattleSystems(bytes);
    const auto &b = sections.combat;
    require(b.size() >= 16, "truncated native combat header");
    const bool bound = std::equal(magic3.begin(), magic3.end(), b.begin());
    const bool extended = bound || std::equal(magic2.begin(), magic2.end(), b.begin());
    require(extended || std::equal(magic.begin(), magic.end(), b.begin()),
            "unknown native combat subsystem format");
    std::size_t p = 8;
    const auto word = [&]() {
        require(p <= b.size() && b.size() - p >= 4, "truncated native combat state");
        std::uint32_t v = 0;
        for (unsigned i = 0; i < 4; ++i)
            v |= std::uint32_t(b[p++]) << (8 * i);
        return v;
    };
    CombatState c;
    c.acquisitionRange = word();
    const auto count = word();
    if (bound) {
        const auto low = word(), high = word();
        c.healthRulesIdentity = std::uint64_t(high) << 32 | low;
        require(c.healthRulesIdentity != 0, "empty v3 health rules identity");
    }
    require(count <= 32 &&
                b.size() >= (bound ? 24U : 16U) + std::size_t(count) * (extended ? 68 : 48),
            "invalid native combat record count/size");
    const auto point = [&]() {
        Point v;
        auto signedWord = [&]() {
            const auto n = word();
            return static_cast<std::int32_t>(n <= 0x7fffffffU ? n
                                                              : std::int64_t(n) - 0x100000000LL);
        };
        v.x = signedWord();
        v.y = signedWord();
        v.z = signedWord();
        return v;
    };
    for (std::uint32_t i = 0; i < count; ++i) {
        Fighter f;
        f.actor.slot = word();
        f.actor.generation = word();
        f.stats.maximumHealth = word();
        f.stats.damage = word();
        f.stats.windupTicks = word();
        f.stats.recoveryTicks = word();
        f.health = word();
        f.action = static_cast<CombatAction>(word());
        f.remaining = word();
        const auto present = word();
        Handle target;
        target.slot = word();
        target.generation = word();
        require(present <= 1 && (present || (!target.slot && !target.generation)),
                "invalid native combat target encoding");
        if (present)
            f.target = target;
        if (extended) {
            f.control = static_cast<CombatControl>(word());
            const auto flag = word();
            const auto pos = point();
            require(flag <= 1 && (flag || pos == Point{}), "invalid combat destination flag");
            if (flag)
                f.destination = pos;
        }
        c.fighters.push_back(f);
    }
    if (extended) {
        const auto n = word();
        require(n <= 64 && b.size() - p == std::size_t(n) * 36, "invalid combat order count/size");
        for (unsigned i = 0; i < n; ++i) {
            CombatOrder o;
            o.kind = static_cast<CombatOrderKind>(word());
            o.subject = {word(), word()};
            const auto flag = word();
            Handle target{word(), word()};
            const auto pos = point();
            require(flag <= 1 && (flag || target == Handle{}), "invalid combat order target flag");
            if (flag)
                o.target = target;
            if (o.kind == CombatOrderKind::move)
                o.destination = pos;
            else
                require(pos == Point{}, "unexpected combat order coordinates");
            orderValid(o);
            c.pending.push_back(o);
        }
    }
    require(p == b.size(), "native combat trailing bytes");
    return c;
}
void validateCombat(const State &state, const CombatState &c) {
    (void)encodeCombat(c);
    std::optional<std::uint32_t> previous;
    for (const auto &f : c.fighters) {
        const auto *e = entity(state, f.actor);
        require(e && e->family == Family::creature && e->motion,
                "combat actor identity unavailable");
        require(!previous || f.actor.slot > *previous,
                "combat actors must be unique and ordered by slot");
        previous = f.actor.slot;
        require(f.health <= f.stats.maximumHealth && static_cast<std::uint32_t>(f.action) <= 4,
                "invalid combat health/action");
        require(static_cast<unsigned>(f.control) <= 3 &&
                    ((f.control == CombatControl::move) == bool(f.destination)),
                "invalid combat control/destination");
        if (f.destination)
            require(state.navigation && contains(*f.destination, *state.navigation) && f.health,
                    "invalid ordered movement destination");
        if (f.control == CombatControl::hold || f.control == CombatControl::move)
            require(!f.target && !f.remaining &&
                        (f.action == CombatAction::idle || f.action == CombatAction::dead),
                    "nonattack control retains combat target/clock");
        if (f.control == CombatControl::hold)
            require(!active(e->motion->action), "held actor retains active movement");
        if (f.control == CombatControl::move)
            require(active(e->motion->action) && e->motion->destination == *f.destination,
                    "ordered movement/control mismatch");
        if (f.control == CombatControl::attack)
            require(f.target.has_value(), "explicit attack lost its target");
        require((f.health == 0) == (f.action == CombatAction::dead) &&
                    (f.health == 0) == e->cleaned,
                "combat death/cleanup mismatch");
        const bool timed =
            f.action == CombatAction::windingUp || f.action == CombatAction::recovering;
        require(timed ? (f.remaining >= 1 && f.remaining <= (f.action == CombatAction::windingUp
                                                                 ? f.stats.windupTicks
                                                                 : f.stats.recoveryTicks))
                      : f.remaining == 0,
                "invalid combat phase clock");
        require((f.action == CombatAction::idle || f.action == CombatAction::dead) ? !f.target
                                                                                   : bool(f.target),
                "combat phase requires a living target");
        if (f.target) {
            const auto it = std::find_if(c.fighters.begin(), c.fighters.end(),
                                         [&](const Fighter &b) { return b.actor == *f.target; });
            require(it != c.fighters.end() && eligible(state, f, *it, c.acquisitionRange),
                    "invalid combat target identity/hostility/range");
        }
    }
    const auto effects = state.systems.empty() ? BattleEffectsState{} : decodeBattleSystems(state.systems).effects;
    for (auto h : effects.entities) {
        const auto *e = entity(state, h);
        require(e && e->family == Family::missileEffect && !e->motion && !e->cleaned,
                "invalid registered battle effect");
    }
    for (std::uint32_t i = 0; i < state.slots.size(); ++i)
        if (state.slots[i].entity) {
            const auto it =
                std::find_if(c.fighters.begin(), c.fighters.end(), [&](const Fighter &f) {
                    return f.actor == Handle{i, state.slots[i].generation};
                });
            const auto h = Handle{i, state.slots[i].generation};
            require(it != c.fighters.end() ||
                        std::find(effects.entities.begin(), effects.entities.end(), h) != effects.entities.end(),
                    "combat world contains an unregistered entity");
        }
    for (const auto &o : c.pending) {
        orderValid(o);
        if (o.destination)
            require(state.navigation && contains(*o.destination, *state.navigation),
                    "ordered move outside map");
    }
}
CombatSession::CombatSession(World world, std::shared_ptr<const Navigation> nav,
                             std::shared_ptr<const HealthRules> rules)
    : movement_(std::move(world), nav), navigation_(std::move(nav)),
      healthRules_(rules ? std::move(rules) : nativeFixtureHealthRules()) {
    require(navigation_->maxMovingCreatures() > 1 && navigation_->creatureFootprint().x == 1 &&
                navigation_->creatureFootprint().y == 1,
            "combat requires bounded same-profile multi-creature navigation");
    auto state = movement_.world().state();
    if (state.systems.empty()) {
        CombatState initial;
        initial.healthRulesIdentity = healthRules_->identity();
        state.systems = encodeCombat(initial);
    }
    const auto c = decodeCombat(state.systems);
    require(c.healthRulesIdentity == healthRules_->identity(),
            "combat health rules binding mismatch");
    validateCombat(state, c);
    World initialized(0, movement_.world().limits());
    initialized.restore(std::move(state));
    movement_ = MovementSession(std::move(initialized), navigation_);
}
void CombatSession::publish(MovementSession candidate, CombatState c) {
    auto state = candidate.world().state();
    auto sections = decodeBattleSystems(world().state().systems);
    sections.combat = encodeCombat(c);
    state.systems = encodeBattleSystems(sections);
    validateCombat(state, c);
    World world(0, candidate.world().limits());
    world.restore(std::move(state));
    MovementSession complete(std::move(world), navigation_);
    movement_ = std::move(complete);
}
Handle CombatSession::spawn(Entity e, CombatStats stats, bool fineMotion, CombatControl control) {
    statsValid(stats);
    require(e.family == Family::creature && !e.cleaned, "combat spawn requires a living creature");
    require(control == CombatControl::automatic || control == CombatControl::hold,
            "unsupported initial combat control");
    auto candidate = movement_;
    auto c = combat();
    const auto h = candidate.spawn(std::move(e), fineMotion, fineMotion, fineMotion);
    c.fighters.push_back({h, stats, stats.maximumHealth, CombatAction::idle, 0, {}, control, {}});
    std::sort(c.fighters.begin(), c.fighters.end(),
              [](const Fighter &a, const Fighter &b) { return a.actor.slot < b.actor.slot; });
    publish(std::move(candidate), std::move(c));
    return h;
}
void CombatSession::bindEffects(std::uint64_t identity, Bytes data) {
    require(identity && !effects().identity, "battle effects already bound or missing identity");
    auto sections = decodeBattleSystems(world().state().systems);
    sections.effects = {identity, std::move(data), {}};
    auto state = world().state();state.systems = encodeBattleSystems(sections);
    World candidate(0, world().limits());candidate.restore(std::move(state));
    movement_ = MovementSession(std::move(candidate), navigation_);
}
void CombatSession::updateEffects(Bytes data) {
    auto sections = decodeBattleSystems(world().state().systems);
    require(sections.effects.identity != 0, "battle effects are unbound");
    sections.effects.data = std::move(data);
    auto state=world().state();state.systems=encodeBattleSystems(sections);
    World candidate(0,world().limits());candidate.restore(std::move(state));
    movement_=MovementSession(std::move(candidate),navigation_);
}
Handle CombatSession::spawnEffect(Entity e) {
    auto sections=decodeBattleSystems(world().state().systems);
    require(sections.effects.identity && e.family==Family::missileEffect && !e.motion && !e.cleaned && sections.effects.entities.size()<32,
            "invalid battle effect spawn");
    World candidate(0,world().limits());candidate.restore(world().state());
    auto h=candidate.spawn(std::move(e));sections.effects.entities.push_back(h);
    auto state=candidate.state();state.systems=encodeBattleSystems(sections);
    validateCombat(state,combat());candidate.restore(std::move(state));
    movement_=MovementSession(std::move(candidate),navigation_);return h;
}
void CombatSession::relocateEffect(Handle h, Point p) {
    auto e=effects();require(std::find(e.entities.begin(),e.entities.end(),h)!=e.entities.end(),"unregistered effect relocation");
    auto state=world().state();auto &body=*state.slots.at(h.slot).entity;
    require(state.navigation && contains(p,*state.navigation),"effect outside map");
    body.x=p.x;body.y=p.y;body.z=p.z;
    World candidate(0,world().limits());candidate.restore(std::move(state));movement_=MovementSession(std::move(candidate),navigation_);
}
void CombatSession::releaseEffect(Handle h) {
    auto sections=decodeBattleSystems(world().state().systems);auto &entities=sections.effects.entities;
    auto it=std::find(entities.begin(),entities.end(),h);require(it!=entities.end(),"unregistered effect release");
    entities.erase(it);World candidate(0,world().limits());candidate.restore(world().state());
    candidate.release(h);auto state=candidate.state();state.systems=encodeBattleSystems(sections);
    validateCombat(state,combat());candidate.restore(std::move(state));movement_=MovementSession(std::move(candidate),navigation_);
}
CombatReport CombatSession::step(TickInput input) {
    require(!effects().identity, "Use the composed battle session to advance effects");
    return stepBattle(input);
}
CombatReport CombatSession::stepBattle(TickInput input) {
    CombatReport report;
    if (!input.admitted)
        return report;
    auto c = combat();
    auto candidate = movement_;
    // Movement-only advancement refuses opaque systems. Combat owns this blob
    // and republishes it only after all movement/combat checks succeed.
    auto plain = candidate.world().state();
    plain.systems.clear();
    World world(0, candidate.world().limits());
    world.restore(std::move(plain));
    candidate = MovementSession(std::move(world), navigation_);
    if (input.alternateMode) {
        report.movement = candidate.step(input);
        publish(std::move(candidate), std::move(c));
        return report;
    }
    const auto emit = [&](CombatEventKind k, Handle a, Handle b, std::uint32_t amount = 0) {
        report.events.push_back({k, a, b, amount});
    };
    for (const auto &o : c.pending) {
        auto *f = fighter(c, o.subject);
        auto *target = o.target ? fighter(c, *o.target) : nullptr;
        if (!f || !f->health ||
            (o.kind == CombatOrderKind::attack &&
             (!target || !eligible(candidate.world().state(), *f, *target, c.acquisitionRange)))) {
            ++report.rejectedOrders;
            continue;
        }
        clearAttack(*f);
        f->destination.reset();
        if (o.kind == CombatOrderKind::move) {
            f->control = CombatControl::move;
            f->destination = o.destination;
            candidate.move(f->actor, *o.destination);
        } else if (o.kind == CombatOrderKind::stop) {
            f->control = CombatControl::hold;
            candidate.stop(f->actor);
        } else {
            f->control = CombatControl::attack;
            f->target = o.target;
            f->action = CombatAction::approaching;
        }
    }
    c.pending.clear();
    for (auto &f : c.fighters) {
        if (!f.health)
            continue;
        if (f.control == CombatControl::hold || f.control == CombatControl::move)
            continue;
        const auto &own = *candidate.world().find(f.actor);
        auto *target = f.target ? fighter(c, *f.target) : nullptr;
        if (!target || !eligible(candidate.world().state(), f, *target, c.acquisitionRange)) {
            const bool ordered = f.control == CombatControl::attack;
            clearAttack(f);
            if (ordered) {
                if (active(own.motion->action))
                    candidate.stop(f.actor);
                continue;
            }
            Fighter *best = nullptr;
            for (auto &b : c.fighters)
                if (eligible(candidate.world().state(), f, b, c.acquisitionRange) &&
                    (!best || distance(own, *candidate.world().find(b.actor)) <
                                  distance(own, *candidate.world().find(best->actor))))
                    best = &b;
            if (best) {
                f.target = best->actor;
                f.action = CombatAction::approaching;
                emit(CombatEventKind::acquired, f.actor, best->actor);
                target = best;
            }
        }
        if (!target) {
            if (active(own.motion->action))
                candidate.stop(f.actor);
            continue;
        }
        const auto &other = *candidate.world().find(target->actor);
        if (adjacent(own, other)) {
            if (active(own.motion->action))
                candidate.stop(f.actor);
        } else {
            if (f.action == CombatAction::windingUp)
                emit(CombatEventKind::missed, f.actor, target->actor);
            f.action = CombatAction::approaching;
            f.remaining = 0;
            const auto retained = own.motion->destination;
            const auto endpointDistance = std::max(std::llabs(std::int64_t(retained.x) - other.x),
                                                   std::llabs(std::int64_t(retained.y) - other.y));
            // Keep an admitted approach while its endpoint still borders the
            // target. Rechoosing against transient reservations every tick can
            // repeatedly reset fine movement before its first edge completes.
            if (active(own.motion->action) && retained.z == other.z && endpointDistance == 1)
                continue;
            const auto dest = approach(candidate.world().state(), *navigation_, f.actor, other);
            if (dest && (own.motion->destination != *dest || own.motion->action == Action::idle ||
                         own.motion->action == Action::cancelled)) {
                candidate.move(f.actor, *dest);
                emit(CombatEventKind::approach, f.actor, target->actor);
            } else if (!dest && active(own.motion->action))
                candidate.stop(f.actor);
        }
    }
    report.movement = candidate.step(input);
    auto after = candidate.world().state();
    for (auto &f : c.fighters) {
        if (f.control == CombatControl::move) {
            const auto a = after.slots[f.actor.slot].entity->motion->action;
            if (!active(a)) {
                f.control = a == Action::arrived ? CombatControl::automatic : CombatControl::hold;
                f.destination.reset();
            }
        }
        if (!f.health || !f.target)
            continue;
        auto *target = fighter(c, *f.target);
        auto &own = *after.slots[f.actor.slot].entity;
        if (!target || !eligible(after, f, *target, c.acquisitionRange)) {
            clearAttack(f);
            stopMotion(own);
            continue;
        }
        auto &other = *after.slots[target->actor.slot].entity;
        if (!adjacent(own, other)) {
            if (f.action == CombatAction::windingUp)
                emit(CombatEventKind::missed, f.actor, target->actor);
            f.action = CombatAction::approaching;
            f.remaining = 0;
            continue;
        }
        // Grid-adjacent combat stops any remaining route at the committed cell.
        stopMotion(own);
        if (f.action == CombatAction::windingUp) {
            if (--f.remaining)
                continue;
            applyDamage(after, c, *target, f.stats.damage, f.actor, report.events, *healthRules_);
            if (target->health) {
                f.action = CombatAction::recovering;
                f.remaining = f.stats.recoveryTicks;
            }
        } else if (f.action == CombatAction::recovering) {
            if (--f.remaining)
                continue;
            f.action = CombatAction::windingUp;
            f.remaining = f.stats.windupTicks;
            emit(CombatEventKind::windup, f.actor, target->actor);
        } else {
            f.action = CombatAction::windingUp;
            f.remaining = f.stats.windupTicks;
            emit(CombatEventKind::windup, f.actor, target->actor);
        }
    }
    // Targets that left acquisition range during movement cannot survive in a
    // published checkpoint, even if the actor was processed before its target.
    for (auto &f : c.fighters)
        if (f.target) {
            auto *target = fighter(c, *f.target);
            if (!target || !eligible(after, f, *target, c.acquisitionRange)) {
                clearAttack(f);
                stopMotion(*after.slots[f.actor.slot].entity);
            }
        }
    World complete(0, candidate.world().limits());
    complete.restore(std::move(after));
    publish(MovementSession(std::move(complete), navigation_), std::move(c));
    return report;
}
void CombatSession::enqueue(CombatOrder order) {
    orderValid(order);
    auto c = combat();
    require(c.pending.size() < 64, "combat order capacity exceeded");
    if (order.destination)
        require(contains(*order.destination, navigation_->binding()), "ordered move outside map");
    c.pending.push_back(order);
    publish(movement_, std::move(c));
}
void CombatSession::remove(Handle h) {
    auto c=combat();auto it=std::find_if(c.fighters.begin(),c.fighters.end(),[&](const auto &f){return f.actor==h;});
    require(it!=c.fighters.end(),"stale combat removal");c.fighters.erase(it);
    c.pending.erase(std::remove_if(c.pending.begin(),c.pending.end(),[&](const auto &o){return o.subject==h || o.target==std::optional<Handle>(h);}),c.pending.end());
    World candidate(0,world().limits());candidate.restore(world().state());candidate.release(h);auto state=candidate.state();
    for(auto &f:c.fighters) {
        auto &e=*state.slots.at(f.actor.slot).entity;
        if(f.target==std::optional<Handle>(h)) {clearAttack(f);stopMotion(e);}
        if(f.control==CombatControl::move && !active(e.motion->action)) {f.control=CombatControl::hold;f.destination.reset();}
    }
    candidate.restore(std::move(state));publish(MovementSession(std::move(candidate),navigation_),std::move(c));
}
CombatReport CombatSession::damage(Handle target, std::uint32_t amount, Handle source) {
    require(amount >= 1 && amount <= 1000000, "invalid external damage");
    auto c = combat();
    auto state = world().state();
    auto *victim = fighter(c, target);
    require(victim && fighter(c, source), "external damage identity unavailable");
    CombatReport r;
    applyDamage(state, c, *victim, amount, source, r.events, *healthRules_);
    World w(0, world().limits());
    w.restore(std::move(state));
    publish(MovementSession(std::move(w), navigation_), std::move(c));
    return r;
}
CombatReport CombatSession::heal(Handle target, std::uint32_t amount, Handle source) {
    require(amount >= 1 && amount <= 1000000, "invalid healing amount");
    auto c = combat();
    auto state = world().state();
    auto *victim = fighter(c, target);
    require(victim && victim->health && fighter(c, source),
            "healing identity/living target unavailable");
    const auto before = victim->health;
    const auto after = healthRules_->change(before, victim->stats.maximumHealth,
                                            static_cast<std::int32_t>(amount), false);
    require(after >= before && after <= victim->stats.maximumHealth,
            "healing backend returned invalid health");
    CombatReport report;
    victim->health = after;
    if (after != before)
        report.events.push_back({CombatEventKind::healed, source, target, after - before});
    World w(0, world().limits());
    w.restore(std::move(state));
    publish(MovementSession(std::move(w), navigation_), std::move(c));
    return report;
}
void CombatSession::holdAll() {
    auto c = combat();
    auto state = world().state();
    c.pending.clear();
    state.pending.clear();
    for (auto &f : c.fighters) {
        clearAttack(f);
        f.control = CombatControl::hold;
        f.destination.reset();
        stopMotion(*state.slots[f.actor.slot].entity);
    }
    World w(0, world().limits());
    w.restore(std::move(state));
    publish(MovementSession(std::move(w), navigation_), std::move(c));
}
void CombatSession::restore(const std::filesystem::path &path, const NavigationResolver &resolver) {
    require(bool(resolver), "combat restore requires a resource resolver");
    auto state = readSnapshot(path, world().limits());
    require(!state.systems.empty(), "checkpoint contains no combat subsystem");
    auto nav = resolver(state);
    require(bool(nav), "combat restore resource unavailable");
    World world(0, this->world().limits());
    world.restore(std::move(state));
    CombatSession candidate(std::move(world), std::move(nav), healthRules_);
    *this = std::move(candidate);
}
} // namespace mnm::game
